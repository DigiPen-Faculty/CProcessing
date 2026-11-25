//---------------------------------------------------------
// file:	matrices.c
//
// Example: matrices: parts attached to parts. A tank drives, its turret
// turns on it toward the mouse, a drone circles it, and shots leave from the
// end of the gun.
//
// A matrix holds a move, a turn and a size all in one. Matrices multiply
// together, so a part on a part on a part is one matrix, and the same matrix
// that draws a part also tells you where any point on it is in the world.
//
// Shows how to:
// - make a matrix that moves, turns and sizes something with
//   CP_Matrix_Translate, CP_Matrix_Rotate and CP_Matrix_Scale, put together
//   with CP_Matrix_Multiply
// - attach a part to another: its matrix is its parent's matrix times its
//   own, so it moves, turns and grows with its parent
// - draw through a matrix with CP_Settings_ApplyMatrix
// - find where a point on a part is in the world, like the end of the gun,
//   with CP_Vector_MatrixMultiply
// - go the other way with CP_Matrix_Inverse: where the mouse is on the
//   tank, to aim the turret and to tell if the mouse is over the tank
// - make a matrix from where its axes point with CP_Matrix_FromVector, to
//   face a shot along the way it's going without working out an angle
// - write a matrix out number by number with CP_Matrix_Set, to see where
//   a move lives in it (the shadows)
// - start from CP_Matrix_Identity (no change), and turn by radians with
//   CP_Matrix_RotateRadians
//
// CP_Settings_Translate, CP_Settings_Rotate and CP_Settings_Scale draw
// moved and turned things too (rotation.c). Matrices are for when you also
// need to know where things end up, or keep a part's place for later.
//
// Keys: W and S drive, A and D turn the tank, Z and X make it bigger and
// smaller. The turret aims at the mouse, and a click fires.
//
// Try this:
// - Put the tank on a ship that moves: give the body the ship's matrix as
//   its parent instead of the world's.
// - Give the turret a second gun beside the first, and fire from both ends.
// - Make the drone circle the turret instead of the body.
//
// See also: vectors.c, for turning toward a target at a set speed;
// rotation.c, for drawing turned things; camera.c, for a world bigger than
// the window.
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

// The tank, in the world
CP_Vector tankPosition;
float tankDegrees = -90.0f; // facing up the screen
float tankScale = 1.0f;
float turretDegrees = 0.0f; // the turret's turn on the tank, not in the world

// Each part has its own space: (0, 0) is its middle, and it faces right
// (along x). These are in those spaces, so they never change, however the
// tank moves.
const float bodyLength = 70.0f;
const float bodyWidth = 46.0f;
const float turretBack = -8.0f; // where the turret sits on the body, along it
const float barrelLength = 52.0f;

// The matrices, made again every frame from the values above
CP_Matrix bodyMatrix;
CP_Matrix turretMatrix;
CP_Matrix droneMatrix;
bool mouseOverTank = false;

// The shots, in the world
#define MAX_SHOTS 40
CP_Vector shotPositions[MAX_SHOTS];
CP_Vector shotVelocities[MAX_SHOTS];
bool shotFlying[MAX_SHOTS];
const float shotSpeed = 600.0f;

//---------------------------------------------------------
// Making the matrices

// A matrix that sizes, then turns, then moves. Multiply(a, b) does b
// first, then a, so read each line from right to left.
CP_Matrix Place(CP_Vector position, float degrees, float scale)
{
	CP_Matrix turnAndSize = CP_Matrix_Multiply(CP_Matrix_Rotate(degrees), CP_Matrix_Scale(CP_Vector_Set(scale, scale)));
	return CP_Matrix_Multiply(CP_Matrix_Translate(position), turnAndSize);
}

