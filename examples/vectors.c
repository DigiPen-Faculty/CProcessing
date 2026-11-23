//---------------------------------------------------------
// file:	vectors.c
//
// Example: vectors for games: aiming, seeing, which side, chasing and
// running away.
//
// A vector is an arrow: a direction and a length, as x and y. One vector can
// be a place, a speed or a direction, and a few operations on them answer
// most "where" and "which way" questions in a game.
//
// Shows how to:
// - turn a turret toward a target at a limited speed, the shorter way round,
//   with CP_Vector_AngleCW and CP_Vector_Rotate
// - tell if a guard can see the player: in front, within the angle of its
//   view (CP_Vector_DotProduct, or CP_Vector_Angle), and close enough
// - tell which side of a line something is on (CP_Vector_CrossProduct)
// - chase something and slow down to arrive, or run away from it
//   (CP_Vector_Negate), turning smoothly (CP_Vector_Lerp) at a top speed
//   (CP_Vector_Limit)
//
// Each panel says what to do in it; the mouse is the target, the player or
// the point in all four.
//
// Try this:
// - Make the guard turn toward the player once it has seen them.
// - Let the turret lead its target: aim where the target will be when the
//   bullet gets there.
// - Make the creature run away only when the mouse is close, and wander
//   otherwise.
//
// See also: movement.c, for moving with a direction and a speed;
// rotation.c, for drawing turned things; collision.c, for what touches.
//
// To run it, copy this file over the template's main.c.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include <math.h>
#include <stdio.h>

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

#define PANEL_WIDTH  610.0f
#define PANEL_HEIGHT 310.0f
const float panelLeft[4] = { 20.0f, 650.0f, 20.0f, 650.0f };
const float panelTop[4] = { 60.0f, 60.0f, 390.0f, 390.0f };

//---------------------------------------------------------
// Helpers

CP_Vector MousePosition(void)
{
	return CP_Vector_Set(CP_Input_GetMouseX(), CP_Input_GetMouseY());
}

bool InPanel(int panel, CP_Vector point)
{
	return point.x >= panelLeft[panel] && point.x < panelLeft[panel] + PANEL_WIDTH && point.y >= panelTop[panel] + 60.0f &&
		   point.y < panelTop[panel] + PANEL_HEIGHT;
}

// An arrow from a point along a vector, with a small head
void DrawArrow(CP_Vector from, CP_Vector vector, CP_Color color)
{
	CP_Vector to = CP_Vector_Add(from, vector);
	CP_Settings_Stroke(color);
	CP_Settings_StrokeWeight(3.0f);
	CP_Graphics_DrawLine(from.x, from.y, to.x, to.y);
	if (CP_Vector_Length(vector) > 1.0f)
	{
		// the head: two short lines turned back from the tip
		CP_Vector back = CP_Vector_Scale(CP_Vector_Normalize(CP_Vector_Negate(vector)), 12.0f);
		CP_Vector left = CP_Vector_Add(to, CP_Vector_Rotate(back, 30.0f));
		CP_Vector right = CP_Vector_Add(to, CP_Vector_Rotate(back, -30.0f));
		CP_Graphics_DrawLine(to.x, to.y, left.x, left.y);
		CP_Graphics_DrawLine(to.x, to.y, right.x, right.y);
	}
	CP_Settings_NoStroke();
}

void DrawVectorPanel(int panel, const char* title, const char* hint)
{
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(40, 44, 60, 255));
	CP_Graphics_DrawRect(panelLeft[panel], panelTop[panel], PANEL_WIDTH, PANEL_HEIGHT);
	CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
	CP_Settings_TextSize(22.0f);
	CP_Font_DrawText(title, panelLeft[panel] + 14.0f, panelTop[panel] + 30.0f);
	CP_Settings_Fill(CP_Color_Create(150, 200, 255, 255));
	CP_Settings_TextSize(15.0f);
	CP_Font_DrawText(hint, panelLeft[panel] + 14.0f, panelTop[panel] + 52.0f);
}

void DrawSmallText(const char* text, float x, float y)
{
	CP_Settings_Fill(CP_Color_Create(200, 200, 215, 255));
	CP_Settings_TextSize(16.0f);
	CP_Font_DrawText(text, x, y);
}

//---------------------------------------------------------
// 1. A turret that turns toward its target

CP_Vector turretFacing; // a direction, length 1
CP_Vector turretTarget;
const float turretTurnSpeed = 120.0f; // degrees a second
#define BULLET_COUNT 8
CP_Vector bullets[BULLET_COUNT];
CP_Vector bulletVelocities[BULLET_COUNT];
float bulletLife[BULLET_COUNT]; // seconds left, 0 for a free slot

