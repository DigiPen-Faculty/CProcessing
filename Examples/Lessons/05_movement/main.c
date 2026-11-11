//---------------------------------------------------------
// file:	main.c
//
// Lesson 5: Moving with vectors and delta time.
// https://github.com/DigiPen-Faculty/CProcessing/wiki/Lesson-05-Moving-with-Vectors-and-Delta-Time
//
// This is the finished program from the lesson. The top half is an
// experiment on frame rate, and the bottom half is a player to move.
//
// The experiment: two balls cross the window, and bounce back. The top one
// moves by speed * dt. The bottom one moves the same amount every frame.
//   1, 2, 3   run the game at 15, 30 or 60 frames a second, and watch which
//             ball keeps the same speed
//
// The player (the bottom half):
//   WASD, the arrow keys or the left stick of a gamepad   move
//   N                  turn the diagonal fix on and off, and watch the speed
//
// To run it, copy this file over the template's main.c.
//
// Try this:
// - Make the player bigger while Shift is held, and faster too.
// - Add a second player that moves with the other keys.
// - Make the player slow down gradually instead of stopping at once.
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

//---------------------------------------------------------
// The frame rate experiment
//---------------------------------------------------------
const float ballSpeed = 300.0f;			// pixels per second
const float stepPerFrame = 5.0f;		// 5 pixels a frame is 300 a second, at 60 frames a second

CP_Vector dtBall;						// moves by speed * dt
CP_Vector frameBall;					// moves by a fixed step every frame
float dtDirection = 1.0f;				// +1 is right, -1 is left
float frameDirection = 1.0f;

// Moves a ball along x by `amount` pixels, and turns it round at the edges
void MoveAndBounce(CP_Vector* ball, float* direction, float amount)
{
	ball->x += *direction * amount;
	const float left = 30.0f;
	const float right = (float)CP_System_GetWindowWidth() - 30.0f;
	if (ball->x > right)
	{
		ball->x = right;
		*direction = -1.0f;
	}
	else if (ball->x < left)
	{
		ball->x = left;
		*direction = 1.0f;
	}
}

void DoExperiment(void)
{
	// Keys 1, 2 and 3 limit how many frames a second the game runs at
	if (CP_Input_KeyTriggered(KEY_1))
	{
		CP_System_SetFrameRate(15.0f);
	}
	if (CP_Input_KeyTriggered(KEY_2))
	{
		CP_System_SetFrameRate(30.0f);
	}
	if (CP_Input_KeyTriggered(KEY_3))
	{
		CP_System_SetFrameRate(60.0f);
	}

	// CP_System_GetDt() is how many seconds the last frame took. Speed (pixels
	// per second) times seconds is pixels: how far to move THIS frame.
	MoveAndBounce(&dtBall, &dtDirection, ballSpeed * CP_System_GetDt());
	MoveAndBounce(&frameBall, &frameDirection, stepPerFrame);

	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(90, 220, 120, 255));
	CP_Graphics_DrawCircle(dtBall.x, dtBall.y, 40.0f);
	CP_Settings_Fill(CP_Color_Create(240, 90, 90, 255));
	CP_Graphics_DrawCircle(frameBall.x, frameBall.y, 40.0f);

	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Font_DrawText("speed * dt: the same speed at any frame rate", 30.0f, dtBall.y - 32.0f);
	CP_Font_DrawText("a fixed step each frame: the speed depends on the frame rate", 30.0f, frameBall.y - 32.0f);

	char text[80];
	snprintf(text, sizeof(text), "Frame rate limit: %.0f   running at: %.0f   (keys 1, 2, 3)",
		CP_System_GetTargetFrameRate(), CP_System_GetFrameRate());
	CP_Font_DrawText(text, 30.0f, 28.0f);
}

//---------------------------------------------------------
// The player
//---------------------------------------------------------
const float playerSpeed = 250.0f;		// pixels per second
const float playerSize = 50.0f;
const float areaTop = 235.0f;			// the player stays below this line

