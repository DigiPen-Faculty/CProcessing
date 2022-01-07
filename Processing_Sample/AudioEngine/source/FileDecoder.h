//---------------------------------------------------------------------------------------------------------------------
// file:    FileDecoder.h
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#pragma once

#include <fstream>
#include "AudioEngine.h"
#include "Resampler.h"

struct stb_vorbis;

namespace AudioEngine
{
    struct WavRiffHeader
    {
        char riff_chunk[4];
        unsigned chunk_size;
        char wave_fmt[4];
    };

    struct WavChunkHeader
    {
        char chunk_name[4];
        unsigned chunk_size;
    };

    struct WavFmtData
    {
        unsigned short audio_format;
        unsigned short number_of_channels;
        unsigned sampling_rate;
        unsigned bytes_per_second;
        unsigned short bytes_per_sample;
        unsigned short bits_per_sample;
    };

    const static float Normalize16Bit = 1 << 15;
    const static float Normalize24Bit = 1 << 23;

    namespace DecoderEventTypes
    {
        enum Enum { DecodeSemaphore, StopDecodeThread, DecodeThreadExit, Count };
    }

    //--------------------------------------------------------------------------------------------------- AudioFileData
    struct AudioFileData
    {
        unsigned mNumberOfFrames = 0;
        short mNumberOfChannels = 0;
        unsigned mSampleRate = 0;
        unsigned mBytesPerSample = 0;
        unsigned mNumberOfSamples = 0;
    };

    //--------------------------------------------------------------------------------------------------- DecodedPacket

    class DecodedPacket
    {
    public:
        DecodedPacket(unsigned size) :
            mSamples(size) {}

        BufferType mSamples;
    };

    //------------------------------------------------------------------------------------------------ AudioFileDecoder

    class SoundAsset;

    class AudioFileDecoder
    {
    public:
        AudioFileDecoder(SoundAsset* parent);
        virtual ~AudioFileDecoder();

        AE_RESULT OpenFile(const char* fileName);
        void StartDecoding();

        AudioFileData mFileData;

    private:
        enum class FileTypes { NotSet, WAV, Ogg };
        SoundAsset* mParent;
        std::fstream mFile;
        stb_vorbis* mOggDecoder = nullptr;
        unsigned mDataChunkSize = 0;
        unsigned mFramesPerChunk = 0;
        unsigned mMixerSampleRate = 0;
        void* ThreadEvents[DecoderEventTypes::Count];
        bool mDecoding = false;
        bool mResampling = false;
        Resampler ResamplerObject;
        FileTypes mFileType = FileTypes::NotSet;

        void DecodingLoopThreaded();
        bool DecodeSectionThreaded(char* rawData, DecodedPacket& packet, BufferType& resampledData);
        static unsigned _stdcall StartThread(void* param);
        void StopDecodingThread();
        FileTypes CheckFileTypes(const char* data);
        stb_vorbis* ReadOggHeader(const char* fileName);
        int TranslateOggData(stb_vorbis* decoder, float* result, unsigned samplesToRead);
        int ReadWavHeader();
        bool TranslateWavData(char* rawDataBuffer, float* result, unsigned samplesToRead);
    };
}
