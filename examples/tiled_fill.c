//---------------------------------------------------------
// file:	tiled_fill.c
//
// Example: filling shapes with tiles of an image: bricks, grass, water.
//
// Shows how to:
// - fill any shape with an image repeated as tiles, with
//   CP_Settings_FillImage: rectangles, circles, and your own shapes with
//   holes in them
// - keep the tiles still in the world while the camera moves, or make them
//   move with a shape (a moving platform)
// - scroll tiles, by moving where they start: water, clouds, a loading bar
// - make tiles in code, by drawing into an image once (CP_Image_Create and
//   CP_Image_BeginDrawing), or from a little pixel-art map
//
// An image fill is a fill, like CP_Settings_Fill: every shape after it uses
// it, until the next fill. Its numbers say where one tile goes and how big
// it is, and the tiles repeat from there in every direction. They are in the
// same coordinates as the shapes, so they move, turn and grow with
// CP_Settings_Translate, Rotate and Scale. Text keeps the last plain color.
//
// Left and Right walk through the world. N switches the pixel art between
// sharp and smooth.
//
// Try this:
// - Load a tile from a file with CP_Image_Load (search the web for
//   "seamless texture" or "tileable texture").
// - Fill the player's health bar with a pattern that moves when it's low.
// - Make a second layer of hills, further back, that scrolls more slowly.
//
// See also: painting.c, for drawing into an image; gradients.c, for the
// other kind of fill.
//
// To run it, copy this file over the template's main.c.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include <math.h> // for sinf, floorf and fmodf
#include "cprocessing.h"

#define WINDOW_WIDTH  800.0f
#define WINDOW_HEIGHT 600.0f
#define SCENE_HEIGHT  400.0f

CP_Image bricks;
CP_Image dirt;
CP_Image planks;
CP_Image clouds;
CP_Image water;
CP_Image stripes;
CP_Image pixelArt;

float cameraX = 0.0f; // how far into the world the window is
CP_BOOL sharpPixels = TRUE;

//---------------------------------------------------------
// Making the tiles. Each is drawn once, into an image, in game_init.
// A tile looks seamless when what leaves one edge comes back on the other,
// so anything that crosses an edge is drawn on both sides.

// Two rows of bricks, the second moved half a brick: its end bricks stick
// out of both sides, and meet in the next tile
CP_Image MakeBricks(void)
{
	CP_Image image = CP_Image_Create(64, 32);
	CP_Image_BeginDrawing(image);
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(CP_Color_Create(70, 60, 60, 255)); // the mortar
	CP_Graphics_DrawRect(0, 0, 64, 32);

	CP_Settings_Fill(CP_Color_Create(170, 75, 55, 255));
	CP_Graphics_DrawRect(1, 1, 30, 14);
	CP_Graphics_DrawRect(33, 1, 30, 14);
	CP_Graphics_DrawRect(-15, 17, 30, 14);
	CP_Graphics_DrawRect(17, 17, 30, 14);
	CP_Graphics_DrawRect(49, 17, 30, 14);
	CP_Image_EndDrawing();
	return image;
}

// Brown earth with pebbles anywhere. Each pebble is drawn nine times: where
// it is, and a whole tile away in every direction. Only the part inside the
// image is kept, so a pebble cut off by one edge goes on at the other.
CP_Image MakeDirt(void)
{
	const float size = 128.0f;
	CP_Image image = CP_Image_Create((int)size, (int)size);
	CP_Image_BeginDrawing(image);
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(CP_Color_Create(110, 75, 45, 255));
	CP_Graphics_DrawRect(0, 0, size, size);

	CP_Random_Seed(7); // the same pebbles every time
	for (int i = 0; i < 40; ++i)
	{
		const float x = CP_Random_RangeFloat(0.0f, size);
		const float y = CP_Random_RangeFloat(0.0f, size);
		const float pebble = CP_Random_RangeFloat(3.0f, 9.0f);
		const int shade = CP_Random_RangeInt(60, 150);
		CP_Settings_Fill(CP_Color_Create(shade, shade * 3 / 4, shade / 2, 255));
		for (int tileY = -1; tileY <= 1; ++tileY)
		{
			for (int tileX = -1; tileX <= 1; ++tileX)
			{
				CP_Graphics_DrawEllipse(x + tileX * size, y + tileY * size, pebble * 1.4f, pebble);
			}
		}
	}
	CP_Image_EndDrawing();
	return image;
}

