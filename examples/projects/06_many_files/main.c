//---------------------------------------------------------
// file:	main.c
//
// Project 6: A game in many files.
// https://github.com/DigiPen-Faculty/CProcessing/wiki/Project-06-A-Game-in-Many-Files
//
// This is the finished program from the project: three game states, each in
// its own .c and .h file. A splash moves on to the menu after two seconds;
// the menu has Play and Quit buttons and shows the last score; the game
// gives ten seconds to click targets. score.c keeps the score that the game
// and the menu share.
//
//   Click a target   score a point (in the game)
//   Escape           quit, from any state
//
// To run it, copy all nine files next to the template's main.c (replacing
// it), and add the .c and .h files to the Visual Studio project with
// Add > Existing Item. The CMake starter builds every .c file next to
// main.c by itself.
//
// Try this:
// - Add a game over state between the game and the menu.
// - Keep a best score in score.c, and show it on the menu.
// - Move the button functions into a ui.c and ui.h of their own.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include "gamestate_splash.h"

// Runs every frame, before whichever state is running
void PreUpdate(void)
{
	if (CP_Input_KeyTriggered(CP_KEY_ESCAPE))
	{
		CP_Engine_Terminate();
	}
}

int main(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_Engine_SetNextGameState(gamestate_splash_init, gamestate_splash_update, gamestate_splash_exit);
	CP_Engine_SetPreUpdateFunction(PreUpdate);
	CP_Engine_Run();
	return 0;
}
