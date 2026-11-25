//---------------------------------------------------------
// file:	gamestate_menu.c
//
// brief:	The menu: the last score, and Play and Quit buttons
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>
#include "gamestate_menu.h"
#include "gamestate_play.h"
#include "score.h"

// True if the mouse is inside the rectangle with its middle at (x, y)
static bool MouseIsInside(float x, float y, float width, float height)
{
	float mouseX = CP_Input_GetMouseX();
	float mouseY = CP_Input_GetMouseY();
	bool insideX = mouseX > x - width / 2.0f && mouseX < x + width / 2.0f;
	bool insideY = mouseY > y - height / 2.0f && mouseY < y + height / 2.0f;
	return insideX && insideY;
}

// Draws a button with its middle at (x, y). Returns true on the frame it's clicked
static bool Button(const char* label, float x, float y, float width, float height)
{
	bool over = MouseIsInside(x, y, width, height);

	CP_Settings_Save();
	if (over)
	{
		CP_Settings_Fill(CP_Color_Create(110, 160, 250, 255));
	}
	else
	{
		CP_Settings_Fill(CP_Color_Create(70, 120, 210, 255));
	}
	CP_Graphics_DrawRoundedRect(x, y, width, height, 12.0f);

	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_TextSize(24.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Font_DrawText(label, x, y);
	CP_Settings_Restore();

	return over && CP_Input_MouseClicked();
}

void gamestate_menu_init(void)
{
}

void gamestate_menu_update(void)
{
	CP_Graphics_ClearBackground(CP_Color_Create(30, 30, 50, 255));

	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Settings_TextSize(60.0f);
	CP_Font_DrawText("My Game", 400.0f, 120.0f);

	char text[50] = { 0 };
	CP_Settings_TextSize(24.0f);
	snprintf(text, sizeof(text), "Last score: %d", GetScore());
	CP_Font_DrawText(text, 400.0f, 190.0f);

	if (Button("Play", 400.0f, 300.0f, 200.0f, 70.0f))
	{
		CP_Engine_SetNextGameState(gamestate_play_init, gamestate_play_update, gamestate_play_exit);
	}

	if (Button("Quit", 400.0f, 400.0f, 200.0f, 70.0f))
	{
		CP_Engine_Terminate();
	}
}

void gamestate_menu_exit(void)
{
}
