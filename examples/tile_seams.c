//---------------------------------------------------------
// file:	tile_seams.c
//
// Example: tiles without seams: why thin lines appear between tiles, and
// each way to get rid of them.
//
// Thin lines between tiles are a problem in every tile game, and no one fix
// covers every case. Each key below switches one cause or one fix, and the
// magnifier on the right shows the pixels around the mouse four times as
// big. The background is a loud pink, so every gap shows.
//
// Shows how to:
// - see the causes: a camera or a zoom that puts tiles between pixels,
//   anti-aliased edges that don't quite add up, a sprite sheet whose
//   neighbor tiles bleed in, and edges that fade with CP_IMAGE_WRAP_CLAMP
// - fix them: round the camera to whole pixels (the final screen position,
//   so movement stays smooth), zoom in whole steps, sharp pixels
//   (CP_IMAGE_FILTER_NEAREST), a sheet with each tile's edge pixels repeated
//   around it, CP_Settings_AntiAlias(false), drawing each tile a pixel
//   bigger, a solid shape behind the tiles, or one image fill for a whole
//   area (CP_Settings_FillImage)
// - make a magnifier: a small screenshot drawn big with sharp pixels
//
// Keys: each key and what it does is listed on the right. Space pauses the
// scrolling, and Left and Right move the camera by hand.
//
// Try this:
// - Find the fewest fixes that leave no lines at every zoom.
// - Make the pool's water a little see-through, then turn on overdraw (O):
//   where tiles overlap, the water is darker.
// - Draw the world into an image at 1x, then draw that image scaled with
//   sharp pixels, for any zoom without seams.
//
// See also: tiled_fill.c, for image fills; camera.c, for a camera that
// follows and zooms; sprite_animation.c, for drawing from a sprite sheet.
//
// To run it, copy this file over the template's main.c.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include <math.h>
#include <stdio.h>

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720
#define SCENE_WIDTH   880 // the panel is to the right of this

//---------------------------------------------------------
// The tiles: four of them, 8 by 8 pixels each, made in code

#define TILE_PIXELS 8
#define TILE_SIZE   32.0f // each tile is drawn 32 by 32 in the world
#define TILE_TYPES  4

enum
{
	TILE_GRASS,
	TILE_DIRT,
	TILE_BRICK,
	TILE_WATER
};

// One tile's color at one of its pixels
CP_Color TilePixel(int type, int x, int y)
{
	switch (type)
	{
	case TILE_GRASS:
		if (y < 2 || (y == 2 && x % 3 != 1))
		{
			return (x + y) % 3 == 0 ? CP_Color_Create(120, 210, 90, 255) : CP_Color_Create(80, 170, 60, 255);
		}
		return (x * 5 + y * 3) % 7 == 0 ? CP_Color_Create(110, 75, 45, 255) : CP_Color_Create(140, 95, 55, 255);
	case TILE_DIRT:
		return (x * 5 + y * 3) % 7 == 0 ? CP_Color_Create(110, 75, 45, 255) : CP_Color_Create(140, 95, 55, 255);
	case TILE_BRICK:
		if (y == 3 || y == 7 || (y < 3 && x == 3) || (y > 3 && y < 7 && x == 7))
		{
			return CP_Color_Create(200, 190, 170, 255); // mortar
		}
		return CP_Color_Create(170, 70, 50, 255);
	default:
		return (x + y * 2) % 5 == 0 ? CP_Color_Create(120, 190, 240, 255) : CP_Color_Create(60, 120, 200, 255);
	}
}

// The main color of each tile, for the solid shape behind them
CP_Color TileMainColor(int type)
{
	switch (type)
	{
	case TILE_GRASS: return CP_Color_Create(140, 95, 55, 255);
	case TILE_DIRT:  return CP_Color_Create(140, 95, 55, 255);
	case TILE_BRICK: return CP_Color_Create(170, 70, 50, 255);
	default:         return CP_Color_Create(60, 120, 200, 255);
	}
}

