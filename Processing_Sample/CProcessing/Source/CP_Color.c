//------------------------------------------------------------------------------
// File:	CP_Color.c
// Author:	Daniel Hamilton
// Brief:	Create and edit colors
//
// GitHub Repository:
// https://github.com/DigiPen-Faculty/CProcessing
//
//---------------------------------------------------------
// MIT License
//
// Copyright (C) 2021 DigiPen Institute of Technology
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//---------------------------------------------------------

//------------------------------------------------------------------------------
// Include Files:
//------------------------------------------------------------------------------

#include <math.h>
#include "cprocessing.h"
#include "Internal_System.h"

//------------------------------------------------------------------------------
// Private Function Declarations:
//------------------------------------------------------------------------------

unsigned char clampRGBA(unsigned int value)
{
	return (unsigned char)CP_Math_ClampInt(value, 0, 255);
}

unsigned short clampH(unsigned short value)
{
	// special clamp to allow for continual wrapping of hue values
	unsigned short hue = value % 360;
	return (hue < 0) ? hue + 360 : hue;
}

unsigned short clampSL(unsigned short value)
{
	return (unsigned short)CP_Math_ClampInt(value, 0, 100);
}

//------------------------------------------------------------------------------
// Public Functions:
//------------------------------------------------------------------------------

CP_API CP_Color CP_Color_Create(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
	CP_Color c = {
		(unsigned char)clampRGBA(red),
		(unsigned char)clampRGBA(green),
		(unsigned char)clampRGBA(blue),
		(unsigned char)clampRGBA(alpha) };
	return c;
}

CP_API CP_Color CP_Color_CreateHex(unsigned int hexCode)
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
		(unsigned char)((1.0f - lerp_factor) * a.red + lerp_factor * b.red),
		(unsigned char)((1.0f - lerp_factor) * a.green + lerp_factor * b.green),
		(unsigned char)((1.0f - lerp_factor) * a.blue + lerp_factor * b.blue),
		(unsigned char)((1.0f - lerp_factor) * a.alpha + lerp_factor * b.alpha) };
	return result;
}

CP_API CP_Color CP_Color_FromColorHSL(CP_ColorHSL hsl)
{
	hsl.hue = clampH(hsl.hue);
	hsl.saturation = clampSL(hsl.saturation);
	hsl.luminance = clampSL(hsl.luminance);
	hsl.alpha = clampRGBA(hsl.alpha);

	NVGcolor c1 = nvgHSLA(hsl.hue / 360.0f, hsl.saturation / 100.0f, hsl.luminance / 100.0f, (unsigned char)hsl.alpha);
	CP_Color c = {
		(unsigned char)(c1.r * 255.0f),
		(unsigned char)(c1.g * 255.0f),
		(unsigned char)(c1.b * 255.0f),
		hsl.alpha };
	return c;
}

CP_API CP_ColorHSL CP_ColorHSL_Create(unsigned short hue, unsigned char saturation, unsigned char luminance, unsigned char alpha)
{
	CP_ColorHSL c = {
		clampH(hue),
		clampSL(saturation),
		clampSL(luminance),
		clampRGBA(alpha) };
	return c;
}

CP_API CP_ColorHSL CP_ColorHSL_Lerp(CP_ColorHSL a, CP_ColorHSL b, float lerp_factor)
{
	lerp_factor = CP_Math_ClampFloat(lerp_factor, 0, 1.0f);
	CP_ColorHSL result = {
		(int)((1.0f - lerp_factor) * a.hue + lerp_factor * b.hue),
		(int)((1.0f - lerp_factor) * a.saturation + lerp_factor * b.saturation),
		(int)((1.0f - lerp_factor) * a.luminance + lerp_factor * b.luminance),
		(int)((1.0f - lerp_factor) * a.alpha + lerp_factor * b.alpha) };
	return result;
}

//
// Math from: https://www.rapidtables.com/convert/color/rgb-to-hsl.html
//
CP_API CP_ColorHSL CP_ColorHSL_FromColor(CP_Color rgb)
{
	int CmaxI = max(max(rgb.red, rgb.green), rgb.blue);
	int CminI = min(min(rgb.red, rgb.green), rgb.blue);
	int deltaI = CmaxI - CminI;

	float red = rgb.red / 255.0f;
	float green = rgb.green / 255.0f;
	float blue = rgb.blue / 255.0f;

	float Cmax = (float)CmaxI / 255.0f;
	float Cmin = (float)CminI / 255.0f;
	float delta = Cmax - Cmin;

	float hue = 0;
	float saturation = 0;
	float luminance = ((Cmax + Cmin) * 0.5f);

	if (deltaI != 0)
	{
		saturation = delta / (1.0f - fabsf(2.0f * luminance - 1.0f));

		if (rgb.red == CmaxI)
			hue = ((green - blue) / delta);
		else if (rgb.green == CmaxI)
			hue = ((blue - red) / delta + 2.0f);
		else if (rgb.blue == CmaxI)
			hue = ((red - green) / delta + 4.0f);

		// correct hue
		if (hue < 0) hue += 6.0f;
		else if (hue > 6.0f) hue -= 6.0f;
	}

	CP_ColorHSL c = {
		(unsigned short)(hue * 60.0f),
		(int)(saturation * 100.0f),
		(int)(luminance * 100.0f),
		rgb.alpha };
	return c;
}
