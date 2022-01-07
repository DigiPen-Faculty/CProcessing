//---------------------------------------------------------------------------------------------------------------------
// file:    SoundInstance.cpp
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#include "SoundInstance.h"
#include "SoundAsset.h"
#include "Mixer.h"
#include <cmath>
#include <cstring>
#include "Atomics.h"
#include "Functor.hpp"
#include "ChannelTranslation.h"

namespace AudioEngine
{
    //------------------------------------------------------------------------------------------- ThreadedSoundInstance

    //*****************************************************************************************************************
    ThreadedSoundInstance::ThreadedSoundInstance(SoundAsset* asset, bool loop, bool startPaused, InstanceIDType ID, unsigned group) :
        mAsset(asset),
        mChannels(asset->mChannels),
        mTotalFrames(asset->mFrames),
        mPaused(startPaused),
        mID(ID),
        mLooping(loop),
        mGroup(group)
    {
        // Note: constructor happens on the main thread

        // Set the volume interpolator starting values
        VolumeInterpolator.SetValues(1.0f, 1.0f, (unsigned)0);

        // Add this instance to the mixer's threaded list
        gMixer->AddTaskForMixThread(CreateFunctor(&Mixer::AddInstanceThreaded, gMixer, this, group));

        // Add 1 to asset instance tracking
        AtomicExchange(&mAsset->mCurrentInstances_Shared, mAsset->mCurrentInstances_Shared + 1);
    }

    //*****************************************************************************************************************
    ThreadedSoundInstance::~ThreadedSoundInstance()
    {
        // This destructor happens on the main thread! Should already be removed from the mixer's list.
        // If shutting down, doesn't matter if it's on the list.

    }

