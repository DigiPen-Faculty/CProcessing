//---------------------------------------------------------
// file:	debug_draw.c
//
// Example: drawing what the game knows, to find bugs by looking.
//
// Shows how to:
// - turn a debug view on and off with a key, so it's ready whenever
//   something looks wrong
// - draw each object's hitbox (the shape collisions use, which may not be
//   the shape you see), red while it touches something
// - draw velocities as arrows, and numbers next to what they belong to
// - mark where something happened (the last bounce), and let it fade
// - show a grid, the mouse position and the frame rate
// - keep the debug drawing from changing the game's drawing: draw it last,
//   between CP_Settings_Save and CP_Settings_Restore
//
// Move the mouse to move the paddle. D turns the debug view on and off, and
// Space pauses, to look at the arrows while everything stands still. B
// plants a bug: the paddle's hitbox is placed by its corner, while the
// paddle is drawn by its center. With the debug view off, balls bounce off
// thin air; with it on, you can see why.
//
// Try this:
// - Draw each ball's path over the last second, as a line of dots.
// - Show the debug view only while a key is held down.
// - Add a debug view to a game of your own, starting with the hitboxes.
//
// See also: collision.c, for the checks the hitboxes use; movement.c, for
// velocity; score_and_text.c, for numbers on the screen.
//
// To run it, copy this file over the template's main.c.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include <stdio.h> // for snprintf
#include "cprocessing.h"

#define WINDOW_WIDTH  800.0f
#define WINDOW_HEIGHT 600.0f
#define BALL_COUNT    5
#define GRAVITY       600.0f // pixels per second, every second

// Shapes are stored by their center, as CProcessing draws them by default
// (CP_POSITION_CENTER). Box, Circle, MakeBox and CircleTouchesBox are the
// same as in collision.c.
typedef struct Box
{
	CP_Vector center;
	float width;
	float height;
} Box;

typedef struct Circle
{
	CP_Vector center;
	float radius;
} Circle;

Box MakeBox(float x, float y, float width, float height)
{
	Box box;
	box.center = CP_Vector_Set(x, y);
	box.width = width;
	box.height = height;
	return box;
}

// A circle touches a box when the box's closest point to the circle's
// center is inside the circle. The closest point is the center, clamped to
// the box's edges.
//
// This one compares squares: the distance squared with the radius squared.
// It gives the same answer as comparing the distance with the radius, and
// skips the square root a distance takes. A vector's dot product with
// itself is its length squared. You'll see this in games that check
// thousands of things a frame; for a few, either way is fine.
bool CircleTouchesBox(Circle circle, Box box)
{
	CP_Vector closest;
	closest.x = CP_Math_ClampFloat(circle.center.x, box.center.x - box.width / 2.0f, box.center.x + box.width / 2.0f);
	closest.y = CP_Math_ClampFloat(circle.center.y, box.center.y - box.height / 2.0f, box.center.y + box.height / 2.0f);
	CP_Vector toClosest = CP_Vector_Subtract(closest, circle.center);
	return CP_Vector_DotProduct(toClosest, toClosest) <= CP_Math_Square(circle.radius);
}

//-------------------------
// THE GAME

typedef struct Ball
{
	Circle body;
	CP_Vector velocity;
	bool touching; // touching the paddle this frame
} Ball;

Ball balls[BALL_COUNT];
Box paddle; // where the paddle is drawn

bool debugView = true;
bool paused = false;
bool bugPlanted = false;

float seconds = 0.0f;           // game time, which stops while paused
CP_Vector lastBounce;           // where the last bounce happened
float lastBounceTime = -100.0f; // and when

// The shape the paddle's collisions use. With the bug planted, its center is
// the paddle's top left corner: the mistake of working out a hitbox as if
// the paddle were placed by its corner (CP_POSITION_CORNER) when it's drawn
// by its center.
Box PaddleHitbox(void)
{
	if (bugPlanted)
	{
		return MakeBox(paddle.center.x - paddle.width / 2.0f, paddle.center.y - paddle.height / 2.0f, paddle.width, paddle.height);
	}
	return paddle;
}

void DropBall(int i)
{
	balls[i].body.center = CP_Vector_Set(CP_Random_RangeFloat(60.0f, WINDOW_WIDTH - 60.0f), CP_Random_RangeFloat(40.0f, 160.0f));
	balls[i].body.radius = CP_Random_RangeFloat(14.0f, 26.0f);
	balls[i].velocity = CP_Vector_Set(CP_Random_RangeFloat(-200.0f, 200.0f), 0.0f);
	balls[i].touching = false;
}

void NoteBounce(CP_Vector where)
{
	lastBounce = where;
	lastBounceTime = seconds;
}

