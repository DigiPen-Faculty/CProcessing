//---------------------------------------------------------------------------------------------------------------------
// file:    PositionalAudio.cpp
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#include "PositionalAudio.h"
#include <string.h>

namespace AudioEngine
{
    //------------------------------------------------------------------------------------------------- PositionalAudio

    //*****************************************************************************************************************
    PositionalAudio::PositionalAudio() :
        mAttenuationStartDist(1.0f),
        mAttenuationEndDist(20.0f),
        mPreviousAttenuatedVolume(0.0f),
        mPreviousGain1(0.0f),
        mPreviousGain2(0.0f),
        mMinimumChannelGain(0.2f),
        mPreviousRelativePosition(10000.0f, 10000.0f, 10000.0f)
    {
        Vec2 normalVec(1.0f, 0.0f);
        Mat2 rotationMat;
        rotationMat.Rotate(30.0f * TwoPi / 360.0f);
        Vec2 rightSpeakerVec = rotationMat * normalVec;
        rotationMat.Rotate(330.0f * TwoPi / 360.0f);
        Vec2 leftSpeakerVec = rotationMat * normalVec;
        Mat2 speakerMat(rightSpeakerVec.X, rightSpeakerVec.Y, leftSpeakerVec.X, leftSpeakerVec.Y);
        speakerMat.Invert();
        mLeftSpeakerValues = Vec2(speakerMat.m00, speakerMat.m10);
        mRightSpeakerValues = Vec2(speakerMat.m01, speakerMat.m11);
    }

    //*****************************************************************************************************************
    void PositionalAudio::ProcessBuffer(float* buffer, unsigned howManyFrames, unsigned channels,
        const Vec3& objectPosition, const Vec3& listenerPosition)
    {
        if (channels != 2)
            return;

        Vec3 relativePosition = listenerPosition - objectPosition;

        // Store the distance between the sound and the listener
        float distance = relativePosition.Length();

        // If distance is greater than the end distance, set buffer to zero and return
        if (distance > mAttenuationEndDist)
        {
            memset(buffer, 0, howManyFrames * channels * sizeof(float));
            return;
        }

        float attenuatedVolume(1.0f);

        // If the distance is between the start and end distances, find the volume
        if (distance > mAttenuationStartDist)
            attenuatedVolume = Interpolate(1.0f, 0.0f, (distance - mAttenuationStartDist) /
                (mAttenuationEndDist - mAttenuationStartDist));

        // If behind listener, mirror to front for stereo
        // (could apply low pass here)
        if (relativePosition.X < 0)
            relativePosition.X *= -1.0f;

        float gain1(1.0f), gain2(1.0f);

        // Only run calculations if not too close
        if (distance > 0.1f || !(relativePosition.X == 0.0f && relativePosition.Z == 0.0f))
        {
            CalculateValues(relativePosition, distance, &gain1, &gain2);

            float volume = attenuatedVolume;

            for (unsigned frame = 0; frame < howManyFrames; ++frame)
            {
                float percent = (float)frame / (float)howManyFrames;

                if (attenuatedVolume != mPreviousAttenuatedVolume)
                    volume = Interpolate(mPreviousAttenuatedVolume, attenuatedVolume, percent);

                unsigned sampleIndex(frame * channels);

                float monoValue = buffer[sampleIndex] + buffer[sampleIndex + 1];
                monoValue /= 2.0f;

                buffer[sampleIndex] *= mMinimumChannelGain * volume;
                buffer[sampleIndex + 1] *= mMinimumChannelGain * volume;

                buffer[sampleIndex] += monoValue * volume * Interpolate(mPreviousGain1, gain1, percent);
                buffer[sampleIndex + 1] += monoValue * volume * Interpolate(mPreviousGain2, gain2, percent);
            }
        }

        mPreviousAttenuatedVolume = attenuatedVolume;
        mPreviousGain1 = gain1;
        mPreviousGain2 = gain2;
    }

    //*****************************************************************************************************************
    void PositionalAudio::CalculateValues(const Vec3& relativePosition, const float distance,
        float* gain1Ptr, float* gain2Ptr)
    {
        float& gain1 = *gain1Ptr;
        float& gain2 = *gain2Ptr;

        if (relativePosition != mPreviousRelativePosition)
        {
            Vec2 normalizedVector(relativePosition.X / distance, relativePosition.Z / distance);

            gain1 = normalizedVector.Dot(mLeftSpeakerValues);
            gain2 = normalizedVector.Dot(mRightSpeakerValues);

            if (gain1 < 0)
            {
                gain1 = 0.0f;
                gain2 = 1.0f;
            }
            else if (gain2 < 0)
            {
                gain1 = 1.0f;
                gain2 = 0.0f;
            }

            if (gain1 < mMinimumChannelGain)
                gain1 = mMinimumChannelGain;
            if (gain2 < mMinimumChannelGain)
                gain2 = mMinimumChannelGain;

            float scaleFactor = 1.0f / sqrtf(gain1 * gain1 + gain2 * gain2);
            gain1 *= scaleFactor;
            gain2 *= scaleFactor;

            mPreviousRelativePosition = relativePosition;
        }
        else
        {
            gain1 = mPreviousGain1;
            gain2 = mPreviousGain2;
        }
    }

}
