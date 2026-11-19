//------------------------------------------------------------------------------
// file:	CP_Math.c
// author:	Daniel Hamilton
// brief:	Math functions for use in the processing environment
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology and CProcessing contributors
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include Files:
//------------------------------------------------------------------------------

#define _USE_MATH_DEFINES
#include <math.h>
#include "cprocessing.h"
#include "nanovg.h"
#include "Internal_System.h"

//------------------------------------------------------------------------------
// Defines and Internal Variables:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Internal Functions:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Library Functions:
//------------------------------------------------------------------------------

CP_API int CP_Math_ClampInt(int value, int min, int max)
{
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

CP_API float CP_Math_ClampFloat(float value, float min, float max)
{
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

CP_API int CP_Math_LerpInt(int a, int b, float lerpFactor)
{
	lerpFactor = CP_Math_ClampFloat(lerpFactor, 0, 1.0f);
	// a + t * (b - a) gives a exactly when a and b are the same, and the
	// result is rounded to the nearest whole number (a cast would cut the
	// fraction off, always toward zero). Worked out in double so b - a can't
	// overflow.
	return (int)lround((double)a + (double)lerpFactor * ((double)b - (double)a));
}

CP_API float CP_Math_LerpFloat(float a, float b, float lerpFactor)
{
	lerpFactor = CP_Math_ClampFloat(lerpFactor, 0, 1.0f);
	return (1.0f - lerpFactor) * a + (lerpFactor)*b;
}

CP_API float CP_Math_Square(float value)
{
	return value * value;
}

CP_API float CP_Math_Distance(float x1, float y1, float x2, float y2)
{
	return sqrtf(CP_Math_Square(x2 - x1) + CP_Math_Square(y2 - y1));
}

CP_API float CP_Math_Degrees(float radians)
{
	return radians * (180.0f / (float)M_PI);
}

CP_API float CP_Math_Radians(float degrees)
{
	return degrees * ((float)M_PI / 180.0f);
}

CP_API float CP_Math_Map(float value, float inMin, float inMax, float outMin, float outMax)
{
	// An empty input range has no "how far along": use the start of the output
	if (inMax == inMin)
	{
		return outMin;
	}
	return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
}

CP_API void CP_Math_ScreenToWorld(float xIn, float yIn, float* xOut, float* yOut)
{
	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg)
	{
		return;
	}

	float worldToScreen[6] = { 0 };
	float screenToWorld[6] = { 0 };
	nvgCurrentTransform(CORE->nvg, worldToScreen);
	nvgTransformInverse(screenToWorld, worldToScreen);
	nvgTransformPoint(xOut, yOut, screenToWorld, xIn, yIn);
}

CP_API void CP_Math_WorldToScreen(float xIn, float yIn, float* xOut, float* yOut)
{
	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg)
	{
		return;
	}

	float t[6] = { 0 };
	nvgCurrentTransform(CORE->nvg, t);
	nvgTransformPoint(xOut, yOut, t, xIn, yIn);
}

//-------------------------------------
// Vector

CP_API CP_Vector CP_Vector_Zero(void) { return (CP_Vector){ 0, 0 }; }
CP_API CP_Vector CP_Vector_Set(float x, float y) { return (CP_Vector){ x, y }; }
CP_API CP_Vector CP_Vector_Negate(CP_Vector vec) { return (CP_Vector){ -vec.x, -vec.y }; }
CP_API CP_Vector CP_Vector_Add(CP_Vector a, CP_Vector b) { return (CP_Vector){ a.x + b.x, a.y + b.y }; }
CP_API CP_Vector CP_Vector_Subtract(CP_Vector a, CP_Vector b) { return (CP_Vector){ a.x - b.x, a.y - b.y }; }
CP_API CP_Vector CP_Vector_Scale(CP_Vector vec, float scale) { return (CP_Vector){ vec.x * scale, vec.y * scale }; }

CP_API CP_Vector CP_Vector_Normalize(CP_Vector vec)
{
	if (vec.x == 0 && vec.y == 0)
	{
		return CP_Vector_Zero();
	}
	float hyp = sqrtf((vec.x * vec.x) + (vec.y * vec.y));
	return (CP_Vector){ vec.x / hyp, vec.y / hyp };
}

