//---------------------------------------------------------
// file:	main.c
//
// Lesson 6: Collision.
// https://github.com/DigiPen-Faculty/CProcessing/wiki/Lesson-06-Collision
//
// This is the finished program from the lesson: collect all the coins and
// keep away from the red balls.
//
//   WASD or the arrow keys   move the player (the blue circle)
//   R, or click the button   put the coins back and start again
//
// Four kinds of collision are used:
//   point in a box      is the mouse over the button?
//   circle and circle   the player and a coin, or the player and a ball
//   circle and a box    the player and a wall, or a ball and a wall
//   bouncing            a ball turns round when it hits a wall
//
// To run it, copy this file over the template's main.c.
//
// Try this:
// - Give the player three lives, and end the game when they are gone.
// - Add a wall that moves.
// - Make each coin worth a different amount, by its size.
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

//---------------------------------------------------------
// Shapes. A box is stored by its center, the way CProcessing draws a rectangle
// by default.
//---------------------------------------------------------
typedef struct Box
{
	CP_Vector center;
	float width;
	float height;
} Box;

Box MakeBox(float x, float y, float width, float height)
{
	Box box;
	box.center = CP_Vector_Set(x, y);
	box.width = width;
	box.height = height;
	return box;
}

//---------------------------------------------------------
// THE CHECKS. Each answers one question: are these two shapes touching?
//---------------------------------------------------------

// A point is inside a box when it is between the left and right edges AND
// between the top and bottom edges.
CP_BOOL PointInBox(CP_Vector point, Box box)
{
	const float left = box.center.x - box.width / 2.0f;
	const float right = box.center.x + box.width / 2.0f;
	const float top = box.center.y - box.height / 2.0f;
	const float bottom = box.center.y + box.height / 2.0f;
	return point.x >= left && point.x <= right && point.y >= top && point.y <= bottom;
}

// Two circles touch when the distance between their centers is no more than
// their two radii added together.
CP_BOOL CirclesTouch(CP_Vector centerA, float radiusA, CP_Vector centerB, float radiusB)
{
	return CP_Vector_Distance(centerA, centerB) <= radiusA + radiusB;
}

// A circle touches a box when the point of the box that is closest to the
// circle's center is inside the circle. Clamping the center to the box's
// edges finds that point.
CP_BOOL CircleTouchesBox(CP_Vector center, float radius, Box box)
{
	CP_Vector closest;
	closest.x = CP_Math_ClampFloat(center.x, box.center.x - box.width / 2.0f, box.center.x + box.width / 2.0f);
	closest.y = CP_Math_ClampFloat(center.y, box.center.y - box.height / 2.0f, box.center.y + box.height / 2.0f);
	return CP_Vector_Distance(center, closest) <= radius;
}

//---------------------------------------------------------
// THE GAME
//---------------------------------------------------------
#define WALL_COUNT 3
#define COIN_COUNT 8
#define BALL_COUNT 2

const float playerRadius = 20.0f;
const float playerSpeed = 260.0f;
const float coinRadius = 12.0f;
const float ballRadius = 18.0f;

Box walls[WALL_COUNT];
Box resetButton;

CP_Vector playerStart;
CP_Vector playerPosition;

CP_Vector coinPositions[COIN_COUNT];
CP_BOOL coinTaken[COIN_COUNT];			// has this coin been collected?
int coinsTaken = 0;

CP_Vector ballPositions[BALL_COUNT];
CP_Vector ballVelocities[BALL_COUNT];	// pixels per second
int hits = 0;

// Can a circle be at this spot? It must be inside the window and not touch a wall.
CP_BOOL CanBeAt(CP_Vector center, float radius)
{
	if (center.x - radius < 0.0f || center.x + radius > (float)CP_System_GetWindowWidth()
		|| center.y - radius < 0.0f || center.y + radius > (float)CP_System_GetWindowHeight())
	{
		return FALSE;
	}
	for (int i = 0; i < WALL_COUNT; i++)
	{
		if (CircleTouchesBox(center, radius, walls[i]))
		{
			return FALSE;
		}
	}
	return TRUE;
}

// Moves a circle by `move`, one direction at a time: across first, then down.
// A step that would end in a wall (or outside the window) is not taken, and
// the circle slides along the wall in the other direction. Moving one axis at
// a time also tells us WHICH way it was blocked, which is what a bouncing ball
// needs to know. The two flags say which directions were blocked.
void MoveCircle(CP_Vector* position, CP_Vector move, float radius, CP_BOOL* blockedAcross, CP_BOOL* blockedDown)
{
	*blockedAcross = FALSE;
	*blockedDown = FALSE;

	CP_Vector next = *position;
	next.x += move.x;
	if (CanBeAt(next, radius))
	{
		position->x = next.x;
	}
	else
	{
		*blockedAcross = TRUE;
	}

	next = *position;
	next.y += move.y;
	if (CanBeAt(next, radius))
	{
		position->y = next.y;
	}
	else
	{
		*blockedDown = TRUE;
	}
}

