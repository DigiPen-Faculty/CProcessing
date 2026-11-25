//---------------------------------------------------------
// file:	main.c
//
// Project 4: Drawing with functions.
// https://github.com/DigiPen-Faculty/CProcessing/wiki/Project-04-Drawing-with-Functions
//
// This is the finished program from the project: three faces drawn by one
// function, DrawFace, each in its own place, and the middle one with a
// wobbling mouth.
//
// To run it, copy this file over the template's main.c.
//
// Try this:
// - Give DrawFace a parameter for the head color.
// - Give it a size, and place the eyes and mouth from it.
// - Draw a row of faces with a loop.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include <math.h>

// Draws a face with its middle at (x, y), and its mouth turned by mouthTilt degrees
void DrawFace(float x, float y, float mouthTilt)
{
	// The head
	CP_Settings_Fill(CP_Color_Create(255, 210, 60, 255));
	CP_Graphics_DrawCircle(x, y, 160.0f);

	// The mouth, turned around its own middle
	CP_Settings_Fill(CP_Color_Create(150, 30, 30, 255));
	CP_Settings_Save();
	CP_Settings_Translate(x, y + 45.0f);
	CP_Settings_Rotate(mouthTilt);
	CP_Graphics_DrawRoundedRect(0.0f, 0.0f, 70.0f, 14.0f, 7.0f);
	CP_Settings_Restore();

	// The eyes
	CP_Settings_Fill(CP_Color_Create(30, 30, 30, 255));
	CP_Graphics_DrawCircle(x - 28.0f, y - 22.0f, 22.0f);
	CP_Graphics_DrawCircle(x + 28.0f, y - 22.0f, 22.0f);
}

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
}

void game_update(void)
{
	CP_Graphics_ClearBackground(CP_Color_Create(255, 255, 255, 255));

	float wobble = sinf(CP_System_GetSeconds() * 2.0f) * 20.0f;

	DrawFace(150.0f, 300.0f, -20.0f);
	DrawFace(400.0f, 300.0f, wobble);
	DrawFace(650.0f, 300.0f, 20.0f);
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
