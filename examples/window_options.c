//---------------------------------------------------------
// file:	window_options.c
//
// Example: the window, the display and frame rate options.
//
// Shows how to:
// - switch between fullscreen and a window, with CP_System_IsFullscreen to
//   know which one it is now
// - read the display's size and refresh rate with CP_System_GetDisplayWidth,
//   CP_System_GetDisplayHeight and CP_System_GetDisplayRefreshRate
// - put the window in the middle of the display with
//   CP_System_SetWindowPosition
// - let the player resize the window with CP_System_SetWindowResizable, and
//   draw to fit whatever size it has
// - turn vsync on and off with CP_System_SetVSync
// - limit the frame rate with CP_System_SetFrameRate, and read the limit
//   back with CP_System_GetTargetFrameRate
// - hide the mouse cursor, keep it in the window, or free it for mouse look
//   with CP_System_SetCursorMode, or only show and hide it with
//   CP_System_ShowCursor and CP_System_GetCursorVisible
// - count frames with CP_System_GetFrameCount, and read the time since the
//   program started in milliseconds with CP_System_GetMillis
// - show a console for printf with CP_System_ShowConsole (on Windows, a
//   game built as a window app starts without one)
// - read the fill and stroke back with CP_Settings_GetFill,
//   CP_Settings_GetStroke and CP_Settings_GetStrokeWeight, to put them back
//   after drawing something else
//
// Keys:
//   F - fullscreen or window       R - resizable or not (try dragging an edge)
//   P - the window to the middle of the display
//   V - vsync on or off            1, 2, 3 - frame rate limit 30, 60, none
//   M - the mouse cursor mode      H - the cursor shown or hidden
//   C - the console on or off
//
// With vsync on, the frame rate is the monitor's. To see that, turn the frame
// rate limit off (3) first: vsync alone then sets the speed.
//
// Try this:
// - Keep the circle round, and as big as fits, when the window is not square.
// - Show the window size only while it is being resized.
// - Start the game with its window in the middle of the display, from
//   game_init.
// - In the disabled cursor mode, turn a view with CP_Input_GetMouseDeltaX,
//   the way a first person game looks around.
//
// See also: camera.c, for drawing a world bigger than the window.
//
// To run it, copy this file over the template's main.c.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include <math.h>
#include <stdio.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

// Draws a line of text in a color, then puts the fill color back as it was
void DrawLine(const char* text, CP_Color color, float x, float y)
{
	const CP_Color before = CP_Settings_GetFill();
	CP_Settings_Fill(color);
	CP_Font_DrawText(text, x, y);
	CP_Settings_Fill(before);
}

// Draws a frame 6 pixels wide around the window, then puts the stroke color
// and weight back as they were. Each line goes all the way across, so the
// corners are filled.
void DrawFrame(float width, float height, CP_Color color)
{
	const CP_Color strokeBefore = CP_Settings_GetStroke();
	const float weightBefore = CP_Settings_GetStrokeWeight();
	CP_Settings_Stroke(color);
	CP_Settings_StrokeWeight(6.0f);
	CP_Graphics_DrawLine(0.0f, 3.0f, width, 3.0f);
	CP_Graphics_DrawLine(0.0f, height - 3.0f, width, height - 3.0f);
	CP_Graphics_DrawLine(3.0f, 0.0f, 3.0f, height);
	CP_Graphics_DrawLine(width - 3.0f, 0.0f, width - 3.0f, height);
	CP_Settings_Stroke(strokeBefore);
	CP_Settings_StrokeWeight(weightBefore);
}

const char* OnOff(CP_BOOL value)
{
	return value ? "on" : "off";
}

// The cursor modes, in the order of the CP_CURSOR_MODE values
const char* cursorModeNames[] = {
	"normal",
	"hidden over the window",
	"kept in the window (not on macOS)",
	"disabled: hidden, no limits, for mouse look",
};

void game_init(void)
{
	CP_System_SetWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	CP_System_SetWindowTitle("Window options");
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
	CP_Settings_TextSize(22.0f);
}

