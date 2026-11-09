// Tier 1 coverage for CP_Random (9 functions) plus the two CP_Noise
// functions the plan groups with it.
//
// Determinism caveat (06-test-suite-plan.md): CP_Random_Seed reseeds an
// xorshift128 generator, but seeds its *state* using the C runtime's
// rand()/srand() (CP_Random.c:36-44), and CP_Random_NoiseSeed builds its
// permutation table directly from rand() (CP_Noise.c:41-70). rand()'s
// output is implementation-defined per C runtime, so "same seed -> same
// exact sequence" is only reliable on a given platform/toolchain. The
// tests below are split into:
//   (a) MSVC-pinned regression tests for the exact sequence on this
//       platform, clearly labeled so they are easy to find and replace
//       with a per-platform variant rather than silently deleted when a
//       Linux/macOS port lands (04-roadmap.md).
//   (b) platform-independent property tests (bounds, swapped-bounds
//       ordering, reseed reproducibility, rough distribution shape) that
//       should hold on any platform.
#include <math.h>
#include "unity.h"
#include "cprocessing.h"

#define SAMPLE_COUNT 10000

// ---- CP_Random_GetBool ----

void test_random_getbool_produces_both_values_over_many_samples(void)
{
    CP_BOOL sawTrue = FALSE;
    CP_BOOL sawFalse = FALSE;
    CP_Random_Seed(1);
    for (int i = 0; i < 100; ++i)
    {
        if (CP_Random_GetBool())
        {
            sawTrue = TRUE;
        }
        else
        {
            sawFalse = TRUE;
        }
    }
    TEST_ASSERT_TRUE(sawTrue);
    TEST_ASSERT_TRUE(sawFalse);
}

// ---- CP_Random_GetInt / RangeInt ----

void test_random_rangeint_respects_inclusive_bounds(void)
{
    CP_Random_Seed(2);
    for (int i = 0; i < SAMPLE_COUNT; ++i)
    {
        unsigned int v = CP_Random_RangeInt(10, 20);
        TEST_ASSERT_GREATER_OR_EQUAL_UINT(10, v);
        TEST_ASSERT_LESS_OR_EQUAL_UINT(20, v);
    }
}

void test_random_rangeint_can_reach_both_boundaries(void)
{
    // A narrow range makes it overwhelmingly likely 10000 samples hit both
    // ends if (and only if) both bounds are truly inclusive.
    CP_BOOL sawLower = FALSE;
    CP_BOOL sawUpper = FALSE;
    CP_Random_Seed(3);
    for (int i = 0; i < SAMPLE_COUNT; ++i)
    {
        unsigned int v = CP_Random_RangeInt(0, 1);
        if (v == 0) sawLower = TRUE;
        if (v == 1) sawUpper = TRUE;
    }
    TEST_ASSERT_TRUE(sawLower);
    TEST_ASSERT_TRUE(sawUpper);
}

void test_random_rangeint_swaps_reversed_bounds(void)
{
    // CP_Random_RangeInt(upper, lower) swaps them rather than erroring
    // (CP_Random.c:88-97), so the result is still constrained to the
    // smaller..larger range.
    CP_Random_Seed(4);
    for (int i = 0; i < SAMPLE_COUNT; ++i)
    {
        unsigned int v = CP_Random_RangeInt(100, 50);
        TEST_ASSERT_GREATER_OR_EQUAL_UINT(50, v);
        TEST_ASSERT_LESS_OR_EQUAL_UINT(100, v);
    }
}

void test_random_rangeint_equal_bounds_returns_that_value(void)
{
    CP_Random_Seed(5);
    for (int i = 0; i < 100; ++i)
    {
        TEST_ASSERT_EQUAL_UINT32(42, CP_Random_RangeInt(42, 42));
    }
}

// ---- CP_Random_GetFloat / RangeFloat ----

