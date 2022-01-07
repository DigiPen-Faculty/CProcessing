//---------------------------------------------------------------------------------------------------------------------
// file:    Resampler.cpp
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#include "Resampler.h"


namespace AudioEngine
{
    //------------------------------------------------------------------------------------------------------- Resampler

    //*****************************************************************************************************************
    Resampler::Resampler()
    {

    }

    //*****************************************************************************************************************
    void Resampler::SetFactor(double factor)
    {
        mResampleFactor = factor;
    }

    //*****************************************************************************************************************
    unsigned Resampler::GetNextOutputFrameCount(unsigned inputFrames)
    {
        // Get initial frame count
        double frames = inputFrames / mResampleFactor;
        // Add the leftover fraction from the last buffer
        frames += mBufferFraction;
        // Save the current fraction
        mBufferFraction = frames - (int)frames;
        // Return the integer buffer size
        return (unsigned)frames;
    }

    //*****************************************************************************************************************
    void Resampler::SetInputBuffer(const float* inputSamples, unsigned frameCount, unsigned channels)
    {
        mInputSamples = inputSamples;
        mInputFrames = frameCount;
        mInputChannels = channels;
        mResampleFrameIndex = mResampleFrameIndex - (int)mResampleFrameIndex;
    }

    //*****************************************************************************************************************
    bool Resampler::GetNextFrame(float* output)
    {
        // Save the integer frame index
        unsigned frameIndex = (unsigned)mResampleFrameIndex;

        // Check if this would be past the end of the buffer
        if (frameIndex >= mInputFrames)
        {
            memcpy(output, mPreviousFrame, sizeof(float) * mInputChannels);
            return false;
        }

        // Translate to the sample index
        unsigned sampleIndex = frameIndex * mInputChannels;

        // Get the pointer to the first frame. If the index is at 0, use the PreviousFrame values.
        const float* firstFrame(mPreviousFrame);
        if (mResampleFrameIndex > 1.0)
            firstFrame = mInputSamples + (sampleIndex - mInputChannels);

        // Get the pointer to the second frame
        const float* secondFrame(mInputSamples + sampleIndex);

        // Interpolate between the two frames for each channel
        for (unsigned i = 0; i < mInputChannels; ++i)
        {
            output[i] = firstFrame[i] + ((secondFrame[i] - firstFrame[i])
                * (float)(mResampleFrameIndex - frameIndex));
        }

        // Advance the frame index
        mResampleFrameIndex += mResampleFactor;

        // If we are now past the end of the buffer, save the PreviousFrame values
        if ((unsigned)mResampleFrameIndex >= mInputFrames)
        {
            // Get the last frame of samples in the buffer
            memcpy(mPreviousFrame, mInputSamples + ((mInputFrames - 1) * mInputChannels),
                sizeof(float) * mInputChannels);
            return false;
        }
        else
            return true;
    }
}
