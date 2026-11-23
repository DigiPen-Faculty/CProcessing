//---------------------------------------------------------
// file:	noise.c
//
// Example: noise, random that changes smoothly: wobbles, hills, clouds and
// maps.
//
// Shows how to:
// - see the difference between random numbers and noise, with CP_Random_Noise
// - choose how far apart to look in the noise (the scale): close together is
//   smooth, far apart is as jumpy as random numbers
// - make something wander and a flame flicker, by moving through the noise
//   as time passes
// - draw hills from layers of noise (octaves) with CP_Random_NoiseOctaves,
//   and scroll them
// - make clouds from noise in 2D, drawn into an image's pixels, that change
//   shape as they drift (the third number of the noise)
// - make the same island map from the same seed, with CP_Random_NoiseSeed
//
// Noise gives a number from 0 to 1 for every point. Points close together
// give numbers close together, so as you move through it, the number rises
// and falls smoothly instead of jumping. CP_Random_Noise takes the point as
// x, y and z: use x alone for a wavy line, x and y for a picture, and the
// third number (often the time) to make either change.
//
// Up and Down change the scale in box 2. 1 to 6 choose how many layers make
// the hills in box 4. C switches the colors of the clouds in box 5. Left and
// Right change the noise seed in box 6, which changes all the noise.
//
// Try this:
// - Make the firefly wander faster when the mouse comes close.
// - Shake a camera with noise: a small x and y from the noise, added to
//   everything, that dies down after a hit.
// - Give the island rivers, or trees where the land is green.
// - Use noise for the wind: how far grass leans, a little different along
//   the field.
//
// See also: random.c, for dice, loot and other random choices; curves.c,
// for curves you shape yourself; gradients.c and tiled_fill.c, for hills
// made from noise.
//
// To run it, copy this file over the template's main.c.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include <math.h>
#include <stdio.h>

//---------------------------------------------------------
// What the boxes share

#define COLUMNS    3
#define BOX_WIDTH  400.0f
#define BOX_HEIGHT 320.0f
#define MARGIN     18.0f

CP_Color dimColor;

// The box itself, its title and two lines of code at the bottom
void DrawBox(float left, float top, const char* title, const char* code1, const char* code2)
{
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(40, 44, 60, 255));
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Graphics_DrawRect(left, top, BOX_WIDTH, BOX_HEIGHT);

	CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
	CP_Settings_TextSize(20.0f);
	CP_Font_DrawText(title, left + 12.0f, top + 26.0f);

	CP_Settings_Fill(CP_Color_Create(150, 200, 255, 255));
	CP_Settings_TextSize(16.0f);
	CP_Font_DrawText(code1, left + 12.0f, top + BOX_HEIGHT - 32.0f);
	CP_Font_DrawText(code2, left + 12.0f, top + BOX_HEIGHT - 12.0f);
}

// A line through values from 0 to 1, one every step pixels, from left to
// right: 0 at the bottom, 1 at the top
void DrawGraph(float left, float bottom, float height, float step, const float values[], int count, CP_Color color)
{
	CP_Settings_Stroke(color);
	CP_Settings_StrokeWeight(2.0f);
	for (int i = 1; i < count; ++i)
	{
		CP_Graphics_DrawLine(left + (i - 1) * step, bottom - values[i - 1] * height, left + i * step, bottom - values[i] * height);
	}
	CP_Settings_NoStroke();
}

//---------------------------------------------------------
// 1. Random numbers, or noise

#define GRAPH_POINTS 95
const float graphStep = 4.0f; // pixels between the points: 95 points fill 376 pixels
float randomValues[GRAPH_POINTS];

void DrawRandomOrNoiseBox(float left, float top)
{
	DrawBox(left, top, "1. Random numbers, or noise", "y = CP_Random_RangeFloat(0, 1);", "y = CP_Random_Noise(i * 0.05f, 0, 0);");

	float noiseValues[GRAPH_POINTS];
	for (int i = 0; i < GRAPH_POINTS; ++i)
	{
		noiseValues[i] = CP_Random_Noise(i * 0.05f, 0.0f, 0.0f);
	}
	CP_Settings_Fill(dimColor);
	CP_Settings_TextSize(16.0f);
	CP_Font_DrawText("Random: every number jumps", left + 12.0f, top + 54.0f);
	DrawGraph(left + 12.0f, top + 150.0f, 80.0f, graphStep, randomValues, GRAPH_POINTS, CP_Color_Create(255, 120, 90, 255));
	CP_Settings_Fill(dimColor);
	CP_Font_DrawText("Noise: each close to the one before", left + 12.0f, top + 172.0f);
	DrawGraph(left + 12.0f, top + 268.0f, 80.0f, graphStep, noiseValues, GRAPH_POINTS, CP_Color_Create(90, 200, 255, 255));
}

