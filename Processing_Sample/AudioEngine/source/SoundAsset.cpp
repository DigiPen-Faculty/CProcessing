//---------------------------------------------------------------------------------------------------------------------
// file:    SoundAsset.cpp
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#include "SoundAsset.h"
#include "SoundInstance.h"
#include "Mixer.h"
#include <cmath>
#include <cstring>
#include "Atomics.h"
#include "Functor.hpp"

namespace AudioEngine
{
    //------------------------------------------------------------------------------------------------------ SoundAsset

    //*****************************************************************************************************************
    SoundAsset::SoundAsset() :
        mDecoder(this)
    {
        
    }

    //*****************************************************************************************************************
    SoundAsset::~SoundAsset()
    {
        
    }

    //*****************************************************************************************************************
    AE_RESULT SoundAsset::Load(const char* fileName, bool streaming)
    {
        AE_RESULT result = mDecoder.OpenFile(fileName);
        if (result != AE_OK)
            return result;

        mChannels = mDecoder.mFileData.mNumberOfChannels;
        if (mDecoder.mFileData.mSampleRate != gMixer->GetSampleRate())
            mFrames = (unsigned)(mDecoder.mFileData.mNumberOfFrames * (double)mDecoder.mFileData.mSampleRate
                / (double)gMixer->GetSampleRate());
        else
            mFrames = mDecoder.mFileData.mNumberOfFrames;

        mSamples.resize(mFrames * mChannels, 0);

        mDecoder.StartDecoding();

        return AE_OK;
    }

    //*****************************************************************************************************************
    AE_RESULT SoundAsset::Play(InstanceIDType* newID, bool loop, bool startPaused, unsigned group)
    {
        if (mStreaming_Thr && HasInstances())
            return AE_STREAMING_SOUND_ALREADY_PLAYING;

        if (!IsValid())
            return AE_SOUND_NOT_LOADED;

        if (newID)
            *newID = (new SoundInstance(this, loop, startPaused, group))->mID;

        return AE_OK;
    }

    //*****************************************************************************************************************
    bool SoundAsset::HasInstances() const
    {
        return mCurrentInstances_Shared != 0;
    }

    //*****************************************************************************************************************
    bool SoundAsset::IsValid() const
    {
        return mSamples.size() > 0;
    }

    //*****************************************************************************************************************
    void SoundAsset::AppendSamplesThreaded(BufferType* buffer, unsigned howManySamples, unsigned frameIndex)
    {
        // Translate from frames to sample location
        unsigned sampleIndex = frameIndex * mChannels;

        // Keep the original size of the buffer
        unsigned originalBufferSize = (unsigned)buffer->size();
        // Resize the buffer to hold the new samples
        buffer->resize(originalBufferSize + howManySamples);
        float* bufferStart = buffer->data() + originalBufferSize;

        unsigned samplesAvailable = (unsigned)mSamplesAvailable_Shared;

        // Check if we have enough samples available
        if (sampleIndex + howManySamples < samplesAvailable)
        {
            memcpy(bufferStart, mSamples.data() + sampleIndex, sizeof(float) * howManySamples);
        }
        // If not, copy what we can and set the rest to zero
        else
        {
            int samplesToCopy = samplesAvailable - sampleIndex;

            // Check if there are any samples to copy 
            if (samplesToCopy > 0)
            {
                // Copy what we have into the buffer
                memcpy(bufferStart, mSamples.data() + sampleIndex, sizeof(float) * samplesToCopy);
                // Set the remaining samples to zero
                memset(bufferStart + samplesToCopy, 0, sizeof(float) * (howManySamples - samplesToCopy));
            }
            // If not, set all samples to zero
            else
                memset(bufferStart, 0, sizeof(float) * howManySamples);
        }
    }

    //*****************************************************************************************************************
    void SoundAsset::DecodingCallback(DecodedPacket* packet)
    {
        unsigned samplesCopied = (int)packet->mSamples.size();
        if (samplesCopied > mSamples.size() - mSamplesAvailable_Shared)
            samplesCopied = (int)mSamples.size() - (int)mSamplesAvailable_Shared;

        memcpy(mSamples.data() + mSamplesAvailable_Shared, packet->mSamples.data(), samplesCopied * sizeof(float));

        AtomicExchange(&mSamplesAvailable_Shared, mSamplesAvailable_Shared + samplesCopied);
    }

}
