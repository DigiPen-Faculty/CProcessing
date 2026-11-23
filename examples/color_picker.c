//---------------------------------------------------------
// file:	color_picker.c
//
// Example: a color picker, the companion to Lesson 3 (Color). Keep it open
// while you read the lesson: it shows everything on that page.
//
// Shows how to:
// - mix a color from red, green and blue (CP_Color_Create), or from hue,
//   saturation and lightness (CP_ColorHSL_Create), and convert between
//   the two (CP_Color_FromColorHSL, CP_ColorHSL_FromColor)
// - make a color see-through with its alpha, over a checkerboard so you
//   can tell
// - find the colors between two colors with CP_Color_Lerp
// - write a color as one number for CP_Color_CreateHex: 0xRRGGBBAA, all
//   eight digits, because six digits leave the alpha at 0 (invisible)
// - make sliders that the mouse drags (DoSliders below: copy it for a
//   volume slider or an options menu)
//
// Drag the sliders to change the color. The red, green and blue sliders
// and the hue, saturation and lightness sliders describe the same color, so
// moving one set moves the other. Press A or B to keep the current color
// in slot A or B. The row of boxes below them shows the colors between the
// two.
//
// Try this:
// - Add a slider for the size of the swatch, or for the window's
//   background color.
// - Show the color's complement: the same saturation and lightness, with
//   the hue 180 further round.
// - Print the CP_Color_Create line to the console when C is pressed, to
//   paste into your own game.
//
// See also: gradients.c, for colors that blend across a shape.
//
// To run it, copy this file over the template's main.c.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include <stdio.h> // for snprintf
#include "cprocessing.h"

// The sliders, in the order they're drawn
enum
{
	RED,
	GREEN,
	BLUE,
	HUE,
	SATURATION,
	LIGHTNESS,
	ALPHA,
	SLIDER_COUNT
};

const char* sliderNames[SLIDER_COUNT] = { "Red", "Green", "Blue", "Hue", "Saturation", "Lightness", "Alpha" };
const int sliderMax[SLIDER_COUNT] = { 255, 255, 255, 359, 100, 100, 255 };

// Where the sliders are: a column on the left of the window
const float sliderLeft = 150.0f;
const float sliderWidth = 420.0f;
const float sliderTop = 80.0f;
const float sliderSpacing = 70.0f;
const float sliderHeight = 26.0f;

// The color, kept both ways. Changing an RGB slider recalculates the HSL
// numbers, and the other way round. They're both kept, rather than
// converting every frame, because a gray has no hue: converting it
// back would lose the hue chosen with the slider.
int value[SLIDER_COUNT];
int draggedSlider = -1; // the slider being dragged, or -1

// The colors kept in slots A and B
CP_Color slotA;
CP_Color slotB;

CP_Color CurrentColor(void)
{
	return CP_Color_Create(value[RED], value[GREEN], value[BLUE], value[ALPHA]);
}

void SetFromRGB(void)
{
	CP_ColorHSL hsl = CP_ColorHSL_FromColor(CP_Color_Create(value[RED], value[GREEN], value[BLUE], 255));
	value[HUE] = hsl.h;
	value[SATURATION] = hsl.s;
	value[LIGHTNESS] = hsl.l;
}

void SetFromHSL(void)
{
	CP_Color rgb = CP_Color_FromColorHSL(CP_ColorHSL_Create(value[HUE], value[SATURATION], value[LIGHTNESS], 255));
	value[RED] = rgb.r;
	value[GREEN] = rgb.g;
	value[BLUE] = rgb.b;
}

