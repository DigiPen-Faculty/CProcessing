//---------------------------------------------------------
// file:	window_options.c
//
// Example: the window and frame rate options.
//
// Shows how to:
// - switch between fullscreen and a window, with CP_System_IsFullscreen to
//   know which one it is now
// - let the player resize the window with CP_System_SetWindowResizable, and
//   draw to fit whatever size it has
// - turn vsync on and off with CP_System_SetVSync
// - limit the frame rate with CP_System_SetFrameRate, and read the limit
//   back with CP_System_GetTargetFrameRate
// - read the fill color back with CP_Settings_GetFill, to put it back after
//   drawing something in another color
//
// Keys:
//   F - fullscreen or window       R - resizable or not (try dragging an edge)
//   V - vsync on or off            1, 2, 3 - frame rate limit 30, 60, none
//
// With vsync on, the frame rate is the monitor's. To see that, turn the frame
// rate limit off (3) first: vsync alone then sets the speed.
//
// Try this:
// - Keep the circle round, and as big as fits, when the window is not square.
// - Show the window size only while it is being resized.
//
// To run it, copy this file over the template's main.c.
//---------------------------------------------------------

#include "cprocessing.h"
#include <math.h>
#include <stdio.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// Draws a line of text in a color, then puts the fill color back as it was
void DrawLine(const char* text, CP_Color color, float x, float y)
{
	const CP_Color before = CP_Settings_GetFill();
	CP_Settings_Fill(color);
	CP_Font_DrawText(text, x, y);
	CP_Settings_Fill(before);
}

const char* OnOff(CP_BOOL value)
{
	return value ? "on" : "off";
}

void game_init(void)
{
	CP_System_SetWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	CP_System_SetWindowTitle("Window options");
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
	CP_Settings_TextSize(24.0f);
}

void game_update(void)
{
	if (CP_Input_KeyTriggered(KEY_F))
	{
		if (CP_System_IsFullscreen())
		{
			CP_System_SetWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
		}
		else
		{
			CP_System_Fullscreen();
		}
	}
	if (CP_Input_KeyTriggered(KEY_R))
	{
		CP_System_SetWindowResizable(!CP_System_GetWindowResizable());
	}
	if (CP_Input_KeyTriggered(KEY_V))
	{
		CP_System_SetVSync(!CP_System_GetVSync());
	}
	if (CP_Input_KeyTriggered(KEY_1))
	{
		CP_System_SetFrameRate(30.0f);
	}
	if (CP_Input_KeyTriggered(KEY_2))
	{
		CP_System_SetFrameRate(60.0f);
	}
	if (CP_Input_KeyTriggered(KEY_3))
	{
		CP_System_SetFrameRate(0.0f);		// 0: no limit
	}

	const float width = (float)CP_System_GetWindowWidth();
	const float height = (float)CP_System_GetWindowHeight();

	// Everything is placed from the window's size, so it fits when the size changes
	CP_Graphics_ClearBackground(CP_Color_Create(30, 30, 50, 255));
	CP_Settings_Stroke(CP_Color_Create(255, 200, 60, 255));
	CP_Settings_StrokeWeight(6.0f);
	CP_Settings_NoFill();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Graphics_DrawRect(3.0f, 3.0f, width - 6.0f, height - 6.0f);

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
	CP_Font_DrawText(text, 30.0f, 66.0f);

	const CP_Color note = CP_Color_Create(200, 200, 200, 255);
	snprintf(text, sizeof(text), "F  Fullscreen: %s", OnOff(CP_System_IsFullscreen()));
	DrawLine(text, note, 30.0f, 130.0f);
	snprintf(text, sizeof(text), "R  Resizable: %s", OnOff(CP_System_GetWindowResizable()));
	DrawLine(text, note, 30.0f, 166.0f);
	snprintf(text, sizeof(text), "V  Vsync: %s", OnOff(CP_System_GetVSync()));
	DrawLine(text, note, 30.0f, 202.0f);
	if (CP_System_GetTargetFrameRate() > 0.0f)
	{
		snprintf(text, sizeof(text), "1 2 3  Frame rate limit: %.0f", CP_System_GetTargetFrameRate());
	}
	else
	{
		snprintf(text, sizeof(text), "1 2 3  Frame rate limit: none");
	}
	DrawLine(text, note, 30.0f, 238.0f);
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