void MakeMatrices(void)
{
	// The world is the tank's parent. Its matrix is the identity: it
	// changes nothing. A tank on a ship would use the ship's matrix here.
	CP_Matrix world = CP_Matrix_Identity();
	bodyMatrix = CP_Matrix_Multiply(world, Place(tankPosition, tankDegrees, tankScale));

	// Where the turret sits on the body, before it turns
	CP_Matrix turretBase = CP_Matrix_Multiply(bodyMatrix, CP_Matrix_Translate(CP_Vector_Set(turretBack, 0.0f)));

	// The mouse in the turret base's own space: the inverse undoes the
	// matrix, taking a point in the world back into the part's space. There,
	// the angle to the mouse is the turn the turret needs, whatever the body
	// is doing. (For a matrix that only turns, CP_Matrix_Transpose gives the
	// same as the inverse.)
	CP_Vector mouse = CP_Vector_Set(CP_Input_GetMouseX(), CP_Input_GetMouseY());
	CP_Vector mouseOnBase = CP_Vector_MatrixMultiply(CP_Matrix_Inverse(turretBase), mouse);
	turretDegrees = CP_Math_Degrees(atan2f(mouseOnBase.y, mouseOnBase.x));
	turretMatrix = CP_Matrix_Multiply(turretBase, CP_Matrix_Rotate(turretDegrees));

	// The drone circles the body: turn by the time, then move out 80 along
	// the turned x. CP_Matrix_RotateRadians takes radians, like sinf and
	// atan2f give: a whole turn is 2 pi, so this turns once every 4 seconds.
	CP_Matrix circling = CP_Matrix_RotateRadians(CP_System_GetSeconds() * 2.0f * 3.14159265f / 4.0f);
	CP_Matrix out = CP_Matrix_Translate(CP_Vector_Set(80.0f, 0.0f));
	droneMatrix = CP_Matrix_Multiply(bodyMatrix, CP_Matrix_Multiply(circling, out));

	// Is the mouse over the tank? In the body's own space the tank isn't
	// turned or sized, so it's a plain rectangle check
	CP_Vector mouseOnBody = CP_Vector_MatrixMultiply(CP_Matrix_Inverse(bodyMatrix), mouse);
	mouseOverTank = fabsf(mouseOnBody.x) <= bodyLength / 2.0f && fabsf(mouseOnBody.y) <= bodyWidth / 2.0f;
}

//---------------------------------------------------------
// Moving

void DriveTank(float dt)
{
	if (CP_Input_KeyDown(CP_KEY_A))
	{
		tankDegrees -= 120.0f * dt;
	}
	if (CP_Input_KeyDown(CP_KEY_D))
	{
		tankDegrees += 120.0f * dt;
	}
	float speed = 0.0f;
	if (CP_Input_KeyDown(CP_KEY_W))
	{
		speed = 200.0f;
	}
	if (CP_Input_KeyDown(CP_KEY_S))
	{
		speed = -120.0f;
	}
	tankPosition = CP_Vector_Add(tankPosition, CP_Vector_Scale(CP_Vector_FromAngle(tankDegrees), speed * dt));
	tankPosition.x = CP_Math_ClampFloat(tankPosition.x, 40.0f, WINDOW_WIDTH - 40.0f);
	tankPosition.y = CP_Math_ClampFloat(tankPosition.y, 40.0f, WINDOW_HEIGHT - 40.0f);

	if (CP_Input_KeyDown(CP_KEY_Z))
	{
		tankScale += dt;
	}
	if (CP_Input_KeyDown(CP_KEY_X))
	{
		tankScale -= dt;
	}
	tankScale = CP_Math_ClampFloat(tankScale, 0.5f, 2.0f);
}

void Fire(void)
{
	// The end of the barrel and the turret's middle, in the turret's space,
	// taken into the world by its matrix. They're wherever the tank has
	// driven, turned and grown to, and the shot goes from one to the other.
	CP_Vector tip = CP_Vector_MatrixMultiply(turretMatrix, CP_Vector_Set(barrelLength, 0.0f));
	CP_Vector middle = CP_Vector_MatrixMultiply(turretMatrix, CP_Vector_Zero());
	CP_Vector direction = CP_Vector_Normalize(CP_Vector_Subtract(tip, middle));

	for (int i = 0; i < MAX_SHOTS; ++i)
	{
		if (!shotFlying[i])
		{
			shotFlying[i] = true;
			shotPositions[i] = tip;
			shotVelocities[i] = CP_Vector_Scale(direction, shotSpeed);
			return;
		}
	}
}

void MoveShots(float dt)
{
	for (int i = 0; i < MAX_SHOTS; ++i)
	{
		if (!shotFlying[i])
		{
			continue;
		}
		shotPositions[i] = CP_Vector_Add(shotPositions[i], CP_Vector_Scale(shotVelocities[i], dt));
		CP_Vector p = shotPositions[i];
		if (p.x < -20.0f || p.x > WINDOW_WIDTH + 20.0f || p.y < -20.0f || p.y > WINDOW_HEIGHT + 20.0f)
		{
			shotFlying[i] = false;
		}
	}
}

//---------------------------------------------------------
// Drawing

// Draws a vector as an arrow from a point
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

