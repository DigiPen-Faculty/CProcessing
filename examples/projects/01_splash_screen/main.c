//---------------------------------------------------------
// file:	main.c
//
// Project 1: A splash screen.
// https://github.com/DigiPen-Faculty/CProcessing/wiki/Project-01-Splash-Screen
//
// This is the finished program from the project: the DigiPen logo fades in,
// holds still for two seconds, then slides up and shrinks.
//
// To run it, copy this file over the template's main.c.
//
// Try this:
// - Change the background from one color to another with CP_Color_Lerp.
// - Fade the logo out as it moves.
// - Move on to another game state when the splash is over.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"

CP_Image logo;
float timer;

// 0 until startTime, then grows to 1 at endTime, and stays at 1
float Progress(float startTime, float endTime)
{
	return CP_Math_ClampFloat((timer - startTime) / (endTime - startTime), 0.0f, 1.0f);
}

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	logo = CP_Image_Load("assets/DigiPen_WHITE.png");
	timer = 0.0f;
}

void game_update(void)
{
	timer += CP_System_GetDt();

	CP_Graphics_ClearBackground(CP_Color_Create(20, 20, 30, 255));

	float centerX = CP_System_GetWindowWidth() / 2.0f;
	float centerY = CP_System_GetWindowHeight() / 2.0f;

	// Fade in during the first second, then stay solid
	int alpha = (int)(255.0f * Progress(0.0f, 1.0f));

	// After the hold, from second 3 to 4.5: slide up and shrink
	float move = Progress(3.0f, 4.5f);
	float y = CP_Math_LerpFloat(centerY, 120.0f, move);
	float size = CP_Math_LerpFloat(0.5f, 0.25f, move);

	float width = (float)CP_Image_GetWidth(logo) * size;
	float height = (float)CP_Image_GetHeight(logo) * size;
	CP_Image_Draw(logo, centerX, y, width, height, alpha);
}

void game_exit(void)
{
	CP_Image_Free(&logo);
}

int main(void)
{
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
