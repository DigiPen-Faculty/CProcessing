//---------------------------------------------------------------------------------------------------------------------
// file:    Definitions.h
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <vector>
using std::vector;

namespace AudioEngine
{
    typedef unsigned InstanceIDType;
    typedef unsigned GroupIDType;
    typedef intptr_t AtomicType;
    typedef std::vector<float> BufferType;
    typedef unsigned char byte;

    namespace AudioThreads
    {

        enum Enum { MainThread = 0, MixThread };

    } // namespace AudioThreads

    namespace AudioConstants
    {

        // The sample rate used by the audio engine for the output mix
        //const unsigned cSystemSampleRate = 48000;
        // The time increment per audio frame corresponding to the sample rate
        //const double cSystemTimeIncrement = 1.0 / 48000.0;
        // The number of frames used to interpolate instant property changes
        const unsigned  cPropertyChangeFrames = (unsigned)(48000 * 0.01f);
        // Maximum number of channels in audio output
        const unsigned cMaxChannels = 8;
        // Volume modifier applied to all generated waves
        const float cGeneratedWaveVolume = 0.5f;

        const float cMaxVolumeValue = 50.0f;
        const float cMaxDecibelsValue = 20.0f;
        const float cMinDecibelsValue = -100.0f;
        const float cMinPitchValue = -5.0f;
        const float cMaxPitchValue = 5.0f;
        const float cMinSemitonesValue = -100.0f;
        const float cMaxSemitonesValue = 100.0f;

        const int cTrue = 1;
        const int cFalse = 0;

        bool IsWithinLimit(float valueToCheck, float centralValue, float limit);

        void AppendToBuffer(BufferType* destinationBuffer, const BufferType& sourceBuffer,
            unsigned sourceStartIndex, unsigned numberOfSamples);

        float PitchToSemitones(float pitch);

        float SemitonesToPitch(float semitone);

        float VolumeToDecibels(float volume);

        float DecibelsToVolume(float decibels);

    } // namespace AudioConstants


}