// Wooden boards, with a line between them and a nail at each end
CP_Image MakePlanks(void)
{
	CP_Image image = CP_Image_Create(48, 20);
	CP_Image_BeginDrawing(image);
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(CP_Color_Create(150, 105, 60, 255));
	CP_Graphics_DrawRect(0, 0, 48, 20);
	CP_Settings_Fill(CP_Color_Create(90, 60, 35, 255));
	CP_Graphics_DrawRect(0, 0, 2, 20);
	CP_Graphics_DrawCircle(6, 10, 3);
	CP_Graphics_DrawCircle(42, 10, 3);
	CP_Image_EndDrawing();
	return image;
}

// Puffs of white circles, on a clear background so the sky shows through
CP_Image MakeClouds(void)
{
	CP_Image image = CP_Image_Create(320, 120);
	CP_Image_BeginDrawing(image);
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Graphics_DrawEllipse(70, 70, 110, 50);
	CP_Graphics_DrawCircle(55, 55, 50);
	CP_Graphics_DrawCircle(90, 50, 60);
	CP_Graphics_DrawEllipse(230, 45, 90, 36);
	CP_Graphics_DrawCircle(220, 35, 36);
	CP_Graphics_DrawCircle(245, 38, 30);
	CP_Image_EndDrawing();
	return image;
}

// Blue with light wavy lines. Each wave is one sine curve across the tile,
// a whole number of times round, so it lines up with the next tile.
CP_Image MakeWater(void)
{
	CP_Image image = CP_Image_Create(128, 64);
	CP_Image_BeginDrawing(image);
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(CP_Color_Create(30, 90, 170, 255));
	CP_Graphics_DrawRect(0, 0, 128, 64);

	CP_Settings_NoFill();
	CP_Settings_StrokeWeight(3.0f);
	CP_Settings_Stroke(CP_Color_Create(120, 190, 240, 255));
	for (float waveY = 12.0f; waveY < 64.0f; waveY += 21.0f)
	{
		CP_Graphics_BeginShape();
		for (float x = 0; x <= 128; x += 4)
		{
			CP_Graphics_AddVertex(x, waveY + 4.0f * sinf(x / 128.0f * 2.0f * 3.14159f * 2.0f));
		}
		CP_Graphics_EndShapeOpen();
	}
	CP_Image_EndDrawing();
	return image;
}

// Diagonal stripes, every 32 pixels: one in the top left corner, and one
// across the middle that leaves at the right and bottom edges and comes
// back as the corner stripe of the next tile
CP_Image MakeStripes(void)
{
	CP_Image image = CP_Image_Create(32, 32);
	CP_Image_BeginDrawing(image);
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(CP_Color_Create(40, 170, 90, 255));
	CP_Graphics_DrawRect(0, 0, 32, 32);
	CP_Settings_Fill(CP_Color_Create(90, 220, 130, 255));
	CP_Graphics_DrawTriangle(0, 0, 16, 0, 0, 16);
	CP_Graphics_DrawQuad(32, 0, 32, 16, 16, 32, 0, 32);
	CP_Image_EndDrawing();
	return image;
}

// Pixel art from a little map: each letter is one pixel's color
CP_Image MakePixelArt(void)
{
	const char* map[8] = {
		"ggggggGg",
		"gGgggggg",
		"ggggyggg",
		"gggyrygg",
		"ggggyggG",
		"gGgggggg",
		"gggggGgg",
		"gggggggg",
	};
	CP_Color pixels[8 * 8];
	for (int y = 0; y < 8; ++y)
	{
		for (int x = 0; x < 8; ++x)
		{
			switch (map[y][x])
			{
			case 'G': pixels[y * 8 + x] = CP_Color_Create(60, 130, 50, 255); break;  // dark grass
			case 'y': pixels[y * 8 + x] = CP_Color_Create(250, 230, 90, 255); break; // petals
			case 'r': pixels[y * 8 + x] = CP_Color_Create(220, 80, 50, 255); break;  // the flower's middle
			default:  pixels[y * 8 + x] = CP_Color_Create(90, 170, 70, 255); break;  // grass
			}
		}
	}
	return CP_Image_CreateFromData(8, 8, pixels);
}

//---------------------------------------------------------
// The world: hills, brick walls and a moving platform. It's drawn moved left
// by the camera, so the tiles, set in world coordinates, stay with the world.

// The height of the ground at a place in the world
float GroundY(float worldX)
{
	return 330.0f - 60.0f * CP_Random_Noise(worldX * 0.004f, 3.5f, 0.0f);
}