void ResetGame(void)
{
	playerPosition = playerStart;
	for (int i = 0; i < COIN_COUNT; i++)
	{
		coinTaken[i] = FALSE;
	}
	coinsTaken = 0;
	hits = 0;
	ballPositions[0] = CP_Vector_Set(600.0f, 150.0f);
	ballVelocities[0] = CP_Vector_Set(-200.0f, 160.0f);
	ballPositions[1] = CP_Vector_Set(90.0f, 250.0f);
	ballVelocities[1] = CP_Vector_Set(150.0f, -170.0f);
}

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Lesson 6: Collision");
	CP_Settings_TextSize(20.0f);

	walls[0] = MakeBox(250.0f, 200.0f, 200.0f, 40.0f);
	walls[1] = MakeBox(560.0f, 330.0f, 40.0f, 260.0f);
	walls[2] = MakeBox(300.0f, 450.0f, 240.0f, 40.0f);
	resetButton = MakeBox(700.0f, 30.0f, 160.0f, 40.0f);

	playerStart = CP_Vector_Set(60.0f, 540.0f);

	coinPositions[0] = CP_Vector_Set(100.0f, 100.0f);
	coinPositions[1] = CP_Vector_Set(400.0f, 100.0f);
	coinPositions[2] = CP_Vector_Set(700.0f, 100.0f);
	coinPositions[3] = CP_Vector_Set(150.0f, 320.0f);
	coinPositions[4] = CP_Vector_Set(450.0f, 330.0f);
	coinPositions[5] = CP_Vector_Set(700.0f, 500.0f);
	coinPositions[6] = CP_Vector_Set(120.0f, 480.0f);
	coinPositions[7] = CP_Vector_Set(420.0f, 540.0f);

	ResetGame();
}

void game_update(void)
{
	const float dt = CP_System_GetDt();
	const CP_Vector mouse = CP_Vector_Set(CP_Input_GetMouseX(), CP_Input_GetMouseY());

	// Point in a box: is the mouse over the button when it is clicked?
	const CP_BOOL overButton = PointInBox(mouse, resetButton);
	if ((overButton && CP_Input_MouseTriggered(MOUSE_BUTTON_LEFT)) || CP_Input_KeyTriggered(KEY_R))
	{
		ResetGame();
	}

	// Move the player (lesson 5), but through MoveCircle so walls stop it
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
	if (direction.x != 0.0f || direction.y != 0.0f)
	{
		direction = CP_Vector_Normalize(direction);
	}
	CP_BOOL blockedAcross, blockedDown;
	MoveCircle(&playerPosition, CP_Vector_Scale(direction, playerSpeed * dt), playerRadius, &blockedAcross, &blockedDown);

	// Circle and circle: the player takes a coin by touching it
	for (int i = 0; i < COIN_COUNT; i++)
	{
		if (!coinTaken[i] && CirclesTouch(playerPosition, playerRadius, coinPositions[i], coinRadius))
		{
			coinTaken[i] = TRUE;
			coinsTaken++;
		}
	}

	// The balls move, and BOUNCE: when a ball is blocked across, it turns
	// round across, and when it is blocked down, it turns round down. And
	// touching one sends the player back to the start.
	for (int i = 0; i < BALL_COUNT; i++)
	{
		MoveCircle(&ballPositions[i], CP_Vector_Scale(ballVelocities[i], dt), ballRadius, &blockedAcross, &blockedDown);
		if (blockedAcross)
		{
			ballVelocities[i].x = -ballVelocities[i].x;
		}
		if (blockedDown)
		{
			ballVelocities[i].y = -ballVelocities[i].y;
		}

		if (CirclesTouch(playerPosition, playerRadius, ballPositions[i], ballRadius))
		{
			playerPosition = playerStart;
			hits++;
		}
	}

	//-----------------------------------------------------
	// Draw
	CP_Graphics_ClearBackground(CP_Color_Create(30, 30, 50, 255));
	CP_Settings_NoStroke();

	CP_Settings_Fill(CP_Color_Create(150, 150, 160, 255));
	for (int i = 0; i < WALL_COUNT; i++)
	{
		CP_Graphics_DrawRect(walls[i].center.x, walls[i].center.y, walls[i].width, walls[i].height);
	}

	CP_Settings_Fill(CP_Color_Create(250, 210, 60, 255));
	for (int i = 0; i < COIN_COUNT; i++)
	{
		if (!coinTaken[i])
		{
			// DrawCircle takes the diameter, so twice the radius
			CP_Graphics_DrawCircle(coinPositions[i].x, coinPositions[i].y, coinRadius * 2.0f);
		}
	}

	CP_Settings_Fill(CP_Color_Create(235, 80, 80, 255));
	for (int i = 0; i < BALL_COUNT; i++)
	{
		CP_Graphics_DrawCircle(ballPositions[i].x, ballPositions[i].y, ballRadius * 2.0f);
	}

	CP_Settings_Fill(CP_Color_Create(80, 200, 255, 255));
	CP_Graphics_DrawCircle(playerPosition.x, playerPosition.y, playerRadius * 2.0f);

	// The button is lighter while the mouse is over it
	CP_Settings_Fill(overButton ? CP_Color_Create(110, 110, 160, 255) : CP_Color_Create(80, 80, 120, 255));
	CP_Graphics_DrawRect(resetButton.center.x, resetButton.center.y, resetButton.width, resetButton.height);

	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Font_DrawText("Start again (R)", resetButton.center.x, resetButton.center.y);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);

	char text[60];
	if (coinsTaken == COIN_COUNT)
	{
		snprintf(text, sizeof(text), "You got every coin! Hits: %d", hits);
	}
	else
	{
		snprintf(text, sizeof(text), "Coins: %d of %d    Hits: %d", coinsTaken, COIN_COUNT, hits);
	}
	CP_Font_DrawText(text, 20.0f, 30.0f);
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