// The color a slider's track shows at a point along it: the current color,
// with only that slider's value changed. This is what makes the tracks show
// what dragging will do.
CP_Color TrackColor(int slider, float t)
{
	int v[SLIDER_COUNT];
	for (int i = 0; i < SLIDER_COUNT; i++)
	{
		v[i] = value[i];
	}
	v[slider] = (int)(t * sliderMax[slider]);

	if (slider <= BLUE)
	{
		return CP_Color_Create(v[RED], v[GREEN], v[BLUE], 255);
	}
	if (slider == ALPHA)
	{
		return CP_Color_Create(v[RED], v[GREEN], v[BLUE], v[ALPHA]);
	}
	return CP_Color_FromColorHSL(CP_ColorHSL_Create(v[HUE], v[SATURATION], v[LIGHTNESS], 255));
}

// A gray and white checkerboard, to show through see-through colors
void DrawCheckerboard(float x, float y, float w, float h, float square)
{
	for (int row = 0; row * square < h; row++)
	{
		for (int column = 0; column * square < w; column++)
		{
			int light = (row + column) % 2 == 0;
			CP_Settings_Fill(light ? CP_Color_Create(200, 200, 200, 255) : CP_Color_Create(140, 140, 140, 255));
			float squareW = (column + 1) * square > w ? w - column * square : square;
			float squareH = (row + 1) * square > h ? h - row * square : square;
			CP_Graphics_DrawRect(x + column * square, y + row * square, squareW, squareH);
		}
	}
}

void DrawWhiteText(const char* text, float x, float y)
{
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Font_DrawText(text, x, y);
}

// The sliders: each frame, pick one up, move it, and draw them all
void DoSliders(void)
{
	float mouseX = CP_Input_GetMouseX();
	float mouseY = CP_Input_GetMouseY();

	// Pick up a slider with a click on it, drop it when the button is let go
	if (CP_Input_MouseTriggered(CP_MOUSE_LEFT))
	{
		for (int i = 0; i < SLIDER_COUNT; i++)
		{
			float top = sliderTop + i * sliderSpacing;
			if (mouseX >= sliderLeft - 10.0f && mouseX <= sliderLeft + sliderWidth + 10.0f &&
				mouseY >= top - 8.0f && mouseY <= top + sliderHeight + 8.0f)
			{
				draggedSlider = i;
			}
		}
	}
	if (!CP_Input_MouseDown(CP_MOUSE_LEFT))
	{
		draggedSlider = -1;
	}

	// While dragging, the mouse's place along the slider is its value
	if (draggedSlider >= 0)
	{
		float t = CP_Math_ClampFloat((mouseX - sliderLeft) / sliderWidth, 0.0f, 1.0f);
		value[draggedSlider] = (int)(t * sliderMax[draggedSlider] + 0.5f);
		if (draggedSlider <= BLUE)
		{
			SetFromRGB();
		}
		else if (draggedSlider <= LIGHTNESS)
		{
			SetFromHSL();
		}
	}

	// Draw them
	const int segments = 64;
	for (int i = 0; i < SLIDER_COUNT; i++)
	{
		float top = sliderTop + i * sliderSpacing;

		// The track, as a row of small boxes in the colors along it. Alpha
		// goes over a checkerboard.
		if (i == ALPHA)
		{
			DrawCheckerboard(sliderLeft, top, sliderWidth, sliderHeight, 13.0f);
		}
		// Each box is drawn a pixel wider than its place, so no gaps show
		// between them. The boxes of the alpha track must not overlap: where
		// they did, the see-through colors would add up and show as stripes.
		float segmentWidth = sliderWidth / segments;
		float overlap = (i == ALPHA) ? 0.0f : 1.0f;
		for (int s = 0; s < segments; s++)
		{
			CP_Settings_Fill(TrackColor(i, (s + 0.5f) / segments));
			CP_Graphics_DrawRect(sliderLeft + s * segmentWidth, top, segmentWidth + overlap, sliderHeight);
		}

		// The handle
		float handleX = sliderLeft + sliderWidth * value[i] / sliderMax[i];
		CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
		CP_Graphics_DrawRect(handleX - 4.0f, top - 6.0f, 8.0f, sliderHeight + 12.0f);
		CP_Settings_Fill(CP_Color_Create(30, 30, 30, 255));
		CP_Graphics_DrawRect(handleX - 2.0f, top - 4.0f, 4.0f, sliderHeight + 8.0f);

		// The name and number
		char text[40];
		DrawWhiteText(sliderNames[i], 40.0f, top + 20.0f);
		snprintf(text, sizeof(text), "%d", value[i]);
		DrawWhiteText(text, sliderLeft + sliderWidth + 20.0f, top + 20.0f);
	}
}

