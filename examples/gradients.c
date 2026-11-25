//---------------------------------------------------------
// file:	gradients.c
//
// Example: gradient fills, for a sunset over the hills.
//
// Shows how to:
// - fill a shape with colors that blend from one point to another, with
//   CP_Settings_FillLinearGradient
// - make a glow, a color fading out from the middle, with
//   CP_Settings_FillRadialGradient
// - give a gradient more than two colors, with one shape for each pair
// - fill a health bar with one gradient across its whole width, so it goes
//   from green to red as it empties
// - give a bar one color that goes from green to red, through yellow, with
//   CP_ColorHSL_Lerp, and see why CP_Color_Lerp goes through brown
//
// A gradient is a fill, like CP_Settings_Fill: every shape after it uses it,
// until the next fill. Its points are in the same coordinates as the shapes,
// so a gradient from (0, 0) to (0, 600) runs down the whole window, whatever
// shape you draw. Past its ends, it keeps the end colors.
//
// Left and Right change the health, and F turns a flashlight on and off.
// Point at the button to see it light up.
//
// Try this:
// - Move the sun down with time, and darken the sky as it sets.
// - Give the coin in a game a glow, with a radial gradient that fades out.
// - Put a dark gradient across the bottom of the screen, behind the text,
//   so it's easy to read over anything.
//
// See also: tiled_fill.c, for shapes filled with an image; color_picker.c,
// for choosing the colors.
//
// To run it, copy this file over the template's main.c.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"

#define WINDOW_WIDTH  800.0f
#define WINDOW_HEIGHT 600.0f

float health = 0.7f; // 0 (empty) to 1 (full)
bool flashlight = false;

//---------------------------------------------------------
// The sky: three colors, so two rectangles, each blending one pair

void DrawSky(void)
{
	const CP_Color night = CP_Color_Create(20, 24, 70, 255);
	const CP_Color dusk = CP_Color_Create(170, 80, 130, 255);
	const CP_Color glow = CP_Color_Create(255, 170, 80, 255);

	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);

	// From the top of the window down to y = 250
	CP_Settings_FillLinearGradient(0, 0, night, 0, 250, dusk);
	CP_Graphics_DrawRect(0, 0, WINDOW_WIDTH, 250);

	// From y = 250 down to the horizon
	CP_Settings_FillLinearGradient(0, 250, dusk, 0, 430, glow);
	CP_Graphics_DrawRect(0, 250, WINDOW_WIDTH, WINDOW_HEIGHT - 250);
}

// The sun, and its glow: a radial gradient from a warm yellow in the
// middle to the same color with alpha 0, on a circle as wide as its outer
// diameter, so it fades away to nothing at the circle's edge
void DrawSun(float x, float y)
{
	CP_Settings_NoStroke();
	CP_Settings_FillRadialGradient(x, y, 70, CP_Color_Create(255, 220, 130, 220), 340, CP_Color_Create(255, 200, 100, 0));
	CP_Graphics_DrawCircle(x, y, 340);

	CP_Settings_Fill(CP_Color_Create(255, 235, 170, 255));
	CP_Graphics_DrawCircle(x, y, 80);
}

// A line of hills from layered noise, filled with a gradient from the
// tallest a hill can be down to the bottom of the window
void DrawHills(float baseY, float height, float noiseY, CP_Color top, CP_Color bottom)
{
	CP_Settings_NoStroke();
	CP_Settings_FillLinearGradient(0, baseY - height, top, 0, WINDOW_HEIGHT, bottom);

	CP_Graphics_BeginShape();
	for (float x = 0; x <= WINDOW_WIDTH; x += 20)
	{
		const float y = baseY - height * CP_Random_NoiseOctaves(x * 0.004f, noiseY, 0, 3, 0.5f);
		CP_Graphics_AddVertex(x, y);
	}
	CP_Graphics_AddVertex(WINDOW_WIDTH, WINDOW_HEIGHT);
	CP_Graphics_AddVertex(0, WINDOW_HEIGHT);
	CP_Graphics_EndShape();
}

//---------------------------------------------------------
// The health bar. The gradient goes across the bar's whole width, and only
// the full part is drawn, so a full bar shows all of it, and an emptier one
// only the red and orange end.

void DrawHealthBar(float left, float top, float width, float height)
{
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);

	// The empty bar behind
	CP_Settings_Fill(CP_Color_Create(20, 20, 30, 200));
	CP_Graphics_DrawRoundedRect(left, top, width, height, height / 2.0f);

	// The full part: red at the left end, green at the right end
	if (health > 0.0f)
	{
		CP_Settings_FillLinearGradient(left, 0, CP_Color_Create(230, 40, 40, 255), left + width, 0, CP_Color_Create(60, 220, 90, 255));
		CP_Graphics_DrawRoundedRect(left, top, width * health, height, height / 2.0f);
	}

	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255)); // text takes a plain color
	CP_Settings_TextSize(22.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BOTTOM);
	CP_Font_DrawText("Health", left, top - 6);
}