CP_API CP_Vector CP_Vector_Lerp(CP_Vector a, CP_Vector b, float lerpFactor)
{
	return (CP_Vector){
		CP_Math_LerpFloat(a.x, b.x, lerpFactor),
		CP_Math_LerpFloat(a.y, b.y, lerpFactor)
	};
}

CP_API CP_Vector CP_Vector_Rotate(CP_Vector vec, float degrees)
{
	// The same turn as CP_Matrix_Rotate and CP_Settings_Rotate: with y
	// pointing down, positive degrees turn clockwise on the screen
	float radians = CP_Math_Radians(degrees);
	float c = cosf(radians);
	float s = sinf(radians);
	return (CP_Vector){ vec.x * c - vec.y * s, vec.x * s + vec.y * c };
}

CP_API CP_Vector CP_Vector_FromAngle(float degrees)
{
	// 0 degrees points right, 90 points down the screen
	float radians = CP_Math_Radians(degrees);
	return (CP_Vector){ cosf(radians), sinf(radians) };
}

CP_API CP_Vector CP_Vector_Limit(CP_Vector vec, float maxLength)
{
	float length = CP_Vector_Length(vec);
	if (maxLength <= 0.0f)
	{
		return CP_Vector_Zero();
	}
	if (length <= maxLength)
	{
		return vec;
	}
	return CP_Vector_Scale(vec, maxLength / length);
}

CP_API CP_Vector CP_Vector_MatrixMultiply(CP_Matrix mat, CP_Vector vec)
{
	// this is actually transforming a Point because it assumes the third component is 1
	CP_Vector result = CP_Vector_Zero();
	result.x = mat.m00 * vec.x + mat.m01 * vec.y + mat.m02;
	result.y = mat.m10 * vec.x + mat.m11 * vec.y + mat.m12;
	return result;
}

CP_API float CP_Vector_Length(CP_Vector vec) { return sqrtf((vec.x * vec.x) + (vec.y * vec.y)); }
CP_API float CP_Vector_Distance(CP_Vector a, CP_Vector b) { return sqrtf(CP_Math_Square(b.x - a.x) + CP_Math_Square(b.y - a.y)); }
CP_API float CP_Vector_DotProduct(CP_Vector a, CP_Vector b) { return (a.x * b.x) + (a.y * b.y); }
CP_API float CP_Vector_CrossProduct(CP_Vector a, CP_Vector b) { return (a.x * b.y) - (a.y * b.x); }

// The angle from one vector to the other, in degrees from -180 to 180,
// positive for a clockwise turn on the screen (y points down). atan2 of the
// cross and dot products is exact for vectors that point the same way or
// opposite ways. (The arc cosine of the dot product over the lengths, used
// before, could get a value just over 1 for those and return NaN.) A zero
// vector gives 0.
static float signedAngle(CP_Vector from, CP_Vector to)
{
	return CP_Math_Degrees((float)atan2((double)CP_Vector_CrossProduct(from, to), (double)CP_Vector_DotProduct(from, to)));
}

CP_API float CP_Vector_Angle(CP_Vector a, CP_Vector b)
{
	return fabsf(signedAngle(a, b));
}

CP_API float CP_Vector_AngleCW(CP_Vector from, CP_Vector to)
{
	float angle = signedAngle(from, to);
	if (angle < 0.0f)
	{
		angle += 360.0f;
	}
	// A tiny negative angle plus 360 can round to 360, which is 0 again; and
	// -0 becomes 0
	return (angle >= 360.0f || angle == 0.0f) ? 0.0f : angle;
}

CP_API float CP_Vector_AngleCCW(CP_Vector from, CP_Vector to)
{
	float ang = CP_Vector_AngleCW(from, to);
	return ang == 0 ? ang : ang - 360.0f;
}

//-------------------------------------
// Matrix

CP_API CP_Matrix CP_Matrix_Set(
	float m00, float m01, float m02,
	float m10, float m11, float m12,
	float m20, float m21, float m22)
{
	return (CP_Matrix){
		m00, m01, m02,
		m10, m11, m12,
		m20, m21, m22
	};
}