void MoveBalls(float dt)
{
	const Box hitbox = PaddleHitbox();
	for (int i = 0; i < BALL_COUNT; ++i)
	{
		Ball* ball = &balls[i];
		const float r = ball->body.radius;

		ball->velocity.y += GRAVITY * dt;
		ball->body.center = CP_Vector_Add(ball->body.center, CP_Vector_Scale(ball->velocity, dt));

		// The walls: the left and right ones and the ceiling bounce
		if (ball->body.center.x < r && ball->velocity.x < 0.0f)
		{
			ball->velocity.x = -ball->velocity.x;
			NoteBounce(ball->body.center);
		}
		if (ball->body.center.x > WINDOW_WIDTH - r && ball->velocity.x > 0.0f)
		{
			ball->velocity.x = -ball->velocity.x;
			NoteBounce(ball->body.center);
		}
		if (ball->body.center.y < r && ball->velocity.y < 0.0f)
		{
			ball->velocity.y = -ball->velocity.y;
			NoteBounce(ball->body.center);
		}

		// The paddle bounces a ball that is falling onto it
		ball->touching = CircleTouchesBox(ball->body, hitbox);
		if (ball->touching && ball->velocity.y > 0.0f)
		{
			ball->velocity.y = -ball->velocity.y;
			NoteBounce(ball->body.center);
		}

		// A ball that falls out of the window starts again at the top
		if (ball->body.center.y > WINDOW_HEIGHT + r)
		{
			DropBall(i);
		}
	}
}

void DrawGame(void)
{
	CP_Graphics_ClearBackground(CP_Color_Create(30, 30, 45, 255));
	CP_Settings_NoStroke();

	CP_Settings_Fill(CP_Color_Create(240, 170, 60, 255));
	CP_Graphics_DrawRoundedRect(paddle.center.x, paddle.center.y, paddle.width, paddle.height, 8.0f);

	for (int i = 0; i < BALL_COUNT; ++i)
	{
		const Circle body = balls[i].body;
		CP_Settings_Fill(CP_Color_Create(90, 180, 255, 255));
		CP_Graphics_DrawCircle(body.center.x, body.center.y, body.radius * 2.0f);
		// a shine, so the ball looks round: just a picture, not part of its hitbox
		CP_Settings_Fill(CP_Color_Create(200, 235, 255, 255));
		CP_Graphics_DrawCircle(body.center.x - body.radius * 0.35f, body.center.y - body.radius * 0.35f, body.radius * 0.5f);
	}

	CP_Settings_Fill(CP_Color_Create(160, 160, 180, 255));
	CP_Settings_TextSize(18.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_BOTTOM);
	CP_Font_DrawText("D: debug view   Space: pause   B: plant the bug", WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT - 8.0f);
}

//-------------------------
// THE DEBUG VIEW

// A line with a head at "to", for a velocity or a direction
void DrawArrow(CP_Vector from, CP_Vector to)
{
	CP_Graphics_DrawLine(from.x, from.y, to.x, to.y);
	CP_Vector back = CP_Vector_Scale(CP_Vector_Normalize(CP_Vector_Subtract(from, to)), 10.0f);
	CP_Vector left = CP_Vector_Add(to, CP_Vector_Rotate(back, 30.0f));
	CP_Vector right = CP_Vector_Add(to, CP_Vector_Rotate(back, -30.0f));
	CP_Graphics_DrawLine(to.x, to.y, left.x, left.y);
	CP_Graphics_DrawLine(to.x, to.y, right.x, right.y);
}

// Lines every 100 pixels, with their numbers, to read positions off the screen
void DrawGrid(void)
{
	char text[16];
	CP_Settings_Stroke(CP_Color_Create(255, 255, 255, 30));
	CP_Settings_StrokeWeight(1.0f);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 90));
	CP_Settings_TextSize(12.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
	for (float x = 100.0f; x < WINDOW_WIDTH; x += 100.0f)
	{
		CP_Graphics_DrawLine(x, 0.0f, x, WINDOW_HEIGHT);
		snprintf(text, sizeof(text), "%.0f", x);
		CP_Font_DrawText(text, x + 3.0f, 3.0f);
	}
	for (float y = 100.0f; y < WINDOW_HEIGHT; y += 100.0f)
	{
		CP_Graphics_DrawLine(0.0f, y, WINDOW_WIDTH, y);
		snprintf(text, sizeof(text), "%.0f", y);
		CP_Font_DrawText(text, 3.0f, y + 3.0f);
	}
}

