//---------------------------------------------------------------------------------------------------------------------
// file:    Interpolator.cpp
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#include "Interpolator.h"
#include "AEMath.h"

namespace AudioEngine
{
    //*****************************************************************************************************************
    float GetValueLinearCurve(const float current, const float total, const float startValue,
        const float endValue)
    {
        return ((current / total) * (endValue - startValue)) + startValue;
    }

    //*****************************************************************************************************************
    float GetValueSquaredCurve(const float current, const float total, const float startValue,
        const float endValue)
    {
        float percent = current / total;
        return ((percent * percent) * (endValue - startValue)) + startValue;
    }

    //*****************************************************************************************************************
    float GetValueSineCurve(const float current, const float total, const float startValue,
        const float endValue)
    {
        return (sinf((current / total) * TwoPi) * (endValue - startValue)) + startValue;
    }

    //*****************************************************************************************************************
    float GetValueSquareRootCurve(const float current, const float total, const float startValue,
        const float endValue)
    {
        float percent = current / total;

        if (percent < 0)
            return startValue;

        return (sqrtf(percent) * (endValue - startValue)) + startValue;
    }

    //*****************************************************************************************************************
    const float expDenominator = logf(101.0f);
    float GetValueExponentialCurve(const float current, const float total, const float startValue,
        const float endValue)
    {
        return ((logf(((current / total) * 100.0f) + 1.0f) / expDenominator) *
            (endValue - startValue)) + startValue;
    }

    //---------------------------------------------------------------------------------------------------- Interpolator

    //*****************************************************************************************************************
    Interpolator::Interpolator() :
        mStartValue(0),
        mEndValue(0),
        mTotalFrames(0),
        mCurrentFrame(0),
        mTotalDistance(0),
        mCurrentCurveType(FalloffCurveType::Linear)
    {
        GetValue = GetValueLinearCurve;
    }

    //*****************************************************************************************************************
    float Interpolator::NextValue()
    {
        if (mTotalFrames == 0 || mCurrentFrame >= mTotalFrames || mEndValue == mStartValue)
            return mEndValue;

        return GetValue((float)mCurrentFrame++, (float)mTotalFrames, mStartValue, mEndValue);
    }

    //*****************************************************************************************************************
    float Interpolator::ValueAtIndex(const unsigned index)
    {
        if (mTotalFrames == 0 || index >= mTotalFrames || mEndValue == mStartValue)
            return mEndValue;

        if (index == 0)
            return mStartValue;

        return GetValue((float)index, (float)mTotalFrames, mStartValue, mEndValue);
    }

    //*****************************************************************************************************************
    float Interpolator::ValueAtDistance(const float currentDistance)
    {
        if (currentDistance == 0.0f)
            return mStartValue;

        if (currentDistance >= mTotalDistance || mEndValue == mStartValue)
            return mEndValue;

        return GetValue(currentDistance, mTotalDistance, mStartValue, mEndValue);
    }

    //*****************************************************************************************************************
    void Interpolator::SetValues(const float start, const float end, const unsigned frames)
    {
        mStartValue = start;
        mEndValue = end;
        mTotalFrames = frames;
        mCurrentFrame = 0;
        mTotalDistance = 0;

        if (start == end)
            mCurrentFrame = mTotalFrames;
    }

    //*****************************************************************************************************************
    void Interpolator::SetValues(const float endValue, const unsigned numberOfFrames)
    {
        mStartValue = GetCurrentValue();

        mEndValue = endValue;
        mTotalFrames = numberOfFrames;
        mCurrentFrame = 0;
        mTotalDistance = 0;

        if (mStartValue == mEndValue)
            mCurrentFrame = mTotalFrames;
    }

    //*****************************************************************************************************************
    void Interpolator::SetValues(const float start, const float end, const float distance)
    {
        mStartValue = start;
        mEndValue = end;
        mTotalFrames = 0;
        mCurrentFrame = 0;
        mTotalDistance = distance;
    }

    //*****************************************************************************************************************
    void Interpolator::JumpForward(const unsigned howManyFrames)
    {
        mCurrentFrame += howManyFrames;
    }

    //*****************************************************************************************************************
    void Interpolator::JumpBackward(const unsigned howManyFrames)
    {
        if (mCurrentFrame > howManyFrames)
            mCurrentFrame -= howManyFrames;
        else
            mCurrentFrame = 0;
    }

    //*****************************************************************************************************************
    bool Interpolator::Finished()
    {
        return mCurrentFrame >= mTotalFrames;
    }

    //*****************************************************************************************************************
    //bool InterpolatingObject::Finished()
    //{
    //    if (mCurrentFrame < mTotalFrames)
    //        return false;
    //    else
    //    {
    //        //if (nodeForEvent)
    //        //{
    //        //    // Notify the external object that the interpolation is done
    //        //    Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundNode::DispatchEventFromMixThread,
    //        //        *nodeForEvent, Events::AudioInterpolationDone), nodeForEvent);
    //        //}

    //        return true;
    //    }
    //}

    //*****************************************************************************************************************
    void Interpolator::SetCurve(const FalloffCurveType curveType)
    {
        mCurrentCurveType = curveType;

        switch (curveType)
        {
        case FalloffCurveType::Linear:
            GetValue = GetValueLinearCurve;
            break;
        case FalloffCurveType::Squared:
            GetValue = GetValueSquaredCurve;
            break;
        case FalloffCurveType::Sine:
            GetValue = GetValueSineCurve;
            break;
        case FalloffCurveType::SquareRoot:
            GetValue = GetValueSquareRootCurve;
            break;
        case FalloffCurveType::Log:
            GetValue = GetValueExponentialCurve;
            break;
        default:
            GetValue = GetValueLinearCurve;
            break;
        }
    }

    //*****************************************************************************************************************
    float Interpolator::GetStartValue()
    {
        return mStartValue;
    }

    //*****************************************************************************************************************
    float Interpolator::GetEndValue() const
    {
        return mEndValue;
    }

    //*****************************************************************************************************************
    const float Interpolator::GetCurrentValue()
    {
        if (mTotalFrames == 0 || mCurrentFrame >= mTotalFrames)
            return mEndValue;

        if (mCurrentFrame == 0)
            return mStartValue;

        return GetValue((float)mCurrentFrame, (float)mTotalFrames, mStartValue, mEndValue);
    }

    //*****************************************************************************************************************
    void Interpolator::SetFrame(const unsigned frame)
    {
        mCurrentFrame = frame;
    }

    //*****************************************************************************************************************
    unsigned Interpolator::GetTotalFrames()
    {
        return mTotalFrames;
    }

    //*****************************************************************************************************************
    unsigned Interpolator::GetCurrentFrame()
    {
        return mCurrentFrame;
    }

    //*****************************************************************************************************************
    const FalloffCurveType Interpolator::GetCurveType() const
    {
        return mCurrentCurveType;
    }

}
