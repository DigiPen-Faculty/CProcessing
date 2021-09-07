//---------------------------------------------------------
// file:	CP_Noise.c
// author:	Justin Chambers
// brief:	Noise implementation based on Ken Perlin's work with some improvements
//
// Copyright © 2019 DigiPen, All rights reserved.
//---------------------------------------------------------

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "cprocessing.h"
#include "Internal_Noise.h"

#define permutationArraySize 256
int repeatNoise = -1;

int p[permutationArraySize * 2];  // Doubled permutation to avoid overflow

//double OctavePerlin(double x, double y, double z, int octaves, double persistence)
//{
//	double total = 0;
//	double frequency = 1;
//	double amplitude = 1;
//	double maxValue = 0;			// Used for normalizing result to 0.0 - 1.0
//	for (int i = 0; i < octaves; i++) {
//		total += noise(x * frequency, y * frequency, z * frequency) * amplitude;
//
//		maxValue += amplitude;
//
//		amplitude *= persistence;
//		frequency *= 2;
//	}
//
//	return total / maxValue;
//}													

// Sets the seed value for noise().
// By default, noise() produces different results each time the program is run.
// Set the value parameter to a constant to return the same pseudo - random numbers each time the software is run.
CP_API void CP_Random_NoiseSeed(int seed)
{
	srand(seed);
	int permutation[permutationArraySize];
	memset(permutation, -1, sizeof(permutation));
	for (int val = 0; val < permutationArraySize; ++val)
	{
		int steps = -1;
		int randSteps = rand() % (permutationArraySize - val);
		int index = 0;
		for (; index < permutationArraySize; ++index)
		{
			if (permutation[index] < 0)
			{
				++steps;
			}

			if (steps >= randSteps)
			{
				break;
			}
		}
		permutation[index] = val;
	}

	memset(p, -1, sizeof(p));
	for (int x = 0; x < permutationArraySize * 2; x++) {
		p[x] = permutation[x % 256];				// Doubled permutation to avoid overflow
	}
}

void CP_NoiseInit(void)
{
  CP_Random_NoiseSeed((int)time(NULL));
}

static int inc(int num)
{
	num++;
	if (repeatNoise > 0) num %= repeatNoise;

	return num;
}

static double grad(int hash, double x, double y, double z)
{
	switch (hash & 0xF)
	{
		case 0x0: return  x + y;
		case 0x1: return -x + y;
		case 0x2: return  x - y;
		case 0x3: return -x - y;
		case 0x4: return  x + z;
		case 0x5: return -x + z;
		case 0x6: return  x - z;
		case 0x7: return -x - z;
		case 0x8: return  y + z;
		case 0x9: return -y + z;
		case 0xA: return  y - z;
		case 0xB: return -y - z;
		case 0xC: return  y + x;
		case 0xD: return -y + z;
		case 0xE: return  y - x;
		case 0xF: return -y - z;
		default: return 0; // never happens
	}
}

static double fade(double t)
{
	// Fade function as defined by Ken Perlin.  This eases coordinate values
	// so that they will "ease" towards integral values.  This ends up smoothing
	// the final output.
	return t * t * t * (t * (t * 6 - 15) + 10);		// 6t^5 - 15t^4 + 10t^3
}

static double lerp(double a, double b, double x)
{
	return a + x * (b - a);
}

//		Noise returns a value in the range [0, 1.0] based on three dimensional input values
CP_API float CP_Random_Noise(float x, float y, float z)
{
	double xD = (double)x;
	double yD = (double)y;
	double zD = (double)z;

	if (repeatNoise > 0) {						// If we have any repeatNoise on, change the coordinates to their "local" repetitions
		xD = fmod(xD, repeatNoise);
		yD = fmod(yD, repeatNoise);
		zD = fmod(zD, repeatNoise);
	}

	int xi = (int)xD & 255;						// Calculate the "unit cube" that the point asked will be located in
	int yi = (int)yD & 255;						// The left bound is ( |_x_|,|_y_|,|_z_| ) and the right bound is that
	int zi = (int)zD & 255;						// plus 1.  Next we calculate the location (from 0.0 to 1.0) in that cube.
	double xf = xD - (int)xD;					// We also fade the location to smooth the result.
	double yf = yD - (int)yD;
	double zf = zD - (int)zD;
	double u = fade(xf);
	double v = fade(yf);
	double w = fade(zf);

	int aaa, aba, aab, abb, baa, bba, bab, bbb;
	aaa = p[p[p[xi] + yi] + zi];
	aba = p[p[p[xi] + inc(yi)] + zi];
	aab = p[p[p[xi] + yi] + inc(zi)];
	abb = p[p[p[xi] + inc(yi)] + inc(zi)];
	baa = p[p[p[inc(xi)] + yi] + zi];
	bba = p[p[p[inc(xi)] + inc(yi)] + zi];
	bab = p[p[p[inc(xi)] + yi] + inc(zi)];
	bbb = p[p[p[inc(xi)] + inc(yi)] + inc(zi)];

	double x1, x2, y1, y2;
	x1 = lerp(grad(aaa, xf, yf, zf),			// The gradient function calculates the dot product between a pseudorandom
		grad(baa, xf - 1, yf, zf),				// gradient vector and the vector from the input coordinate to the 8
		u);										// surrounding points in its unit cube.
	x2 = lerp(grad(aba, xf, yf - 1, zf),		// This is all then lerped together as a sort of weighted average based on the faded (u,v,w)
		grad(bba, xf - 1, yf - 1, zf),			// values we made earlier.
		u);
	y1 = lerp(x1, x2, v);

	x1 = lerp(grad(aab, xf, yf, zf - 1),
		grad(bab, xf - 1, yf, zf - 1),
		u);
	x2 = lerp(grad(abb, xf, yf - 1, zf - 1),
		grad(bbb, xf - 1, yf - 1, zf - 1),
		u);
	y2 = lerp(x1, x2, v);

	return (float)((lerp(y1, y2, w) + 1) / 2);	// For convenience we bound it to 0 - 1 (theoretical min/max before is -1 - 1)
}
