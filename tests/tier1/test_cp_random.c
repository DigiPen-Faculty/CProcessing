// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors

// Tier 1 coverage for CP_Random (9 functions) plus the two CP_Noise
// functions the plan groups with it.
//
// Determinism: a seed gives the same numbers, and the same noise, on every
// platform. CP_Random_Seed and CP_Random_NoiseSeed don't use the C runtime's
// srand and rand (whose output differs between compilers and operating
// systems); they use SplitMix64 and xoshiro128**, which are plain integer
// arithmetic. So the tests below are of two kinds:
//   (a) pinned values for seed 42 (the exact numbers), which run on
//       Windows, Linux and macOS in CI. If one fails after a change to the
//       generator or the seeding, every program that seeds will get
//       different numbers: change them on purpose, with a CHANGELOG entry.
//   (b) property tests (bounds, swapped-bounds ordering, reseed
//       reproducibility, rough distribution shape).
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include "unity.h"
#include "cprocessing.h"
#include "Internal_Random.h"

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
		int v = CP_Random_RangeInt(10, 20);
		TEST_ASSERT_GREATER_OR_EQUAL_INT(10, v);
		TEST_ASSERT_LESS_OR_EQUAL_INT(20, v);
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
		int v = CP_Random_RangeInt(0, 1);
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
		int v = CP_Random_RangeInt(100, 50);
		TEST_ASSERT_GREATER_OR_EQUAL_INT(50, v);
		TEST_ASSERT_LESS_OR_EQUAL_INT(100, v);
	}
}

void test_random_rangeint_equal_bounds_returns_that_value(void)
{
	CP_Random_Seed(5);
	for (int i = 0; i < 100; ++i)
	{
		TEST_ASSERT_EQUAL_INT(42, CP_Random_RangeInt(42, 42));
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
		(void)CP_Random_RangeInt(INT_MIN, INT_MAX);
		(void)CP_Random_RangeInt(INT_MAX, INT_MIN);
	}
	TEST_PASS();
}

void test_random_rangeint_large_range_is_even(void)
{
	// A range of 3 * 2^30 values doesn't divide 2^32. Taking the random
	// number modulo the range made the first 2^30 values twice as likely:
	// a 1/2 share instead of 1/3.
	// (INT_MIN to 2^30 - 1 is exactly 3 * 2^30 values)
	const int upper = (1 << 30) - 1;
	int low = 0;
	CP_Random_Seed(11);
	for (int i = 0; i < 30000; ++i)
	{
		if (CP_Random_RangeInt(INT_MIN, upper) < INT_MIN + (1 << 30))
		{
			++low;
		}
	}
	TEST_ASSERT_FLOAT_WITHIN(0.03f, 1.0f / 3.0f, (float)low / 30000.0f);
}

void test_random_rangeint_negative_bounds_stay_inside(void)
{
	// Both bounds negative: before the bounds were signed, a negative bound
	// became a number near 4 billion and every answer was wrong
	CP_Random_Seed(12);
	for (int i = 0; i < SAMPLE_COUNT; ++i)
	{
		int v = CP_Random_RangeInt(-100, -90);
		TEST_ASSERT_GREATER_OR_EQUAL_INT(-100, v);
		TEST_ASSERT_LESS_OR_EQUAL_INT(-90, v);
	}
}

void test_random_rangeint_across_zero_reaches_both_ends_and_both_signs(void)
{
	int counts[11] = { 0 };
	CP_Random_Seed(13);
	for (int i = 0; i < 11000; ++i)
	{
		int v = CP_Random_RangeInt(-5, 5);
		TEST_ASSERT_GREATER_OR_EQUAL_INT(-5, v);
		TEST_ASSERT_LESS_OR_EQUAL_INT(5, v);
		++counts[v + 5];
	}
	// every value from -5 to 5 turns up, about 1000 times each
	for (int value = 0; value < 11; ++value)
	{
		TEST_ASSERT_INT_WITHIN(250, 1000, counts[value]);
	}
}

void test_random_rangeint_swaps_reversed_negative_bounds(void)
{
	CP_Random_Seed(14);
	for (int i = 0; i < SAMPLE_COUNT; ++i)
	{
		int v = CP_Random_RangeInt(10, -10);
		TEST_ASSERT_GREATER_OR_EQUAL_INT(-10, v);
		TEST_ASSERT_LESS_OR_EQUAL_INT(10, v);
	}
}

void test_random_rangeint_the_extremes_return_the_value(void)
{
	CP_Random_Seed(15);
	TEST_ASSERT_EQUAL_INT(INT_MIN, CP_Random_RangeInt(INT_MIN, INT_MIN));
	TEST_ASSERT_EQUAL_INT(INT_MAX, CP_Random_RangeInt(INT_MAX, INT_MAX));
	TEST_ASSERT_EQUAL_INT(-1, CP_Random_RangeInt(-1, -1));
}

