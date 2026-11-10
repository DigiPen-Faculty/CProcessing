//---------------------------------------------------------
// file:	main.c
//
// Lesson 1: Your first window.
// https://github.com/DigiPen-Faculty/CProcessing/wiki/Lesson-01-Your-First-Window
//
// This is the finished program from the lesson: a night sky with a moon
// that follows the mouse. Clicking prints where you clicked to the console.
//
// To run it, copy this file over the template's main.c.
//
// Try this:
// - Change the window size and the sky color.
// - Add a second, smaller moon that sits at a fixed spot.
// - Make the moon bigger while the left mouse button is held down.
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

// How wide the moon is, in pixels. A name like this makes the code easier
// to read, and there is one place to change if you want a bigger moon.
const float moonSize = 80.0f;

void game_init(void)
{
	// Runs once, when the game starts: set up the window
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Lesson 1: Your first window");
}

void game_update(void)
{
	// Runs every frame, about 60 times a second.

	// Step 1: paint the whole window dark blue. This erases the last frame,
	// so each frame starts from a clean sky.
	CP_Graphics_ClearBackground(CP_Color_Create(10, 15, 50, 255));

	// Step 2: a fixed star. Colors are red, green, blue and opacity, each
	// from 0 to 255. (0, 0) is the top left corner of the window; x grows to
	// the right and y grows DOWN.
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Graphics_DrawCircle(650.0f, 120.0f, 10.0f);

	// Step 3: the moon, where the mouse is. The mouse's position is read
	// again every frame, so the moon follows it.
	CP_Settings_Fill(CP_Color_Create(250, 240, 200, 255));
	CP_Graphics_DrawCircle(CP_Input_GetMouseX(), CP_Input_GetMouseY(), moonSize);

	// Step 4: a click prints the mouse's position. Triggered means "pressed
	// this frame", so a click prints once, not for every frame it's held.
	if (CP_Input_MouseTriggered(MOUSE_BUTTON_LEFT))
	{
		printf("Clicked at (%.0f, %.0f)\n", CP_Input_GetMouseX(), CP_Input_GetMouseY());
	}
}

void game_exit(void)
{
	// Runs once, when the game ends. This program loaded nothing to free.
}

int main(void)
{
	// Windows opens a console window for printf. On Linux and macOS, run the
	// game from a terminal and the text appears there.
	CP_System_ShowConsole(TRUE);

	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