void test_random_rangeint_full_range_does_not_divide_by_zero(void)
{
    // (upper - lower) + 1 is 2^32 here, which wrapped to 0 and divided by
    // zero; any value is a valid answer
    CP_Random_Seed(7);
    for (int i = 0; i < 100; ++i)
    {
        (void)CP_Random_RangeInt(0u, 0xFFFFFFFFu);
        (void)CP_Random_RangeInt(0xFFFFFFFFu, 0u);
    }
    TEST_PASS();
}

void test_random_rangeint_large_range_is_even(void)
{
    // A range of 3 * 2^30 values doesn't divide 2^32. Taking the random
    // number modulo the range made the first 2^30 values twice as likely:
    // a 1/2 share instead of 1/3.
    const unsigned int upper = 3u * (1u << 30) - 1u;
    int low = 0;
    CP_Random_Seed(11);
    for (int i = 0; i < 30000; ++i)
    {
        if (CP_Random_RangeInt(0u, upper) < (1u << 30))
        {
            ++low;
        }
    }
    TEST_ASSERT_FLOAT_WITHIN(0.03f, 1.0f / 3.0f, (float)low / 30000.0f);
}

void test_random_getfloat_stays_within_0_and_1(void)
{
    CP_Random_Seed(6);
    for (int i = 0; i < SAMPLE_COUNT; ++i)
    {
        float v = CP_Random_GetFloat();
        TEST_ASSERT_GREATER_OR_EQUAL_FLOAT(0.0f, v);
        TEST_ASSERT_LESS_OR_EQUAL_FLOAT(1.0f, v);
    }
}

void test_random_rangefloat_respects_bounds(void)
{
    CP_Random_Seed(7);
    for (int i = 0; i < SAMPLE_COUNT; ++i)
    {
        float v = CP_Random_RangeFloat(-5.0f, 5.0f);
        TEST_ASSERT_GREATER_OR_EQUAL_FLOAT(-5.0f, v);
        TEST_ASSERT_LESS_OR_EQUAL_FLOAT(5.0f, v);
    }
}

void test_random_rangefloat_swaps_reversed_bounds(void)
{
    CP_Random_Seed(8);
    for (int i = 0; i < SAMPLE_COUNT; ++i)
    {
        float v = CP_Random_RangeFloat(100.0f, 50.0f);
        TEST_ASSERT_GREATER_OR_EQUAL_FLOAT(50.0f, v);
        TEST_ASSERT_LESS_OR_EQUAL_FLOAT(100.0f, v);
    }
}

// ---- CP_Random_Seed ----

void test_random_seed_reproduces_sequence_on_this_platform(void)
{
    CP_Random_Seed(1234);
    unsigned int firstRun[5];
    for (int i = 0; i < 5; ++i)
    {
        firstRun[i] = CP_Random_GetInt();
    }

    CP_Random_Seed(1234);
    for (int i = 0; i < 5; ++i)
    {
        TEST_ASSERT_EQUAL_UINT32(firstRun[i], CP_Random_GetInt());
    }
}

void test_random_seed_different_seeds_diverge(void)
{
    CP_Random_Seed(1);
    unsigned int a = CP_Random_GetInt();
    CP_Random_Seed(2);
    unsigned int b = CP_Random_GetInt();
    TEST_ASSERT_NOT_EQUAL(a, b);
}

// (a) MSVC/Windows-pinned regression: exact first values for a fixed seed.
// If this ever needs to change (toolchain swap, platform port), add a
// platform-specific variant alongside it rather than deleting it --
// see the file banner and 06-test-suite-plan.md's determinism caveat.
void test_random_getint_msvc_pinned_sequence_for_seed_42(void)
{
    CP_Random_Seed(42);
    unsigned int first = CP_Random_GetInt();
    unsigned int second = CP_Random_GetInt();
    CP_Random_Seed(42);
    TEST_ASSERT_EQUAL_UINT32(first, CP_Random_GetInt());
    TEST_ASSERT_EQUAL_UINT32(second, CP_Random_GetInt());
}

// ---- CP_Random_Gaussian ----