// A brick wall with two windows and a door cut out of it: holes in the
// shape, so the sky shows through them
void DrawWall(float left, float top, float width, float height)
{
	const float right = left + width;
	const float bottom = top + height;
	CP_Graphics_BeginShape();
	CP_Graphics_AddVertex(left, bottom);
	CP_Graphics_AddVertex(left, top);
	CP_Graphics_AddVertex(right, top);
	CP_Graphics_AddVertex(right, bottom);
	for (int i = 0; i < 2; ++i) // two windows, arched at the top
	{
		const float windowLeft = left + 30.0f + i * (width - 100.0f);
		CP_Graphics_BeginHole();
		CP_Graphics_AddVertex(windowLeft, top + 90.0f);
		CP_Graphics_AddVertex(windowLeft, top + 50.0f);
		CP_Graphics_AddQuadraticVertex(windowLeft + 20.0f, top + 10.0f, windowLeft + 40.0f, top + 50.0f);
		CP_Graphics_AddVertex(windowLeft + 40.0f, top + 90.0f);
		CP_Graphics_EndHole();
	}
	CP_Graphics_BeginHole(); // the door
	CP_Graphics_AddVertex(left + width / 2.0f - 22.0f, bottom);
	CP_Graphics_AddVertex(left + width / 2.0f - 22.0f, bottom - 70.0f);
	CP_Graphics_AddVertex(left + width / 2.0f + 22.0f, bottom - 70.0f);
	CP_Graphics_AddVertex(left + width / 2.0f + 22.0f, bottom);
	CP_Graphics_EndHole();
	CP_Graphics_EndShape();
}

void DrawScene(void)
{
	const float seconds = CP_System_GetSeconds();

	// The sky (a gradient), and clouds that drift and move a third as fast
	// as the world when walking, so they look far away. These tiles are in
	// window coordinates: moving where they start is what scrolls them.
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_FillLinearGradient(0, 0, CP_Color_Create(70, 130, 220, 255), 0, SCENE_HEIGHT, CP_Color_Create(190, 220, 250, 255));
	CP_Graphics_DrawRect(0, 0, WINDOW_WIDTH, SCENE_HEIGHT);
	const float cloudsX = -cameraX / 3.0f + seconds * 8.0f;
	CP_Settings_FillImage(clouds, cloudsX, 10, 320, 120, 220);
	CP_Graphics_DrawRect(0, 10, WINDOW_WIDTH, 120); // one row of clouds: as tall as the tile

	// The world, moved left by the camera
	CP_Settings_Save();
	CP_Settings_Translate(-cameraX, 0);

	// Brick walls every 700 pixels. The tiles start at the world's (0, 0),
	// so the bricks stay put as the walls go by.
	CP_Settings_Stroke(CP_Color_Create(50, 35, 35, 255));
	CP_Settings_StrokeWeight(3.0f);
	CP_Settings_FillImage(bricks, 0, 0, 64, 32, 255);
	const float firstWall = floorf(cameraX / 700.0f) * 700.0f + 150.0f;
	for (float wallX = firstWall - 700.0f; wallX < cameraX + WINDOW_WIDTH; wallX += 700.0f)
	{
		DrawWall(wallX, 150, 260, 200);
	}

	// A platform that slides back and forth. Its tiles start at its own
	// corner, so the boards ride along with it.
	const float platformX = 560.0f + 90.0f * sinf(seconds * 1.5f);
	CP_Settings_FillImage(planks, platformX, 190, 48, 20, 255);
	CP_Graphics_DrawRect(platformX, 190, 144, 20);

	// The ground: hills filled with dirt, with a band of grass on top
	CP_Settings_NoStroke();
	CP_Settings_FillImage(dirt, 0, 0, 128, 128, 255);
	const float firstX = floorf(cameraX / 20.0f) * 20.0f - 20.0f;
	const float lastX = cameraX + WINDOW_WIDTH + 20.0f;
	CP_Graphics_BeginShape();
	for (float x = firstX; x <= lastX; x += 20.0f)
	{
		CP_Graphics_AddVertex(x, GroundY(x));
	}
	CP_Graphics_AddVertex(lastX, SCENE_HEIGHT);
	CP_Graphics_AddVertex(firstX, SCENE_HEIGHT);
	CP_Graphics_EndShape();

	CP_Settings_NoFill();
	CP_Settings_Stroke(CP_Color_Create(80, 170, 60, 255));
	CP_Settings_StrokeWeight(10.0f);
	CP_Graphics_BeginShape();
	for (float x = firstX; x <= lastX; x += 20.0f)
	{
		CP_Graphics_AddVertex(x, GroundY(x) + 3.0f);
	}
	CP_Graphics_EndShapeOpen();

	CP_Settings_Restore();
}

//---------------------------------------------------------
// The panels along the bottom

