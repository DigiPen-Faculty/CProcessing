//---------------------------------------------------------
// file:	game_states.c
//
// Example: a game with a menu, play, pause and game over.
//
// Shows how to:
// - give each part of the game its own init, update and exit functions
// - switch between them with CP_Engine_SetNextGameState
// - restart the current state with CP_Engine_SetNextGameStateForced
// - pause by pushing a state on top of the game (CP_Engine_PushGameState):
//   the game freezes behind it, and CP_Engine_PopGameState lets it carry on
// - keep values, such as the best score, across states
//
// Examples/state_stack.c goes further: menus on top of menus, a state that
// keeps running under another, transitions, and drawing into an image.
//
// The game: dodge the falling blocks with the mouse for as long as you can.
//
// Try this:
// - Add a "How to play" state, reached from the menu with H.
// - Make the blocks fall faster the longer you survive.
//
// To run it, copy this file over the template's main.c.
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

// Each state is three functions. They're declared here so any state can
// switch to any other, whatever order they're written in below.
void menu_init(void);
void menu_update(void);
void menu_exit(void);
void play_init(void);
void play_update(void);
void play_exit(void);
void pause_update(void);
void game_over_init(void);
void game_over_update(void);
void game_over_exit(void);

// Values every state can use. A state's own values are set again in its
// init function each time the state starts.
float lastTime = 0.0f;		// how long the last game lasted, in seconds
float bestTime = 0.0f;

CP_Color background;
CP_Color white;

// Text in the middle of the window, at a height
void DrawCenteredText(const char* text, float y, float size)
{
	CP_Settings_TextSize(size);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Settings_Fill(white);
	CP_Font_DrawText(text, CP_System_GetWindowWidth() / 2.0f, y);
}

//-------------------------
// MENU

void menu_init(void)
{
}

void menu_update(void)
{
	CP_Graphics_ClearBackground(background);
	DrawCenteredText("DODGE", 200.0f, 80.0f);
	DrawCenteredText("Move the mouse to dodge the falling blocks", 300.0f, 24.0f);
	DrawCenteredText("Click or press Enter to play", 380.0f, 24.0f);

	if (CP_Input_MouseTriggered(MOUSE_BUTTON_LEFT) || CP_Input_KeyTriggered(KEY_ENTER))
	{
		// The switch happens at the start of the next frame: menu_exit runs,
		// then play_init, then play_update every frame
		CP_Engine_SetNextGameState(play_init, play_update, play_exit);
	}
}

void menu_exit(void)
{
}

//-------------------------
// PLAY

#define BLOCK_COUNT 8

CP_Vector blocks[BLOCK_COUNT];
float blockSpeeds[BLOCK_COUNT];		// pixels per second
const float blockSize = 40.0f;
const float playerSize = 30.0f;
float survivedTime;

// Put a block back above the top of the window, somewhere across it
void ResetBlock(int i)
{
	blocks[i] = CP_Vector_Set(CP_Random_RangeFloat(20.0f, CP_System_GetWindowWidth() - 20.0f), CP_Random_RangeFloat(-600.0f, -blockSize));
	blockSpeeds[i] = CP_Random_RangeFloat(150.0f, 350.0f);
}

void play_init(void)
{
	// Everything starts fresh each time play starts, including a restart
	for (int i = 0; i < BLOCK_COUNT; ++i)
	{
		ResetBlock(i);
	}
	survivedTime = 0.0f;
}