// Three ways to keep the tiles:
// - a sheet: the tiles side by side in one image, touching
// - a padded sheet: each tile with a 1 pixel border of its own edge pixels
//   repeated, so a smooth filter that reaches past the edge finds the same
//   color instead of the next tile
// - one image for each tile
CP_Image sheet;
CP_Image paddedSheet;
CP_Image tileImages[TILE_TYPES];

void MakeTiles(void)
{
	CP_Color sheetPixels[TILE_TYPES * TILE_PIXELS * TILE_PIXELS];
	CP_Color paddedPixels[TILE_TYPES * (TILE_PIXELS + 2) * (TILE_PIXELS + 2)];
	CP_Color onePixels[TILE_PIXELS * TILE_PIXELS];
	const int sheetWidth = TILE_TYPES * TILE_PIXELS;
	const int paddedWidth = TILE_TYPES * (TILE_PIXELS + 2);

	for (int type = 0; type < TILE_TYPES; ++type)
	{
		for (int y = 0; y < TILE_PIXELS; ++y)
		{
			for (int x = 0; x < TILE_PIXELS; ++x)
			{
				sheetPixels[y * sheetWidth + type * TILE_PIXELS + x] = TilePixel(type, x, y);
				onePixels[y * TILE_PIXELS + x] = TilePixel(type, x, y);
			}
		}
		tileImages[type] = CP_Image_CreateFromData(TILE_PIXELS, TILE_PIXELS, onePixels);

		// the padded tile: 10 by 10, the border copying the nearest edge pixel
		for (int y = 0; y < TILE_PIXELS + 2; ++y)
		{
			for (int x = 0; x < TILE_PIXELS + 2; ++x)
			{
				int fromX = CP_Math_ClampInt(x - 1, 0, TILE_PIXELS - 1);
				int fromY = CP_Math_ClampInt(y - 1, 0, TILE_PIXELS - 1);
				paddedPixels[y * paddedWidth + type * (TILE_PIXELS + 2) + x] = TilePixel(type, fromX, fromY);
			}
		}
	}
	sheet = CP_Image_CreateFromData(sheetWidth, TILE_PIXELS, sheetPixels);
	paddedSheet = CP_Image_CreateFromData(paddedWidth, TILE_PIXELS + 2, paddedPixels);
}

//---------------------------------------------------------
// The world: a map of tiles

#define MAP_COLUMNS 40
#define MAP_ROWS    16
#define GROUND_ROW  12 // the row of grass; dirt below it
const char* map[MAP_ROWS] = {
	"........................................",
	"........................................",
	"...........................bbbb.........",
	"........................................",
	"....bbbbb...............................",
	"................bbbbbb..................",
	"........................................",
	"...........................bbbbbb.......",
	"..................................bb....",
	".........bbbb.....................bb....",
	"..................................bb....",
	"..................................bb....",
	"ggggggggggggggggggggwwwwwggggggggggggggg",
	"ddddddddddddddddddddwwwwwddddddddddddddd",
	"dddddddddddddddddddddddddddddddddddddddd",
	"dddddddddddddddddddddddddddddddddddddddd",
};

int TileAt(int column, int row)
{
	switch (map[row][column])
	{
	case 'g': return TILE_GRASS;
	case 'd': return TILE_DIRT;
	case 'b': return TILE_BRICK;
	case 'w': return TILE_WATER;
	default:  return -1;
	}
}

//---------------------------------------------------------
// The causes and fixes, each switched by a key

typedef enum TileSource
{
	FROM_SHEET,
	FROM_PADDED_SHEET,
	FROM_ONE_IMAGE_EACH
} TileSource;

const float zooms[3] = { 1.3f, 2.0f, 1.0f };
int zoomIndex = 0;
bool sharpPixels = false;       // F: CP_IMAGE_FILTER_NEAREST
bool clampToNothing = false;    // W: CP_IMAGE_WRAP_CLAMP instead of the default CLAMP_EDGE
bool antiAlias = true;          // A
bool roundCamera = false;       // R
TileSource source = FROM_SHEET; // T
bool overdraw = false;          // O
bool backing = false;           // B
bool groundFill = false;        // G: one image fill for the dirt
bool calmSky = false;           // C