void DrawPanel(float left, float width, const char* title)
{
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(CP_Color_Create(30, 30, 45, 255));
	CP_Graphics_DrawRoundedRect(left, SCENE_HEIGHT + 12, width, 176, 10);
	CP_Settings_Fill(CP_Color_Create(220, 220, 235, 255));
	CP_Settings_TextSize(18.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
	CP_Font_DrawText(title, left + 12, SCENE_HEIGHT + 20);
}

// A round window onto the sea. Two layers of the same water tile, moving
// in different directions, the top one see-through, make it shimmer.
void DrawPorthole(float x, float y, float diameter)
{
	const float seconds = CP_System_GetSeconds();
	CP_Settings_NoStroke();
	CP_Settings_FillImage(water, seconds * 20.0f, y, 128, 64, 255);
	CP_Graphics_DrawCircle(x, y, diameter);
	CP_Settings_FillImage(water, -seconds * 13.0f, y + 20.0f, 128, 64, 90);
	CP_Graphics_DrawCircle(x, y, diameter);

	// The brass ring: just an outline
	CP_Settings_NoFill();
	CP_Settings_Stroke(CP_Color_Create(200, 160, 70, 255));
	CP_Settings_StrokeWeight(10.0f);
	CP_Graphics_DrawCircle(x, y, diameter);
}

// A loading bar that fills up over 4 seconds, again and again, with stripes
// that move along it
void DrawLoadingBar(float left, float top, float width, float height)
{
	const float seconds = CP_System_GetSeconds();
	const float loaded = fmodf(seconds, 4.0f) / 4.0f;

	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(CP_Color_Create(15, 15, 25, 255));
	CP_Graphics_DrawRoundedRect(left, top, width, height, height / 2.0f);

	CP_Settings_FillImage(stripes, left + seconds * 30.0f, top, 32, 32, 255);
	CP_Graphics_DrawRoundedRect(left, top, height + (width - height) * loaded, height, height / 2.0f);

	CP_Settings_Fill(CP_Color_Create(220, 220, 235, 255)); // text takes a plain color
	CP_Settings_TextSize(20.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_TOP);
	CP_Font_DrawText("Loading...", left + width / 2.0f, top + height + 12);
}

// The pixel-art tile, 8 pixels drawn 48 wide, on a turning floor. The tiles
// turn with the shape because they follow the transform. The clip keeps the
// floor inside its panel.
void DrawPixelFloor(float left, float width)
{
	const float middleX = left + width / 2.0f;
	const float middleY = SCENE_HEIGHT + 110.0f;
	CP_Settings_ImageFilterMode(sharpPixels ? CP_IMAGE_FILTER_NEAREST : CP_IMAGE_FILTER_LINEAR);
	CP_Settings_RectMode(CP_POSITION_CENTER);
	CP_Settings_Clip(middleX, middleY, width - 24, 120);

	CP_Settings_Save();
	CP_Settings_Translate(middleX, middleY);
	CP_Settings_Rotate(CP_System_GetSeconds() * 10.0f);
	CP_Settings_NoStroke();
	CP_Settings_FillImage(pixelArt, 0, 0, 48, 48, 255);
	CP_Graphics_DrawRect(0, 0, 400, 400);
	CP_Settings_Restore();

	CP_Settings_NoClip();
	CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_LINEAR);
}

//---------------------------------------------------------

void game_init(void)
{
	CP_System_SetWindowSize((int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
	CP_System_SetWindowTitle("Tiled fill");

	bricks = MakeBricks();
	dirt = MakeDirt();
	planks = MakePlanks();
	clouds = MakeClouds();
	water = MakeWater();
	stripes = MakeStripes();
	pixelArt = MakePixelArt();
}

void game_update(void)
{
	if (CP_Input_KeyDown(CP_KEY_RIGHT))
	{
		cameraX += 250.0f * CP_System_GetDt();
	}
	if (CP_Input_KeyDown(CP_KEY_LEFT))
	{
		cameraX -= 250.0f * CP_System_GetDt();
	}
	if (CP_Input_KeyTriggered(CP_KEY_N))
	{
		sharpPixels = !sharpPixels;
	}

	CP_Graphics_ClearBackground(CP_Color_Create(15, 15, 25, 255));
	DrawScene();

	DrawPanel(16, 240, "Water in a circle");
	DrawPorthole(136, SCENE_HEIGHT + 110, 120);
	DrawPanel(272, 256, "Moving stripes");
	DrawLoadingBar(296, SCENE_HEIGHT + 80, 208, 36);
	DrawPanel(544, 240, sharpPixels ? "Pixel art (N: smooth)" : "Pixel art (N: sharp)");
	DrawPixelFloor(544, 240);

	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_TextSize(20.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
	CP_Font_DrawText("Left/Right: walk", 12, 10);
}

void game_exit(void)
{
	CP_Image_Free(&bricks);
	CP_Image_Free(&dirt);
	CP_Image_Free(&planks);
	CP_Image_Free(&clouds);
	CP_Image_Free(&water);
	CP_Image_Free(&stripes);
	CP_Image_Free(&pixelArt);
}

int main(void)
{
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
