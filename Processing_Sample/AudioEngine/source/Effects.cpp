//---------------------------------------------------------------------------------------------------------------------
// file:    Effects.cpp
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#include "Effects.h"
#include "AEMath.h"

namespace AudioEngine
{
    //---------------------------------------------------------------------------------------------------------- BiQuad

    //*****************************************************************************************************************
    BiQuad::BiQuad() :
        a0(0),
        a1(0),
        a2(0),
        b1(0),
        b2(0),
        x_1(0),
        x_2(0),
        y_1(0),
        y_2(0)
    {

    }

    //*****************************************************************************************************************
    float BiQuad::DoBiQuad(float x)
    {
        float y = (a0 * x) + (a1 * x_1) + (a2 * x_2) - (b1 * y_1) - (b2 * y_2);

        y_2 = y_1;
        y_1 = y;
        x_2 = x_1;
        x_1 = x;

        return y;
    }

    //--------------------------------------------------------------------------------------------------- LowPassFilter

    //*****************************************************************************************************************
    LowPassFilter::LowPassFilter() :
        mCutoffFreq(21000.0f),
        mSampleRate(48000)
    {
        SetValues();
    }

    //*****************************************************************************************************************
    void LowPassFilter::ProcessBuffer(float* buffer, unsigned frames, unsigned channels)
    {
        if (mCutoffFreq > 20000.0f)
            return;

        for (unsigned i = 0; i < frames; ++i)
        {
            for (unsigned j = 0; j < channels; ++j)
            {
                unsigned index = (i * channels) + j;
                buffer[index] = BiQuadPerChannel[j].DoBiQuad(buffer[index]);
            }
        }
    }

    //*****************************************************************************************************************
    void LowPassFilter::SetSampleRate(int sampleRate)
    {
        mSampleRate = sampleRate;
        SetValues();
    }

    //*****************************************************************************************************************
    void LowPassFilter::SetCutoffFrequency(float freq)
    {
        mCutoffFreq = freq;
        SetValues();
    }

    //*****************************************************************************************************************
    void LowPassFilter::SetValues()
    {
        float tanValue = Pi * mCutoffFreq / mSampleRate;
        if (tanValue >= HalfPi)
            tanValue = HalfPi - 0.001f;

        float C = 1.0f / std::tan(tanValue);
        float Csq = C * C;

        float alpha = 1.0f / (1.0f + (SqRt2 * C) + Csq);
        float beta1 = 2.0f * alpha * (1.0f - Csq);
        float beta2 = alpha * (1.0f - (SqRt2 * C) + Csq);

        BiQuadPerChannel[0].a0 = alpha;
        BiQuadPerChannel[0].a1 = alpha * 2.0f;
        BiQuadPerChannel[0].a2 = alpha;
        BiQuadPerChannel[0].b1 = beta1;
        BiQuadPerChannel[0].b2 = beta2;

        BiQuadPerChannel[1].a0 = alpha;
        BiQuadPerChannel[1].a1 = alpha * 2.0f;
        BiQuadPerChannel[1].a2 = alpha;
        BiQuadPerChannel[1].b1 = beta1;
        BiQuadPerChannel[1].b2 = beta2;
    }

}