void test_random_rangeint_full_signed_range_is_about_half_negative(void)
{
	int negative = 0;
	CP_Random_Seed(16);
	for (int i = 0; i < 20000; ++i)
	{
		if (CP_Random_RangeInt(INT_MIN, INT_MAX) < 0)
		{
			++negative;
		}
	}
	TEST_ASSERT_FLOAT_WITHIN(0.03f, 0.5f, (float)negative / 20000.0f);
}

void test_random_getfloat_stays_within_0_and_1(void)
{
	CP_Random_Seed(6);
	for (int i = 0; i < SAMPLE_COUNT; ++i)
	{
		float v = CP_Random_GetFloat();
		TEST_ASSERT_GREATER_OR_EQUAL_FLOAT(0.0f, v);
		TEST_ASSERT_LESS_THAN_FLOAT(1.0f, v); // from 0 up to, but not including, 1
	}
}

void test_random_rangefloat_respects_bounds(void)
{
	CP_Random_Seed(7);
	for (int i = 0; i < SAMPLE_COUNT; ++i)
	{
		float v = CP_Random_RangeFloat(-5.0f, 5.0f);
		TEST_ASSERT_GREATER_OR_EQUAL_FLOAT(-5.0f, v);
		TEST_ASSERT_LESS_THAN_FLOAT(5.0f, v); // up to, but not including, the upper bound
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

void test_random_seed_reproduces_sequence(void)
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
// ---- Pinned values: the same on every platform (see the top of this file) ----

void test_random_getint_pinned_sequence_for_seed_42(void)
{
	CP_Random_Seed(42);
	TEST_ASSERT_EQUAL_UINT32(1776835114u, CP_Random_GetInt());
	TEST_ASSERT_EQUAL_UINT32(4165204688u, CP_Random_GetInt());
	TEST_ASSERT_EQUAL_UINT32(17111135u, CP_Random_GetInt());
}

void test_random_rangeint_pinned_dice_for_seed_42(void)
{
	const int expected[6] = { 5, 3, 6, 3, 6, 5 };
	CP_Random_Seed(42);
	for (int i = 0; i < 6; ++i)
	{
		TEST_ASSERT_EQUAL_INT(expected[i], CP_Random_RangeInt(1, 6));
	}
}

void test_random_float_pinned_values_for_seed_42(void)
{
	// the third and fourth draws are RangeFloat's
	CP_Random_Seed(42);
	TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.4137017f, CP_Random_GetFloat());
	TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.9697873f, CP_Random_GetFloat());
	TEST_ASSERT_FLOAT_WITHIN(1e-5f, 0.0199200f, CP_Random_RangeFloat(0.0f, 5.0f));
	TEST_ASSERT_FLOAT_WITHIN(1e-4f, 11.8612108f, CP_Random_RangeFloat(-150.0f, 150.0f));
}

void test_random_gaussian_pinned_values_for_seed_42(void)
{
	CP_Random_Seed(42);
	TEST_ASSERT_FLOAT_WITHIN(1e-5f, -0.077276f, CP_Random_Gaussian());
	TEST_ASSERT_FLOAT_WITHIN(1e-5f, 0.420673f, CP_Random_Gaussian());
	TEST_ASSERT_FLOAT_WITHIN(1e-5f, -0.138602f, CP_Random_Gaussian());
}

void test_random_noise_pinned_values_for_noiseseed_42(void)
{
	CP_Random_NoiseSeed(42);
	TEST_ASSERT_FLOAT_WITHIN(1e-5f, 0.354557f, CP_Random_Noise(1.5f, 2.5f, 0.5f));
	TEST_ASSERT_FLOAT_WITHIN(1e-5f, 0.293741f, CP_Random_Noise(10.25f, 0.0f, 0.0f));
}

// ---- Seeding ----

void test_random_seed_zero_and_one_differ(void)
{
	// glibc's srand treats 0 as 1, so these used to be the same sequence there
	CP_Random_Seed(0);
	unsigned int a = CP_Random_GetInt();
	CP_Random_Seed(1);
	unsigned int b = CP_Random_GetInt();
	TEST_ASSERT_NOT_EQUAL(a, b);

	CP_Random_NoiseSeed(0);
	float noiseA = CP_Random_Noise(1.5f, 2.5f, 0.5f);
	CP_Random_NoiseSeed(1);
	float noiseB = CP_Random_Noise(1.5f, 2.5f, 0.5f);
	TEST_ASSERT_TRUE(noiseA != noiseB);
}

void test_random_seeding_does_not_disturb_the_programs_own_rand(void)
{
	srand(1);
	const int expected = rand();
	srand(1);
	CP_Random_Seed(99);
	CP_Random_NoiseSeed(99);
	TEST_ASSERT_EQUAL_INT(expected, rand());
}

