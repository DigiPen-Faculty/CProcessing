//---------------------------------------------------------------------------------------------------------------------
// file:    Definitions.cpp
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#include "Definitions.h"
#include <cmath>

namespace AudioEngine
{
    namespace AudioConstants
    {
        //*************************************************************************************************************
        bool IsWithinLimit(float valueToCheck, float centralValue, float limit)
        {
            if (valueToCheck > centralValue + limit || valueToCheck < centralValue - limit)
                return false;
            else
                return true;
        }

        //*************************************************************************************************************
        void AppendToBuffer(BufferType* destinationBuffer, const BufferType& sourceBuffer,
            unsigned sourceStartIndex, unsigned numberOfSamples)
        {
            size_t destIndex = destinationBuffer->size();
            destinationBuffer->resize(destinationBuffer->size() + numberOfSamples);
            memcpy(destinationBuffer->data() + destIndex, sourceBuffer.data() + sourceStartIndex,
                sizeof(float) * numberOfSamples);
        }

        //*************************************************************************************************************
        float PitchToSemitones(float pitch)
        {
            if (pitch == 0)
                return 0.0f;
            else
                return 3986.0f * std::log10f(std::powf(2, pitch)) / 100.0f;
        }

        //*************************************************************************************************************
        float SemitonesToPitch(float semitone)
        {
            return std::log2(std::powf(2, semitone / 12.0f));
        }

        //*************************************************************************************************************
        float VolumeToDecibels(float volume)
        {
            if (volume == 0.0f)
                return -100.0f;
            else
            {
                float decibels = 20.0f * std::log10(volume);
                if (decibels < -100.0f)
                    decibels = -100.0f;
                return decibels;
            }
        }

        //*************************************************************************************************************
        float DecibelsToVolume(float decibels)
        {
            return std::powf(10.0f, decibels / 20.0f);
        }
    }
}
