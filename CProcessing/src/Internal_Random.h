//---------------------------------------------------------
// file:	Internal_Random.h
// author:	Justin Chambers
// brief:	Header for all internal random helper functions
//
// Internal header: not part of the public API.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#pragma once

#include <math.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Internal use
void CP_Random_Init(void);

//---------------------------------------------------------
// Pure helpers: no state, no window. They live here so that the Tier 1 tests
// can reach their edge cases directly.

// SplitMix64 (Steele, Lea and Flood; this is Sebastiano Vigna's public-domain
// version). Turns any 64-bit number, even 0 or a counter, into well-mixed
// 64-bit values: CP_Random_Seed and CP_Random_NoiseSeed use it to make the
// generator's starting state from a seed, so that a seed means the same
// numbers on every platform (the C runtime's srand and rand don't).
static inline uint64_t CP_Random_SplitMix64(uint64_t* state)
{
	uint64_t z = (*state += 0x9E3779B97F4A7C15ull);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
	return z ^ (z >> 31);
}

// 32 random bits to a float from 0 up to but not including 1. The bits times
// 2^-32 is exact in double, but rounding it to a float rounds the 128 largest
// inputs up to exactly 1.0 (about 1 draw in 33 million), so those get the
// largest float below 1 instead. Every other value is unchanged.
static inline float CP_Random_FloatFromBits(uint32_t bits)
{
	const float f = (float)((double)bits * (1.0 / 4294967296.0));
	return f < 1.0f ? f : 0.99999994f; // 1 - 2^-24, the largest float below 1
}

// lowerBound + f * (upperBound - lowerBound) for f from 0 up to but not
// including 1: a float from lowerBound up to but not including upperBound
// (the bounds are swapped if they are the wrong way round). It is worked out
// in double and rounded to a float once, so it doesn't depend on whether the
// compiler fuses the multiply and the add (some do, some don't), and the
// result can't round up to upperBound.
static inline float CP_Random_RangeFloatFromUnit(float lowerBound, float upperBound, float f)
{
	if (upperBound < lowerBound)
	{
		const float temp = lowerBound;
		lowerBound = upperBound;
		upperBound = temp;
	}
	const float value = (float)((double)lowerBound + (double)f * ((double)upperBound - (double)lowerBound));
	if (value < upperBound)
	{
		return value;
	}
	return upperBound > lowerBound ? nextafterf(upperBound, lowerBound) : lowerBound;
}

#ifdef __cplusplus
}
#endif