void DoTurret(float dt)
{
	DrawVectorPanel(0, "Aim a turret", "Move the mouse here to aim. Click to fire.");
	CP_Vector turret = CP_Vector_Set(panelLeft[0] + 200.0f, panelTop[0] + 190.0f);
	CP_Vector mouse = MousePosition();
	if (InPanel(0, mouse))
	{
		turretTarget = mouse;
	}

	// How far round to the target, clockwise: 0 to 360. Over 180, the other
	// way is shorter, so turn back the other way (a negative angle).
	CP_Vector toTarget = CP_Vector_Subtract(turretTarget, turret);
	float clockwise = CP_Vector_AngleCW(turretFacing, toTarget);
	float turn = clockwise > 180.0f ? clockwise - 360.0f : clockwise;

	// Turn at most turretTurnSpeed degrees a second toward it
	float most = turretTurnSpeed * dt;
	turn = CP_Math_ClampFloat(turn, -most, most);
	turretFacing = CP_Vector_Rotate(turretFacing, turn);

	// Fire along where it faces, not at the mouse
	if (CP_Input_MouseClicked() && InPanel(0, mouse))
	{
		for (int i = 0; i < BULLET_COUNT; ++i)
		{
			if (bulletLife[i] <= 0.0f)
			{
				bullets[i] = CP_Vector_Add(turret, CP_Vector_Scale(turretFacing, 50.0f));
				bulletVelocities[i] = CP_Vector_Scale(turretFacing, 400.0f);
				bulletLife[i] = 0.8f;
				break;
			}
		}
	}
	CP_Settings_Fill(CP_Color_Create(255, 210, 60, 255));
	for (int i = 0; i < BULLET_COUNT; ++i)
	{
		if (bulletLife[i] > 0.0f)
		{
			bullets[i] = CP_Vector_Add(bullets[i], CP_Vector_Scale(bulletVelocities[i], dt));
			bulletLife[i] -= dt;
			if (InPanel(0, bullets[i]))
			{
				CP_Graphics_DrawCircle(bullets[i].x, bullets[i].y, 10.0f);
			}
		}
	}

	// The turret, its barrel, and the target
	CP_Settings_Fill(CP_Color_Create(120, 130, 160, 255));
	CP_Graphics_DrawCircle(turret.x, turret.y, 60.0f);
	DrawArrow(turret, CP_Vector_Scale(turretFacing, 55.0f), CP_Color_Create(230, 230, 240, 255));
	CP_Settings_NoFill();
	CP_Settings_StrokeWeight(2.0f);
	CP_Settings_Stroke(CP_Color_Create(255, 110, 100, 255));
	CP_Graphics_DrawCircle(turretTarget.x, turretTarget.y, 24.0f);
	CP_Settings_NoStroke();

	char text[64];
	float x = panelLeft[0] + 360.0f;
	snprintf(text, sizeof(text), "AngleCW: %.0f", clockwise);
	DrawSmallText(text, x, panelTop[0] + 110.0f);
	snprintf(text, sizeof(text), "AngleCCW: %.0f", CP_Vector_AngleCCW(turretFacing, toTarget));
	DrawSmallText(text, x, panelTop[0] + 135.0f);
	snprintf(text, sizeof(text), "Turning: %s", turn > 0.0f ? "clockwise" : (turn < 0.0f ? "the other way" : "on target"));
	DrawSmallText(text, x, panelTop[0] + 160.0f);
}

//---------------------------------------------------------
// 2. Can the guard see you?

CP_Vector guardFacing;
float guardSweep = 0.0f;
const float viewAngle = 60.0f; // the whole cone; 30 each side of straight ahead
const float viewDistance = 230.0f;