void DrawDebugView(void)
{
	char text[96];

	// Everything set here goes back as it was at Restore, so the game's own
	// drawing, next frame, doesn't pick up a debug color or a thin line
	CP_Settings_Save();
	DrawGrid();

	const CP_Color hitboxColor = CP_Color_Create(255, 230, 0, 255);
	const CP_Color touchingColor = CP_Color_Create(255, 60, 60, 255);
	CP_Settings_NoFill();
	CP_Settings_StrokeWeight(2.0f);
	CP_Settings_RectMode(CP_POSITION_CENTER);

	// The hitboxes, as the collisions see them
	bool paddleTouched = false;
	for (int i = 0; i < BALL_COUNT; ++i)
	{
		const Circle body = balls[i].body;
		CP_Settings_Stroke(balls[i].touching ? touchingColor : hitboxColor);
		CP_Graphics_DrawCircle(body.center.x, body.center.y, body.radius * 2.0f);
		paddleTouched = paddleTouched || balls[i].touching;
	}
	const Box hitbox = PaddleHitbox();
	CP_Settings_Stroke(paddleTouched ? touchingColor : hitboxColor);
	CP_Graphics_DrawRect(hitbox.center.x, hitbox.center.y, hitbox.width, hitbox.height);

	// Velocities: where each ball will be in a fifth of a second, and its speed
	CP_Settings_Stroke(CP_Color_Create(80, 255, 160, 255));
	CP_Settings_Fill(CP_Color_Create(80, 255, 160, 255));
	CP_Settings_TextSize(14.0f);
	for (int i = 0; i < BALL_COUNT; ++i)
	{
		const Ball ball = balls[i];
		DrawArrow(ball.body.center, CP_Vector_Add(ball.body.center, CP_Vector_Scale(ball.velocity, 0.2f)));

		// to the right of the ball, or to its left near the right edge
		const float gap = ball.body.radius + 6.0f;
		const bool nearRightEdge = ball.body.center.x > WINDOW_WIDTH - 60.0f;
		CP_Settings_TextAlignment(nearRightEdge ? CP_TEXT_ALIGN_H_RIGHT : CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_MIDDLE);
		snprintf(text, sizeof(text), "%.0f", CP_Vector_Length(ball.velocity));
		CP_Font_DrawText(text, ball.body.center.x + (nearRightEdge ? -gap : gap), ball.body.center.y);
	}

	// The last bounce: a ring that fades over a second
	const float sinceBounce = seconds - lastBounceTime;
	if (sinceBounce < 1.0f)
	{
		CP_Settings_NoFill();
		CP_Settings_Stroke(CP_Color_Create(255, 255, 255, (int)(255.0f * (1.0f - sinceBounce))));
		CP_Graphics_DrawCircle(lastBounce.x, lastBounce.y, 20.0f + 40.0f * sinceBounce);
	}

	// The mouse, as numbers
	const float mouseX = CP_Input_GetMouseX();
	const float mouseY = CP_Input_GetMouseY();
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_TextSize(14.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BOTTOM);
	snprintf(text, sizeof(text), "(%.0f, %.0f)", mouseX, mouseY);
	CP_Font_DrawText(text, mouseX + 10.0f, mouseY - 6.0f);

	// What the game is doing, in a corner
	CP_Settings_TextSize(16.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_RIGHT, CP_TEXT_ALIGN_V_TOP);
	snprintf(text, sizeof(text), "Frame rate: %.0f   dt: %.4f", CP_System_GetFrameRate(), CP_System_GetDt());
	CP_Font_DrawText(text, WINDOW_WIDTH - 10.0f, 10.0f);
	if (paused)
	{
		CP_Font_DrawText("PAUSED", WINDOW_WIDTH - 10.0f, 32.0f);
	}
	if (bugPlanted)
	{
		CP_Settings_Fill(touchingColor);
		CP_Font_DrawText("Bug planted: the hitbox is off by half the paddle", WINDOW_WIDTH - 10.0f, 54.0f);
	}

	CP_Settings_Restore();
}

//-------------------------

void game_init(void)
{
	CP_System_SetWindowSize((int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
	CP_System_SetWindowTitle("Debug drawing");

	paddle = MakeBox(WINDOW_WIDTH / 2.0f, 520.0f, 160.0f, 24.0f);
	for (int i = 0; i < BALL_COUNT; ++i)
	{
		DropBall(i);
	}
}

void game_update(void)
{
	if (CP_Input_KeyTriggered(CP_KEY_D))
	{
		debugView = !debugView;
	}
	if (CP_Input_KeyTriggered(CP_KEY_SPACE))
	{
		paused = !paused;
	}
	if (CP_Input_KeyTriggered(CP_KEY_B))
	{
		bugPlanted = !bugPlanted;
	}

	if (!paused)
	{
		const float dt = CP_System_GetDt();
		seconds += dt;
		paddle.center.x = CP_Math_ClampFloat(CP_Input_GetMouseX(), paddle.width / 2.0f, WINDOW_WIDTH - paddle.width / 2.0f);
		MoveBalls(dt);
	}

	DrawGame();

	// The debug view comes last, so it's on top of everything
	if (debugView)
	{
		DrawDebugView();
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
