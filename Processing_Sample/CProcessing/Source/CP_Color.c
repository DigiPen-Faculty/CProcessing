//------------------------------------------------------------------------------
// file:	CP_Color.c
// author:	Daniel Hamilton
// brief:	Create and edit colors
//
// Copyright © 2019 DigiPen, All rights reserved.
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

int clampRGBA(int value)
{
	return CP_Math_ClampInt(value, 0, 255);
}

int clampH(int value)
{
	// special clamp to allow for continual wrapping of hue values
	int hue = value % 360;
	return (hue < 0) ? hue + 360 : hue;
}

int clampSL(int value)
{
	return CP_Math_ClampInt(value, 0, 100);
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
		(unsigned char)clampRGBA(a) };
    return c;
}

CP_API CP_Color CP_Color_CreateHex(int hexCode)
{
	CP_Color c = {
		(unsigned char)clampRGBA((hexCode & 0xFF000000) >> 24),
		(unsigned char)clampRGBA((hexCode & 0x00FF0000) >> 16),
		(unsigned char)clampRGBA((hexCode & 0x0000FF00) >> 8),
		(unsigned char)clampRGBA(hexCode & 0x000000FF) };
	return c;
}

CP_API CP_Color CP_Color_Lerp(CP_Color a, CP_Color b, float lerp_factor)
{
	lerp_factor = CP_Math_ClampFloat(lerp_factor, 0, 1.0f);
	CP_Color result = {
		(unsigned char)((1.0f - lerp_factor) * a.r + lerp_factor * b.r),
		(unsigned char)((1.0f - lerp_factor) * a.g + lerp_factor * b.g),
		(unsigned char)((1.0f - lerp_factor) * a.b + lerp_factor * b.b),
		(unsigned char)((1.0f - lerp_factor) * a.a + lerp_factor * b.a) };
    return result;
}

CP_API CP_Color CP_Color_FromColorHSL(CP_ColorHSL hsl)
{
	hsl.h = clampH(hsl.h);
	hsl.s = clampSL(hsl.s);
	hsl.l = clampSL(hsl.l);
	hsl.a = clampRGBA(hsl.a);

	NVGcolor c1 = nvgHSLA(hsl.h / 360.0f, hsl.s / 100.0f, hsl.l / 100.0f, (unsigned char)hsl.a);
	CP_Color c = {
		(unsigned char)(c1.r * 255.0f),
		(unsigned char)(c1.g * 255.0f),
		(unsigned char)(c1.b * 255.0f),
		(unsigned char)hsl.a };
	return c;
}

CP_API CP_ColorHSL CP_ColorHSL_Create(int h, int s, int l, int a)
{
	CP_ColorHSL c = {
		clampH(h),
		clampSL(s),
		clampSL(l),
		clampRGBA(a) };
	return c;
}

CP_API CP_ColorHSL CP_ColorHSL_Lerp(CP_ColorHSL a, CP_ColorHSL b, float lerp_factor)
{
	lerp_factor = CP_Math_ClampFloat(lerp_factor, 0, 1.0f);
	CP_ColorHSL result = {
		(int)((1.0f - lerp_factor) * a.h + lerp_factor * b.h),
		(int)((1.0f - lerp_factor) * a.s + lerp_factor * b.s),
		(int)((1.0f - lerp_factor) * a.l + lerp_factor * b.l),
		(int)((1.0f - lerp_factor) * a.a + lerp_factor * b.a) };
	return result;
}

//
// Math from: https://www.rapidtables.com/convert/color/rgb-to-hsl.html
//
CP_API CP_ColorHSL CP_ColorHSL_FromColor(CP_Color rgb)
{
	int CmaxI = max(max(rgb.r, rgb.g), rgb.b);
	int CminI = min(min(rgb.r, rgb.g), rgb.b);
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

	if(deltaI != 0)
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

	CP_ColorHSL c = {
		(int)(h * 60.0f),
		(int)(s * 100.0f),
		(int)(l * 100.0f),
		(int)rgb.a };
	return c;
}

//------------------------------------------------------------------------------
// Private Functions:
//------------------------------------------------------------------------------