void game_update(void)
{
	if (CP_Input_KeyTriggered(CP_KEY_F))
	{
		if (CP_System_IsFullscreen())
		{
			CP_System_SetWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
		}
		else
		{
			// Fullscreen at the display's own size. For a game drawn at a
			// lower resolution, draw it into an image and stretch that to
			// fit, rather than switching the monitor to another size (the
			// wiki's CP_System_FullscreenAdvanced shows how, and why).
			CP_System_Fullscreen();
		}
	}
	if (CP_Input_KeyTriggered(CP_KEY_R))
	{
		CP_System_SetWindowResizable(!CP_System_GetWindowResizable());
	}
	if (CP_Input_KeyTriggered(CP_KEY_P) && !CP_System_IsFullscreen())
	{
		// The window's top left corner, so its middle is the display's middle
		int x = (CP_System_GetDisplayWidth() - CP_System_GetWindowWidth()) / 2;
		int y = (CP_System_GetDisplayHeight() - CP_System_GetWindowHeight()) / 2;
		CP_System_SetWindowPosition(x, y);
	}
	if (CP_Input_KeyTriggered(CP_KEY_V))
	{
		CP_System_SetVSync(!CP_System_GetVSync());
	}
	if (CP_Input_KeyTriggered(CP_KEY_1))
	{
		CP_System_SetFrameRate(30.0f);
	}
	if (CP_Input_KeyTriggered(CP_KEY_2))
	{
		CP_System_SetFrameRate(60.0f);
	}
	if (CP_Input_KeyTriggered(CP_KEY_3))
	{
		CP_System_SetFrameRate(0.0f); // 0: no limit
	}
	if (CP_Input_KeyTriggered(CP_KEY_M))
	{
		// the next of the 4 modes, back to the first after the last
		CP_CURSOR_MODE mode = (CP_CURSOR_MODE)(((int)CP_System_GetCursorMode() + 1) % 4);
		CP_System_SetCursorMode(mode);
		printf("Cursor mode: %s\n", cursorModeNames[mode]);
	}
	if (CP_Input_KeyTriggered(CP_KEY_H))
	{
		// Only normal and hidden: it sets the cursor mode to one of them
		CP_System_ShowCursor(!CP_System_GetCursorVisible());
	}
	if (CP_Input_KeyTriggered(CP_KEY_C))
	{
		CP_System_ShowConsole(!CP_System_GetConsoleVisible());
		printf("printf writes here. Press C again to hide the console.\n");
	}

	const float width = (float)CP_System_GetWindowWidth();
	const float height = (float)CP_System_GetWindowHeight();

	// Everything is placed from the window's size, so it fits when the size changes
	CP_Graphics_ClearBackground(CP_Color_Create(30, 30, 50, 255));
	DrawFrame(width, height, CP_Color_Create(255, 200, 60, 255));

	// A ball that crosses the window once every two seconds, then starts again
	const float progress = fmodf(CP_System_GetSeconds() * 0.5f, 1.0f);
	const float across = CP_Math_Map(progress, 0.0f, 1.0f, 60.0f, width - 60.0f);
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(90, 200, 255, 255));
	CP_Graphics_DrawCircle(across, height - 80.0f, 60.0f);

	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	char text[100];
	snprintf(text, sizeof(text), "Window: %d x %d%s", CP_System_GetWindowWidth(), CP_System_GetWindowHeight(),
		CP_System_IsFullscreen() ? " (fullscreen)" : "");
	CP_Font_DrawText(text, 30.0f, 30.0f);
	snprintf(text, sizeof(text), "Frames per second: %.0f", CP_System_GetFrameRate());
	CP_Font_DrawText(text, 30.0f, 62.0f);
	snprintf(text, sizeof(text), "Display: %d x %d at %d Hz", CP_System_GetDisplayWidth(), CP_System_GetDisplayHeight(),
		CP_System_GetDisplayRefreshRate());
	CP_Font_DrawText(text, 30.0f, 94.0f);
	snprintf(text, sizeof(text), "Frame %u, %.0f milliseconds since the start", CP_System_GetFrameCount(), CP_System_GetMillis());
	CP_Font_DrawText(text, 30.0f, 126.0f);

	const CP_Color note = CP_Color_Create(200, 200, 200, 255);
	float y = 180.0f;
	snprintf(text, sizeof(text), "F  Fullscreen: %s", OnOff(CP_System_IsFullscreen()));
	DrawLine(text, note, 30.0f, y);
	y += 30.0f;
	snprintf(text, sizeof(text), "R  Resizable: %s", OnOff(CP_System_GetWindowResizable()));
	DrawLine(text, note, 30.0f, y);
	y += 30.0f;
	DrawLine("P  The window to the middle of the display", note, 30.0f, y);
	y += 30.0f;
	snprintf(text, sizeof(text), "V  Vsync: %s", OnOff(CP_System_GetVSync()));
	DrawLine(text, note, 30.0f, y);
	y += 30.0f;
	if (CP_System_GetTargetFrameRate() > 0.0f)
	{
		snprintf(text, sizeof(text), "1 2 3  Frame rate limit: %.0f", CP_System_GetTargetFrameRate());
	}
	else
	{
		snprintf(text, sizeof(text), "1 2 3  Frame rate limit: none");
	}
	DrawLine(text, note, 30.0f, y);
	y += 30.0f;
	snprintf(text, sizeof(text), "M  Mouse cursor: %s", cursorModeNames[CP_System_GetCursorMode()]);
	DrawLine(text, note, 30.0f, y);
	y += 30.0f;
	snprintf(text, sizeof(text), "H  Cursor shown: %s", CP_System_GetCursorVisible() ? "yes" : "no");
	DrawLine(text, note, 30.0f, y);
	y += 30.0f;
	snprintf(text, sizeof(text), "C  Console: %s", OnOff(CP_System_GetConsoleVisible()));
	DrawLine(text, note, 30.0f, y);
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
