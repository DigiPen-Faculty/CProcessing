//---------------------------------------------------------------------------------------------------------------------
// file:    AEMath.h
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#pragma once

#include <cmath>

namespace AudioEngine
{

    static const float Pi = std::acos(-1.0f);
    static const float HalfPi = Pi / 2.0f;
    static const float TwoPi = Pi * 2.0f;
    static const float SqRt2 = std::sqrt(2.0f);

    static inline float Interpolate(float startValue, float endValue, float currentPercent)
    {
        return (currentPercent * (endValue - startValue)) + startValue;
    }

    //------------------------------------------------------------------------------------------------------------ Vec2
    struct Vec2
    {
        Vec2() : X(0), Y(0) {}
        Vec2(float _x, float _y) : X(_x), Y(_y) {}

        float Dot(const Vec2& other) const;
        bool operator==(const Vec2& rhs) const;
        bool operator!=(const Vec2& rhs) const;

        float X;
        float Y;
    };

    //------------------------------------------------------------------------------------------------------------ Vec3
    struct Vec3
    {
        Vec3() : X(0), Y(0), Z(0) {}
        Vec3(float _x, float _y, float _z) : X(_x), Y(_y), Z(_z) {}

        float Length() const;
        bool operator==(const Vec3& rhs) const;
        bool operator!=(const Vec3& rhs) const;
        Vec3 operator-(const Vec3& rhs) const;

        float X;
        float Y;
        float Z;
    };

    //------------------------------------------------------------------------------------------------------------ Mat2
    struct Mat2
    {
        Mat2() : m00(0), m01(0), m10(0), m11(0) {}
        Mat2(float _00, float _01, float _10, float _11) : m00(_00), m01(_01), m10(_10), m11(_11) {}

        void Rotate(float radians);
        void Invert();
        Vec2 operator*(const Vec2& rhs) const;

        float m00;
        float m01;
        float m10;
        float m11;
    };
}