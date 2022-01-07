//---------------------------------------------------------------------------------------------------------------------
// file:    SoundInstance.h
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#pragma once

#include "Definitions.h"
#include "PositionalAudio.h"
#include "FileDecoder.h"
#include "Interpolator.h"
#include "InList.hpp"
#include "Resampler.h"
#include "PitchChange.h"

namespace AudioEngine
{
    //------------------------------------------------------------------------------------------- ThreadedSoundInstance
    class ThreadedSoundInstance
    {
    private:
        ThreadedSoundInstance(SoundAsset* asset, bool loop, bool startPaused, InstanceIDType ID, unsigned group);
        ~ThreadedSoundInstance();

        bool GetNextSamples(float* outputBuffer, unsigned howManyFrames, unsigned channels);
        void Loop();
        static void TranslateChannels(BufferType* inputSamples, unsigned inputFrames,
            unsigned inputChannels, unsigned outputChannels);
        void FinishedCleanup();
        void SetPitch(float pitchFactor);

        AtomicType mStopFlag_Shared = 0;
        AtomicType mPauseFlag_Shared = 0;
        AtomicType mResumeFlag_Shared = 0;

        SoundAsset* mAsset;
        unsigned mFrameIndex = 0;
        PositionalAudio PositionalObject;
        Interpolator VolumeInterpolator;
        PitchChangeHandler PitchObject;
        unsigned mChannels;
        unsigned mTotalFrames;
        bool mActive = true;
        bool mPaused;
        float mVolume = 1.0f;
        float mSavedVolume = 0.0f;
        bool mLooping = false;
        bool mPitchShifting = false;
        InstanceIDType mID;
        unsigned mGroup;

    public:

        InListLink<ThreadedSoundInstance> Link;

        friend class Mixer;
        friend class SoundInstance;
    };

    //--------------------------------------------------------------------------------------------------- SoundInstance
    class SoundInstance
    {
    private:
        SoundInstance(SoundAsset* asset, bool loop, bool startPaused, unsigned group);
        ~SoundInstance();

    public:
        void Stop();
        float GetVolume() { return mVolume; }
        void SetVolume(float newVolume);
        float GetPitch() { return mPitch; }
        void SetPitch(float pitchFactor);
        void SetPaused(bool paused);

    private:
        ThreadedSoundInstance ThreadedInstance;

        InstanceIDType mID;
        float mVolume = 1.0f;
        float mPitch = 1.0f;

        friend class SoundAsset;
        friend class Mixer;
    };
}