// Two bars in one color each, green when full and red when empty.
// CP_Color_Lerp blends the red, green and blue on their own, so halfway
// it's a dull brown. CP_ColorHSL_Lerp turns the hue around the color wheel
// instead (the shorter way), so halfway it's yellow, as bright as the ends.
void DrawOneColorBars(float left, float top, float width)
{
	const CP_Color empty = CP_Color_Create(230, 40, 40, 255);
	const CP_Color full = CP_Color_Create(60, 220, 90, 255);
	const CP_Color straight = CP_Color_Lerp(empty, full, health);
	const CP_ColorHSL around = CP_ColorHSL_Lerp(CP_ColorHSL_FromColor(empty), CP_ColorHSL_FromColor(full), health);

	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(CP_Color_Create(20, 20, 30, 200));
	CP_Graphics_DrawRoundedRect(left, top, width, 12, 6);
	CP_Graphics_DrawRoundedRect(left, top + 26, width, 12, 6);
	if (health > 0.0f)
	{
		CP_Settings_Fill(straight);
		CP_Graphics_DrawRoundedRect(left, top, width * health, 12, 6);
		CP_Settings_Fill(CP_Color_FromColorHSL(around));
		CP_Graphics_DrawRoundedRect(left, top + 26, width * health, 12, 6);
	}

	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_TextSize(16.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Font_DrawText("CP_Color_Lerp", left + width + 10, top + 6);
	CP_Font_DrawText("CP_ColorHSL_Lerp", left + width + 10, top + 32);
}

//---------------------------------------------------------
// A button that looks lit from above: lighter at the top, darker at the
// bottom. It lights up under the mouse, and the colors swap when pressed,
// so it looks pushed in.

void DrawButton(float x, float y, float width, float height, const char* text)
{
	const float left = x - width / 2.0f;
	const float top = y - height / 2.0f;
	const float mouseX = CP_Input_GetMouseX();
	const float mouseY = CP_Input_GetMouseY();
	const bool over = mouseX >= left && mouseX < left + width && mouseY >= top && mouseY < top + height;

	CP_Color light = CP_Color_Create(90, 140, 230, 255);
	CP_Color dark = CP_Color_Create(40, 70, 150, 255);
	if (over)
	{
		light = CP_Color_Create(130, 180, 255, 255);
		dark = CP_Color_Create(60, 100, 200, 255);
	}
	if (over && CP_Input_MouseDown(CP_MOUSE_LEFT))
	{
		const CP_Color swap = light;
		light = dark;
		dark = swap;
	}

	CP_Settings_Stroke(CP_Color_Create(20, 30, 70, 255));
	CP_Settings_StrokeWeight(2.0f);
	CP_Settings_FillLinearGradient(0, top, light, 0, top + height, dark);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Graphics_DrawRoundedRect(left, top, width, height, 12);

	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_TextSize(30.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Font_DrawText(text, x, y);
}

//---------------------------------------------------------
// A flashlight: a rectangle over the whole window, clear around the mouse
// and dark further out. Clear in the middle is a color with alpha 0, and
// past the outer diameter the dark keeps going to the window's edges.

void DrawFlashlight(void)
{
	CP_Settings_NoStroke();
	CP_Settings_FillRadialGradient(CP_Input_GetMouseX(), CP_Input_GetMouseY(), 120, CP_Color_Create(0, 0, 10, 0), 460, CP_Color_Create(0, 0, 10, 235));
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Graphics_DrawRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}

//---------------------------------------------------------

void game_init(void)
{
	CP_System_SetWindowSize((int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
	CP_System_SetWindowTitle("Gradients");
}

void game_update(void)
{
	if (CP_Input_KeyDown(CP_KEY_LEFT))
	{
		health -= 0.5f * CP_System_GetDt();
	}
	if (CP_Input_KeyDown(CP_KEY_RIGHT))
	{
		health += 0.5f * CP_System_GetDt();
	}
	health = CP_Math_ClampFloat(health, 0.0f, 1.0f);
	if (CP_Input_KeyTriggered(CP_KEY_F))
	{
		flashlight = !flashlight;
	}

	DrawSky();
	DrawSun(560, 330);
	DrawHills(440, 140, 1.0f, CP_Color_Create(110, 70, 120, 255), CP_Color_Create(45, 30, 70, 255));
	DrawHills(540, 120, 7.0f, CP_Color_Create(50, 110, 80, 255), CP_Color_Create(15, 40, 35, 255));
	DrawButton(400, 520, 180, 56, "Play");
	if (flashlight)
	{
		DrawFlashlight();
	}
	DrawHealthBar(30, 50, 260, 24);
	DrawOneColorBars(30, 92, 260);

	CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
	CP_Settings_TextSize(20.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_RIGHT, CP_TEXT_ALIGN_V_BOTTOM);
	CP_Font_DrawText("Left/Right: health    F: flashlight", WINDOW_WIDTH - 20, WINDOW_HEIGHT - 12);
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