float cameraX = 0.0f;
float cameraSpeed = 14.0f; // world units a second: slow, so it's between pixels most frames
bool paused = false;

void DrawTile(int type, float x, float y, float size)
{
	switch (source)
	{
	case FROM_SHEET:
		CP_Image_DrawSubImage(sheet, x, y, size, size, (float)(type * TILE_PIXELS), 0.0f, (float)((type + 1) * TILE_PIXELS), (float)TILE_PIXELS, 255);
		break;
	case FROM_PADDED_SHEET:
	{
		float left = (float)(type * (TILE_PIXELS + 2) + 1); // inside the border
		CP_Image_DrawSubImage(paddedSheet, x, y, size, size, left, 1.0f, left + TILE_PIXELS, 1.0f + TILE_PIXELS, 255);
		break;
	}
	default:
		CP_Image_Draw(tileImages[type], x, y, size, size, 255);
		break;
	}
}

void DrawWorld(void)
{
	float zoom = zooms[zoomIndex];

	// The camera: where the world's top left lands on the screen. Rounding
	// this final screen position, not cameraX itself, keeps the movement
	// smooth while every tile lands on whole pixels.
	float offsetX = -cameraX * zoom;
	float offsetY = WINDOW_HEIGHT - MAP_ROWS * TILE_SIZE * zoom;
	if (roundCamera)
	{
		offsetX = roundf(offsetX);
		offsetY = roundf(offsetY);
	}

	CP_Settings_Save();
	CP_Settings_ImageFilterMode(sharpPixels ? CP_IMAGE_FILTER_NEAREST : CP_IMAGE_FILTER_LINEAR);
	CP_Settings_ImageWrapMode(clampToNothing ? CP_IMAGE_WRAP_CLAMP : CP_IMAGE_WRAP_CLAMP_EDGE);
	CP_Settings_AntiAlias(antiAlias);
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_NoStroke();
	CP_Settings_Translate(offsetX, offsetY);
	CP_Settings_Scale(zoom, zoom);

	// A solid shape behind each run of tiles in a row, in their main color:
	// a gap shows that color instead of the background
	if (backing)
	{
		for (int row = 0; row < MAP_ROWS; ++row)
		{
			int column = 0;
			while (column < MAP_COLUMNS)
			{
				int type = TileAt(column, row);
				int end = column;
				while (end < MAP_COLUMNS && TileAt(end, row) == type)
				{
					++end;
				}
				if (type >= 0)
				{
					CP_Settings_Fill(TileMainColor(type));
					CP_Graphics_DrawRect(column * TILE_SIZE, row * TILE_SIZE, (end - column) * TILE_SIZE, TILE_SIZE);
				}
				column = end;
			}
		}
	}

	// One image fill for all the dirt: a single shape with the tile repeated
	// inside it, so there are no edges between its tiles at all. It's drawn
	// first, so the grass and the pool are drawn over it.
	if (groundFill)
	{
		float top = (GROUND_ROW + 1) * TILE_SIZE;
		CP_Settings_FillImage(tileImages[TILE_DIRT], 0.0f, top, TILE_SIZE, TILE_SIZE, 255);
		CP_Graphics_DrawRect(0.0f, top, MAP_COLUMNS * TILE_SIZE, (MAP_ROWS - GROUND_ROW - 1) * TILE_SIZE);
	}

	// Overdraw: each tile a screen pixel bigger, so neighbors overlap
	float size = overdraw ? TILE_SIZE + 1.0f / zoom : TILE_SIZE;

	for (int row = 0; row < MAP_ROWS; ++row)
	{
		for (int column = 0; column < MAP_COLUMNS; ++column)
		{
			int type = TileAt(column, row);
			if (type < 0 || (groundFill && type == TILE_DIRT))
			{
				continue;
			}
			DrawTile(type, column * TILE_SIZE, row * TILE_SIZE, size);
		}
	}

	CP_Settings_Restore();
}

