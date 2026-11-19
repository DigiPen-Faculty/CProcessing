//------------------------------------------------------------------------------
// file:	CP_Color.c
// author:	Daniel Hamilton
// brief:	Create and edit colors
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology and CProcessing contributors
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include Files:
//------------------------------------------------------------------------------

#include <math.h>
#include "cprocessing.h"
#include "Internal_System.h"

//------------------------------------------------------------------------------
// Defines:
//------------------------------------------------------------------------------

static inline int CP_ColorMaxInt(int a, int b) { return a > b ? a : b; }
static inline int CP_ColorMinInt(int a, int b) { return a < b ? a : b; }

//------------------------------------------------------------------------------
// Private Consts:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Private Structures:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Variables:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Private Variables:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Private Function Declarations:
//------------------------------------------------------------------------------

static int clampRGBA(int value)
{
	return CP_Math_ClampInt(value, 0, 255);
}

static int clampH(int value)
{
	// special clamp to allow for continual wrapping of hue values
	int hue = value % 360;
	return (hue < 0) ? hue + 360 : hue;
}

static int clampSL(int value)
{
	return CP_Math_ClampInt(value, 0, 100);
}

// a + t * (b - a), rounded to the nearest whole number. It gives a exactly
// when a and b are the same; (1 - t) * a + t * b, cut off by a cast, could
// come out one less (and was half a step too low on average).
static int lerpRounded(int a, int b, float t)
{
	return (int)lroundf((float)a + t * (float)(b - a));
}

//------------------------------------------------------------------------------
// Public Functions:
//------------------------------------------------------------------------------

CP_API CP_Color CP_Color_Create(int r, int g, int b, int a)
{
	CP_Color c = {
		(unsigned char)clampRGBA(r),
		(unsigned char)clampRGBA(g),
		(unsigned char)clampRGBA(b),
		(unsigned char)clampRGBA(a)
	};
	return c;
}

CP_API CP_Color CP_Color_CreateHex(unsigned hexCode)
{
	CP_Color c = {
		(unsigned char)clampRGBA((hexCode & 0xFF000000) >> 24),
		(unsigned char)clampRGBA((hexCode & 0x00FF0000) >> 16),
		(unsigned char)clampRGBA((hexCode & 0x0000FF00) >> 8),
		(unsigned char)clampRGBA(hexCode & 0x000000FF)
	};
	return c;
}

CP_API CP_Color CP_Color_Lerp(CP_Color a, CP_Color b, float lerpFactor)
{
	lerpFactor = CP_Math_ClampFloat(lerpFactor, 0, 1.0f);
	CP_Color result = {
		(unsigned char)lerpRounded(a.r, b.r, lerpFactor),
		(unsigned char)lerpRounded(a.g, b.g, lerpFactor),
		(unsigned char)lerpRounded(a.b, b.b, lerpFactor),
		(unsigned char)lerpRounded(a.a, b.a, lerpFactor)
	};
	return result;
}

CP_API CP_Color CP_Color_FromColorHSL(CP_ColorHSL hsl)
{
	hsl.h = clampH(hsl.h);
	hsl.s = clampSL(hsl.s);
	hsl.l = clampSL(hsl.l);
	hsl.a = clampRGBA(hsl.a);

	NVGcolor c1 = nvgHSLA(hsl.h / 360.0f, hsl.s / 100.0f, hsl.l / 100.0f, (unsigned char)hsl.a);
	// Rounded to the nearest value, so a color survives RGB to HSL and back
	// as closely as whole-number HSL allows
	CP_Color c = {
		(unsigned char)lroundf(c1.r * 255.0f),
		(unsigned char)lroundf(c1.g * 255.0f),
		(unsigned char)lroundf(c1.b * 255.0f),
		(unsigned char)hsl.a
	};
	return c;
}

CP_API CP_ColorHSL CP_ColorHSL_Create(int h, int s, int l, int a)
{
	CP_ColorHSL c = {
		clampH(h),
		clampSL(s),
		clampSL(l),
		clampRGBA(a)
	};
	return c;
}

CP_API CP_ColorHSL CP_ColorHSL_Lerp(CP_ColorHSL a, CP_ColorHSL b, float lerpFactor)
{
	lerpFactor = CP_Math_ClampFloat(lerpFactor, 0, 1.0f);

	// Hue goes round a circle: take the shorter way, so 350 to 10 passes
	// through red (0), not through cyan (180)
	const int fromHue = clampH(a.h);
	int hueStep = clampH(b.h) - fromHue;
	if (hueStep > 180)
	{
		hueStep -= 360;
	}
	else if (hueStep < -180)
	{
		hueStep += 360;
	}

	CP_ColorHSL result = {
		clampH(lerpRounded(fromHue, fromHue + hueStep, lerpFactor)),
		lerpRounded(a.s, b.s, lerpFactor),
		lerpRounded(a.l, b.l, lerpFactor),
		lerpRounded(a.a, b.a, lerpFactor)
	};
	return result;
}

//
// Math from: https://www.rapidtables.com/convert/color/rgb-to-hsl.html
//
CP_API CP_ColorHSL CP_ColorHSL_FromColor(CP_Color rgb)
{
	// (min/max used to come from <windows.h> macros)
	int CmaxI = CP_ColorMaxInt(CP_ColorMaxInt(rgb.r, rgb.g), rgb.b);
	int CminI = CP_ColorMinInt(CP_ColorMinInt(rgb.r, rgb.g), rgb.b);
	int deltaI = CmaxI - CminI;

	float r = rgb.r / 255.0f;
	float g = rgb.g / 255.0f;
	float b = rgb.b / 255.0f;

	float Cmax = (float)CmaxI / 255.0f;
	float Cmin = (float)CminI / 255.0f;
	float delta = Cmax - Cmin;

	float h = 0;
	float s = 0;
	float l = ((Cmax + Cmin) * 0.5f);

	if (deltaI != 0)
	{
		s = delta / (1.0f - fabsf(2.0f * l - 1.0f));

		if (rgb.r == CmaxI)
		{
			h = ((g - b) / delta);
		}
		else if (rgb.g == CmaxI)
		{
			h = ((b - r) / delta + 2.0f);
		}
		else if (rgb.b == CmaxI)
		{
			h = ((r - g) / delta + 4.0f);
		}

		// correct hue
		if (h < 0) h += 6.0f;
		else if (h > 6.0f) h -= 6.0f;
	}

	// Rounded to the nearest value (a hue that rounds up to 360 is 0)
	CP_ColorHSL c = {
		clampH((int)lroundf(h * 60.0f)),
		clampSL((int)lroundf(s * 100.0f)),
		clampSL((int)lroundf(l * 100.0f)),
		(int)rgb.a
	};
	return c;
}

//------------------------------------------------------------------------------
// Private Functions:
//------------------------------------------------------------------------------