void play_update(void)
{
	// P pauses: the pause state goes on top of this one. play_exit doesn't
	// run, this state stops and stays visible behind the pause, and when the
	// pause pops, it carries on where it was (play_init doesn't run again).
	if (CP_Input_KeyTriggered(KEY_P))
	{
		CP_Engine_PushGameState(NULL, pause_update, NULL);
	}

	// R starts over. The next state is the current one, so
	// CP_Engine_SetNextGameState would do nothing; the Forced version runs
	// play_exit and play_init anyway.
	if (CP_Input_KeyTriggered(KEY_R))
	{
		CP_Engine_SetNextGameStateForced(play_init, play_update, play_exit);
	}

	CP_Vector player = CP_Vector_Set(CP_Input_GetMouseX(), CP_System_GetWindowHeight() - 60.0f);

	float dt = CP_System_GetDt();
	survivedTime += dt;
	for (int i = 0; i < BLOCK_COUNT; ++i)
	{
		blocks[i].y += blockSpeeds[i] * dt;
		if (blocks[i].y > CP_System_GetWindowHeight() + blockSize)
		{
			ResetBlock(i);
		}

		// Hit? The block and the player overlap across and down.
		float reach = (blockSize + playerSize) / 2.0f;
		float dx = blocks[i].x - player.x;
		float dy = blocks[i].y - player.y;
		if (dx < reach && dx > -reach && dy < reach && dy > -reach)
		{
			lastTime = survivedTime;
			CP_Engine_SetNextGameState(game_over_init, game_over_update, game_over_exit);
		}
	}

	CP_Graphics_ClearBackground(background);
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(230, 80, 60, 255));
	for (int i = 0; i < BLOCK_COUNT; ++i)
	{
		CP_Graphics_DrawRect(blocks[i].x, blocks[i].y, blockSize, blockSize);
	}
	CP_Settings_Fill(CP_Color_Create(80, 200, 255, 255));
	CP_Graphics_DrawRect(player.x, player.y, playerSize, playerSize);

	char text[60];
	snprintf(text, sizeof(text), "%.1f seconds", survivedTime);
	DrawCenteredText(text, 30.0f, 24.0f);
}

void play_exit(void)
{
}

//-------------------------
// PAUSE: on top of the play state, which is frozen behind it

void pause_update(void)
{
	// A state pushed on top draws on a clear sheet over the one below.
	// Clearing it with a see-through color darkens the frozen game.
	CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 150));
	DrawCenteredText("PAUSED", 250.0f, 60.0f);
	DrawCenteredText("P to keep playing, R to start over", 320.0f, 24.0f);

	if (CP_Input_KeyTriggered(KEY_P))
	{
		CP_Engine_PopGameState();
	}
	if (CP_Input_KeyTriggered(KEY_R))
	{
		// Ends the pause and the game under it, and starts a new game
		CP_Engine_SetNextGameState(play_init, play_update, play_exit);
	}
}

//-------------------------
// GAME OVER

CP_BOOL newBest;

void game_over_init(void)
{
	newBest = lastTime > bestTime;
	if (newBest)
	{
		bestTime = lastTime;
	}
}

void game_over_update(void)
{
	CP_Graphics_ClearBackground(background);

	char text[60];
	DrawCenteredText("GAME OVER", 200.0f, 70.0f);
	snprintf(text, sizeof(text), "You lasted %.1f seconds", lastTime);
	DrawCenteredText(text, 290.0f, 28.0f);
	if (newBest)
	{
		DrawCenteredText("A new best!", 330.0f, 24.0f);
	}
	else
	{
		snprintf(text, sizeof(text), "Best: %.1f seconds", bestTime);
		DrawCenteredText(text, 330.0f, 24.0f);
	}
	DrawCenteredText("Enter to play again, M for the menu", 420.0f, 24.0f);

	if (CP_Input_KeyTriggered(KEY_ENTER))
	{
		CP_Engine_SetNextGameState(play_init, play_update, play_exit);
	}
	if (CP_Input_KeyTriggered(KEY_M))
	{
		CP_Engine_SetNextGameState(menu_init, menu_update, menu_exit);
	}
}

void game_over_exit(void)
{
}

//-------------------------

int main(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Game states");
	background = CP_Color_Create(30, 30, 50, 255);
	white = CP_Color_Create(255, 255, 255, 255);

	// The game starts at the menu
	CP_Engine_SetNextGameState(menu_init, menu_update, menu_exit);
	CP_Engine_Run();
	return 0;
}
