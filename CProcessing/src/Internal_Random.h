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
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Internal use
void CP_Random_Init(void);

//---------------------------------------------------------
// Pure helpers: no state, no window. They live here so that the Tier 1 tests
// can reach their edge cases directly.

// Watches CP_Random_Seed for the Debugging page's "seeding every frame":
// seeded on frame after frame with a seed that stays the same for a while
// and then changes (time(NULL), which changes once a second), so the
// "random" numbers are the same until it changes. True once, when the seed
// has changed twice that way. The same seed on every frame (to draw the same
// random stars every frame) is a real technique, and a seed that changes
// every frame gives new numbers every frame, so neither is the mistake. Only
// the first seed of a frame counts.
typedef struct CP_SeedWatch
{
	int lastFrame;
	int lastSeed;
	int sameRun; // frames in a row that repeated the seed before
	int changes; // times the seed changed after repeating
	bool told;
} CP_SeedWatch;

#define CP_SEED_WATCH_START { -2, 0, 0, 0, false }

static inline bool CP_Random_WatchSeed(CP_SeedWatch* watch, int frame, int seed)
{
	if (watch->told || frame == watch->lastFrame)
	{
		return false;
	}
	if (frame != watch->lastFrame + 1)
	{
		watch->sameRun = 0;
		watch->changes = 0;
	}
	else if (seed == watch->lastSeed)
	{
		++watch->sameRun;
	}
	else
	{
		watch->changes += watch->sameRun >= 2 ? 1 : 0;
		watch->sameRun = 0;
	}
	watch->lastFrame = frame;
	watch->lastSeed = seed;
	watch->told = watch->changes >= 2;
	return watch->told;
}

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
