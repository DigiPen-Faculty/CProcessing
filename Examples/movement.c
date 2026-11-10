//---------------------------------------------------------
// file:	movement.c
//
// Example: moving a player with the keyboard or a gamepad.
//
// Shows how to:
// - read held keys (WASD and the arrow keys) and a gamepad's left stick
// - move at the same speed at any frame rate, with CP_System_GetDt
// - keep diagonal movement from being faster, with CP_Vector_Normalize
// - keep the player inside the window
//
// Try this:
// - Hold Shift to run at twice the speed.
// - Make the player speed up and slow down smoothly instead of starting
//   and stopping at once.
//
// To run it, copy this file over the template's main.c.
//---------------------------------------------------------

#include "cprocessing.h"

// The player: where it is, how big it is, and how fast it moves
CP_Vector playerPosition;
const float playerSize = 50.0f;
const float playerSpeed = 300.0f;	// pixels per second

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Movement");

	// start in the middle of the window
	playerPosition = CP_Vector_Set(400.0f, 300.0f);
}

void game_update(void)
{
	// 1. Which way does the player want to go? Each key adds to a direction:
	//    x is -1 for left and +1 for right, y is -1 for up and +1 for down.
	CP_Vector direction = CP_Vector_Zero();
	if (CP_Input_KeyDown(KEY_A) || CP_Input_KeyDown(KEY_LEFT))
	{
		direction.x -= 1.0f;
	}
	if (CP_Input_KeyDown(KEY_D) || CP_Input_KeyDown(KEY_RIGHT))
	{
		direction.x += 1.0f;
	}
	if (CP_Input_KeyDown(KEY_W) || CP_Input_KeyDown(KEY_UP))
	{
		direction.y -= 1.0f;
	}
	if (CP_Input_KeyDown(KEY_S) || CP_Input_KeyDown(KEY_DOWN))
	{
		direction.y += 1.0f;
	}

	// Holding two keys, such as W and D, gives (1, -1), which is longer than
	// 1 (about 1.41), so the player would move faster diagonally. Normalizing
	// makes its length exactly 1, so every direction is the same speed.
	if (direction.x != 0.0f || direction.y != 0.0f)
	{
		direction = CP_Vector_Normalize(direction);
	}

	// A gamepad's left stick already gives a direction with a length from 0
	// to 1, so pushing it halfway moves at half speed. CProcessing reports 0
	// while the stick is only slightly off center, so any other value means
	// the player is pushing it. On the stick, up is positive y, but in the
	// window y grows downwards, so flip it.
	CP_Vector stick = CP_Input_GamepadLeftStick();
	if (stick.x != 0.0f || stick.y != 0.0f)
	{
		direction = CP_Vector_Set(stick.x, -stick.y);

		// some sticks reach past a length of 1 in the corners
		if (CP_Vector_Length(direction) > 1.0f)
		{
			direction = CP_Vector_Normalize(direction);
		}
	}

	// 2. Move. CP_System_GetDt() is the time since the last frame, in
	//    seconds. Speed (pixels per second) times time (seconds) is the
	//    distance to move this frame, so the player covers 300 pixels each
	//    second whether the game runs at 30 or 144 frames per second.
	CP_Vector velocity = CP_Vector_Scale(direction, playerSpeed);
	playerPosition = CP_Vector_Add(playerPosition, CP_Vector_Scale(velocity, CP_System_GetDt()));

	// 3. Stay inside the window. The position is the player's center, so it
	//    can go no closer to an edge than half its size.
	float half = playerSize / 2.0f;
	playerPosition.x = CP_Math_ClampFloat(playerPosition.x, half, CP_System_GetWindowWidth() - half);
	playerPosition.y = CP_Math_ClampFloat(playerPosition.y, half, CP_System_GetWindowHeight() - half);

	// 4. Draw
	CP_Graphics_ClearBackground(CP_Color_Create(40, 40, 40, 255));

	CP_Settings_Fill(CP_Color_Create(80, 200, 255, 255));
	CP_Graphics_DrawRect(playerPosition.x, playerPosition.y, playerSize, playerSize);

	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Font_DrawText("WASD, the arrow keys or a gamepad's left stick", 20, 30);
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