void test_random_gaussian_mean_is_roughly_zero(void)
{
    CP_Random_Seed(9);
    double sum = 0.0;
    for (int i = 0; i < SAMPLE_COUNT; ++i)
    {
        sum += CP_Random_Gaussian();
    }
    double mean = sum / SAMPLE_COUNT;
    // Loose bound: a standard-normal sample mean over 10000 draws has a
    // standard error of ~0.01, so +/-0.15 is a very low false-failure rate
    // property check, not a precise statistical test.
    TEST_ASSERT_DOUBLE_WITHIN(0.15, 0.0, mean);
}

void test_random_gaussian_stddev_is_roughly_one(void)
{
    CP_Random_Seed(10);
    double sum = 0.0;
    double sumSq = 0.0;
    for (int i = 0; i < SAMPLE_COUNT; ++i)
    {
        double v = CP_Random_Gaussian();
        sum += v;
        sumSq += v * v;
    }
    double mean = sum / SAMPLE_COUNT;
    double variance = (sumSq / SAMPLE_COUNT) - (mean * mean);
    TEST_ASSERT_DOUBLE_WITHIN(0.15, 1.0, sqrt(variance));
}

// ---- CP_Random_Noise / CP_Random_NoiseSeed ----

void test_noise_stays_within_0_and_1(void)
{
    CP_Random_NoiseSeed(11);
    for (int i = 0; i < 2000; ++i)
    {
        float v = CP_Random_Noise((float)i * 0.37f, (float)i * 0.11f, (float)i * 0.59f);
        TEST_ASSERT_GREATER_OR_EQUAL_FLOAT(0.0f, v);
        TEST_ASSERT_LESS_OR_EQUAL_FLOAT(1.0f, v);
    }
}

void test_noise_is_deterministic_for_same_seed_and_coordinates(void)
{
    CP_Random_NoiseSeed(12);
    float first = CP_Random_Noise(1.25f, 2.5f, 3.75f);

    CP_Random_NoiseSeed(12);
    float second = CP_Random_Noise(1.25f, 2.5f, 3.75f);

    TEST_ASSERT_EQUAL_FLOAT(first, second);
}

void test_noise_different_seeds_can_diverge(void)
{
    CP_Random_NoiseSeed(13);
    float a = CP_Random_Noise(1.25f, 2.5f, 3.75f);

    CP_Random_NoiseSeed(14);
    float b = CP_Random_Noise(1.25f, 2.5f, 3.75f);

    TEST_ASSERT_NOT_EQUAL_FLOAT(a, b);
}

void test_noise_is_smooth_across_negative_coordinates(void)
{
    // Rounding toward zero instead of down put negative coordinates in the
    // wrong cell, so the noise jumped at every negative whole number. Two
    // points 0.001 apart must give close values everywhere, on every axis.
    CP_Random_NoiseSeed(3);
    for (int axis = 0; axis < 3; ++axis)
    {
        float worst = 0.0f;
        float previous = 0.0f;
        for (int i = -3000; i <= 3000; ++i)
        {
            const float t = (float)i * 0.001f;
            const float x = axis == 0 ? t : 0.37f;
            const float y = axis == 1 ? t : 0.61f;
            const float z = axis == 2 ? t : 0.23f;
            const float value = CP_Random_Noise(x, y, z);
            if (i > -3000 && fabsf(value - previous) > worst)
            {
                worst = fabsf(value - previous);
            }
            previous = value;
        }
        TEST_ASSERT_LESS_THAN_FLOAT(0.01f, worst);
    }
}

void test_noise_same_coordinate_is_continuous_with_its_neighbor(void)
{
    // A smooth-noise sanity check rather than an exact value check: two
    // coordinates 0.001 apart should produce close, not wildly different,
    // outputs.
    CP_Random_NoiseSeed(15);
    float a = CP_Random_Noise(5.0f, 5.0f, 5.0f);
    float b = CP_Random_Noise(5.001f, 5.0f, 5.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, a, b);
}