void DoGuard(float dt)
{
	DrawVectorPanel(1, "Can the guard see you?", "Move the mouse into the guard's view.");
	CP_Vector guard = CP_Vector_Set(panelLeft[1] + 150.0f, panelTop[1] + 190.0f);
	CP_Vector player = MousePosition();
	bool playerHere = InPanel(1, player);

	// The guard looks back and forth, 50 degrees either side of right
	guardSweep += dt;
	guardFacing = CP_Vector_FromAngle(50.0f * sinf(guardSweep));

	// The dot product of two directions (each length 1) is 1 when they point
	// the same way, 0 at a right angle, and -1 opposite. Inside the cone
	// means more than the dot at its edge, cos(30 degrees).
	CP_Vector toPlayer = CP_Vector_Subtract(player, guard);
	float dot = CP_Vector_DotProduct(guardFacing, CP_Vector_Normalize(toPlayer));
	float edge = cosf(CP_Math_Radians(viewAngle / 2.0f));
	bool inView = dot > edge;
	bool closeEnough = CP_Vector_Length(toPlayer) < viewDistance;
	bool seen = playerHere && inView && closeEnough;

	// The cone: a pie slice centered on where the guard faces, kept inside
	// the panel (CP_Settings_Clip)
	float heading = CP_Vector_AngleCW(CP_Vector_Set(1.0f, 0.0f), guardFacing);
	CP_Settings_Clip(panelLeft[1], panelTop[1], PANEL_WIDTH, PANEL_HEIGHT);
	CP_Settings_Fill(seen ? CP_Color_Create(255, 90, 80, 110) : CP_Color_Create(255, 230, 120, 70));
	CP_Graphics_DrawPie(guard.x, guard.y, viewDistance * 2.0f, heading - viewAngle / 2.0f, viewAngle);
	CP_Settings_NoClip();
	CP_Settings_Fill(CP_Color_Create(90, 140, 230, 255));
	CP_Graphics_DrawCircle(guard.x, guard.y, 36.0f);
	DrawArrow(guard, CP_Vector_Scale(guardFacing, 40.0f), CP_Color_Create(230, 230, 240, 255));
	if (playerHere)
	{
		CP_Settings_Fill(seen ? CP_Color_Create(255, 90, 80, 255) : CP_Color_Create(120, 220, 140, 255));
		CP_Graphics_DrawCircle(player.x, player.y, 24.0f);
	}

	char text[64];
	float x = panelLeft[1] + 420.0f;
	snprintf(text, sizeof(text), "Dot: %.2f (edge %.2f)", dot, edge);
	DrawSmallText(text, x, panelTop[1] + 110.0f);
	// CP_Vector_Angle gives the same answer in degrees: under 30 is inside
	snprintf(text, sizeof(text), "Angle: %.0f", CP_Vector_Angle(guardFacing, toPlayer));
	DrawSmallText(text, x, panelTop[1] + 135.0f);
	DrawSmallText(inView ? "In front: yes" : "In front: no", x, panelTop[1] + 160.0f);
	DrawSmallText(closeEnough ? "Close enough: yes" : "Close enough: no", x, panelTop[1] + 185.0f);
	CP_Settings_TextSize(24.0f);
	CP_Settings_Fill(seen ? CP_Color_Create(255, 90, 80, 255) : CP_Color_Create(120, 220, 140, 255));
	CP_Font_DrawText(seen ? "Seen!" : "Hidden", x, panelTop[1] + 230.0f);
}

//---------------------------------------------------------
// 3. Which side of a line?

void DoSide(void)
{
	DrawVectorPanel(2, "Which side of the line?", "Move the mouse to either side of the road.");
	CP_Vector start = CP_Vector_Set(panelLeft[2] + 60.0f, panelTop[2] + 270.0f);
	CP_Vector end = CP_Vector_Set(panelLeft[2] + 360.0f, panelTop[2] + 100.0f);
	CP_Vector point = MousePosition();
	bool pointHere = InPanel(2, point);

	// The cross product of the road's direction and the direction to the
	// point is positive when the point is clockwise from the road: to its
	// right, going from start to end. Negative is to its left, 0 on it.
	CP_Vector road = CP_Vector_Subtract(end, start);
	float cross = CP_Vector_CrossProduct(road, CP_Vector_Subtract(point, start));

	CP_Settings_StrokeWeight(26.0f);
	CP_Settings_Stroke(CP_Color_Create(90, 95, 110, 255));
	CP_Graphics_DrawLine(start.x, start.y, end.x, end.y);
	CP_Settings_NoStroke();
	DrawArrow(start, road, CP_Color_Create(230, 230, 240, 255));

	const char* side = cross > 0.0f ? "right" : (cross < 0.0f ? "left" : "on the road");
	if (pointHere)
	{
		CP_Settings_Fill(cross > 0.0f ? CP_Color_Create(255, 180, 60, 255) : CP_Color_Create(120, 200, 255, 255));
		CP_Graphics_DrawCircle(point.x, point.y, 24.0f);
	}

	char text[64];
	float x = panelLeft[2] + 410.0f;
	snprintf(text, sizeof(text), "Cross: %+.0f", cross);
	DrawSmallText(text, x, panelTop[2] + 110.0f);
	snprintf(text, sizeof(text), "Side: %s", side);
	DrawSmallText(text, x, panelTop[2] + 135.0f);
	DrawSmallText("Only the sign matters,", x, panelTop[2] + 170.0f);
	DrawSmallText("facing along the arrow.", x, panelTop[2] + 192.0f);
}