//---------------------------------------------------------
// 2. The scale: how far apart to look

#define SCALE_COUNT 6
const float scales[SCALE_COUNT] = { 0.01f, 0.03f, 0.1f, 0.3f, 1.0f, 3.0f };
int scaleChoice = 1;

void DrawScaleBox(float left, float top)
{
	const float scale = scales[scaleChoice];
	char code[60];
	snprintf(code, sizeof(code), "y = CP_Random_Noise(i * %.2ff, 0, 0);", scale);
	DrawBox(left, top, "2. How far apart: the scale", code, "Small: smooth hills. Big: jumpy, like random.");

	float values[GRAPH_POINTS];
	for (int i = 0; i < GRAPH_POINTS; ++i)
	{
		values[i] = CP_Random_Noise(i * scale, 0.0f, 0.0f);
	}
	DrawGraph(left + 12.0f, top + 230.0f, 160.0f, graphStep, values, GRAPH_POINTS, CP_Color_Create(90, 200, 255, 255));

	char text[60];
	snprintf(text, sizeof(text), "Scale %.2f (Up/Down)", scale);
	CP_Settings_Fill(dimColor);
	CP_Settings_TextSize(16.0f);
	CP_Font_DrawText(text, left + 12.0f, top + 54.0f);
}

//---------------------------------------------------------
// 3. Wander and flicker: moving through the noise as time passes

void DrawWanderBox(float left, float top)
{
	DrawBox(left, top, "3. Wander and flicker", "x = CP_Random_Noise(time * 0.3f, 0, 0);", "y = CP_Random_Noise(time * 0.3f, 50, 0);");
	const float time = CP_System_GetSeconds();

	// The firefly: x and y come from places far apart in the noise (y at 50),
	// so they don't rise and fall together. Moving the time slowly through
	// the noise moves the firefly slowly.
	const float x = left + 20.0f + CP_Random_Noise(time * 0.3f, 0.0f, 0.0f) * 220.0f;
	const float y = top + 50.0f + CP_Random_Noise(time * 0.3f, 50.0f, 0.0f) * 200.0f;
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(240, 250, 120, 50));
	CP_Graphics_DrawCircle(x, y, 30.0f);
	CP_Settings_Fill(CP_Color_Create(240, 250, 120, 255));
	CP_Graphics_DrawCircle(x, y, 10.0f);

	// The flame: moving fast through the noise makes it flicker. Its size
	// and its brightness come from different places (100 and 150).
	const float flameX = left + 330.0f;
	const float flameBottom = top + 240.0f;
	const float flicker = CP_Random_Noise(time * 6.0f, 100.0f, 0.0f);
	const float glow = CP_Random_Noise(time * 4.0f, 150.0f, 0.0f);
	CP_Settings_Fill(CP_Color_Create(255, 160, 40, (int)(30.0f + glow * 60.0f)));
	CP_Graphics_DrawCircle(flameX, flameBottom - 40.0f, 90.0f + glow * 40.0f);
	const float height = 50.0f + flicker * 40.0f;
	CP_Settings_Fill(CP_Color_Create(255, 140, 30, 255));
	CP_Graphics_DrawEllipse(flameX, flameBottom - height / 2.0f, 30.0f, height);
	CP_Settings_Fill(CP_Color_Create(255, 230, 120, 255));
	CP_Graphics_DrawEllipse(flameX, flameBottom - height / 3.0f, 14.0f, height * 0.6f);
	CP_Settings_RectMode(CP_POSITION_CENTER);
	CP_Settings_Fill(CP_Color_Create(120, 80, 50, 255));
	CP_Graphics_DrawRect(flameX, flameBottom + 8.0f, 50.0f, 16.0f);
}

//---------------------------------------------------------
// 4. Hills: layers of noise

int octaves = 4;

// Hills along the box, from layered noise: the first layer (octave) is the
// big shape, and each after it adds bumps half the size and, with a
// persistence of 0.5, half as tall. scroll moves along the noise, so the
// hills slide past.
void DrawHills(float left, float bottom, float height, float scale, float scroll, float noiseY, CP_Color color)
{
	CP_Settings_Stroke(color);
	CP_Settings_StrokeWeight(2.5f);
	for (float x = 1.0f; x < BOX_WIDTH; x += 2.0f)
	{
		const float n = CP_Random_NoiseOctaves((x + scroll) * scale, noiseY, 0.0f, octaves, 0.5f);
		CP_Graphics_DrawLine(left + x, bottom, left + x, bottom - n * height);
	}
	CP_Settings_NoStroke();
}

