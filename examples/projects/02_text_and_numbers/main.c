//---------------------------------------------------------
// file:	main.c
//
// Project 2: Text and numbers.
// https://github.com/DigiPen-Faculty/CProcessing/wiki/Project-02-Text-and-Numbers
//
// This is the finished program from the project: a click counter, a clock
// and a frame count, each placed against a different edge of the window.
//
//   Click anywhere   add one to the click counter
//
// To run it, copy this file over the template's main.c.
//
// Try this:
// - Count down from 30 seconds instead of up.
// - Show the mouse position next to the mouse.
// - Draw a title, and a smaller line just under it.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

int clicks;

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	clicks = 0;
}

void game_update(void)
{
	if (CP_Input_MouseTriggered(CP_MOUSE_LEFT))
	{
		clicks++;
	}

	CP_Graphics_ClearBackground(CP_Color_Create(30, 30, 50, 255));
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));

	float centerX = CP_System_GetWindowWidth() / 2.0f;
	float centerY = CP_System_GetWindowHeight() / 2.0f;
	float rightX = CP_System_GetWindowWidth() - 20.0f;
	char text[50] = { 0 };

	CP_Settings_TextSize(28.0f);

	// Top left: placed by its left end
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
	snprintf(text, sizeof(text), "Clicks: %d", clicks);
	CP_Font_DrawText(text, 20.0f, 20.0f);

	// Top middle: placed by its middle
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_TOP);
	snprintf(text, sizeof(text), "Time: %.1f", CP_System_GetSeconds());
	CP_Font_DrawText(text, centerX, 20.0f);

	// Top right: placed by its right end, 20 pixels from the edge
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_RIGHT, CP_TEXT_ALIGN_V_TOP);
	snprintf(text, sizeof(text), "Frame: %u", CP_System_GetFrameCount());
	CP_Font_DrawText(text, rightX, 20.0f);

	// A message in the middle of the window
	CP_Settings_TextSize(48.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Font_DrawText("Click anywhere", centerX, centerY);
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
