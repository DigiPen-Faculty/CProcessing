//---------------------------------------------------------
// file:	score_and_text.c
//
// Example: a score, a timer and other text on the screen.
//
// Shows how to:
// - put numbers into text with snprintf
// - place text by its left, center or right edge with
//   CP_Settings_TextAlignment, to fill the corners of the window
// - change the text size with CP_Settings_TextSize
// - wrap a long line onto several lines with CP_Font_DrawTextBox
// - fade text out by lowering its color's alpha
// - make a score count up to its new value with CP_Math_LerpInt, so the
//   points roll in
//
// The game: click as many targets as you can in 20 seconds.
//
// Try this:
// - Give more points for a target clicked quickly.
// - Show the best three scores instead of only the best.
//
// See also: fonts.c, for loading a font and measuring text;
// text_effects.c, for glowing text and dialog boxes; text_input.c, for
// typing.
//
// To run it, copy this file over the template's main.c.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

const float roundLength = 20.0f; // seconds
const float targetSize = 60.0f;
const int targetPoints = 100;

int score = 0;
int bestScore = 0;
float timeLeft = 0.0f;
CP_BOOL playing = FALSE;
CP_Vector target;

// The score shown rolls up to the real one in 0.4 seconds after a hit
int rollFrom = 0;      // the score shown when it started rolling
float rollTime = 0.0f; // seconds since then
const float rollLength = 0.4f;

// The "+1" that floats up from a target when it's hit, and fades out
CP_Vector popupPosition;
float popupAge = 10.0f; // seconds since the last hit; it shows for 0.6 seconds

// CP_Math_LerpInt gives the whole number part of the way from one to
// another: 0 gives the first, 1 the second, and past 1 stays at the second
int ShownScore(void)
{
	return CP_Math_LerpInt(rollFrom, score, rollTime / rollLength);
}

void MoveTarget(void)
{
	float half = targetSize / 2.0f;
	target.x = CP_Random_RangeFloat(half, CP_System_GetWindowWidth() - half);
	target.y = CP_Random_RangeFloat(80.0f + half, CP_System_GetWindowHeight() - half);
}

void StartRound(void)
{
	score = 0;
	rollFrom = 0;
	timeLeft = roundLength;
	playing = TRUE;
	MoveTarget();
}

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Score and text");
}

void game_update(void)
{
	float dt = CP_System_GetDt();
	CP_BOOL clicked = CP_Input_MouseTriggered(CP_MOUSE_LEFT);
	CP_Vector mouse = CP_Vector_Set(CP_Input_GetMouseX(), CP_Input_GetMouseY());

	if (playing)
	{
		timeLeft -= dt;
		if (timeLeft <= 0.0f)
		{
			timeLeft = 0.0f;
			playing = FALSE;
			if (score > bestScore)
			{
				bestScore = score;
			}
		}
		else if (clicked && CP_Vector_Distance(mouse, target) <= targetSize / 2.0f)
		{
			rollFrom = ShownScore(); // roll on from where it's got to
			rollTime = 0.0f;
			score += targetPoints;
			popupPosition = target;
			popupAge = 0.0f;
			MoveTarget();
		}
	}
	else if (clicked)
	{
		StartRound();
	}
	popupAge += dt;
	rollTime += dt;

	CP_Graphics_ClearBackground(CP_Color_Create(30, 30, 50, 255));
	CP_Color white = CP_Color_Create(255, 255, 255, 255);
	char text[80];

	if (playing)
	{
		// the target
		CP_Settings_NoStroke();
		CP_Settings_Fill(CP_Color_Create(230, 80, 60, 255));
		CP_Graphics_DrawCircle(target.x, target.y, targetSize);
		CP_Settings_Fill(white);
		CP_Graphics_DrawCircle(target.x, target.y, targetSize * 0.5f);

		// "+100", rising 60 pixels and fading out over 0.6 seconds
		if (popupAge < 0.6f)
		{
			float progress = popupAge / 0.6f; // 0 to 1
			CP_Settings_TextSize(32.0f);
			CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
			CP_Settings_Fill(CP_Color_Create(255, 230, 80, (int)(255 * (1.0f - progress))));
			snprintf(text, sizeof(text), "+%d", targetPoints);
			CP_Font_DrawText(text, popupPosition.x, popupPosition.y - 60.0f * progress);
		}
	}

	// The score in the top left corner: aligned by its left edge
	CP_Settings_TextSize(28.0f);
	CP_Settings_Fill(white);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
	snprintf(text, sizeof(text), "Score: %d", ShownScore());
	CP_Font_DrawText(text, 20.0f, 20.0f);

	// The time in the middle: aligned by its center
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_TOP);
	snprintf(text, sizeof(text), "%.1f", timeLeft);
	CP_Font_DrawText(text, CP_System_GetWindowWidth() / 2.0f, 20.0f);

	// The best score in the top right corner: aligned by its right edge, so
	// it stays inside the window however wide the number gets
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_RIGHT, CP_TEXT_ALIGN_V_TOP);
	snprintf(text, sizeof(text), "Best: %d", bestScore);
	CP_Font_DrawText(text, CP_System_GetWindowWidth() - 20.0f, 20.0f);

	if (!playing)
	{
		CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
		CP_Settings_TextSize(56.0f);
		CP_Font_DrawText(timeLeft <= 0.0f && score > 0 ? "Time's up!" : "Target practice", 400.0f, 200.0f);

		// A text box wraps the text onto as many lines as it needs to fit
		// the width, here 500 pixels starting at x = 150
		CP_Settings_TextSize(24.0f);
		CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_TOP);
		CP_Font_DrawTextBox("Click the targets as fast as you can. You have 20 seconds, and each target you hit is worth 100 points. Click anywhere to start.",
			150.0f, 300.0f, 500.0f);
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
