//---------------------------------------------------------------------------------------------------------------------
// file:    Effects.h
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#pragma once

#include "InList.hpp"

namespace AudioEngine
{
    //---------------------------------------------------------------------------------------------------------- Filter
    class Filter
    {
    public:

        virtual void ProcessBuffer(float* buffer, unsigned frames, unsigned channels) = 0;

        InListLink<Filter> Link;
    };

    //---------------------------------------------------------------------------------------------------------- BiQuad
    class BiQuad
    {
    public:
        BiQuad();

        float DoBiQuad(float x);

        float x_1;
        float x_2;
        float y_1;
        float y_2;
        float a0;
        float a1;
        float a2;
        float b1;
        float b2;
    };

    //--------------------------------------------------------------------------------------------------- LowPassFilter
    class LowPassFilter : public Filter
    {
    public:
        LowPassFilter();

        void ProcessBuffer(float* buffer, unsigned frames, unsigned channels) override;
        void SetSampleRate(int sampleRate);
        void SetCutoffFrequency(float freq);

    private:
        void SetValues();

        float mCutoffFreq;
        int mSampleRate;
        BiQuad BiQuadPerChannel[2];
    };
}