//---------------------------------------------------------
// The panel: the keys, and the magnifier

void DrawKeyLine(const char* key, const char* text, bool fixed, float y)
{
	float x = SCENE_WIDTH + 20.0f;
	CP_Settings_Fill(CP_Color_Create(150, 200, 255, 255));
	CP_Font_DrawText(key, x, y);
	CP_Settings_Fill(fixed ? CP_Color_Create(140, 230, 140, 255) : CP_Color_Create(255, 150, 130, 255));
	CP_Font_DrawText(text, x + 24.0f, y);
}

void DrawSidePanel(CP_Image lens, float lensX, float lensY)
{
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(30, 32, 44, 255));
	CP_Graphics_DrawRect(SCENE_WIDTH, 0.0f, WINDOW_WIDTH - SCENE_WIDTH, WINDOW_HEIGHT);

	CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
	CP_Settings_TextSize(26.0f);
	CP_Font_DrawText("Tiles without seams", SCENE_WIDTH + 20.0f, 36.0f);
	CP_Settings_TextSize(15.0f);
	CP_Settings_Fill(CP_Color_Create(170, 170, 185, 255));
	CP_Font_DrawText("Red: can show lines. Green: a fix.", SCENE_WIDTH + 20.0f, 60.0f);

	char text[80];
	float zoom = zooms[zoomIndex];
	bool wholeZoom = zoom == floorf(zoom);
	CP_Settings_TextSize(16.0f);
	float y = 92.0f;
	const float step = 22.0f;
	snprintf(text, sizeof(text), "zoom %.1f%s", zoom, wholeZoom ? ", a whole step" : ": tiles fall between pixels");
	DrawKeyLine("Z", text, wholeZoom, y);
	y += step;
	DrawKeyLine("R", roundCamera ? "camera rounded to whole pixels" : "camera between pixels", roundCamera, y);
	y += step;
	DrawKeyLine("A", antiAlias ? "anti-aliased edges (the default)" : "anti-aliasing off", !antiAlias, y);
	y += step;
	DrawKeyLine("F", sharpPixels ? "sharp pixels: NEAREST" : "smooth pixels: LINEAR (the default)", sharpPixels, y);
	y += step;
	const char* sources[3] = { "tiles from a sheet, touching", "tiles from a padded sheet", "one image for each tile" };
	DrawKeyLine("T", sources[source], source != FROM_SHEET, y);
	y += step;
	DrawKeyLine("W", clampToNothing ? "wrap CLAMP: edges fade out" : "wrap CLAMP_EDGE (the default)", !clampToNothing, y);
	y += step;
	DrawKeyLine("O", overdraw ? "each tile a pixel bigger" : "no overdraw", overdraw, y);
	y += step;
	DrawKeyLine("B", backing ? "a solid shape behind the tiles" : "no shape behind the tiles", backing, y);
	y += step;
	DrawKeyLine("G", groundFill ? "the dirt is one image fill" : "the dirt is tiles", groundFill, y);
	y += step;
	CP_Settings_Fill(CP_Color_Create(170, 170, 185, 255));
	CP_Font_DrawText(calmSky ? "C: a calm sky" : "C: a loud pink background, so gaps show", SCENE_WIDTH + 20.0f, y);
	y += step;
	CP_Font_DrawText(paused ? "Space: paused. Left, Right: move" : "Space: pause. Left, Right: move", SCENE_WIDTH + 20.0f, y);

	// The magnifier: what's around the mouse, four times as big, with sharp
	// pixels so each one shows as a square
	float boxX = SCENE_WIDTH + 20.0f;
	float boxY = 400.0f;
	CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
	CP_Font_DrawText("Around the mouse, 4 times as big:", boxX, boxY - 10.0f);
	CP_Settings_Save();
	CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_NEAREST);
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_Image_Draw(lens, boxX, boxY, 360.0f, 280.0f, 255);
	CP_Settings_Restore();

	// and where that is in the scene
	CP_Settings_NoFill();
	CP_Settings_StrokeWeight(1.0f);
	CP_Settings_Stroke(CP_Color_Create(255, 255, 255, 255));
	CP_Graphics_DrawRect(lensX, lensY, 90.0f, 70.0f);
	CP_Graphics_DrawRect(boxX, boxY, 360.0f, 280.0f);
	CP_Settings_NoStroke();
}

