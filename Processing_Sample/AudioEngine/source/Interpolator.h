//---------------------------------------------------------------------------------------------------------------------
// file:    Interpolator.h
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#pragma once

namespace AudioEngine
{
    enum class FalloffCurveType
    {
        Linear, Squared, Sine, SquareRoot, Log, 
    };

    //---------------------------------------------------------------------------------------------------- Interpolator

    // Object to interpolate either sequentially or with direct access. 
    class Interpolator
    {
    public:
        // Sets default values to 0. Uses LinearCurve by default. 
        Interpolator();

        // Calculates the next sequential value in the interpolation. If past the end point, returns end value. 
        float NextValue();
        // Calculates the interpolated value at a specified index. If past the end, returns end value.
        float ValueAtIndex(const unsigned index);
        // Calculates the interpolated value at a specified point. If past the total distance, returns end value. 
        float ValueAtDistance(const float distance);
        // Moves sequential interpolation forward by a specified number.
        void JumpForward(const unsigned howManyFrames);
        // Moves sequential interpolation backward by a specified number.
        void JumpBackward(const unsigned howManyFrames);
        // Sets the interpolation to a specified position.
        void SetFrame(const unsigned frame);
        // Gets the total number of frames in the interpolation
        unsigned GetTotalFrames();
        // Gets the current frame of the interpolation.
        unsigned GetCurrentFrame();
        // Checks if the sequential interpolation has reached its end point.
        bool Finished();
        // Gets the end value of the interpolation.
        float GetEndValue() const;
        // Gets the start value of the interpolation.
        float GetStartValue();
        // Gets the current value of a sequential interpolation.
        const float GetCurrentValue();
        // Gets the type of curve currently being used. 
        const FalloffCurveType GetCurveType() const;
        // Changes the curve type of this interpolator. 
        void SetCurve(const FalloffCurveType curveType);
        // Resets the object for sequential interpolation using NextValue or ValueAtIndex.
        // If no start value is specified, will use the current value
        void SetValues(const float startValue, const float endValue, const unsigned numberOfFrames);
        void SetValues(const float endValue, const unsigned numberOfFrames);
        // Resets the object for direct-access interpolation using ValueAtDistance.
        void SetValues(const float startValue, const float endValue, const float distance);

    private:
        // Starting value to interpolate from. 
        float mStartValue;
        // Ending value to interpolate to. 
        float mEndValue;
        // Total distance to interpolate over using direct-access. 
        float mTotalDistance;
        // Total frames to interpolate over using sequential access. 
        unsigned mTotalFrames;
        // Current frame of sequential interpolation. 
        unsigned mCurrentFrame;
        // The type of curve currently being used
        FalloffCurveType mCurrentCurveType;
        // A pointer to the function used to get values. Set according to curve type.
        float(*GetValue)(const float current, const float total, const float startValue, const float endValue);
    };
}