CP_API CP_Matrix CP_Matrix_Identity(void)
{
	return (CP_Matrix){
		1, 0, 0,
		0, 1, 0,
		0, 0, 1
	};
}

CP_API CP_Matrix CP_Matrix_FromVector(CP_Vector col1, CP_Vector col2, CP_Vector col3)
{
	return (CP_Matrix){
		col1.x,
		col2.x,
		col3.x,
		col1.y,
		col2.y,
		col3.y,
		0,
		0,
		1.0f,
	};
}

CP_API CP_Matrix CP_Matrix_Scale(CP_Vector scale)
{
	return (CP_Matrix){
		scale.x, 0, 0,
		0, scale.y, 0,
		0, 0, 1.0f
	};
}

CP_API CP_Matrix CP_Matrix_Translate(CP_Vector offset)
{
	return (CP_Matrix){
		1.0f, 0, offset.x,
		0, 1.0f, offset.y,
		0, 0, 1.0f
	};
}

CP_API CP_Matrix CP_Matrix_Rotate(float degrees)
{
	return CP_Matrix_RotateRadians(CP_Math_Radians(degrees));
}

CP_API CP_Matrix CP_Matrix_RotateRadians(float radians)
{
	float c = (float)cos((double)radians);
	float s = (float)sin((double)radians);
	return (CP_Matrix){
		c, -s, 0,
		s, c, 0,
		0, 0, 1.0f
	};
}

// The functions below loop over rows and columns, so they work on a 3 x 3
// array of the matrix's named members
static void matrixToArray(CP_Matrix matrix, float out[3][3])
{
	out[0][0] = matrix.m00;
	out[0][1] = matrix.m01;
	out[0][2] = matrix.m02;
	out[1][0] = matrix.m10;
	out[1][1] = matrix.m11;
	out[1][2] = matrix.m12;
	out[2][0] = matrix.m20;
	out[2][1] = matrix.m21;
	out[2][2] = matrix.m22;
}

static CP_Matrix matrixFromArray(const float in[3][3])
{
	return (CP_Matrix){
		in[0][0], in[0][1], in[0][2],
		in[1][0], in[1][1], in[1][2],
		in[2][0], in[2][1], in[2][2]
	};
}

CP_API CP_Matrix CP_Matrix_Transpose(CP_Matrix original)
{
	float source[3][3];
	float result[3][3];
	unsigned i, j;
	matrixToArray(original, source);
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			result[j][i] = source[i][j];
		}
	}

	return matrixFromArray(result);
}

CP_API CP_Matrix CP_Matrix_Inverse(CP_Matrix original)
{
	float m[3][3];
	float result[3][3];
	unsigned i, j;
	float determinant = 0;
	matrixToArray(original, m);

	// finding determinant
	for (i = 0; i < 3; i++)
	{
		determinant += (m[0][i] * (m[1][(i + 1) % 3] * m[2][(i + 2) % 3] - m[1][(i + 2) % 3] * m[2][(i + 1) % 3]));
	}

	// A matrix that flattens space (a scale of 0, say) has no inverse:
	// return the identity rather than dividing by 0 and filling the result
	// with infinities and NaN
	if (determinant == 0.0f || !isfinite(1.0f / determinant))
	{
		return CP_Matrix_Identity();
	}

	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			result[i][j] = ((m[(j + 1) % 3][(i + 1) % 3] * m[(j + 2) % 3][(i + 2) % 3]) - (m[(j + 1) % 3][(i + 2) % 3] * m[(j + 2) % 3][(i + 1) % 3])) / determinant;
		}
	}
	return matrixFromArray(result);
}

CP_API CP_Matrix CP_Matrix_Multiply(CP_Matrix a, CP_Matrix b)
{
	float left[3][3];
	float right[3][3];
	float result[3][3];
	unsigned i, j, k;
	matrixToArray(a, left);
	matrixToArray(b, right);
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			float current_value = 0.0f;

			for (k = 0; k < 3; k++)
			{
				current_value += left[i][k] * right[k][j];
			}

			result[i][j] = current_value;
		}
	}
	return matrixFromArray(result);
}
