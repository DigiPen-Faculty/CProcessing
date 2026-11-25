//---------------------------------------------------------
// file:	main.c
//
// Project 3: Turning and rounding shapes.
// https://github.com/DigiPen-Faculty/CProcessing/wiki/Project-03-Turning-and-Rounding-Shapes
//
// This is the finished program from the project: a face whose rounded mouth
// has a tilt chosen once at the start, and a wobble that changes with time.
//
// To run it, copy this file over the template's main.c.
//
// Try this:
// - Make the eyes blink by changing their height over time.
// - Open and close the mouth with the wobble instead of turning it.
// - Make the face follow the mouse.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include <math.h>

float tilt;

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	tilt = CP_Random_RangeFloat(-15.0f, 15.0f);
}

void game_update(void)
{
	CP_Graphics_ClearBackground(CP_Color_Create(255, 255, 255, 255));

	// The head
	CP_Settings_Fill(CP_Color_Create(255, 210, 60, 255));
	CP_Graphics_DrawCircle(400.0f, 300.0f, 300.0f);

	// The mouth: the tilt chosen once, and a wobble that changes with time
	float wobble = sinf(CP_System_GetSeconds() * 2.0f) * 20.0f;

	CP_Settings_Fill(CP_Color_Create(150, 30, 30, 255));
	CP_Settings_Save();
	CP_Settings_Translate(400.0f, 370.0f);
	CP_Settings_Rotate(tilt + wobble);
	CP_Graphics_DrawRoundedRect(0.0f, 0.0f, 120.0f, 24.0f, 12.0f);
	CP_Settings_Restore();

	// The eyes
	CP_Settings_Fill(CP_Color_Create(30, 30, 30, 255));
	CP_Graphics_DrawCircle(350.0f, 260.0f, 40.0f);
	CP_Graphics_DrawCircle(450.0f, 260.0f, 40.0f);
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
