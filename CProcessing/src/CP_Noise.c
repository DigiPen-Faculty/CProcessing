//---------------------------------------------------------
// file:	CP_Noise.c
// author:	Justin Chambers
// brief:	Noise implementation based on Ken Perlin's work with some improvements
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include <stdint.h>
#include <time.h>
#include <math.h>
#include "cprocessing.h"
#include "Internal_Noise.h"
#include "Internal_Random.h"

#define permutationArraySize 256

static int p[permutationArraySize * 2]; // Doubled permutation to avoid overflow

// Perlin noise is 0 at every whole-number point, so CP_Random_Noise(x, 0, 0)
// at whole numbers was always 0.5, and with y and z left at 0 it only spanned
// 0.25 to 0.75. Moving every point by a fixed amount that isn't a whole number
// (or a half) keeps the points students pick off that grid.
#define NOISE_OFFSET_X 0.3711
#define NOISE_OFFSET_Y 0.6173
#define NOISE_OFFSET_Z 0.1237

// The raw noise is in -1 to 1 but rarely goes beyond +-0.7 (measured: about
// 0.3% of values in 1D, 2D or 3D). Dividing by this before moving it to 0..1
// uses most of 0 to 1; the rare values beyond it are clamped.
#define NOISE_SPREAD 0.7

// Sets the seed value for noise().
// By default, noise() produces different results each time the program is run.
// Set the value parameter to a constant to return the same pseudo - random numbers each time the software is run.
CP_API void CP_Random_NoiseSeed(int seed)
{
	// A shuffle of 0 to 255 (Fisher-Yates), drawn from a SplitMix64 stream of
	// its own. It doesn't use the C runtime's srand and rand (whose numbers
	// differ between compilers and operating systems, and which a program may
	// be using for itself) and it doesn't touch CP_Random's generator, so a
	// seed gives the same noise on every platform.
	uint64_t stream = (uint64_t)(uint32_t)seed ^ 0x6E6F697365ull;
	int permutation[permutationArraySize];
	for (int val = 0; val < permutationArraySize; ++val)
	{
		permutation[val] = val;
	}
	for (int last = permutationArraySize - 1; last > 0; --last)
	{
		// A number from 0 to last, every one equally likely (no modulo bias)
		const uint32_t count = (uint32_t)last + 1;
		const uint32_t threshold = (0u - count) % count;
		uint32_t r;
		do
		{
			r = (uint32_t)(CP_Random_SplitMix64(&stream) >> 32);
		} while (r < threshold);
		const int pick = (int)(r % count);

		const int temp = permutation[last];
		permutation[last] = permutation[pick];
		permutation[pick] = temp;
	}

	for (int x = 0; x < permutationArraySize * 2; x++)
	{
		p[x] = permutation[x % 256]; // Doubled permutation to avoid overflow
	}
}

void CP_NoiseInit(void)
{
	CP_Random_NoiseSeed((int)time(NULL));
}

static double grad(int hash, double x, double y, double z)
{
	switch (hash & 0xF)
	{
	case 0x0: return x + y;
	case 0x1: return -x + y;
	case 0x2: return x - y;
	case 0x3: return -x - y;
	case 0x4: return x + z;
	case 0x5: return -x + z;
	case 0x6: return x - z;
	case 0x7: return -x - z;
	case 0x8: return y + z;
	case 0x9: return -y + z;
	case 0xA: return y - z;
	case 0xB: return -y - z;
	case 0xC: return y + x;
	case 0xD: return -y + z;
	case 0xE: return y - x;
	case 0xF: return -y - z;
	default:  return 0; // never happens
	}
}

static double fade(double t)
{
	// Fade function as defined by Ken Perlin.  This eases coordinate values
	// so that they will "ease" towards integral values.  This ends up smoothing
	// the final output.
	return t * t * t * (t * (t * 6 - 15) + 10); // 6t^5 - 15t^4 + 10t^3
}

static double lerp(double a, double b, double x)
{
	return a + x * (b - a);
}

