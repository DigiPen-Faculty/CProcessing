//---------------------------------------------------------
// file:	gamestate_play.c
//
// brief:	The game: ten seconds to click as many targets as you can
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>
#include "gamestate_play.h"
#include "gamestate_menu.h"
#include "score.h"

static const float targetSize = 60.0f;
static float timeLeft;
static CP_Vector target;

static void MoveTarget(void)
{
	float half = targetSize / 2.0f;
	target.x = CP_Random_RangeFloat(half, CP_System_GetWindowWidth() - half);
	target.y = CP_Random_RangeFloat(80.0f + half, CP_System_GetWindowHeight() - half);
}

void gamestate_play_init(void)
{
	SetScore(0);
	timeLeft = 10.0f;
	MoveTarget();
}

void gamestate_play_update(void)
{
	timeLeft -= CP_System_GetDt();
	if (timeLeft <= 0.0f)
	{
		CP_Engine_SetNextGameState(gamestate_menu_init, gamestate_menu_update, gamestate_menu_exit);
	}

	CP_Vector mouse = CP_Vector_Set(CP_Input_GetMouseX(), CP_Input_GetMouseY());
	if (CP_Input_MouseTriggered(CP_MOUSE_LEFT) && CP_Vector_Distance(mouse, target) <= targetSize / 2.0f)
	{
		IncrementScore();
		MoveTarget();
	}

	CP_Graphics_ClearBackground(CP_Color_Create(30, 70, 40, 255));

	CP_Settings_Fill(CP_Color_Create(230, 80, 60, 255));
	CP_Graphics_DrawCircle(target.x, target.y, targetSize);

	char text[50] = { 0 };
	snprintf(text, sizeof(text), "Score: %d   Time: %.1f", GetScore(), timeLeft);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_TextSize(28.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
	CP_Font_DrawText(text, 20.0f, 20.0f);
}

void gamestate_play_exit(void)
{
}
