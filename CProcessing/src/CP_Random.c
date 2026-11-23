//---------------------------------------------------------
// file:	CP_Random.c
// author:	Justin Chambers
// brief:	Random numbers (uniform and Gaussian) and the seed
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include "Internal_Random.h"
#include <stdint.h>
#include <math.h>
#include <time.h>

//------------------------------------------------------------------------------
// Defines and Internal Variables:
//------------------------------------------------------------------------------

// The generator is xoshiro128** by David Blackman and Sebastiano Vigna
// (public domain): 128 bits of state, 32 random bits per call, and good
// quality in every bit (the old xorshift128's lowest bit was a plain
// linear-feedback bit, and CP_Random_GetBool uses the lowest bit).
// The state must not be all zero; CP_Random_Seed makes sure of that.
static uint32_t state[4];

// Gaussian numbers are made two at a time; this is the second one, waiting
static double gaussianSpare = 0.0;
static int gaussianHasSpare = 0;

//------------------------------------------------------------------------------
// Internal Functions:
//------------------------------------------------------------------------------

static inline uint32_t rotateLeft(uint32_t x, int k)
{
	return (x << k) | (x >> (32 - k));
}

static inline uint32_t internalRand(void)
{
	const uint32_t result = rotateLeft(state[1] * 5, 7) * 9;
	const uint32_t t = state[1] << 9;
	state[2] ^= state[0];
	state[3] ^= state[1];
	state[1] ^= state[2];
	state[0] ^= state[3];
	state[2] ^= t;
	state[3] = rotateLeft(state[3], 11);
	return result;
}

void CP_Random_Init(void)
{
	CP_Random_Seed((int)time(NULL));
}

//------------------------------------------------------------------------------
// Library Functions:
//------------------------------------------------------------------------------

CP_API bool CP_Random_GetBool(void)
{
	return internalRand() & 0x01;
}

CP_API unsigned int CP_Random_GetInt(void)
{
	return internalRand();
}

CP_API int CP_Random_RangeInt(int lowerBound, int upperBound)
{
	if (upperBound < lowerBound)
	{
		int temp = lowerBound;
		lowerBound = upperBound;
		upperBound = temp;
	}

	// The distance in unsigned arithmetic is exact, even from INT_MIN to
	// INT_MAX (a signed upper - lower would overflow)
	const uint32_t range = (uint32_t)upperBound - (uint32_t)lowerBound;

	// The full range of 2^32 values: range + 1 would wrap to 0
	if (range == UINT32_MAX)
	{
		return (int)internalRand();
	}

	// internalRand() % count favors the low numbers when count doesn't divide
	// 2^32 evenly. Throwing away the first 2^32 % count values makes it even.
	// That's under count values out of 2^32, so for ordinary ranges a draw is
	// almost never thrown away.
	const uint32_t count = range + 1;
	const uint32_t threshold = (0u - count) % count; // = 2^32 % count
	uint32_t r;
	do
	{
		r = internalRand();
	} while (r < threshold);
	// Added in unsigned arithmetic, so a negative lowerBound wraps to the right
	// answer instead of overflowing
	return (int)((uint32_t)lowerBound + r % count);
}

CP_API float CP_Random_GetFloat(void)
{
	return CP_Random_FloatFromBits(internalRand());
}

CP_API float CP_Random_RangeFloat(float lowerBound, float upperBound)
{
	return CP_Random_RangeFloatFromUnit(lowerBound, upperBound, CP_Random_GetFloat());
}

CP_API void CP_Random_Seed(int seed)
{
	// SplitMix64 turns the seed into the generator's state without the C
	// runtime's srand and rand, whose numbers differ between compilers and
	// operating systems (and which a program may be using for itself). The
	// same seed now gives the same numbers on every platform.
	uint64_t stream = (uint64_t)(uint32_t)seed;
	const uint64_t a = CP_Random_SplitMix64(&stream);
	const uint64_t b = CP_Random_SplitMix64(&stream);
	state[0] = (uint32_t)a;
	state[1] = (uint32_t)(a >> 32);
	state[2] = (uint32_t)b;
	state[3] = (uint32_t)(b >> 32);
	if ((state[0] | state[1] | state[2] | state[3]) == 0)
	{
		state[0] = 1; // all zero is the one state the generator can't leave
	}

	// A Gaussian number waiting from before the seed would break "the same
	// seed gives the same numbers"
	gaussianHasSpare = 0;
}

//		Gaussian returns a normally distributed value where the mean is 0 and the standard deviation is 1.0
CP_API float CP_Random_Gaussian(void)
{
	if (gaussianHasSpare)
	{
		gaussianHasSpare = 0;
		return (float)gaussianSpare;
	}

	// The polar form of the Box-Muller method: pick a point in the unit
	// circle (not the middle), and turn it into two Gaussian numbers
	double U1, U2, W;
	do
	{
		U1 = -1 + ((double)internalRand() / (double)UINT32_MAX) * 2;
		U2 = -1 + ((double)internalRand() / (double)UINT32_MAX) * 2;
		W = U1 * U1 + U2 * U2;
	} while (W >= 1 || W == 0);

	const double multiplier = sqrt((-2 * log(W)) / W);
	gaussianSpare = U2 * multiplier;
	gaussianHasSpare = 1;
	return (float)(U1 * multiplier);
}