CP_Vector playerPosition;
CP_BOOL normalizeDirection = TRUE;
float measuredSpeed = 0.0f;

// Which way does the player want to go? A vector from the keys, or the stick.
CP_Vector ReadDirection(void)
{
	// Each key adds to the direction: x is -1 for left and +1 for right, y is
	// -1 for up and +1 for down (the window's y grows downward).
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

	// Holding two keys, say W and D, gives (1, -1). That arrow is longer than
	// 1: its length is about 1.41. Moving by it would be 41% faster diagonally!
	// Normalize makes a vector's length exactly 1 and keeps its direction.
	// (With no key held the vector is (0, 0), which has no direction to keep,
	// so Normalize gives (0, 0) back.)
	if (normalizeDirection)
	{
		direction = CP_Vector_Normalize(direction);
	}

	// A stick already gives a direction, and its length is how far it is
	// pushed, so a half push is half speed. Up is +y on the stick, so flip it.
	CP_Vector stick = CP_Input_GamepadLeftStick();
	if (stick.x != 0.0f || stick.y != 0.0f)
	{
		direction = CP_Vector_Limit(CP_Vector_Set(stick.x, -stick.y), 1.0f);
	}
	return direction;
}

void DoPlayer(void)
{
	if (CP_Input_KeyTriggered(KEY_N))
	{
		normalizeDirection = !normalizeDirection;
	}

	// Velocity is a direction times a speed, in pixels per second. The new
	// position is the old one plus velocity * dt, as a vector.
	const CP_Vector direction = ReadDirection();
	const CP_Vector velocity = CP_Vector_Scale(direction, playerSpeed);
	const CP_Vector oldPosition = playerPosition;
	playerPosition = CP_Vector_Add(playerPosition, CP_Vector_Scale(velocity, CP_System_GetDt()));

	// Stay inside the window and below the line. The position is the center,
	// so it can go no closer to an edge than half the player's size.
	const float half = playerSize / 2.0f;
	playerPosition.x = CP_Math_ClampFloat(playerPosition.x, half, (float)CP_System_GetWindowWidth() - half);
	playerPosition.y = CP_Math_ClampFloat(playerPosition.y, areaTop + half, (float)CP_System_GetWindowHeight() - half);

	// How fast did it really go? Distance moved divided by the time it took.
	if (CP_System_GetDt() > 0.0f)
	{
		measuredSpeed = CP_Vector_Distance(oldPosition, playerPosition) / CP_System_GetDt();
	}

	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(80, 200, 255, 255));
	CP_Graphics_DrawCircle(playerPosition.x, playerPosition.y, playerSize);

	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	char text[80];
	snprintf(text, sizeof(text), "Speed: %.0f pixels a second   (the player is set to %.0f)", measuredSpeed, playerSpeed);
	CP_Font_DrawText(text, 30.0f, areaTop + 30.0f);
	snprintf(text, sizeof(text), "Diagonal fix (normalize): %s   (N to switch)", normalizeDirection ? "on" : "off");
	CP_Font_DrawText(text, 30.0f, areaTop + 58.0f);
}

//---------------------------------------------------------

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Lesson 5: Moving with vectors and delta time");
	CP_Settings_TextSize(18.0f);

	dtBall = CP_Vector_Set(30.0f, 100.0f);
	frameBall = CP_Vector_Set(30.0f, 190.0f);
	playerPosition = CP_Vector_Set(400.0f, 450.0f);
}

void game_update(void)
{
	CP_Graphics_ClearBackground(CP_Color_Create(30, 30, 50, 255));

	DoExperiment();

	// A line between the two halves
	CP_Settings_Stroke(CP_Color_Create(120, 120, 150, 255));
	CP_Settings_StrokeWeight(2.0f);
	CP_Graphics_DrawLine(0.0f, areaTop, (float)CP_System_GetWindowWidth(), areaTop);

	DoPlayer();
}

void game_exit(void)
{
	// This program loaded nothing to free.
}

int main(void)
{
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