void DrawHillsBox(float left, float top)
{
	DrawBox(left, top, "4. Hills: layers of noise", "h = CP_Random_NoiseOctaves(x * 0.006f,", "0, 0, octaves, 0.5f);");
	const float scroll = CP_System_GetSeconds() * 40.0f;
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(CP_Color_Create(120, 170, 230, 255));
	CP_Graphics_DrawRect(left, top + 40.0f, BOX_WIDTH, 210.0f);
	// The far hills move slower than the near ones, so they seem farther
	DrawHills(left, top + 250.0f, 190.0f, 0.004f, scroll * 0.4f, 10.0f, CP_Color_Create(150, 180, 210, 255));
	DrawHills(left, top + 250.0f, 140.0f, 0.006f, scroll, 0.0f, CP_Color_Create(60, 130, 80, 255));

	char text[40];
	snprintf(text, sizeof(text), "Octaves: %d (keys 1-6)", octaves);
	CP_Settings_Fill(CP_Color_Create(20, 30, 50, 255));
	CP_Settings_TextSize(16.0f);
	CP_Font_DrawText(text, left + 12.0f, top + 60.0f);
}

//---------------------------------------------------------
// 5. Clouds: noise in 2D, drawn into pixels

#define CLOUD_WIDTH  100
#define CLOUD_HEIGHT 52
CP_Color cloudPixels[CLOUD_WIDTH * CLOUD_HEIGHT]; // row by row from the top left
CP_Image cloudImage;
bool rainbow = false;

// Every pixel gets a number from the noise at its x and y. Adding time to x
// makes the clouds drift; time as z makes them change shape as they go.
void UpdateClouds(void)
{
	const float time = CP_System_GetSeconds();
	for (int y = 0; y < CLOUD_HEIGHT; ++y)
	{
		for (int x = 0; x < CLOUD_WIDTH; ++x)
		{
			const float n = CP_Random_NoiseOctaves((x + time * 4.0f) * 0.05f, y * 0.05f, time * 0.1f, 4, 0.5f);
			CP_Color color;
			if (rainbow)
			{
				// A hue from the noise, and the colors move round as time passes
				color = CP_Color_FromColorHSL(CP_ColorHSL_Create((int)(n * 500.0f + time * 60.0f) % 360, 60, 55, 255));
			}
			else
			{
				// Blue sky below 0.45, white cloud above 0.65, and in between, a blend
				const float cloud = CP_Math_ClampFloat((n - 0.45f) / 0.2f, 0.0f, 1.0f);
				color = CP_Color_Lerp(CP_Color_Create(80, 140, 220, 255), CP_Color_Create(250, 250, 255, 255), cloud);
			}
			cloudPixels[y * CLOUD_WIDTH + x] = color;
		}
	}
	CP_Image_UpdatePixelData(cloudImage, cloudPixels);
}

void DrawCloudsBox(float left, float top)
{
	DrawBox(left, top, "5. Clouds: noise in 2D", "n = CP_Random_NoiseOctaves(x * 0.05f,", "y * 0.05f, time * 0.1f, 4, 0.5f);");
	UpdateClouds();
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_Image_Draw(cloudImage, left + 12.0f, top + 40.0f, BOX_WIDTH - 24.0f, (BOX_WIDTH - 24.0f) * CLOUD_HEIGHT / CLOUD_WIDTH, 255);
	CP_Settings_Fill(dimColor);
	CP_Settings_TextSize(16.0f);
	CP_Font_DrawText(rainbow ? "C: clouds" : "C: rainbow", left + BOX_WIDTH - 100.0f, top + 26.0f);
}

//---------------------------------------------------------
// 6. An island from a seed

#define MAP_WIDTH  100
#define MAP_HEIGHT 52
CP_Color mapPixels[MAP_WIDTH * MAP_HEIGHT];
CP_Image mapImage;
int seed = 1;