//---------------------------------------------------------

void game_init(void)
{
	CP_System_SetWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	CP_System_SetWindowTitle("Tiles without seams");
	CP_Settings_RectMode(CP_POSITION_CORNER);
	MakeTiles();
}

void game_update(void)
{
	float dt = CP_System_GetDt();

	if (CP_Input_KeyTriggered(CP_KEY_Z))
	{
		zoomIndex = (zoomIndex + 1) % 3;
	}
	if (CP_Input_KeyTriggered(CP_KEY_R))
	{
		roundCamera = !roundCamera;
	}
	if (CP_Input_KeyTriggered(CP_KEY_A))
	{
		antiAlias = !antiAlias;
	}
	if (CP_Input_KeyTriggered(CP_KEY_F))
	{
		sharpPixels = !sharpPixels;
	}
	if (CP_Input_KeyTriggered(CP_KEY_T))
	{
		source = (TileSource)((source + 1) % 3);
	}
	if (CP_Input_KeyTriggered(CP_KEY_W))
	{
		clampToNothing = !clampToNothing;
	}
	if (CP_Input_KeyTriggered(CP_KEY_O))
	{
		overdraw = !overdraw;
	}
	if (CP_Input_KeyTriggered(CP_KEY_B))
	{
		backing = !backing;
	}
	if (CP_Input_KeyTriggered(CP_KEY_G))
	{
		groundFill = !groundFill;
	}
	if (CP_Input_KeyTriggered(CP_KEY_C))
	{
		calmSky = !calmSky;
	}
	if (CP_Input_KeyTriggered(CP_KEY_SPACE))
	{
		paused = !paused;
	}

	// The camera drifts slowly back and forth, or moves by hand
	float zoom = zooms[zoomIndex];
	float furthest = MAP_COLUMNS * TILE_SIZE - SCENE_WIDTH / zoom;
	if (!paused)
	{
		cameraX += cameraSpeed * dt;
		if (cameraX < 0.0f || cameraX > furthest)
		{
			cameraSpeed = -cameraSpeed;
		}
	}
	if (CP_Input_KeyDown(CP_KEY_LEFT))
	{
		cameraX -= 60.0f * dt;
	}
	if (CP_Input_KeyDown(CP_KEY_RIGHT))
	{
		cameraX += 60.0f * dt;
	}
	cameraX = CP_Math_ClampFloat(cameraX, 0.0f, furthest);

	CP_Graphics_ClearBackground(calmSky ? CP_Color_Create(150, 200, 240, 255) : CP_Color_Create(255, 0, 200, 255));
	DrawWorld();

	// Take the magnifier's screenshot before the panel is drawn over it:
	// 90 by 70 pixels around the mouse, kept inside the scene
	int lensX = CP_Math_ClampInt((int)CP_Input_GetMouseX() - 45, 0, SCENE_WIDTH - 90);
	int lensY = CP_Math_ClampInt((int)CP_Input_GetMouseY() - 35, 0, WINDOW_HEIGHT - 70);
	CP_Image lens = CP_Image_Screenshot(lensX, lensY, 90, 70);
	DrawSidePanel(lens, (float)lensX, (float)lensY);
	CP_Image_Free(&lens);
}

void game_exit(void)
{
	CP_Image_Free(&sheet);
	CP_Image_Free(&paddedSheet);
	for (int i = 0; i < TILE_TYPES; ++i)
	{
		CP_Image_Free(&tileImages[i]);
	}
}

int main(void)
{
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