// Each part is drawn in its own space, around (0, 0), facing right.
// CP_Settings_ApplyMatrix puts it where its matrix says.
void DrawBody(CP_Matrix matrix, CP_Color tracks, CP_Color hull)
{
	CP_Settings_ResetMatrix();
	CP_Settings_ApplyMatrix(matrix);
	CP_Settings_RectMode(CP_POSITION_CENTER);
	CP_Settings_Fill(tracks);
	CP_Graphics_DrawRect(0.0f, -bodyWidth / 2.0f + 6.0f, bodyLength + 6.0f, 14.0f);
	CP_Graphics_DrawRect(0.0f, bodyWidth / 2.0f - 6.0f, bodyLength + 6.0f, 14.0f);
	CP_Settings_Fill(hull);
	CP_Graphics_DrawRect(0.0f, 0.0f, bodyLength, bodyWidth - 16.0f);
	CP_Graphics_DrawTriangle(bodyLength / 2.0f, -8.0f, bodyLength / 2.0f, 8.0f, bodyLength / 2.0f + 8.0f, 0.0f);
}

void DrawTurret(CP_Matrix matrix, CP_Color color)
{
	CP_Settings_ResetMatrix();
	CP_Settings_ApplyMatrix(matrix);
	CP_Settings_Fill(color);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Graphics_DrawRect(0.0f, -4.0f, barrelLength, 8.0f);
	CP_Graphics_DrawCircle(0.0f, 0.0f, 30.0f);
}

void DrawTank(void)
{
	CP_Settings_NoStroke();

	// The shadows: each part again, moved down and to the right in the
	// world, as if the sun were up and to the left. The move comes after the
	// part's own matrix (it's on the left), so it's the same way whichever
	// way the tank faces. The drone flies higher, so its shadow lands
	// further away.
	// CP_Matrix_Set writes a matrix out number by number. This one is the
	// same as CP_Matrix_Translate(CP_Vector_Set(8.0f, 10.0f)), written out
	// to show where a move lives: in the third column, like the white
	// numbers on the screen. Set makes any matrix, even ones no other
	// function makes.
	CP_Matrix tankShadow = CP_Matrix_Set(1.0f, 0.0f, 8.0f, 0.0f, 1.0f, 10.0f, 0.0f, 0.0f, 1.0f);
	CP_Matrix droneShadow = CP_Matrix_Translate(CP_Vector_Set(30.0f, 38.0f));
	// Solid, not see-through, so where two shadows overlap isn't darker
	CP_Color shadow = CP_Color_Create(92, 84, 60, 255);
	DrawBody(CP_Matrix_Multiply(tankShadow, bodyMatrix), shadow, shadow);
	DrawTurret(CP_Matrix_Multiply(tankShadow, turretMatrix), shadow);
	CP_Settings_ResetMatrix();
	CP_Settings_ApplyMatrix(CP_Matrix_Multiply(droneShadow, droneMatrix));
	CP_Graphics_DrawCircle(0.0f, 0.0f, 14.0f);

	CP_Color hull = mouseOverTank ? CP_Color_Create(130, 170, 100, 255) : CP_Color_Create(100, 130, 80, 255);
	DrawBody(bodyMatrix, CP_Color_Create(50, 55, 45, 255), hull);
	DrawTurret(turretMatrix, CP_Color_Create(80, 105, 65, 255));

	// The drone: a body and four rotors that spin
	CP_Settings_ResetMatrix();
	CP_Settings_ApplyMatrix(droneMatrix);
	CP_Settings_Fill(CP_Color_Create(200, 200, 220, 255));
	CP_Graphics_DrawCircle(0.0f, 0.0f, 14.0f);
	CP_Settings_Stroke(CP_Color_Create(200, 200, 220, 160));
	CP_Settings_StrokeWeight(2.0f);
	float spin = CP_System_GetSeconds() * 1500.0f;
	for (int i = 0; i < 4; ++i)
	{
		CP_Vector arm = CP_Vector_Rotate(CP_Vector_Set(11.0f, 0.0f), 45.0f + i * 90.0f);
		CP_Vector blade = CP_Vector_Rotate(CP_Vector_Set(6.0f, 0.0f), spin);
		CP_Graphics_DrawLine(arm.x - blade.x, arm.y - blade.y, arm.x + blade.x, arm.y + blade.y);
	}
	CP_Settings_NoStroke();
	CP_Settings_ResetMatrix();
}

void DrawShots(void)
{
	CP_Settings_Fill(CP_Color_Create(255, 220, 90, 255));
	for (int i = 0; i < MAX_SHOTS; ++i)
	{
		if (!shotFlying[i])
		{
			continue;
		}
		// A matrix's three columns say where its x axis points, where its y
		// axis points, and where its (0, 0) goes. So a shot faces the way
		// it's going with no angle: x along its direction, y across it, and
		// (0, 0) where the shot is. Then a long ellipse along x points the
		// right way.
		CP_Vector along = CP_Vector_Normalize(shotVelocities[i]);
		CP_Vector across = CP_Vector_Set(-along.y, along.x);
		CP_Settings_ResetMatrix();
		CP_Settings_ApplyMatrix(CP_Matrix_FromVector(along, across, shotPositions[i]));
		CP_Graphics_DrawEllipse(0.0f, 0.0f, 18.0f, 6.0f);
	}
	CP_Settings_ResetMatrix();
}