// The height of the land is noise, minus the distance from the middle, so
// the edges sink under the sea. Then each height gets the color of water,
// sand, grass or rock. The same seed always makes the same island.
void MakeIsland(void)
{
	CP_Random_NoiseSeed(seed);
	for (int y = 0; y < MAP_HEIGHT; ++y)
	{
		for (int x = 0; x < MAP_WIDTH; ++x)
		{
			// From the middle: 0 there, about 1 at the edges
			const float dx = (x - MAP_WIDTH / 2.0f) / (MAP_WIDTH / 2.0f);
			const float dy = (y - MAP_HEIGHT / 2.0f) / (MAP_HEIGHT / 2.0f);
			const float height = CP_Random_NoiseOctaves(x * 0.04f, y * 0.04f, 0.0f, 4, 0.5f) - sqrtf(dx * dx + dy * dy) * 0.7f;

			CP_Color color = CP_Color_Create(40, 90, 170, 255); // deep water
			if (height > 0.5f)
			{
				color = CP_Color_Create(130, 120, 110, 255); // rock
			}
			else if (height > 0.28f)
			{
				color = CP_Color_Create(80, 160, 80, 255); // grass
			}
			else if (height > 0.22f)
			{
				color = CP_Color_Create(230, 210, 140, 255); // sand
			}
			else if (height > 0.14f)
			{
				color = CP_Color_Create(70, 140, 210, 255); // shallow water
			}
			mapPixels[y * MAP_WIDTH + x] = color;
		}
	}
	CP_Image_UpdatePixelData(mapImage, mapPixels);
}

void DrawIslandBox(float left, float top)
{
	DrawBox(left, top, "6. An island from a seed", "CP_Random_NoiseSeed(seed);", "height = noise - distance from the middle");
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_Image_Draw(mapImage, left + 12.0f, top + 40.0f, BOX_WIDTH - 24.0f, (BOX_WIDTH - 24.0f) * MAP_HEIGHT / MAP_WIDTH, 255);

	char text[40];
	snprintf(text, sizeof(text), "Seed: %d (Left/Right)", seed);
	CP_Settings_Fill(dimColor);
	CP_Settings_TextSize(16.0f);
	CP_Font_DrawText(text, left + 12.0f, top + 260.0f);
}

//---------------------------------------------------------

void game_init(void)
{
	CP_System_SetWindowSize((int)(COLUMNS * (BOX_WIDTH + MARGIN) + MARGIN), 720);
	CP_System_SetWindowTitle("Noise");
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);
	dimColor = CP_Color_Create(170, 175, 195, 255);

	for (int i = 0; i < GRAPH_POINTS; ++i)
	{
		randomValues[i] = CP_Random_RangeFloat(0.0f, 1.0f);
	}

	// The images start with whatever is in the arrays, then get their pixels
	// with CP_Image_UpdatePixelData
	cloudImage = CP_Image_CreateFromData(CLOUD_WIDTH, CLOUD_HEIGHT, cloudPixels);
	mapImage = CP_Image_CreateFromData(MAP_WIDTH, MAP_HEIGHT, mapPixels);
	MakeIsland();
}

void game_update(void)
{
	if (CP_Input_KeyTriggered(CP_KEY_UP) && scaleChoice < SCALE_COUNT - 1)
	{
		scaleChoice++;
	}
	if (CP_Input_KeyTriggered(CP_KEY_DOWN) && scaleChoice > 0)
	{
		scaleChoice--;
	}
	for (int i = 0; i < 6; ++i)
	{
		if (CP_Input_KeyTriggered((CP_KEY)(CP_KEY_1 + i)))
		{
			octaves = i + 1;
		}
	}
	if (CP_Input_KeyTriggered(CP_KEY_C))
	{
		rainbow = !rainbow;
	}
	if (CP_Input_KeyTriggered(CP_KEY_LEFT) && seed > 1)
	{
		seed--;
		MakeIsland();
	}
	if (CP_Input_KeyTriggered(CP_KEY_RIGHT))
	{
		seed++;
		MakeIsland();
	}

	CP_Graphics_ClearBackground(CP_Color_Create(24, 26, 36, 255));

	const float column[COLUMNS] = { MARGIN, MARGIN + (BOX_WIDTH + MARGIN), MARGIN + 2.0f * (BOX_WIDTH + MARGIN) };
	const float topRow = MARGIN;
	const float bottomRow = MARGIN * 2.0f + BOX_HEIGHT;

	DrawRandomOrNoiseBox(column[0], topRow);
	DrawScaleBox(column[1], topRow);
	DrawWanderBox(column[2], topRow);
	DrawHillsBox(column[0], bottomRow);
	DrawCloudsBox(column[1], bottomRow);
	DrawIslandBox(column[2], bottomRow);

	CP_Settings_Fill(CP_Color_Create(170, 170, 190, 255));
	CP_Settings_TextSize(20.0f);
	CP_Font_DrawText("Up/Down: the scale    1-6: octaves    C: cloud colors    Left/Right: the noise seed", MARGIN, 705.0f);
}

void game_exit(void)
{
	CP_Image_Free(&cloudImage);
	CP_Image_Free(&mapImage);
}

int main(void)
{
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