void test_random_noiseseed_does_not_change_the_random_sequence(void)
{
	CP_Random_Seed(7);
	const unsigned int expected = CP_Random_GetInt();
	CP_Random_Seed(7);
	CP_Random_NoiseSeed(123);
	TEST_ASSERT_EQUAL_UINT32(expected, CP_Random_GetInt());
}

void test_random_gaussian_repeats_after_reseeding_with_the_same_seed(void)
{
	// Gaussian numbers are made in pairs; the spare of the pair used to
	// survive a reseed and come out first
	CP_Random_Seed(5);
	const float first = CP_Random_Gaussian();
	CP_Random_Seed(5);
	TEST_ASSERT_TRUE(first == CP_Random_Gaussian());
}

// ---- GetFloat and RangeFloat edges ----

void test_random_floatfrombits_never_reaches_1(void)
{
	TEST_ASSERT_EQUAL_FLOAT(0.0f, CP_Random_FloatFromBits(0u));
	TEST_ASSERT_LESS_THAN_FLOAT(1.0f, CP_Random_FloatFromBits(UINT32_MAX));
	// the 128 largest inputs are the ones that round up to 1.0 in a float
	for (uint32_t bits = UINT32_MAX - 200u; bits != 0u; ++bits)
	{
		TEST_ASSERT_LESS_THAN_FLOAT(1.0f, CP_Random_FloatFromBits(bits));
	}
}

void test_random_floatfrombits_is_the_plain_division_below_the_top(void)
{
	// Seeded sequences of GetFloat are unchanged by never reaching 1.0:
	// everything but the top 128 inputs is bits / 2^32 as before
	CP_Random_Seed(21);
	for (int i = 0; i < SAMPLE_COUNT; ++i)
	{
		const uint32_t bits = CP_Random_GetInt();
		if (bits < UINT32_MAX - 200u)
		{
			TEST_ASSERT_TRUE((float)bits / (float)UINT32_MAX == CP_Random_FloatFromBits(bits));
		}
	}
}

void test_random_rangefloat_never_reaches_the_upper_bound(void)
{
	// f at its largest possible value, against ranges where the result
	// used to round up to the upper bound
	const float biggestF = 0.99999994f;
	const float ranges[][2] = {
		{ 0,     5        },
		{ 0,     6        },
		{ 0,     800      },
		{ 40,    760      },
		{ 0.95f, 1.05f    },
		{ -150,  150      },
		{ 0,     360      },
		{ 0,     1        },
		{ 1000,  1000.01f },
		{ -1000, -999.99f },
		{ 1e6f,  1e6f + 8 }
	};
	for (unsigned i = 0; i < sizeof(ranges) / sizeof(ranges[0]); ++i)
	{
		const float v = CP_Random_RangeFloatFromUnit(ranges[i][0], ranges[i][1], biggestF);
		TEST_ASSERT_LESS_THAN_FLOAT(ranges[i][1], v);
		TEST_ASSERT_GREATER_OR_EQUAL_FLOAT(ranges[i][0], v);
	}

	// and for many random ranges
	CP_Random_Seed(22);
	for (int i = 0; i < SAMPLE_COUNT; ++i)
	{
		const float lower = CP_Random_RangeFloat(-1000.0f, 1000.0f);
		const float upper = lower + CP_Random_RangeFloat(0.001f, 2000.0f);
		const float v = CP_Random_RangeFloatFromUnit(lower, upper, biggestF);
		TEST_ASSERT_LESS_THAN_FLOAT(upper, v);
		TEST_ASSERT_GREATER_OR_EQUAL_FLOAT(lower, v);
	}
}

void test_random_rangefloat_equal_bounds_returns_that_value(void)
{
	CP_Random_Seed(23);
	TEST_ASSERT_EQUAL_FLOAT(3.5f, CP_Random_RangeFloat(3.5f, 3.5f));
	TEST_ASSERT_EQUAL_FLOAT(-2.0f, CP_Random_RangeFloatFromUnit(-2.0f, -2.0f, 0.99999994f));
}

