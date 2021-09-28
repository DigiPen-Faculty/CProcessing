//---------------------------------------------------------
// file:	CP_Random.c
// author:	Justin Chambers
// brief:	Header used to manage the DLL export/import declarations
//
// Copyright © 2019 DigiPen, All rights reserved.
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
uint32_t state[4];
#else
static const uint32_t MAX_RAND_INT = RAND_MAX;
#endif

//------------------------------------------------------------------------------
// Internal Functions:
//------------------------------------------------------------------------------

#ifdef USE_XORSHIFT
void xorshiftSeed(int seed)
{
	srand(seed);
	for (unsigned index = 0; index < 4; ++index)
	{
		state[index] = rand() << 16;
		state[index] |= rand();
	}
}

uint32_t xorshift128(void)
{
	// Algorithm "xor128" from p. 5 of Marsaglia, "Xorshift RNGs"
	uint32_t s, t = state[3];
	t ^= t << 11;
	t ^= t >> 8;
	state[3] = state[2]; state[2] = state[1]; state[1] = s = state[0];
	t ^= s;
	t ^= s >> 19;
	state[0] = t;
	return t;
}
#endif

inline uint32_t internalRand(void)
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
	return lowerBound + internalRand() % ((upperBound - lowerBound) + 1);
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


// --------------------------
// TODO:  Move these tests to a test framework 

/*

void perfTest_RAND()
{
	int val = 0;
	for (int i = 0; i < 100000000; ++i)
	{
		val = rand();
	}
}

void perfTest_SHIFT()
{
	int val = 0;
	for (int i = 0; i < 100000000; ++i)
	{
		val = xorshift128();
	}
}

#define STRESS_INT 0
#define STRESS_FLOAT 0

CP_API void test_random(void)
{
	perfTest_RAND();
	perfTest_SHIFT();
	
	LONGLONG counter = 0;
	uint32_t itest = randomInt();
	while (STRESS_INT)
	{
		++counter;
		itest = randomInt();
		if (itest == MAX_RAND_INT)
		{
			//DebugBreak();
		}
		if (itest == 0)
		{
			//DebugBreak();
		}
	}
	itest = randomRangeInt(0x0400, 0x0500);
	itest = randomRangeInt(0x0300, 0x02A0);
	itest = randomRangeInt(MAX_RAND_INT, MAX_RAND_INT);
	itest = randomRangeInt(0, 1);
	itest = randomRangeInt(MAX_RAND_INT - 1, MAX_RAND_INT);

	int randTest[5] = { 0,0,0,0,0 };
	for (int i = 0; i < 1000000; ++i)
	{
		// confirm results never step past the upper bound
		++randTest[randomRangeInt(0, 3)];
	}
	memset(&randTest, 0, sizeof(randTest));
	for (int i = 0; i < 1000000; ++i)
	{
		// smallest variation, modulus 2
		++randTest[randomRangeInt(0, 1)];
	}
	memset(&randTest, 0, sizeof(randTest));
	for (int i = 0; i < 1000000; ++i)
	{
		// smallest variation at the top of the range (uint roll-over to place in lower index)
		++randTest[randomRangeInt(MAX_RAND_INT - 1, MAX_RAND_INT) + 0x02u];
	}

	double ftest = randomFloat();
	ftest = randomFloat();
	ftest = randomRangeFloat(1000.0f, 1100.0f);
	ftest = randomRangeFloat(200.0f, 100.0f);
	ftest = randomRangeFloat(300.0f, 300.0f);

	double fmin = 2.0;
	double fmax = 0;
	double faccum = 0; // min, max, accumulation

	counter = 0;
	while (STRESS_FLOAT)
	{
		++counter;
		ftest = randomFloat();
		if (ftest == 1.0)
		{
			//DebugBreak();
		}
		if (ftest == 0)
		{
			//DebugBreak();
		}
	}

	for (int i = 0; i < 1000000000; ++i)
	{
		ftest = randomFloat();
		fmin = min(fmin, ftest);
		fmax = max(fmax, ftest);
		faccum += ftest;
	}
	double faverage = faccum / 1000000000;
	faverage = faverage;
}
*/

