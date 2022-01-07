//---------------------------------------------------------------------------------------------------------------------
// file:    Resampler.h
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#pragma once

#include "Definitions.h"

namespace AudioEngine
{
    //------------------------------------------------------------------------------------------------------- Resampler
    class Resampler
    {
    public:
        Resampler();

        void SetFactor(double factor);
        unsigned GetNextOutputFrameCount(unsigned inputFrames);
        void SetInputBuffer(const float* inputSamples, unsigned frameCount, unsigned channels);
        bool GetNextFrame(float* output);

    private:
        float mPreviousFrame[AudioConstants::cMaxChannels] = { 0 };
        double mResampleFactor = 0;
        double mResampleFrameIndex = 0;
        double mBufferFraction = 0;
        const float* mInputSamples = nullptr;
        unsigned mInputFrames = 0;
        unsigned mInputChannels = 0;
    };
}