void test_random_rangefloat_can_reach_the_lower_bound(void)
{
	TEST_ASSERT_EQUAL_FLOAT(2.0f, CP_Random_RangeFloatFromUnit(2.0f, 9.0f, 0.0f));
	TEST_ASSERT_EQUAL_FLOAT(2.0f, CP_Random_RangeFloatFromUnit(9.0f, 2.0f, 0.0f));
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

// ---- Noise octaves ----

void test_noise_one_octave_is_plain_noise(void)
{
	CP_Random_NoiseSeed(5);
	for (int i = 0; i < 50; ++i)
	{
		const float x = (float)i * 0.37f - 8.0f;
		TEST_ASSERT_EQUAL_FLOAT(CP_Random_Noise(x, 1.5f, 0.25f), CP_Random_NoiseOctaves(x, 1.5f, 0.25f, 1, 0.5f));
	}
}

void test_noise_octaves_below_one_count_as_one(void)
{
	CP_Random_NoiseSeed(5);
	TEST_ASSERT_EQUAL_FLOAT(CP_Random_Noise(3.3f, 0.4f, 0.0f), CP_Random_NoiseOctaves(3.3f, 0.4f, 0.0f, 0, 0.5f));
	TEST_ASSERT_EQUAL_FLOAT(CP_Random_Noise(3.3f, 0.4f, 0.0f), CP_Random_NoiseOctaves(3.3f, 0.4f, 0.0f, -4, 0.5f));
}

void test_noise_octaves_with_no_persistence_are_plain_noise(void)
{
	CP_Random_NoiseSeed(5);
	// Every layer after the first has strength 0
	TEST_ASSERT_EQUAL_FLOAT(CP_Random_Noise(3.3f, 0.4f, 0.0f), CP_Random_NoiseOctaves(3.3f, 0.4f, 0.0f, 6, 0.0f));
}

void test_noise_octaves_stay_within_0_and_1(void)
{
	CP_Random_NoiseSeed(9);
	for (int octaves = 1; octaves <= 8; ++octaves)
	{
		for (int i = -500; i < 500; ++i)
		{
			const float value = CP_Random_NoiseOctaves((float)i * 0.173f, (float)i * 0.059f, 0.3f, octaves, 0.5f);
			TEST_ASSERT_TRUE(value >= 0.0f && value <= 1.0f);
		}
	}
}

void test_noise_octaves_add_detail_and_are_deterministic(void)
{
	CP_Random_NoiseSeed(9);
	int different = 0;
	for (int i = 0; i < 100; ++i)
	{
		const float x = (float)i * 0.31f;
		const float once = CP_Random_NoiseOctaves(x, 0.7f, 0.1f, 4, 0.5f);
		TEST_ASSERT_EQUAL_FLOAT(once, CP_Random_NoiseOctaves(x, 0.7f, 0.1f, 4, 0.5f));
		if (once != CP_Random_Noise(x, 0.7f, 0.1f))
		{
			++different;
		}
	}
	// Four layers are not the same as one on nearly every sample
	TEST_ASSERT_GREATER_THAN_INT(90, different);
}

// ---- Noise: the traps students fall into ----

void test_noise_at_whole_numbers_is_not_always_the_same(void)
{
	// Perlin noise is 0 at every whole-number point, so this was 0.5 every
	// time: CP_Random_Noise(frameCount, 0, 0) drew a flat line
	CP_Random_NoiseSeed(21);
	float smallest = 1.0f;
	float largest = 0.0f;
	for (int i = 0; i < 20; ++i)
	{
		const float v = CP_Random_Noise((float)i, 0.0f, 0.0f);
		smallest = v < smallest ? v : smallest;
		largest = v > largest ? v : largest;
	}
	TEST_ASSERT_GREATER_THAN_FLOAT(0.2f, largest - smallest);
}

void test_noise_along_one_axis_uses_most_of_0_to_1(void)
{
	// With y and z at 0 it only spanned 0.25 to 0.75
	CP_Random_NoiseSeed(22);
	float smallest = 1.0f;
	float largest = 0.0f;
	for (int i = 0; i < 4000; ++i)
	{
		const float v = CP_Random_Noise((float)i * 0.05f, 0.0f, 0.0f);
		smallest = v < smallest ? v : smallest;
		largest = v > largest ? v : largest;
	}
	TEST_ASSERT_LESS_THAN_FLOAT(0.15f, smallest);
	TEST_ASSERT_GREATER_THAN_FLOAT(0.85f, largest);
}

void test_noise_octaves_keep_the_spread_of_one_layer(void)
{
	// Dividing by the sum of the strengths pulled four layers into about
	// 0.23 to 0.77
	CP_Random_NoiseSeed(23);
	float smallest = 1.0f;
	float largest = 0.0f;
	for (int i = 0; i < 4000; ++i)
	{
		const float v = CP_Random_NoiseOctaves((float)(i % 63) * 0.37f, (float)(i / 63) * 0.41f, 0.0f, 4, 0.5f);
		TEST_ASSERT_GREATER_OR_EQUAL_FLOAT(0.0f, v);
		TEST_ASSERT_LESS_OR_EQUAL_FLOAT(1.0f, v);
		smallest = v < smallest ? v : smallest;
		largest = v > largest ? v : largest;
	}
	TEST_ASSERT_LESS_THAN_FLOAT(0.15f, smallest);
	TEST_ASSERT_GREATER_THAN_FLOAT(0.85f, largest);
}