//---------------------------------------------------------
// 4. Chase, or run away

CP_Vector creature;
CP_Vector creatureVelocity;
bool runAway = false;
const float creatureTopSpeed = 220.0f;
const float slowDownDistance = 150.0f; // closer than this, a chase slows down to arrive

void DoChase(float dt)
{
	DrawVectorPanel(3, "Chase, or run away", "Move the mouse here. Space: chase or run away.");
	CP_Vector mouse = MousePosition();
	if (CP_Input_KeyTriggered(CP_KEY_SPACE))
	{
		runAway = !runAway;
	}

	// Where it wants to go, and how fast:
	// - chasing: straight at the mouse, at top speed while it's far away,
	//   and slower the closer it gets, down to 0 at the mouse. Always
	//   wanting top speed would make it overshoot, turn round, overshoot
	//   again, and never settle.
	// - running away: the same arrow turned round by CP_Vector_Negate, at
	//   top speed
	CP_Vector desired = CP_Vector_Zero();
	if (InPanel(3, mouse))
	{
		CP_Vector toMouse = CP_Vector_Subtract(mouse, creature);
		float speed = creatureTopSpeed;
		if (!runAway)
		{
			speed *= CP_Math_ClampFloat(CP_Vector_Length(toMouse) / slowDownDistance, 0.0f, 1.0f);
		}
		desired = CP_Vector_Scale(CP_Vector_Normalize(toMouse), speed);
		if (runAway)
		{
			desired = CP_Vector_Negate(desired);
		}
	}

	// Turn its speed part of the way toward that each frame, so it curves
	// instead of snapping round, and never goes over the top speed
	creatureVelocity = CP_Vector_Lerp(creatureVelocity, desired, CP_Math_ClampFloat(3.0f * dt, 0.0f, 1.0f));
	creatureVelocity = CP_Vector_Limit(creatureVelocity, creatureTopSpeed);
	creature = CP_Vector_Add(creature, CP_Vector_Scale(creatureVelocity, dt));

	// Keep it in the panel
	creature.x = CP_Math_ClampFloat(creature.x, panelLeft[3] + 20.0f, panelLeft[3] + PANEL_WIDTH - 20.0f);
	creature.y = CP_Math_ClampFloat(creature.y, panelTop[3] + 80.0f, panelTop[3] + PANEL_HEIGHT - 20.0f);

	CP_Settings_Fill(runAway ? CP_Color_Create(255, 200, 90, 255) : CP_Color_Create(230, 100, 90, 255));
	CP_Graphics_DrawCircle(creature.x, creature.y, 30.0f);
	DrawArrow(creature, CP_Vector_Scale(desired, 0.4f), CP_Color_Create(150, 200, 255, 160));
	DrawArrow(creature, CP_Vector_Scale(creatureVelocity, 0.4f), CP_Color_Create(230, 230, 240, 255));

	char text[64];
	float x = panelLeft[3] + 14.0f;
	snprintf(text, sizeof(text), "%s. White: its speed. Blue: where it wants to go.", runAway ? "Running away" : "Chasing");
	DrawSmallText(text, x, panelTop[3] + 295.0f);
}

//---------------------------------------------------------

void game_init(void)
{
	CP_System_SetWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	CP_System_SetWindowTitle("Vectors");
	CP_Settings_RectMode(CP_POSITION_CORNER);

	turretFacing = CP_Vector_Set(1.0f, 0.0f);
	turretTarget = CP_Vector_Set(panelLeft[0] + 450.0f, panelTop[0] + 260.0f);
	creature = CP_Vector_Set(panelLeft[3] + PANEL_WIDTH / 2.0f, panelTop[3] + 190.0f);
	creatureVelocity = CP_Vector_Zero();
}

void game_update(void)
{
	float dt = CP_System_GetDt();

	CP_Graphics_ClearBackground(CP_Color_Create(24, 26, 36, 255));
	CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
	CP_Settings_TextSize(26.0f);
	CP_Font_DrawText("Vectors: aim, see, which side, chase", 20.0f, 38.0f);

	DoTurret(dt);
	DoGuard(dt);
	DoSide();
	DoChase(dt);
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
