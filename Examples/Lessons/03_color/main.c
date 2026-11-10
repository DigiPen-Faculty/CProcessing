//---------------------------------------------------------
// file:	main.c
//
// Lesson 3: Color.
// https://github.com/DigiPen-Faculty/CProcessing/wiki/Lesson-03-Color
//
// This is the finished program from the lesson. The window shows four ways
// of working with color, one in each corner:
//   top left      RGB: red, green and blue light mixing
//   top right     alpha: how see-through a color is (move the mouse left and right)
//   bottom left   HSL: hue, saturation and lightness (move the mouse left and right)
//   bottom right  Lerp: the colors between two colors (move the mouse left and right)
//
// To run it, copy this file over the template's main.c.
//
// Try this:
// - Change the two colors in the Lerp corner, then find a pair that
//   makes a sunset.
// - Make the hue strip show only the greens and blues.
// - Add a fourth circle to the light mixing, in yellow, and see what
//   happens where it overlaps the others.
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

// The window is divided into four corners ("cells"), each this big
const float cellWidth = 400.0f;
const float cellHeight = 300.0f;

// A label in the top left of a cell
void DrawLabel(const char* text, float cellX, float cellY)
{
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Font_DrawText(text, cellX + 10.0f, cellY + 22.0f);
}

//---------------------------------------------------------
// RGB: every color on a screen is a mix of red, green and blue light, each
// from 0 (none) to 255 (full). Mixing light ADDS: red + green is yellow,
// and all three together make white.
//---------------------------------------------------------
void DrawLightMixing(float cellX, float cellY)
{
	// Add the colors' light to what's already there, instead of covering it
	CP_Settings_BlendMode(CP_BLEND_ADD);

	float size = 130.0f;
	float centerX = cellX + cellWidth / 2.0f;
	float centerY = cellY + 175.0f;

	CP_Settings_Fill(CP_Color_Create(255, 0, 0, 255));
	CP_Graphics_DrawCircle(centerX - 35.0f, centerY - 30.0f, size);
	CP_Settings_Fill(CP_Color_Create(0, 255, 0, 255));
	CP_Graphics_DrawCircle(centerX + 35.0f, centerY - 30.0f, size);
	CP_Settings_Fill(CP_Color_Create(0, 0, 255, 255));
	CP_Graphics_DrawCircle(centerX, centerY + 30.0f, size);

	// Back to normal drawing, where a new shape covers what's behind it
	CP_Settings_BlendMode(CP_BLEND_ALPHA);

	DrawLabel("RGB: mixing light", cellX, cellY);
}

//---------------------------------------------------------
// Alpha: the fourth number in a color. 255 is solid, 0 is invisible, and
// in between you can see what's behind it.
//---------------------------------------------------------
void DrawAlpha(float cellX, float cellY)
{
	// The mouse's left to right position becomes an alpha from 0 to 255
	float mouseAcross = CP_Input_GetMouseX() / CP_System_GetWindowWidth();
	int alpha = (int)CP_Math_ClampFloat(mouseAcross * 255.0f, 0.0f, 255.0f);

	// Something behind: a blue box
	CP_Settings_Fill(CP_Color_Create(40, 90, 220, 255));
	CP_Graphics_DrawRect(cellX + 70.0f, cellY + 80.0f, 140.0f, 190.0f);

	// An orange circle in front, with the alpha from the mouse
	CP_Settings_Fill(CP_Color_Create(255, 160, 0, alpha));
	CP_Graphics_DrawCircle(cellX + 230.0f, cellY + 175.0f, 170.0f);

	char text[60];
	snprintf(text, sizeof(text), "Alpha: %d", alpha);
	DrawLabel(text, cellX, cellY);
}

//---------------------------------------------------------
// HSL: another way to describe a color, closer to how people think of
// them. Hue is the color's place on the color wheel (0 to 359 degrees: red
// at 0, green at 120, blue at 240, and back to red). Saturation is how
// vivid it is (0 to 100), and lightness is how bright (0 is black, 50 is
// the pure color, 100 is white).
//---------------------------------------------------------
void DrawHue(float cellX, float cellY)
{
	// The whole color wheel as a strip: 36 boxes, each 10 degrees further on
	float boxWidth = 10.0f;
	for (int i = 0; i < 36; i++)
	{
		CP_ColorHSL hsl = CP_ColorHSL_Create(i * 10, 100, 50, 255);
		CP_Settings_Fill(CP_Color_FromColorHSL(hsl));
		CP_Graphics_DrawRect(cellX + 20.0f + i * boxWidth, cellY + 40.0f, boxWidth, 60.0f);
	}

	// The hue under the mouse, in three lightnesses
	float mouseAcross = CP_Input_GetMouseX() / CP_System_GetWindowWidth();
	int hue = (int)CP_Math_ClampFloat(mouseAcross * 359.0f, 0.0f, 359.0f);
	int lightness[3] = { 25, 50, 75 };
	for (int i = 0; i < 3; i++)
	{
		CP_ColorHSL hsl = CP_ColorHSL_Create(hue, 100, lightness[i], 255);
		CP_Settings_Fill(CP_Color_FromColorHSL(hsl));
		CP_Graphics_DrawRect(cellX + 20.0f + i * 120.0f, cellY + 130.0f, 110.0f, 130.0f);
	}

	char text[60];
	snprintf(text, sizeof(text), "HSL: hue %d (lightness 25, 50 and 75)", hue);
	DrawLabel(text, cellX, cellY);
}

//---------------------------------------------------------
// Lerp: short for "linear interpolation", a way to find a color part of
// the way between two others. CP_Color_Lerp(a, b, 0.0f) is a, (a, b, 1.0f)
// is b, and (a, b, 0.5f) is the middle. It's how to make gradients and
// smooth fades.
//---------------------------------------------------------
void DrawLerp(float cellX, float cellY)
{
	CP_Color colorA = CP_Color_Create(255, 80, 60, 255);	// red-orange
	CP_Color colorB = CP_Color_Create(40, 60, 200, 255);	// blue

	// Ten boxes from colorA to colorB. i / 9 goes from 0.0 to 1.0.
	for (int i = 0; i < 10; i++)
	{
		float t = i / 9.0f;
		CP_Settings_Fill(CP_Color_Lerp(colorA, colorB, t));
		CP_Graphics_DrawRect(cellX + 20.0f + i * 36.0f, cellY + 40.0f, 36.0f, 60.0f);
	}

	// One big box, at the point the mouse chooses
	float mouseAcross = CP_Input_GetMouseX() / CP_System_GetWindowWidth();
	float t = CP_Math_ClampFloat(mouseAcross, 0.0f, 1.0f);
	CP_Settings_Fill(CP_Color_Lerp(colorA, colorB, t));
	CP_Graphics_DrawRect(cellX + 20.0f, cellY + 130.0f, 360.0f, 130.0f);

	char text[60];
	snprintf(text, sizeof(text), "Lerp: %.2f of the way from first to second", t);
	DrawLabel(text, cellX, cellY);
}

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Lesson 3: Color");

	// Rectangles are placed by their top left corner, to make the layout
	// above easy to work out. (The default is the center.)
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_NoStroke();
}

void game_update(void)
{
	CP_Graphics_ClearBackground(CP_Color_Create(25, 25, 30, 255));

	DrawLightMixing(0.0f, 0.0f);
	DrawAlpha(cellWidth, 0.0f);
	DrawHue(0.0f, cellHeight);
	DrawLerp(cellWidth, cellHeight);
}

void game_exit(void)
{
}

int main(void)
{
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
