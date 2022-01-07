//---------------------------------------------------------------------------------------------------------------------
// file:    FileDecoder.cpp
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#include "Definitions.h"
#include "FileDecoder.h"
#include "ogg/stb_vorbis.h"
#include "SoundAsset.h"
#include "Mixer.h"
#include "Atomics.h"
#include <process.h>

namespace AudioEngine
{

    //------------------------------------------------------------------------------------------------ AudioFileDecoder

    //*****************************************************************************************************************
    AudioFileDecoder::AudioFileDecoder(SoundAsset* parent) :
        mParent(parent),
        mMixerSampleRate(gMixer->GetSampleRate())
    {
        // Create the thread events
        ThreadEvents[DecoderEventTypes::DecodeSemaphore] = CreateSemaphore(nullptr, 1, 10, nullptr);
        for (int i = 1; i < DecoderEventTypes::Count; ++i)
            ThreadEvents[i] = CreateEvent(nullptr, false, false, nullptr);
    }

    //*****************************************************************************************************************
    AudioFileDecoder::~AudioFileDecoder()
    {
        mFile.close();
        if (mOggDecoder)
            stb_vorbis_close(mOggDecoder);

        // Close the thread events
        for (int i = 0; i < DecoderEventTypes::Count; ++i)
            CloseHandle(ThreadEvents[i]);
    }

    //*****************************************************************************************************************
    AE_RESULT AudioFileDecoder::OpenFile(const char* fileName)
    {
        // Open the file and check for success
        mFile.open(fileName, std::ios_base::binary | std::ios_base::in);
        if (!mFile)
            return AE_ERROR_OPENING_FILE;

        // Check the file type
        char data[4] = { 0 };
        mFile.read(data, 3);
        mFileType = CheckFileTypes(data);
        if (mFileType == FileTypes::NotSet)
        {
            mFile.close();
            return AE_ERROR_UNSUPPORTED_FILE_TYPE;
        }

        // Reset the file
        mFile.seekg(0);

        // Get initial file data depending on type
        switch (mFileType)
        {
        case FileTypes::WAV:
            if (ReadWavHeader() == 0)
            {
                mFile.close();
                return AE_ERROR_READING_AUDIO_FILE;
            }
            break;
        case FileTypes::Ogg:
            mFile.close();
            mOggDecoder = ReadOggHeader(fileName);
            if (!mOggDecoder)
                return AE_ERROR_READING_AUDIO_FILE;
            break;
        }

        return AE_OK;
    }

    //*****************************************************************************************************************
    void AudioFileDecoder::StartDecoding()
    {
        if (!mFile.is_open() && !mOggDecoder)
            return;

        // Get one second of frames per decoding chunk
        mFramesPerChunk = mMixerSampleRate;

        // Check if the sample rates don't match
        mResampling = mFileData.mSampleRate != mMixerSampleRate;
        if (mResampling)
        {
            // Set the resampling factor
            ResamplerObject.SetFactor((double)mFileData.mSampleRate / (double)mMixerSampleRate);
            // Adjust the frames per decoding chunk so that the resampled result will match the mixer
            mFramesPerChunk = (unsigned)(mFramesPerChunk * (double)mFileData.mSampleRate / (double)mMixerSampleRate);
        }

        // Save the size in bytes of the decoding chunk
        mDataChunkSize = mFramesPerChunk * mFileData.mNumberOfChannels * mFileData.mBytesPerSample;

        mDecoding = true;

        _beginthreadex(nullptr, 0, &AudioFileDecoder::StartThread, this, 0, nullptr);
    }

    //*****************************************************************************************************************
    void AudioFileDecoder::DecodingLoopThreaded()
    {
        // Create a packet at the appropriate size
        DecodedPacket packet(mFramesPerChunk * mFileData.mNumberOfChannels);
        // Create a buffer for reading in raw data
        char* rawData = nullptr;
        if (mFileType == FileTypes::WAV)
            rawData = new char[mDataChunkSize];
        // Create a buffer for the resampled data
        BufferType resampledData;
        bool decoding = true;

        while (decoding)
        {
            // Wait until an event is signaled
            DWORD result = WaitForMultipleObjects(DecoderEventTypes::Count, ThreadEvents, false, INFINITE);

            // Check if we should stop decoding
            if (result == DecoderEventTypes::StopDecodeThread)
                break;

            // Decode the next section
            decoding = DecodeSectionThreaded(rawData, packet, resampledData);

            if (decoding)
                // Decode more immediately
                ReleaseSemaphore(ThreadEvents[DecoderEventTypes::DecodeSemaphore], 1, nullptr);
        }

        // Clean up
        if (rawData)
            delete[] rawData;
        mFile.close();

        // Signal that this thread is done
        SetEvent(ThreadEvents[DecoderEventTypes::DecodeThreadExit]);
    }

