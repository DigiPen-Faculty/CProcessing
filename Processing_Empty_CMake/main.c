//---------------------------------------------------------
// file:	main.c
// author:	[YOUR NAME]
//
// brief:	[WHAT YOUR GAME OR PROGRAM DOES]
//
// CProcessing documentation:
// https://github.com/DigiPen-Faculty/CProcessing/wiki
//---------------------------------------------------------

// This is a complete program. Build and run it, and a circle follows the
// mouse. Then make it your own: change the colors, the size, the window.
//
// main(), at the bottom, tells CProcessing which three functions make up
// the game. CProcessing then calls them for you:
//   game_init    once, when the game starts
//   game_update  every frame, about 60 times a second
//   game_exit    once, when the game ends
// "Your first program" in the documentation explains it step by step.

#include "cprocessing.h"

void game_init(void)
{
	// Set up the window, and anything else the game needs before it starts
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("My Game");
}

void game_update(void)
{
	// Paint the whole window dark grey: this erases the last frame
	CP_Graphics_ClearBackground(CP_Color_Create(40, 40, 40, 255));

	// Orange shapes from here on (red, green, blue, opacity: each 0 to 255)
	CP_Settings_Fill(CP_Color_Create(255, 160, 0, 255));

	// A circle 50 pixels across, wherever the mouse is
	CP_Graphics_DrawCircle(CP_Input_GetMouseX(), CP_Input_GetMouseY(), 50.0f);
}

void game_exit(void)
{
	// Free anything the game loaded, such as images and sounds
}

int main(void)
{
	// To see printf output, uncomment this line. On Windows it opens a
	// console window. On Linux and macOS, run the game from a terminal.
	// CP_System_ShowConsole(TRUE);

	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
