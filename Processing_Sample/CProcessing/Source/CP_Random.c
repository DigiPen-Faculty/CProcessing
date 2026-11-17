//---------------------------------------------------------
// file:	CP_Random.c
// author:	Justin Chambers
// brief:	Header used to manage the DLL export/import declarations
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology
//---------------------------------------------------------

#include "cprocessing.h"
#include "Internal_Random.h"
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

//------------------------------------------------------------------------------
// Defines and Internal Variables:
//------------------------------------------------------------------------------

// define USE_XORSHIFT to use the faster and more complete RNG
#define USE_XORSHIFT

#ifdef USE_XORSHIFT
static const uint32_t MAX_RAND_INT = UINT32_MAX;
// The state array must be initialized to not be all zero
static uint32_t state[4];
#else
static const uint32_t MAX_RAND_INT = RAND_MAX;
#endif

//------------------------------------------------------------------------------
// Internal Functions:
//------------------------------------------------------------------------------

#ifdef USE_XORSHIFT
static void xorshiftSeed(int seed)
{
	srand(seed);
	for (unsigned index = 0; index < 4; ++index)
	{
		// unsigned shift: RAND_MAX is 2^31-1 on glibc/macOS, so shifting the
		// signed int would overflow (MSVC's RAND_MAX is 32767, unaffected)
		state[index] = (uint32_t)rand() << 16;
		state[index] |= (uint32_t)rand();
	}
}

static uint32_t xorshift128(void)
{
	// Algorithm "xor128" from p. 5 of Marsaglia, "Xorshift RNGs"
	uint32_t s, t = state[3];
	t ^= t << 11;
	t ^= t >> 8;
	state[3] = state[2];
	state[2] = state[1];
	state[1] = s = state[0];
	t ^= s;
	t ^= s >> 19;
	state[0] = t;
	return t;
}
#endif

static inline uint32_t internalRand(void)
{
#ifdef USE_XORSHIFT
	return xorshift128();
#else
	return rand();
#endif
}

void CP_Random_Init(void)
{
	CP_Random_Seed((int)time(NULL));
}

//------------------------------------------------------------------------------
// Library Functions:
//------------------------------------------------------------------------------

CP_API CP_BOOL CP_Random_GetBool(void)
{
	return internalRand() & 0x01;
}

CP_API uint32_t CP_Random_GetInt(void)
{
	return internalRand();
}

CP_API uint32_t CP_Random_RangeInt(unsigned int lowerBound, unsigned int upperBound)
{
	if (upperBound < lowerBound)
	{
		uint32_t temp = lowerBound;
		lowerBound = upperBound;
		upperBound = temp;
	}

	// The full range of 2^32 values: (upper - lower) + 1 would wrap to 0
	const uint32_t range = upperBound - lowerBound;
	if (range == UINT32_MAX)
	{
		return internalRand();
	}

	// internalRand() % count favors the low numbers when count doesn't divide
	// 2^32 evenly. Throwing away the first 2^32 % count values makes it even.
	// That's under count values out of 2^32, so for ordinary ranges a draw is
	// almost never thrown away, and seeded sequences stay the same.
	const uint32_t count = range + 1;
	const uint32_t threshold = (0u - count) % count; // = 2^32 % count
	uint32_t r;
	do
	{
		r = internalRand();
	} while (r < threshold);
	return lowerBound + r % count;
}

CP_API float CP_Random_GetFloat(void)
{
	return (float)internalRand() / (float)(MAX_RAND_INT);
}

CP_API float CP_Random_RangeFloat(float lowerBound, float upperBound)
{
	if (upperBound < lowerBound)
	{
		float temp = lowerBound;
		lowerBound = upperBound;
		upperBound = temp;
	}
	return lowerBound + CP_Random_GetFloat() * (upperBound - lowerBound);
}

CP_API void CP_Random_Seed(int seed)
{
#ifdef USE_XORSHIFT
	xorshiftSeed(seed);
#else
	srand(seed);
#endif
}

//		Gaussian returns a normally distributed value where the mean is 0 and the standard deviation is 1.0
CP_API float CP_Random_Gaussian(void)
{
	double mean = 0;
	double stddev = 1.0;
	double U1, U2, W, mult;
	static double X1, X2;
	static int recompute = 1;

	if (recompute == 0)
	{
		recompute = !recompute;
		return (float)(mean + stddev * X2);
	}

	do
	{
		U1 = -1 + ((double)internalRand() / MAX_RAND_INT) * 2;
		U2 = -1 + ((double)internalRand() / MAX_RAND_INT) * 2;
		W = pow(U1, 2) + pow(U2, 2);
	} while (W >= 1 || W == 0);

	mult = sqrt((-2 * log(W)) / W);
	X1 = U1 * mult;
	X2 = U2 * mult;

	recompute = !recompute;

	return (float)(mean + stddev * X1);
}