    //*****************************************************************************************************************
    bool AudioFileDecoder::DecodeSectionThreaded(char* rawData, DecodedPacket& packet, BufferType& resampledData)
    {
        if (mFileType == FileTypes::WAV)
        {
            // Read in the appropriate amount of raw data from the file
            mFile.read(rawData, mDataChunkSize);
            // Check how many bytes were actually read
            unsigned bytesRead = (unsigned)mFile.gcount();
            // If no bytes were read, we're done
            if (bytesRead == 0)
                return false;
            // Resize the packet buffer according to how much data was read in
            packet.mSamples.resize(bytesRead / mFileData.mBytesPerSample);
            // Check if we had a problem translating the data
            if (!TranslateWavData(rawData, packet.mSamples.data(), bytesRead / mFileData.mBytesPerSample))
                return false;
        }
        else if (mFileType == FileTypes::Ogg)
        {
            // Make sure the packet buffer is the right size
            packet.mSamples.resize(mFramesPerChunk * mFileData.mNumberOfChannels);
            // Get the translated samples
            unsigned samplesRead = TranslateOggData(mOggDecoder, packet.mSamples.data(), mFramesPerChunk * mFileData.mNumberOfChannels);
            // If no samples were read, we're done
            if (samplesRead == 0)
                return false;
            // If needed, resize the packet buffer
            if (samplesRead < mFramesPerChunk * mFileData.mNumberOfChannels)
                packet.mSamples.resize(samplesRead);
        }

        // Check if we need to resample the data
        if (mResampling)
        {
            // Save the number of source frames
            int inputFrames = (int)packet.mSamples.size() / mFileData.mNumberOfChannels;
            // Set the data on the resampler
            ResamplerObject.SetInputBuffer(packet.mSamples.data(), inputFrames, mFileData.mNumberOfChannels);
            // Save the number of output frames
            int frameCount = ResamplerObject.GetNextOutputFrameCount(inputFrames);
            // Resize the resampled data buffer to match
            resampledData.resize(frameCount * mFileData.mNumberOfChannels);
            // Get all resampled frames
            for (int i = 0; i < frameCount; ++i)
            {
                ResamplerObject.GetNextFrame(resampledData.data() + (i * mFileData.mNumberOfChannels));
            }
            // Swap the resampled data into the packet
            packet.mSamples.swap(resampledData);
        }

        // Hand off the data to the parent
        mParent->DecodingCallback(&packet);

        return true;
    }

    //*****************************************************************************************************************
    unsigned _stdcall AudioFileDecoder::StartThread(void* param)
    {
        ((AudioFileDecoder*)param)->DecodingLoopThreaded();
        return 0;
    }

    //*****************************************************************************************************************
    void AudioFileDecoder::StopDecodingThread()
    {
        if (!mDecoding)
            return;

        SignalObjectAndWait(
            ThreadEvents[DecoderEventTypes::StopDecodeThread],
            ThreadEvents[DecoderEventTypes::DecodeThreadExit],
            INFINITE, false);

        mDecoding = false;
    }

    //*****************************************************************************************************************
    AudioFileDecoder::FileTypes AudioFileDecoder::CheckFileTypes(const char* data)
    {
        if ((data[0] == 'R' || data[0] == 'r')
            && (data[1] == 'I' || data[1] == 'i')
            && (data[2] == 'F' || data[2] == 'f'))
            return AudioFileDecoder::FileTypes::WAV;
        else if ((data[0] == 'O' || data[0] == 'o')
            && (data[1] == 'G' || data[1] == 'g')
            && (data[2] == 'G' || data[2] == 'g'))
            return AudioFileDecoder::FileTypes::Ogg;
        else
            return AudioFileDecoder::FileTypes::NotSet;
    }

    //*****************************************************************************************************************
    stb_vorbis* AudioFileDecoder::ReadOggHeader(const char* fileName)
    {
        // Create the decoder
        int error = 0;
        stb_vorbis* decoder = stb_vorbis_open_filename(fileName, &error, nullptr);
        if (error != VORBIS__no_error)
        {
            stb_vorbis_close(decoder);
            return nullptr;
        }

        // Get the ogg vorbis file info
        stb_vorbis_info info = stb_vorbis_get_info(decoder);

        // Set the variables on the AudioFileData object
        mFileData.mNumberOfChannels = info.channels;
        mFileData.mNumberOfSamples = stb_vorbis_stream_length_in_samples(decoder);
        mFileData.mNumberOfFrames = mFileData.mNumberOfSamples / mFileData.mNumberOfChannels;
        mFileData.mSampleRate = info.sample_rate;
        mFileData.mBytesPerSample = 0;

        return decoder;
    }

    //*****************************************************************************************************************
    int AudioFileDecoder::TranslateOggData(stb_vorbis* decoder, float* result, unsigned samplesToRead)
    {
        int samplesRead = stb_vorbis_get_samples_float_interleaved(decoder, mFileData.mNumberOfChannels, result,
            samplesToRead);

        return samplesRead;
    }

