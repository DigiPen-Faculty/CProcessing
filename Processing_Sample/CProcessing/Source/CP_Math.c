//------------------------------------------------------------------------------
// file:	CP_Math.c
// author:	Daniel Hamilton
// brief:	Math functions for use in the processing environment
//
// Copyright © 2019 DigiPen, All rights reserved.
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
	return (int)((1.0f - lerpFactor) * a + (lerpFactor)*b);
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

CP_API CP_Vector CP_Vector_Zero(void) { return (CP_Vector) { 0, 0 }; }
CP_API CP_Vector CP_Vector_Set(float x, float y) { return (CP_Vector) { x, y }; }
CP_API CP_Vector CP_Vector_Negate(CP_Vector vec) { return (CP_Vector) { -vec.x, -vec.y }; }
CP_API CP_Vector CP_Vector_Add(CP_Vector a, CP_Vector b) { return (CP_Vector) { a.x + b.x, a.y + b.y }; }
CP_API CP_Vector CP_Vector_Subtract(CP_Vector a, CP_Vector b) { return (CP_Vector) { a.x - b.x, a.y - b.y }; }
CP_API CP_Vector CP_Vector_Scale(CP_Vector vec, float scale) { return (CP_Vector) { vec.x* scale, vec.y* scale }; }

CP_API CP_Vector CP_Vector_Normalize(CP_Vector vec)
{
	if (vec.x == 0 && vec.y == 0)
	{
		return CP_Vector_Zero();
	}
	float hyp = sqrtf((vec.x * vec.x) + (vec.y * vec.y));
	return (CP_Vector) { vec.x / hyp, vec.y / hyp };
}

CP_API CP_Vector CP_Vector_MatrixMultiply(CP_Matrix m, CP_Vector v)
{
	// this is actually transforming a Point because it assumes the third component is 1
	CP_Vector result = CP_Vector_Zero();
	result.x = m.m00 * v.x + m.m01 * v.y + m.m02;
	result.y = m.m10 * v.x + m.m11 * v.y + m.m12;
	return result;
}

CP_API float CP_Vector_Length(CP_Vector vec) { return sqrtf((vec.x * vec.x) + (vec.y * vec.y)); }
CP_API float CP_Vector_Distance(CP_Vector a, CP_Vector b) { return sqrtf(CP_Math_Square(b.x - a.x) + CP_Math_Square(b.y - a.y)); }
CP_API float CP_Vector_DotProduct(CP_Vector a, CP_Vector b) { return (a.x * b.x) + (a.y * b.y); }
CP_API float CP_Vector_CrossProduct(CP_Vector a, CP_Vector b) { return (a.x * b.y) - (a.y * b.x); }

CP_API float CP_Vector_Angle(CP_Vector a, CP_Vector b)
{
	return CP_Math_Degrees((float)acos((double)(CP_Vector_DotProduct(a, b) / (CP_Vector_Length(a) * CP_Vector_Length(b)))));
}

CP_API float CP_Vector_AngleCW(CP_Vector from, CP_Vector to)
{
	// update right vectors
	CP_Vector toRight = CP_Vector_Set(-to.y, to.x);

	float ang = CP_Vector_Angle(from, to);
	if (CP_Vector_DotProduct(from, toRight) <= 0)
	{
		return ang;
	}
	else
	{
		return 360.0f - ang;
	}
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
	return (CP_Matrix) {
		m00, m01, m02,
		m10, m11, m12,
		m20, m21, m22
	};
}

CP_API CP_Matrix CP_Matrix_Identity(void)
{
	return (CP_Matrix) {
		1, 0, 0,
		0, 1, 0,
		0, 0, 1
	};
}

CP_API CP_Matrix CP_Matrix_FromVector(CP_Vector col1, CP_Vector col2, CP_Vector col3)
{
	return (CP_Matrix) {
		col1.x, col2.x, col3.x,
		col1.y, col2.y, col3.y,
		0, 0, 1.0f,
	};
}

CP_API CP_Matrix CP_Matrix_Scale(CP_Vector scale)
{
	return (CP_Matrix) {
		scale.x, 0, 0,
		0, scale.y, 0,
		0, 0, 1.0f
	};
}

CP_API CP_Matrix CP_Matrix_Translate(CP_Vector offset)
{
	return (CP_Matrix) {
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
	return (CP_Matrix) {
		c, -s, 0,
		s, c, 0,
		0, 0, 1.0f
	};
}

CP_API CP_Matrix CP_Matrix_Transpose(CP_Matrix original)
{
	CP_Matrix result = CP_Matrix_Identity();
	unsigned i, j;
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			result.m[j][i] = original.m[i][j];
		}
	}

	return result;
}

CP_API CP_Matrix CP_Matrix_Inverse(CP_Matrix m)
{
	CP_Matrix result;
	unsigned i, j;
	float determinant = 0;

	//finding determinant
	for (i = 0; i < 3; i++)
	{
		determinant += (m.m[0][i] * (m.m[1][(i + 1) % 3] * m.m[2][(i + 2) % 3] - m.m[1][(i + 2) % 3] * m.m[2][(i + 1) % 3]));
	}

	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			result.m[i][j] = ((m.m[(j + 1) % 3][(i + 1) % 3] * m.m[(j + 2) % 3][(i + 2) % 3]) - (m.m[(j + 1) % 3][(i + 2) % 3] * m.m[(j + 2) % 3][(i + 1) % 3])) / determinant;
		}
	}
	return result;
}

CP_API CP_Matrix CP_Matrix_Multiply(CP_Matrix a, CP_Matrix b)
{
	CP_Matrix result = CP_Matrix_Identity();
	unsigned i, j, k;
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			float current_value = 0.0f;

			for (k = 0; k < 3; k++)
			{
				current_value += a.m[i][k] * b.m[k][j];
			}

			result.m[i][j] = current_value;
		}
	}
	return result;
}
