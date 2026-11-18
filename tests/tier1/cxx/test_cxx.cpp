//---------------------------------------------------------
// CProcessing
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology and CProcessing contributors
//
// https://github.com/DigiPen-Faculty/CProcessing
//---------------------------------------------------------
//
// A program written in C++ that uses CProcessing, to check that the public
// headers work from C++ as well as C. CMake builds it with strict warnings
// (-Wpedantic, /permissive-) as C++11, and runs it with the Tier 1 tests.
//
// Building it is most of the test: the headers have to compile as C++, and
// every function used has to link, which needs the extern "C" in the
// headers. Running it checks a few results, using only functions that need
// no window.

#include "cprocessing.h"

#include <cstdio>

static int failures = 0;

static void Check(bool ok, const char* what)
{
	if (!ok)
	{
		std::printf("FAILED: %s\n", what);
		++failures;
	}
}

static bool Near(float a, float b)
{
	return a - b < 0.0001f && b - a < 0.0001f;
}

// The unions keep the layout C programs rely on
static_assert(sizeof(CP_Color) == 4, "CP_Color is 4 bytes");
static_assert(sizeof(CP_Vector) == 2 * sizeof(float), "CP_Vector is two floats");
static_assert(sizeof(CP_Matrix) == 9 * sizeof(float), "CP_Matrix is nine floats");

// The game state names are constants of an unsigned int type, so | gives a
// CP_STATE in C++ too (with an enum type it would give an int, which C++
// won't pass as the enum)
static_assert(CP_STATE_PASSIVE == (CP_STATE_UPDATING | CP_STATE_VISIBLE), "PASSIVE is both bits");
static_assert(CP_STATE_FROZEN == CP_STATE_VISIBLE, "FROZEN is VISIBLE");

int main(int argc, char** argv)
{
	(void)argv;

	// Game states: combining the names, and functions given as pointers
	CP_STATE passThrough = CP_STATE_UPDATING | CP_STATE_VISIBLE;
	Check(passThrough == CP_STATE_PASSIVE, "CP_STATE_UPDATING | CP_STATE_VISIBLE is CP_STATE_PASSIVE");
	passThrough &= ~static_cast<CP_STATE>(CP_STATE_UPDATING);
	Check(passThrough == CP_STATE_FROZEN, "taking UPDATING away from PASSIVE leaves FROZEN");

	// A lambda with no captures can be a state's function
	CP_FunctionPtr update = [] {};
	Check(update != nullptr, "a lambda converts to CProcessing's function pointer");

	// Each of these has to link, with the signature the header gives
	void (*push)(CP_FunctionPtr, CP_FunctionPtr, CP_FunctionPtr) = &CP_Engine_PushGameState;
	void (*setPassThrough)(CP_STATE) = &CP_Engine_SetGameStatePassThrough;
	void (*setPostUpdate)(CP_FunctionPtr) = &CP_Engine_SetPostUpdateFunction;
	Check(push != nullptr && setPassThrough != nullptr && setPostUpdate != nullptr, "engine functions link");

	// The pixel functions take const data, so a const array can be passed.
	// (Not called: they need a window. argc is never this big.)
	const CP_Color pixels[4] = {};
	if (argc > 1000)
	{
		CP_Image_UpdatePixelData(nullptr, pixels);
		CP_Image_CreateFromData(2, 2, pixels);
	}

	// Colors
	const CP_Color color = CP_Color_Create(10, 20, 30, 40);
	Check(color.r == 10 && color.g == 20 && color.b == 30 && color.a == 40, "CP_Color_Create fills r, g, b, a");
	Check(sizeof(CP_Color) == 4 && sizeof(CP_Vector) == 8 && sizeof(CP_Matrix) == 36, "colors, vectors and matrices have no padding");
	const CP_Color hex = CP_Color_CreateHex(0x204080FF);
	Check(hex.r == 0x20 && hex.g == 0x40 && hex.b == 0x80 && hex.a == 0xFF, "CP_Color_CreateHex");
	const CP_ColorHSL hsl = CP_ColorHSL_FromColor(CP_Color_Create(255, 0, 0, 255));
	Check(hsl.h == 0 && hsl.s == 100 && hsl.l == 50 && hsl.a == 255, "red in HSL");

	// Vectors and matrices
	const CP_Vector a = CP_Vector_Set(3.0f, 4.0f);
	Check(Near(CP_Vector_Length(a), 5.0f), "CP_Vector_Length");
	const CP_Vector sum = CP_Vector_Add(a, CP_Vector_Scale(a, 2.0f));
	Check(Near(sum.x, 9.0f) && Near(sum.y, 12.0f), "CP_Vector_Add and CP_Vector_Scale");
	const CP_Matrix identity = CP_Matrix_Identity();
	Check(Near(identity.m00, 1.0f) && Near(identity.m11, 1.0f) && Near(identity.m01, 0.0f), "CP_Matrix_Identity");
	const CP_Vector moved = CP_Vector_MatrixMultiply(CP_Matrix_Translate(CP_Vector_Set(1.0f, 2.0f)), a);
	Check(Near(moved.x, 4.0f) && Near(moved.y, 6.0f), "CP_Matrix_Translate");

	// Math
	Check(CP_Math_ClampInt(15, 0, 10) == 10, "CP_Math_ClampInt");
	Check(Near(CP_Math_LerpFloat(0.0f, 10.0f, 0.25f), 2.5f), "CP_Math_LerpFloat");
	Check(Near(CP_Math_EaseOutCubic(1.0f), 1.0f), "CP_Math_EaseOutCubic");

	// Named values from numbers need a cast in C++, as the templates' README says
	const CP_KEY key = static_cast<CP_KEY>(CP_KEY_A + 1);
	Check(key == CP_KEY_B, "a key from a number, with a cast");

	// CP_BOOL takes a C++ bool
	const CP_BOOL yes = (1 < 2);
	Check(yes == TRUE, "CP_BOOL from a bool");

	if (failures == 0)
	{
		std::printf("C++ check passed\n");
	}
	return failures == 0 ? 0 : 1;
}
