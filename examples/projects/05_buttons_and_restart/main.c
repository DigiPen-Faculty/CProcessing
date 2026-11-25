//---------------------------------------------------------
// file:	main.c
//
// Project 5: Buttons and restart.
// https://github.com/DigiPen-Faculty/CProcessing/wiki/Project-05-Buttons-and-Restart
//
// This is the finished program from the project: a ball that moves, a score,
// an "Add point" button that scores, and a "Restart" button that starts the
// game state over.
//
//   Click a button   it acts when the mouse button is let go over it
//
// To run it, copy this file over the template's main.c.
//
// Try this:
// - Move the "Add point" button to a random place each time it's clicked.
// - Show "You win!" and only the Restart button once the score is 10.
// - Add a Quit button that calls CP_Engine_Terminate.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

void game_exit(void); // a prototype: game_update uses game_exit before it is written

int score;
float ballX;

// True if the mouse is inside the rectangle with its middle at (x, y)
bool MouseIsInside(float x, float y, float width, float height)
{
	float mouseX = CP_Input_GetMouseX();
	float mouseY = CP_Input_GetMouseY();
	bool insideX = mouseX > x - width / 2.0f && mouseX < x + width / 2.0f;
	bool insideY = mouseY > y - height / 2.0f && mouseY < y + height / 2.0f;
	return insideX && insideY;
}

// Draws a button with its middle at (x, y). Returns true on the frame it's clicked
bool Button(const char* label, float x, float y, float width, float height)
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

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	score = 0;
	ballX = 100.0f;
}

void game_update(void)
{
	ballX += 100.0f * CP_System_GetDt(); // 100 pixels per second

	CP_Graphics_ClearBackground(CP_Color_Create(30, 30, 50, 255));

	CP_Settings_Fill(CP_Color_Create(255, 210, 60, 255));
	CP_Graphics_DrawCircle(ballX, 150.0f, 60.0f);

	char text[50] = { 0 };
	snprintf(text, sizeof(text), "Score: %d", score);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_TextSize(28.0f);
	CP_Font_DrawText(text, 20.0f, 40.0f);

	if (Button("Add point", 250.0f, 400.0f, 200.0f, 70.0f))
	{
		score++;
	}

	if (Button("Restart", 550.0f, 400.0f, 200.0f, 70.0f))
	{
		CP_Engine_SetNextGameStateForced(game_init, game_update, game_exit);
	}
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