    //*****************************************************************************************************************
    bool ThreadedSoundInstance::GetNextSamples(float* outputBuffer, unsigned outputFrames, 
        unsigned outputChannels)
    {
        // If the asset has no data or the instance is not active, set the buffer to zero and return
        if (!mAsset->mSamples.size() || !mActive)
        {
            memset(outputBuffer, 0, sizeof(float) * outputFrames * mChannels);
            return false;
        }

        // Check if the instance should stop playing
        bool stopping = AtomicCompareExchange(&mStopFlag_Shared, 0, 1) != 0;

        // Check if we are currently paused
        if (mPaused)
        {
            // If stopping, set the buffer to zero, mark that we're not active, and return
            if (stopping)
            {
                memset(outputBuffer, 0, sizeof(float) * outputFrames * mChannels);
                FinishedCleanup();
                return false;
            }

            // If we should resume playing, mark that we're not paused and interpolate up the volume
            if (AtomicCompareExchange(&mResumeFlag_Shared, 0, 1) != 0)
            {
                mPaused = false;
                VolumeInterpolator.SetValues(0.0f, mSavedVolume, AudioConstants::cPropertyChangeFrames);
            }
            // Otherwise set the buffer to zero and return
            else
            {
                memset(outputBuffer, 0, sizeof(float) * outputFrames * mChannels);
                return true;
            }
        }

        // If we should stop, interpolate the volume down to zero
        if (stopping)
            VolumeInterpolator.SetValues(0.0f, AudioConstants::cPropertyChangeFrames);

        // Check if we should pause
        if (AtomicCompareExchange(&mPauseFlag_Shared, 0, 1) != 0)
        {
            // Mark that we are now paused
            mPaused = true;
            // Save the current volume (use end value in case of interpolation in process)
            mSavedVolume = VolumeInterpolator.GetEndValue();
            // Interpolate volume down to zero
            VolumeInterpolator.SetValues(0.0f, AudioConstants::cPropertyChangeFrames);
        }

        unsigned inputFrames = outputFrames;
        BufferType samples;

        // If pitch shifting, determine number of asset frames we need
        if (mPitchShifting)
        {
            PitchObject.CalculateBufferSize(outputFrames * outputChannels, outputChannels);
            inputFrames = PitchObject.GetInputFrameCount();
        }

        // Store the starting frame
        unsigned startingFrameIndex = mFrameIndex;
        // Move the frame index forward
        mFrameIndex += inputFrames;

        // Check if we are looping and will reach the end 
        if (mLooping && mFrameIndex >= mTotalFrames)
        {
            unsigned sectionFrames = 0;

            if (startingFrameIndex < mTotalFrames)
            {
                // Save the number of frames in the first section
                sectionFrames = inputFrames - (mFrameIndex - mTotalFrames);
                // Get the samples from the asset
                mAsset->AppendSamplesThreaded(&samples, sectionFrames * mChannels, startingFrameIndex);
            }

            // Reset back to the loop start frame
            Loop();

            // Save the number of frames in the second section
            sectionFrames = inputFrames - sectionFrames;
            // Get the samples from the asset
            mAsset->AppendSamplesThreaded(&samples, sectionFrames * mChannels, mFrameIndex);

            // Move frame index forward
            mFrameIndex += sectionFrames;
        }
        // Check if we will reach the end of the audio
        else if (mFrameIndex >= mTotalFrames)
        {
            // Get the available samples
            if (startingFrameIndex < mTotalFrames)
                mAsset->AppendSamplesThreaded(&samples, (inputFrames - (mFrameIndex - mTotalFrames)) 
                    * mChannels, startingFrameIndex);

            // Resize the array to full size, setting the rest of the samples to 0
            samples.resize(inputFrames * mChannels, 0.0f);

            stopping = true;
        }
        // Otherwise, no need to adjust anything
        else
        {
            mAsset->AppendSamplesThreaded(&samples, inputFrames * mChannels, startingFrameIndex);
        }

        // Check if the output channels are different than the audio data
        if (mChannels != outputChannels)
            TranslateChannels(&samples, inputFrames, mChannels, outputChannels);

        // If pitch shifting, interpolate the samples into the buffer
        if (mPitchShifting)
        {
            // Pitch shifts the samples into a temporary buffer
            BufferType pitchShiftedSamples(outputFrames * outputChannels);
            PitchObject.ProcessBuffer(&samples, &pitchShiftedSamples);

            // Move the samples back into the original buffer
            samples.swap(pitchShiftedSamples);

            // Check if we should stop pitch shifting
            if (!PitchObject.Interpolating() && AudioConstants::IsWithinLimit(PitchObject.GetPitchFactor(), 1.0f, 0.01f))
                mPitchShifting = false;
        }

        // Check if we need to apply a volume change
        if (!VolumeInterpolator.Finished() || !AudioConstants::IsWithinLimit(mVolume, 1.0f, 0.01f))
        {
            float volume = mVolume;
            for (int i = 0; i < samples.size(); i += outputChannels)
            {
                // If interpolating volume, get new value
                if (!VolumeInterpolator.Finished())
                {
                    volume = VolumeInterpolator.NextValue();

                    if (VolumeInterpolator.Finished())
                    {
                        mVolume = volume;
                    }
                }

                // Adjust volume for all samples on this frame
                for (unsigned j = 0; j < outputChannels; ++j)
                    samples[i + j] *= volume;
            }
        }

        // Copy the samples to the buffer
        memcpy(outputBuffer, samples.data(), sizeof(float) * samples.size());


        //for (InList<Filter>::iterator it = FilterList.begin(); it != FilterList.end(); ++it)
        //    it->ProcessBuffer(outputBuffer, outputFrames, mChannels_Thr);

        if (stopping)
        {
            FinishedCleanup();
            return false;
        }
        else
            return true;

        // TODO positional stuff
        //mPositionalObject->ProcessBuffer(outputBuffer, frameCount, channelCount, mPosition, 
            //gMixer->mListenerPositionThreaded);
    }