// Ken Perlin's improved noise at a point, from -1 to 1
static double rawNoise(double xD, double yD, double zD)
{
	xD += NOISE_OFFSET_X;
	yD += NOISE_OFFSET_Y;
	zD += NOISE_OFFSET_Z;

	// Round down, not toward zero: (int)-0.5 is 0, which put negative
	// coordinates in the wrong unit cube and made the noise jump at -1, -2, ...
	const double xFloor = floor(xD);
	const double yFloor = floor(yD);
	const double zFloor = floor(zD);

	int xi = (int)xFloor & 255; // Calculate the "unit cube" that the point asked will be located in
	int yi = (int)yFloor & 255; // The left bound is ( |_x_|,|_y_|,|_z_| ) and the right bound is that
	int zi = (int)zFloor & 255; // plus 1.  Next we calculate the location (from 0.0 to 1.0) in that cube.
	double xf = xD - xFloor;    // We also fade the location to smooth the result.
	double yf = yD - yFloor;
	double zf = zD - zFloor;
	double u = fade(xf);
	double v = fade(yf);
	double w = fade(zf);

	int aaa, aba, aab, abb, baa, bba, bab, bbb;
	aaa = p[p[p[xi] + yi] + zi];
	aba = p[p[p[xi] + (yi + 1)] + zi];
	aab = p[p[p[xi] + yi] + (zi + 1)];
	abb = p[p[p[xi] + (yi + 1)] + (zi + 1)];
	baa = p[p[p[(xi + 1)] + yi] + zi];
	bba = p[p[p[(xi + 1)] + (yi + 1)] + zi];
	bab = p[p[p[(xi + 1)] + yi] + (zi + 1)];
	bbb = p[p[p[(xi + 1)] + (yi + 1)] + (zi + 1)];

	double x1, x2, y1, y2;
	x1 = lerp(grad(aaa, xf, yf, zf),     // The gradient function calculates the dot product between a pseudorandom
		grad(baa, xf - 1, yf, zf),       // gradient vector and the vector from the input coordinate to the 8
		u);                              // surrounding points in its unit cube.
	x2 = lerp(grad(aba, xf, yf - 1, zf), // This is all then lerped together as a sort of weighted average based on the faded (u,v,w)
		grad(bba, xf - 1, yf - 1, zf),   // values we made earlier.
		u);
	y1 = lerp(x1, x2, v);

	x1 = lerp(grad(aab, xf, yf, zf - 1),
		grad(bab, xf - 1, yf, zf - 1),
		u);
	x2 = lerp(grad(abb, xf, yf - 1, zf - 1),
		grad(bbb, xf - 1, yf - 1, zf - 1),
		u);
	y2 = lerp(x1, x2, v);

	return lerp(y1, y2, w);
}

// Raw noise to 0..1 (see NOISE_SPREAD)
static float noiseToUnit(double raw)
{
	const double value = 0.5 + 0.5 * raw / NOISE_SPREAD;
	if (value < 0.0)
	{
		return 0.0f;
	}
	if (value > 1.0)
	{
		return 1.0f;
	}
	return (float)value;
}

//		Noise returns a value in the range [0, 1.0] based on three dimensional input values
CP_API float CP_Random_Noise(float x, float y, float z)
{
	return noiseToUnit(rawNoise((double)x, (double)y, (double)z));
}

// Several layers of noise added together, each at twice the detail of the
// one before and persistence times as strong. Returns a value in [0, 1].
CP_API float CP_Random_NoiseOctaves(float x, float y, float z, int octaves, float persistence)
{
	if (octaves < 1)
	{
		octaves = 1;
	}

	// Dividing the sum by the sum of the strengths, as before, pulled the result
	// toward 0.5 with every layer (4 layers spanned only about 0.23 to 0.77).
	// The layers are about independent, so dividing by the square root of the
	// sum of the squared strengths keeps the spread of a single layer.
	double total = 0.0;
	double sumOfSquares = 0.0;
	float frequency = 1.0f;
	double amplitude = 1.0;
	for (int i = 0; i < octaves; ++i)
	{
		total += rawNoise((double)(x * frequency), (double)(y * frequency), (double)(z * frequency)) * amplitude;
		sumOfSquares += amplitude * amplitude;
		amplitude *= persistence;
		frequency *= 2.0f;
	}
	return noiseToUnit(total / sqrt(sumOfSquares));
}