    //*****************************************************************************************************************
    int AudioFileDecoder::ReadWavHeader()
    {
        // RIFF header object
        WavRiffHeader riffHeader;

        // Read in the RIFF header. Return if reading failed.
        mFile.read((char*)&riffHeader, sizeof(WavRiffHeader));
        if (mFile.fail())
            return 0;

        // Make sure the RIFF chunk starts with the characters 'RIFF'.
        if (riffHeader.riff_chunk[0] != 'R' || riffHeader.riff_chunk[1] != 'I')
            return 0;

        // Make sure the wave format is 'WAVE'
        if (riffHeader.wave_fmt[0] != 'W' || riffHeader.wave_fmt[1] != 'A')
            return 0;

        // Chunk header object
        WavChunkHeader chunkHeader;

        // Read in the first chunk header. Return if reading failed.
        mFile.read((char*)&chunkHeader, sizeof(WavChunkHeader));
        if (mFile.fail())
            return 0;

        // Keep reading chunks until the chunk name is 'fmt'.
        while (chunkHeader.chunk_name[0] != 'f' || chunkHeader.chunk_name[1] != 'm'
            || chunkHeader.chunk_name[2] != 't')
        {
            // Skip over the chunk size in the file.
            mFile.seekg(chunkHeader.chunk_size, std::ios_base::cur);
            // Read the next chunk header. Return if reading failed.
            mFile.read((char*)&chunkHeader, sizeof(WavChunkHeader));
            if (mFile.fail())
                return 0;
        }

        // Format data object
        WavFmtData fmtChunkData;

        // Read in the format chunk data. Return if reading failed.
        mFile.read((char*)&fmtChunkData, sizeof(WavFmtData));
        if (mFile.fail())
            return 0;

        // If the chunk size is larger than the format data object, skip ahead in the file.
        if (chunkHeader.chunk_size > sizeof(WavFmtData))
            mFile.seekg(chunkHeader.chunk_size - sizeof(WavFmtData), std::ios_base::cur);

        // Read in the next chunk header. Return if reading failed.
        mFile.read((char*)&chunkHeader, sizeof(WavChunkHeader));
        if (mFile.fail())
            return 0;

        // Keep reading chunks until the chunk name is 'data'.
        while (chunkHeader.chunk_name[0] != 'd' || chunkHeader.chunk_name[1] != 'a'
            || chunkHeader.chunk_name[2] != 't')
        {
            // Skip over the chunk size in the file.
            mFile.seekg(chunkHeader.chunk_size, std::ios_base::cur);
            // Read the next chunk header. Return if reading failed.
            mFile.read((char*)&chunkHeader, sizeof(WavChunkHeader));
            if (mFile.fail())
                return 0;
        }

        // Set the number of audio channels on the AudioFileData object.
        mFileData.mNumberOfChannels = fmtChunkData.number_of_channels;
        // Set the number of audio frames on the data object.
        mFileData.mNumberOfFrames = chunkHeader.chunk_size / fmtChunkData.bytes_per_sample;
        // Set the number of audio samples on the data object.
        mFileData.mNumberOfSamples = mFileData.mNumberOfFrames * mFileData.mNumberOfChannels;
        // Set the sample rate on the data object.
        mFileData.mSampleRate = fmtChunkData.sampling_rate;
        // Set the number of bytes per sample on the data object.
        mFileData.mBytesPerSample = fmtChunkData.bytes_per_sample / mFileData.mNumberOfChannels;

        return chunkHeader.chunk_size;
    }

    //*****************************************************************************************************************
    bool AudioFileDecoder::TranslateWavData(char* rawDataBuffer, float* result, unsigned samplesToRead)
    {
        // 16 bit data can be interpreted as short integers.
        if (mFileData.mBytesPerSample == 2)
        {
            short* input = (short*)rawDataBuffer;

            // For each sample, cast to a float and normalize.
            for (unsigned i = 0; i < samplesToRead; ++i)
                result[i] = (float)input[i] / Normalize16Bit;
        }
        // 24 bit data is interpreted using the individual bytes.
        else if (mFileData.mBytesPerSample == 3)
        {
            // Step through every sample and every three bytes
            for (unsigned sampleIndex = 0, byteIndex = 0; sampleIndex < samplesToRead;
                ++sampleIndex, byteIndex += mFileData.mBytesPerSample)
            {
                // Get first byte
                int sample = rawDataBuffer[byteIndex];
                // Get next two bytes
                memcpy(&sample, rawDataBuffer + byteIndex, sizeof(char) * 3);
                // Account for negative numbers
                if (sample & 0x800000)
                    sample |= 0xff000000;
                else
                    sample &= 0x00ffffff;

                // Cast the sample to a float and normalize.
                result[sampleIndex] = (float)sample / Normalize24Bit;
            }
        }
        // The audio data is not 16 or 24 bit
        else
        {
            return false;
        }

        return true;
    }
}