// Slots A and B, and the colors between them
void DoSlots(void)
{
	if (CP_Input_KeyTriggered(CP_KEY_A))
	{
		slotA = CurrentColor();
	}
	if (CP_Input_KeyTriggered(CP_KEY_B))
	{
		slotB = CurrentColor();
	}

	const float left = 700.0f;
	const float top = 490.0f;

	DrawWhiteText("Slots: press A or B to keep the color", left, top - 14.0f);

	// The two slots, over the checkerboard, with the colors between them
	// in the row of ten boxes
	const float box = 48.0f;
	for (int i = 0; i < 10; i++)
	{
		DrawCheckerboard(left + i * (box + 4.0f), top + 20.0f, box, box, 12.0f);
		CP_Settings_Fill(CP_Color_Lerp(slotA, slotB, i / 9.0f));
		CP_Graphics_DrawRect(left + i * (box + 4.0f), top + 20.0f, box, box);
	}
	DrawWhiteText("A", left + 20.0f, top + 98.0f);
	DrawWhiteText("B", left + 9 * (box + 4.0f) + 20.0f, top + 98.0f);
	DrawWhiteText("The colors between A and B (CP_Color_Lerp)", left, top + 140.0f);
}

void game_init(void)
{
	CP_System_SetWindowSize(1280, 720);
	CP_System_SetWindowTitle("Color picker");
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_TextSize(22.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);

	// Start on an orange
	value[RED] = 255;
	value[GREEN] = 160;
	value[BLUE] = 0;
	value[ALPHA] = 255;
	SetFromRGB();

	slotA = CP_Color_Create(255, 80, 60, 255);
	slotB = CP_Color_Create(40, 60, 200, 255);
}

void game_update(void)
{
	CP_Graphics_ClearBackground(CP_Color_Create(30, 30, 30, 255));
	CP_Settings_NoStroke();

	DoSliders();

	// The color, big, over a checkerboard
	const float swatchLeft = 700.0f;
	const float swatchTop = 80.0f;
	const float swatchWidth = 500.0f;
	const float swatchHeight = 240.0f;
	DrawCheckerboard(swatchLeft, swatchTop, swatchWidth, swatchHeight, 20.0f);
	CP_Settings_Fill(CurrentColor());
	CP_Graphics_DrawRect(swatchLeft, swatchTop, swatchWidth, swatchHeight);

	// The color as code: from red, green and blue, from hue, saturation and
	// lightness, and as one number
	char text[120];
	snprintf(text, sizeof(text), "CP_Color_Create(%d, %d, %d, %d)", value[RED], value[GREEN], value[BLUE], value[ALPHA]);
	DrawWhiteText(text, swatchLeft, swatchTop + swatchHeight + 40.0f);
	snprintf(text, sizeof(text), "CP_ColorHSL_Create(%d, %d, %d, %d)", value[HUE], value[SATURATION], value[LIGHTNESS], value[ALPHA]);
	DrawWhiteText(text, swatchLeft, swatchTop + swatchHeight + 75.0f);
	snprintf(text, sizeof(text), "CP_Color_CreateHex(0x%02X%02X%02X%02X)", value[RED], value[GREEN], value[BLUE], value[ALPHA]);
	DrawWhiteText(text, swatchLeft, swatchTop + swatchHeight + 110.0f);

	DoSlots();
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
