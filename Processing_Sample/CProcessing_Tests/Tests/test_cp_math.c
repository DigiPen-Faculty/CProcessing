// Tier 1 coverage for CP_Math -- pure, stateless functions only.
// CP_Math_ScreenToWorld/WorldToScreen are excluded: they read the current
// nanovg transform from the live engine core and belong in the Tier 2
// engine harness (see 06-test-suite-plan.md).
#include <math.h>
#include "unity.h"
#include "cprocessing.h"

#define PI_F 3.14159265358979323846f

// ---- CP_Math_ClampInt ----

void test_math_clampint_below_min_returns_min(void)
{
    TEST_ASSERT_EQUAL_INT(0, CP_Math_ClampInt(-5, 0, 10));
}

void test_math_clampint_above_max_returns_max(void)
{
    TEST_ASSERT_EQUAL_INT(10, CP_Math_ClampInt(15, 0, 10));
}

void test_math_clampint_within_range_unchanged(void)
{
    TEST_ASSERT_EQUAL_INT(5, CP_Math_ClampInt(5, 0, 10));
}

void test_math_clampint_boundaries_are_inclusive(void)
{
    TEST_ASSERT_EQUAL_INT(0, CP_Math_ClampInt(0, 0, 10));
    TEST_ASSERT_EQUAL_INT(10, CP_Math_ClampInt(10, 0, 10));
}

// ---- CP_Math_ClampFloat ----

void test_math_clampfloat_below_min_returns_min(void)
{
    TEST_ASSERT_EQUAL_FLOAT(0.0f, CP_Math_ClampFloat(-5.0f, 0.0f, 10.0f));
}

void test_math_clampfloat_above_max_returns_max(void)
{
    TEST_ASSERT_EQUAL_FLOAT(10.0f, CP_Math_ClampFloat(15.0f, 0.0f, 10.0f));
}

void test_math_clampfloat_within_range_unchanged(void)
{
    TEST_ASSERT_EQUAL_FLOAT(5.5f, CP_Math_ClampFloat(5.5f, 0.0f, 10.0f));
}

// ---- CP_Math_LerpInt ----

void test_math_lerpint_factor_zero_returns_a(void)
{
    TEST_ASSERT_EQUAL_INT(0, CP_Math_LerpInt(0, 255, 0.0f));
}

void test_math_lerpint_factor_one_returns_b(void)
{
    TEST_ASSERT_EQUAL_INT(255, CP_Math_LerpInt(0, 255, 1.0f));
}

void test_math_lerpint_factor_half(void)
{
    // (1 - 0.5) * 0 + 0.5 * 255 = 127.5, truncated toward zero by the (int) cast.
    TEST_ASSERT_EQUAL_INT(127, CP_Math_LerpInt(0, 255, 0.5f));
}

void test_math_lerpint_factor_below_zero_clamps(void)
{
    TEST_ASSERT_EQUAL_INT(0, CP_Math_LerpInt(0, 255, -1.0f));
}

void test_math_lerpint_factor_above_one_clamps(void)
{
    TEST_ASSERT_EQUAL_INT(255, CP_Math_LerpInt(0, 255, 2.0f));
}

// ---- CP_Math_LerpFloat ----

void test_math_lerpfloat_factor_zero_returns_a(void)
{
    TEST_ASSERT_EQUAL_FLOAT(10.0f, CP_Math_LerpFloat(10.0f, 20.0f, 0.0f));
}

void test_math_lerpfloat_factor_one_returns_b(void)
{
    TEST_ASSERT_EQUAL_FLOAT(20.0f, CP_Math_LerpFloat(10.0f, 20.0f, 1.0f));
}

void test_math_lerpfloat_factor_half(void)
{
    TEST_ASSERT_EQUAL_FLOAT(15.0f, CP_Math_LerpFloat(10.0f, 20.0f, 0.5f));
}

void test_math_lerpfloat_factor_out_of_range_clamps(void)
{
    TEST_ASSERT_EQUAL_FLOAT(10.0f, CP_Math_LerpFloat(10.0f, 20.0f, -5.0f));
    TEST_ASSERT_EQUAL_FLOAT(20.0f, CP_Math_LerpFloat(10.0f, 20.0f, 5.0f));
}

// ---- CP_Math_Square ----

void test_math_square_positive(void)
{
    TEST_ASSERT_EQUAL_FLOAT(9.0f, CP_Math_Square(3.0f));
}

void test_math_square_negative(void)
{
    TEST_ASSERT_EQUAL_FLOAT(9.0f, CP_Math_Square(-3.0f));
}

void test_math_square_zero(void)
{
    TEST_ASSERT_EQUAL_FLOAT(0.0f, CP_Math_Square(0.0f));
}

// ---- CP_Math_Distance ----

void test_math_distance_same_point_is_zero(void)
{
    TEST_ASSERT_EQUAL_FLOAT(0.0f, CP_Math_Distance(1.0f, 1.0f, 1.0f, 1.0f));
}

void test_math_distance_3_4_5_triangle(void)
{
    TEST_ASSERT_EQUAL_FLOAT(5.0f, CP_Math_Distance(0.0f, 0.0f, 3.0f, 4.0f));
}

void test_math_distance_is_symmetric(void)
{
    float ab = CP_Math_Distance(2.0f, -3.0f, -7.0f, 5.0f);
    float ba = CP_Math_Distance(-7.0f, 5.0f, 2.0f, -3.0f);
    TEST_ASSERT_EQUAL_FLOAT(ab, ba);
}

// ---- CP_Math_Degrees / CP_Math_Radians ----

void test_math_degrees_zero(void)
{
    TEST_ASSERT_EQUAL_FLOAT(0.0f, CP_Math_Degrees(0.0f));
}

void test_math_degrees_pi_is_180(void)
{
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 180.0f, CP_Math_Degrees(PI_F));
}

void test_math_degrees_half_pi_is_90(void)
{
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 90.0f, CP_Math_Degrees(PI_F / 2.0f));
}

void test_math_radians_zero(void)
{
    TEST_ASSERT_EQUAL_FLOAT(0.0f, CP_Math_Radians(0.0f));
}

void test_math_radians_180_is_pi(void)
{
    TEST_ASSERT_FLOAT_WITHIN(0.001f, PI_F, CP_Math_Radians(180.0f));
}

void test_math_radians_90_is_half_pi(void)
{
    TEST_ASSERT_FLOAT_WITHIN(0.001f, PI_F / 2.0f, CP_Math_Radians(90.0f));
}

void test_math_degrees_and_radians_are_inverses(void)
{
    float original = 57.3f;
    float roundTripped = CP_Math_Degrees(CP_Math_Radians(original));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, original, roundTripped);
}
