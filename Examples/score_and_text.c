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
//
// The game: click as many targets as you can in 20 seconds.
//
// Try this:
// - Give more points for a target clicked quickly.
// - Show the best three scores instead of only the best.
//
// To run it, copy this file over the template's main.c.
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

const float roundLength = 20.0f;	// seconds
const float targetSize = 60.0f;

int score = 0;
int bestScore = 0;
float timeLeft = 0.0f;
CP_BOOL playing = FALSE;
CP_Vector target;

// The "+1" that floats up from a target when it's hit, and fades out
CP_Vector popupPosition;
float popupAge = 10.0f;		// seconds since the last hit; it shows for 0.6 seconds

void MoveTarget(void)
{
	float half = targetSize / 2.0f;
	target.x = CP_Random_RangeFloat(half, CP_System_GetWindowWidth() - half);
	target.y = CP_Random_RangeFloat(80.0f + half, CP_System_GetWindowHeight() - half);
}

void StartRound(void)
{
	score = 0;
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
	CP_BOOL clicked = CP_Input_MouseTriggered(MOUSE_BUTTON_LEFT);
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
			++score;
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

	CP_Graphics_ClearBackground(CP_Color_Create(30, 30, 50, 255));
	CP_Color white = CP_Color_Create(255, 255, 255, 255);

	if (playing)
	{
		// the target
		CP_Settings_NoStroke();
		CP_Settings_Fill(CP_Color_Create(230, 80, 60, 255));
		CP_Graphics_DrawCircle(target.x, target.y, targetSize);
		CP_Settings_Fill(white);
		CP_Graphics_DrawCircle(target.x, target.y, targetSize * 0.5f);

		// "+1", rising 60 pixels and fading out over 0.6 seconds
		if (popupAge < 0.6f)
		{
			float progress = popupAge / 0.6f;	// 0 to 1
			CP_Settings_TextSize(32.0f);
			CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
			CP_Settings_Fill(CP_Color_Create(255, 230, 80, (int)(255 * (1.0f - progress))));
			CP_Font_DrawText("+1", popupPosition.x, popupPosition.y - 60.0f * progress);
		}
	}

	// The score in the top left corner: aligned by its left edge
	char text[80];
	CP_Settings_TextSize(28.0f);
	CP_Settings_Fill(white);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
	snprintf(text, sizeof(text), "Score: %d", score);
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
		CP_Font_DrawTextBox("Click the targets as fast as you can. You have 20 seconds, and each target you hit is worth one point. Click anywhere to start.",
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
