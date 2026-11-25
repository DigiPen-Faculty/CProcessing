//---------------------------------------------------------
// file:	gamestate_splash.c
//
// brief:	The splash screen: the game's name, for two seconds
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include "gamestate_splash.h"
#include "gamestate_menu.h"

void gamestate_splash_init(void)
{
}

void gamestate_splash_update(void)
{
	CP_Graphics_ClearBackground(CP_Color_Create(20, 20, 30, 255));

	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_TextSize(60.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Font_DrawText("My Game", CP_System_GetWindowWidth() / 2.0f, CP_System_GetWindowHeight() / 2.0f);

	// After two seconds in this state, go to the menu
	if (CP_Engine_GetGameStateTime() > 2.0f)
	{
		CP_Engine_SetNextGameState(gamestate_menu_init, gamestate_menu_update, gamestate_menu_exit);
	}
}

void gamestate_splash_exit(void)
{
}