// The turret's matrix, and its first two columns drawn as arrows from its
// third: the turret's x axis (red) and y axis (green), from its (0, 0)
void DrawTurretMatrix(void)
{
	CP_Vector origin = CP_Vector_Set(turretMatrix.m02, turretMatrix.m12);
	CP_Vector xAxis = CP_Vector_Set(turretMatrix.m00, turretMatrix.m10);
	CP_Vector yAxis = CP_Vector_Set(turretMatrix.m01, turretMatrix.m11);
	const CP_Color red = CP_Color_Create(240, 90, 90, 255);
	const CP_Color green = CP_Color_Create(110, 220, 120, 255);
	const CP_Color white = CP_Color_Create(235, 235, 245, 255);
	DrawArrow(origin, CP_Vector_Scale(xAxis, 40.0f), red);
	DrawArrow(origin, CP_Vector_Scale(yAxis, 40.0f), green);

	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(CP_Color_Create(20, 24, 34, 220));
	CP_Graphics_DrawRect(20.0f, 20.0f, 360.0f, 200.0f);

	CP_Settings_TextSize(20.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);
	CP_Settings_Fill(white);
	CP_Font_DrawText("The turret's matrix", 36.0f, 48.0f);

	// Three columns of numbers, each in its color
	const float columnX[3] = { 70.0f, 160.0f, 250.0f };
	const float rowY[3] = { 84.0f, 110.0f, 136.0f };
	const float numbers[3][3] = {
		{ turretMatrix.m00, turretMatrix.m01, turretMatrix.m02 },
		{ turretMatrix.m10, turretMatrix.m11, turretMatrix.m12 },
		{ turretMatrix.m20, turretMatrix.m21, turretMatrix.m22 },
	};
	const CP_Color columnColor[3] = { red, green, white };
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_RIGHT, CP_TEXT_ALIGN_V_BASELINE);
	for (int row = 0; row < 3; ++row)
	{
		for (int column = 0; column < 3; ++column)
		{
			char text[16];
			snprintf(text, sizeof(text), "%.2f", numbers[row][column]);
			CP_Settings_Fill(columnColor[column]);
			CP_Font_DrawText(text, columnX[column] + 40.0f, rowY[row]);
		}
	}

	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);
	CP_Settings_TextSize(16.0f);
	CP_Settings_Fill(red);
	CP_Font_DrawText("Where its x points (along the gun)", 36.0f, 166.0f);
	CP_Settings_Fill(green);
	CP_Font_DrawText("Where its y points", 36.0f, 186.0f);
	CP_Settings_Fill(white);
	CP_Font_DrawText("Where its (0, 0) is, in the world", 36.0f, 206.0f);
}

void DrawHelp(void)
{
	CP_Settings_TextSize(18.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);
	CP_Settings_Fill(CP_Color_Create(220, 225, 210, 255));
	CP_Font_DrawText("W S: drive   A D: turn   Z X: bigger and smaller   Mouse: aim   Click: fire", 20.0f, WINDOW_HEIGHT - 20.0f);
	if (mouseOverTank)
	{
		CP_Font_DrawText("The mouse is over the tank (checked in the tank's own space)", 20.0f, WINDOW_HEIGHT - 44.0f);
	}
}

//---------------------------------------------------------

void game_init(void)
{
	CP_System_SetWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	CP_System_SetWindowTitle("Matrices");
	tankPosition = CP_Vector_Set(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f + 80.0f);
}

void game_update(void)
{
	float dt = CP_System_GetDt();
	DriveTank(dt);
	MakeMatrices();
	if (CP_Input_MouseTriggered(CP_MOUSE_LEFT))
	{
		Fire();
	}
	MoveShots(dt);

	CP_Graphics_ClearBackground(CP_Color_Create(120, 110, 80, 255));
	CP_Settings_Stroke(CP_Color_Create(135, 125, 92, 255));
	CP_Settings_StrokeWeight(2.0f);
	for (float x = 0.0f; x < WINDOW_WIDTH; x += 80.0f)
	{
		CP_Graphics_DrawLine(x, 0.0f, x, WINDOW_HEIGHT);
	}
	for (float y = 0.0f; y < WINDOW_HEIGHT; y += 80.0f)
	{
		CP_Graphics_DrawLine(0.0f, y, WINDOW_WIDTH, y);
	}

	DrawShots();
	DrawTank();
	DrawTurretMatrix();
	DrawHelp();
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
