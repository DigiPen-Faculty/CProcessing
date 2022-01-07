//---------------------------------------------------------------------------------------------------------------------
// file:    PositionalAudio.h
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#pragma once

#include "AEMath.h"

namespace AudioEngine
{
    //------------------------------------------------------------------------------------------------- PositionalAudio
    class PositionalAudio
    {
    public:
        PositionalAudio();

        void ProcessBuffer(float* buffer, unsigned howManyFrames, unsigned channels,
            const Vec3& objectPosition, const Vec3& listenerPosition);

    private:
        float mAttenuationStartDist;
        float mAttenuationEndDist;
        float mPreviousAttenuatedVolume;
        float mPreviousGain1;
        float mPreviousGain2;
        Vec3 mPreviousRelativePosition;
        float mMinimumChannelGain;
        Vec2 mLeftSpeakerValues;
        Vec2 mRightSpeakerValues;

        void CalculateValues(const Vec3& relativePosition, const float distance, float* gain1, float* gain2);
    };
}

