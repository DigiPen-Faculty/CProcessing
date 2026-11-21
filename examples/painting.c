//---------------------------------------------------------
// file:	painting.c
//
// Example: a paint program that saves your painting as a PNG file, and
// picks it up again the next time it starts.
//
// Shows how to:
// - paint into an image with the mouse (CP_Image_Create and
//   CP_Image_BeginDrawing), so the strokes stay from frame to frame
// - save an image as a PNG file with CP_Image_Save, and load it back with
//   CP_Image_Load
// - save a screenshot of the window: CP_Image_Screenshot, then
//   CP_Image_Save, then CP_Image_Free
// - number files with snprintf, so each screenshot gets its own name
//
// Where the files go: a name without a folder, like "painting.png", is
// saved in the current folder. Running from Visual Studio, that's the
// project folder (where the assets folder is); running the .exe on its
// own, it's the folder the .exe is in. Saving over a file replaces it.
//
// Click a color to pick it, and paint with the left mouse button. The
// mouse wheel changes the brush size. S saves the painting, P saves a
// screenshot, and C clears the canvas.
//
// Try this:
// - Add a key that loads painting.png again, to undo everything since the
//   last save.
// - Keep the screenshot numbers going between runs: try names until
//   CP_Image_Load gives NULL (no such file yet), freeing the ones it finds.
// - Save a picture of your game when the player wins, to show off later.
//
// To run it, copy this file over the template's main.c.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include <stdio.h> // for snprintf
#include "cprocessing.h"

#define WINDOW_WIDTH   800.0f
#define WINDOW_HEIGHT  600.0f
#define TOOLBAR_HEIGHT 70.0f
#define COLOR_COUNT    8

CP_Image canvas;
CP_Color colors[COLOR_COUNT];
int colorIndex = 0;
float brushSize = 12.0f;
float lastX, lastY; // where the mouse was, in the canvas, the frame before
int screenshotNumber = 1;

// A message at the bottom of the window ("Saved painting.png") that fades
char message[64] = "";
float messageTime = 0.0f;

//---------------------------------------------------------

void ShowMessage(const char* text)
{
	snprintf(message, sizeof(message), "%s", text);
	messageTime = 2.0f;
}

// The whole canvas white
void ClearCanvas(void)
{
	CP_Image_BeginDrawing(canvas);
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Graphics_DrawRect(0, 0, (float)CP_Image_GetWidth(canvas), (float)CP_Image_GetHeight(canvas));
	CP_Image_EndDrawing();
}

// The color swatches along the toolbar: where swatch i is
float SwatchX(int i)
{
	return 40.0f + i * 50.0f;
}