    //*****************************************************************************************************************
    void ThreadedSoundInstance::Loop()
    {
        // Reset variables
        mFrameIndex = 0;

        // If streaming, reset to the beginning of the file
        //if (mAssetObject->mStreaming)
        //    mAssetObject->ResetStreamingFile(cNodeID);
    }

    //*****************************************************************************************************************
    void ThreadedSoundInstance::TranslateChannels(BufferType* inputSamples, unsigned inputFrames, 
        unsigned inputChannels, unsigned outputChannels)
    {
        // Temporary array for channel translation
        BufferType adjustedSamples(inputFrames * outputChannels);

        // Step through each frame of audio data
        for (unsigned frame = 0, inputIndex = 0, outputIndex = 0; frame < inputFrames;
            ++frame, inputIndex += inputChannels, outputIndex += outputChannels)
        {
            // Translate the channels
            ChannelTranslation::Translate(inputSamples->data() + inputIndex, adjustedSamples.data() + outputIndex, 
                inputChannels, outputChannels);
        }

        // Move the translated data into the other array
        inputSamples->swap(adjustedSamples);
    }

    //*****************************************************************************************************************
    //void ThreadedSoundInstance::AddFilter(Filter* filter)
    //{
    //    FilterList.PushBack(filter);
    //}

    //*****************************************************************************************************************
    void ThreadedSoundInstance::FinishedCleanup()
    {
        if (!mActive)
            return;

        mActive = false;

        // Remove 1 from asset instance tracking
        AtomicExchange(&mAsset->mCurrentInstances_Shared, mAsset->mCurrentInstances_Shared - 1);
    }

    //*****************************************************************************************************************
    void ThreadedSoundInstance::SetPitch(float pitchFactor)
    {
        // If the pitch factor didn't change, don't do anything
        if (AudioConstants::IsWithinLimit(pitchFactor - PitchObject.GetPitchFactor(), 0.0f, 0.01f))
            return;

        PitchObject.SetPitchFactor(pitchFactor, 0.01f);
        mPitchShifting = true;
    }

    //--------------------------------------------------------------------------------------------------- SoundInstance

    //*****************************************************************************************************************
    SoundInstance::SoundInstance(SoundAsset* asset, bool loop, bool startPaused, unsigned group) :
        ThreadedInstance(asset, loop, startPaused, gMixer->mNextInstanceID, group),
        mID(gMixer->mNextInstanceID)
    {
        ++gMixer->mNextInstanceID;

        // Add this instance to the mixer's ID-to-instance map
        gMixer->InstanceMap[mID] = this;
    }

    //*****************************************************************************************************************
    SoundInstance::~SoundInstance()
    {
        // Remove from the mixer's ID-to-instance map
        gMixer->InstanceMap.erase(mID);
    }

    //*****************************************************************************************************************
    void SoundInstance::Stop()
    {
        // Set the stop flag to be read by the threaded object
        AtomicExchange(&ThreadedInstance.mStopFlag_Shared, 1);
    }

    //*****************************************************************************************************************
    void SoundInstance::SetVolume(float newVolume)
    {
        if (newVolume < 0.0f)
            newVolume = 0.0f;

        mVolume = newVolume;

        gMixer->AddTaskForMixThread(CreateFunctor(&Interpolator::SetValues,
            &ThreadedInstance.VolumeInterpolator, newVolume, AudioConstants::cPropertyChangeFrames));

    }

    //*****************************************************************************************************************
    void SoundInstance::SetPitch(float pitchFactor)
    {
        mPitch = pitchFactor;

        gMixer->AddTaskForMixThread(CreateFunctor(&ThreadedSoundInstance::SetPitch, &ThreadedInstance, pitchFactor));
    }

    //*****************************************************************************************************************
    void SoundInstance::SetPaused(bool paused)
    {
        if (paused)
            AtomicExchange(&ThreadedInstance.mPauseFlag_Shared, 1);
        else
            AtomicExchange(&ThreadedInstance.mResumeFlag_Shared, 1);
    }

}