void DrawToolbar(void)
{
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(CP_Color_Create(40, 40, 55, 255));
	CP_Graphics_DrawRect(0, 0, WINDOW_WIDTH, TOOLBAR_HEIGHT);

	for (int i = 0; i < COLOR_COUNT; ++i)
	{
		// The chosen color gets a white ring around it
		if (i == colorIndex)
		{
			CP_Settings_Stroke(CP_Color_Create(255, 255, 255, 255));
			CP_Settings_StrokeWeight(4.0f);
		}
		else
		{
			CP_Settings_Stroke(CP_Color_Create(20, 20, 30, 255));
			CP_Settings_StrokeWeight(2.0f);
		}
		CP_Settings_Fill(colors[i]);
		CP_Graphics_DrawCircle(SwatchX(i), TOOLBAR_HEIGHT / 2.0f, 36.0f);
	}

	CP_Settings_Fill(CP_Color_Create(220, 220, 235, 255));
	CP_Settings_TextSize(18.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_RIGHT, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Font_DrawText("S: save   P: screenshot   C: clear", WINDOW_WIDTH - 16, TOOLBAR_HEIGHT / 2.0f - 11);
	CP_Font_DrawText("Mouse wheel: brush size", WINDOW_WIDTH - 16, TOOLBAR_HEIGHT / 2.0f + 11);
}

// Paints from where the mouse was to where it is now, into the canvas.
// A line with round ends, as wide as the brush, leaves no gaps however fast
// the mouse moves.
void Paint(void)
{
	const float x = CP_Input_GetMouseX();
	const float y = CP_Input_GetMouseY() - TOOLBAR_HEIGHT; // the canvas starts below the toolbar
	if (CP_Input_MouseTriggered(CP_MOUSE_LEFT))
	{
		lastX = x; // a new stroke starts here, not where the last one ended
		lastY = y;
	}

	CP_Image_BeginDrawing(canvas);
	CP_Settings_Stroke(colors[colorIndex]);
	CP_Settings_StrokeWeight(brushSize);
	CP_Settings_LineCapMode(CP_LINE_CAP_ROUND);
	CP_Graphics_DrawLine(lastX, lastY, x, y);
	CP_Settings_NoStroke();
	CP_Settings_Fill(colors[colorIndex]);
	CP_Graphics_DrawCircle(x, y, brushSize); // so a click without moving makes a dot
	CP_Image_EndDrawing();

	lastX = x;
	lastY = y;
}

//---------------------------------------------------------
// Saving

void SavePainting(void)
{
	if (CP_Image_Save(canvas, "painting.png"))
	{
		ShowMessage("Saved painting.png");
	}
	else
	{
		ShowMessage("Couldn't save painting.png");
	}
}

// What's on the screen now, saved as screenshot_1.png, screenshot_2.png...
// The screenshot is an image of its own, so it's freed once it's saved.
void SaveScreenshot(void)
{
	char name[32];
	snprintf(name, sizeof(name), "screenshot_%d.png", screenshotNumber);
	CP_Image shot = CP_Image_Screenshot(0, 0, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
	if (CP_Image_Save(shot, name))
	{
		char text[64];
		snprintf(text, sizeof(text), "Saved %s", name);
		ShowMessage(text);
		++screenshotNumber;
	}
	CP_Image_Free(&shot);
}

//---------------------------------------------------------

void game_init(void)
{
	CP_System_SetWindowSize((int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
	CP_System_SetWindowTitle("Painting");

	colors[0] = CP_Color_Create(30, 30, 30, 255);
	colors[1] = CP_Color_Create(230, 50, 50, 255);
	colors[2] = CP_Color_Create(250, 150, 30, 255);
	colors[3] = CP_Color_Create(250, 220, 40, 255);
	colors[4] = CP_Color_Create(60, 180, 75, 255);
	colors[5] = CP_Color_Create(50, 120, 230, 255);
	colors[6] = CP_Color_Create(150, 70, 200, 255);
	colors[7] = CP_Color_Create(255, 255, 255, 255); // white paints over mistakes

	canvas = CP_Image_Create((int)WINDOW_WIDTH, (int)(WINDOW_HEIGHT - TOOLBAR_HEIGHT));
	ClearCanvas();

	// The painting from last time, if there is one: CP_Image_Load gives
	// NULL when the file isn't there yet. It's drawn into the canvas, then
	// freed, because the canvas is what gets painted on and saved.
	CP_Image saved = CP_Image_Load("painting.png");
	if (saved)
	{
		CP_Image_BeginDrawing(canvas);
		CP_Settings_ImageMode(CP_POSITION_CORNER);
		CP_Image_Draw(saved, 0, 0, (float)CP_Image_GetWidth(saved), (float)CP_Image_GetHeight(saved), 255);
		CP_Image_EndDrawing();
		CP_Image_Free(&saved);
		ShowMessage("Loaded painting.png");
	}
}

void game_update(void)
{
	const float mouseX = CP_Input_GetMouseX();
	const float mouseY = CP_Input_GetMouseY();

	// Picking a color: a click on a swatch
	if (CP_Input_MouseTriggered(CP_MOUSE_LEFT) && mouseY < TOOLBAR_HEIGHT)
	{
		for (int i = 0; i < COLOR_COUNT; ++i)
		{
			if (CP_Math_Distance(mouseX, mouseY, SwatchX(i), TOOLBAR_HEIGHT / 2.0f) < 18.0f)
			{
				colorIndex = i;
			}
		}
	}
	// Painting: the button held down over the canvas
	else if (CP_Input_MouseDown(CP_MOUSE_LEFT) && mouseY >= TOOLBAR_HEIGHT)
	{
		Paint();
	}

	brushSize = CP_Math_ClampFloat(brushSize + CP_Input_MouseWheel() * 3.0f, 2.0f, 80.0f);
	if (CP_Input_KeyTriggered(CP_KEY_C))
	{
		ClearCanvas();
	}

	CP_Graphics_ClearBackground(CP_Color_Create(40, 40, 55, 255));
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_Image_Draw(canvas, 0, TOOLBAR_HEIGHT, (float)CP_Image_GetWidth(canvas), (float)CP_Image_GetHeight(canvas), 255);
	DrawToolbar();

	// Saving comes after drawing the canvas and toolbar, so a screenshot
	// has them, and before the brush outline and the message, so it hasn't
	if (CP_Input_KeyTriggered(CP_KEY_S))
	{
		SavePainting();
	}
	if (CP_Input_KeyTriggered(CP_KEY_P))
	{
		SaveScreenshot();
	}

	// The brush's outline under the mouse
	if (mouseY >= TOOLBAR_HEIGHT)
	{
		CP_Settings_NoFill();
		CP_Settings_Stroke(CP_Color_Create(120, 120, 140, 255));
		CP_Settings_StrokeWeight(1.5f);
		CP_Graphics_DrawCircle(mouseX, mouseY, brushSize);
	}

	if (messageTime > 0.0f)
	{
		messageTime -= CP_System_GetDt();
		const int alpha = (int)(255.0f * CP_Math_ClampFloat(messageTime, 0.0f, 1.0f));
		CP_Settings_Fill(CP_Color_Create(40, 40, 55, alpha));
		CP_Settings_TextSize(24.0f);
		CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_BOTTOM);
		CP_Font_DrawText(message, WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT - 16);
	}
}

void game_exit(void)
{
	CP_Image_Free(&canvas);
}

int main(void)
{
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
