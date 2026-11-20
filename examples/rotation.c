//---------------------------------------------------------
// file:	rotation.c
//
// Example: turning shapes and sprites around any point you choose.
//
// Turning something takes five steps:
//
//     CP_Settings_Save();             // remember the settings
//     CP_Settings_Translate(x, y);    // move (0, 0) to the point to turn around
//     CP_Settings_Rotate(degrees);    // turn everything drawn after this
//     CP_Graphics_DrawRect(0, 0, w, h);
//     CP_Settings_Restore();          // back to how it was before Save
//
// After Translate and Rotate, (0, 0) is the pivot: the point things turn
// around. Where you draw, compared with (0, 0), decides which part of the
// shape sits on the pivot. Draw a centered rectangle at (0, 0) and it turns
// around its center; draw it from its corner at (0, 0) and it turns around
// that corner; draw it at (-20, -40) and it turns around the point 20 across
// and 40 down from its corner. Draw it at (80, 0) and it circles around the
// pivot without touching it.
//
// The top row shows those basics, and the bottom row uses them: an orbit,
// an arm with two joints, a sprite rocking on its feet, and a cannon facing
// the mouse. In every box the yellow dot is the pivot, and the orange line
// goes from the pivot to the point the shape is drawn at.
//
// BeginRotation, EndRotation and DrawRectRotated below save writing the
// five steps every time: copy them into your own game.
//
// Everything turns on its own until Space stops it (Space again starts it).
// Hold Left or Right to turn back or forward. Drag in the fourth box to slide
// its rectangle across the pivot, or off it.
//
// Try this:
// - Make the moon in the orbit box spin around itself as it goes round.
// - Give the arm a third joint, a hand that opens and closes.
// - Write DrawImageRotated, like DrawRectRotated, with CP_Image_Draw and
//   CP_Settings_ImageMode(CP_POSITION_CORNER).
// - Turn a ship with the arrow keys, and move it forward the way it faces
//   with CP_Vector_FromAngle.
//
// To run it, copy this file over the template's main.c.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include <math.h>
#include <stdio.h>

//---------------------------------------------------------
// Helpers to copy into your own game

// Moves (0, 0) to (x, y) and turns everything drawn after it by degrees
// (clockwise) around that point, until EndRotation. In between, draw as if
// (x, y) were (0, 0) and nothing were turned. Settings changed in between,
// like colors or the rect mode, go back to how they were at EndRotation.
// Each BeginRotation needs its EndRotation; they can go inside each other.
void BeginRotation(float x, float y, float degrees)
{
	CP_Settings_Save();
	CP_Settings_Translate(x, y);
	CP_Settings_Rotate(degrees);
}

void EndRotation(void)
{
	CP_Settings_Restore();
}

// Draws a w by h rectangle turned by degrees around its pivot, and puts the
// pivot at (x, y). The pivot is pivotX across and pivotY down from the
// rectangle's top-left corner: (w / 2, h / 2) turns it around its center,
// (0, 0) around its top-left corner, (w / 2, h) around the middle of its
// bottom edge. At 0 degrees, the corner is at (x - pivotX, y - pivotY).
void DrawRectRotated(float x, float y, float w, float h, float degrees, float pivotX, float pivotY)
{
	BeginRotation(x, y, degrees);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Graphics_DrawRect(-pivotX, -pivotY, w, h);
	EndRotation();
}

//---------------------------------------------------------
// What the boxes share

#define COLUMNS    4
#define BOX_WIDTH  230.0f
#define BOX_HEIGHT 320.0f
#define MARGIN     18.0f

float turnTime = 0.0f;          // seconds of turning, forward or back
CP_BOOL turningOnItsOwn = TRUE; // Space switches it
const float turnSpeed = 45.0f;  // degrees per second

CP_Image slimeSheet; // 8 frames in a row, each 64x64
const float frameSize = 64.0f;

// The fourth box's rectangle, its pivot from its top-left corner (which can
// be off the rectangle), and whether the mouse is dragging it
const float chooseWidth = 64.0f;
const float chooseHeight = 40.0f;
float choosePivotX = 16.0f;
float choosePivotY = 10.0f;
CP_BOOL dragging = FALSE;

CP_Color shapeColor;
CP_Color pivotColor;
CP_Color offsetColor;

// The pivot: a yellow dot. (0, 0) is the pivot after BeginRotation.
void DrawPivot(float x, float y)
{
	CP_Settings_NoStroke();
	CP_Settings_Fill(pivotColor);
	CP_Graphics_DrawCircle(x, y, 10.0f);
}

// An orange line from the pivot to the point a shape is drawn at
void DrawOffset(float x, float y)
{
	CP_Settings_StrokeWeight(2.0f);
	CP_Settings_Stroke(offsetColor);
	CP_Graphics_DrawLine(0.0f, 0.0f, x, y);
	CP_Settings_NoStroke();
	CP_Settings_Fill(offsetColor);
	CP_Graphics_DrawCircle(x, y, 7.0f);
}

// The box itself, its title and two lines of code at the bottom
void DrawBox(float left, float top, const char* title, const char* code1, const char* code2)
{
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(40, 44, 60, 255));
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Graphics_DrawRect(left, top, BOX_WIDTH, BOX_HEIGHT);

	CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
	CP_Settings_TextSize(20.0f);
	CP_Font_DrawText(title, left + 12.0f, top + 26.0f);

	CP_Settings_Fill(CP_Color_Create(150, 200, 255, 255));
	CP_Settings_TextSize(16.0f);
	CP_Font_DrawText(code1, left + 12.0f, top + BOX_HEIGHT - 32.0f);
	CP_Font_DrawText(code2, left + 12.0f, top + BOX_HEIGHT - 12.0f);
}

// The middle of the picture in a box, between the title and the code
float PictureX(float left)
{
	return left + BOX_WIDTH / 2.0f;
}

float PictureY(float top)
{
	return top + 150.0f;
}

//---------------------------------------------------------
// The top row: where you draw is what it turns around

const float rectWidth = 88.0f;
const float rectHeight = 56.0f;

void DrawAroundCenter(float left, float top, float angle)
{
	DrawBox(left, top, "Around its center", "RectMode CENTER", "DrawRect(0, 0, w, h)");

	BeginRotation(PictureX(left), PictureY(top), angle);
	CP_Settings_RectMode(CP_POSITION_CENTER);
	CP_Settings_Fill(shapeColor);
	CP_Graphics_DrawRect(0.0f, 0.0f, rectWidth, rectHeight);
	DrawPivot(0.0f, 0.0f);
	EndRotation();
}

void DrawAroundCorner(float left, float top, float angle)
{
	DrawBox(left, top, "Around its corner", "RectMode CORNER", "DrawRect(0, 0, w, h)");

	BeginRotation(PictureX(left), PictureY(top), angle);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(shapeColor);
	CP_Graphics_DrawRect(0.0f, 0.0f, rectWidth, rectHeight);
	DrawPivot(0.0f, 0.0f);
	EndRotation();
}

void DrawAroundAnyPoint(float left, float top, float angle)
{
	DrawBox(left, top, "Around any point", "RectMode CORNER", "DrawRect(-22, -40, w, h)");

	// The pivot is 22 across and 40 down from the corner, so draw the corner
	// 22 left and 40 up from the pivot
	BeginRotation(PictureX(left), PictureY(top), angle);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(shapeColor);
	CP_Graphics_DrawRect(-22.0f, -40.0f, rectWidth, rectHeight);
	DrawOffset(-22.0f, -40.0f);
	DrawPivot(0.0f, 0.0f);
	EndRotation();
}

void DrawChosenPivot(float left, float top, float angle)
{
	char code[40];
	snprintf(code, sizeof(code), "   angle, %.0f, %.0f)", roundf(choosePivotX), roundf(choosePivotY));
	DrawBox(left, top, "Drag it on its pin", "DrawRectRotated(x, y, w, h,", code);

	// The pin stays in the middle of the box. Dragging slides the rectangle
	// across it, which changes where the pin goes through the rectangle:
	// its pivot. Slide it off the pin and it swings around the pin, like the
	// moon or the forearm below. The mouse moves on the screen, so turn its
	// movement back by the angle to get the movement along the turned
	// rectangle.
	const float mouseX = CP_Input_GetMouseX();
	const float mouseY = CP_Input_GetMouseY();
	if (CP_Input_MouseTriggered(CP_MOUSE_LEFT))
	{
		dragging = mouseX >= left && mouseX < left + BOX_WIDTH && mouseY >= top && mouseY < top + BOX_HEIGHT;
	}
	if (!CP_Input_MouseDown(CP_MOUSE_LEFT))
	{
		dragging = FALSE;
	}
	if (dragging)
	{
		CP_Vector moved = CP_Vector_Rotate(CP_Vector_Set(CP_Input_GetMouseDeltaX(), CP_Input_GetMouseDeltaY()), -angle);
		choosePivotX -= moved.x;
		choosePivotY -= moved.y;

		// Keep the pin within 62 pixels of the rectangle's middle, so the
		// rectangle stays in the box as it swings around
		const CP_Vector middle = CP_Vector_Set(chooseWidth / 2.0f, chooseHeight / 2.0f);
		CP_Vector fromMiddle = CP_Vector_Subtract(CP_Vector_Set(choosePivotX, choosePivotY), middle);
		fromMiddle = CP_Vector_Limit(fromMiddle, 62.0f);
		choosePivotX = middle.x + fromMiddle.x;
		choosePivotY = middle.y + fromMiddle.y;
	}

	const float x = PictureX(left);
	const float y = PictureY(top);
	CP_Settings_NoStroke();
	CP_Settings_Fill(shapeColor);
	DrawRectRotated(x, y, chooseWidth, chooseHeight, angle, choosePivotX, choosePivotY);

	BeginRotation(x, y, angle);
	DrawOffset(-choosePivotX, -choosePivotY);
	DrawPivot(0.0f, 0.0f);
	EndRotation();
}

//---------------------------------------------------------
// The bottom row: putting it to use

void DrawOrbit(float left, float top, float angle)
{
	DrawBox(left, top, "Pivot outside: orbit", "Rotate, then draw away", "DrawCircle(80, 0, d)");

	const float x = PictureX(left);
	const float y = PictureY(top);

	// The planet doesn't turn
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(90, 160, 255, 255));
	CP_Graphics_DrawCircle(x, y, 60.0f);

	// The moon is drawn 80 pixels to the right of the pivot, so turning
	// carries it around the planet
	BeginRotation(x, y, angle * 2.0f);
	DrawOffset(80.0f, 0.0f);
	CP_Settings_Fill(CP_Color_Create(220, 220, 230, 255));
	CP_Graphics_DrawCircle(80.0f, 0.0f, 24.0f);
	DrawPivot(0.0f, 0.0f);
	EndRotation();
}

void DrawArm(float left, float top, float seconds)
{
	DrawBox(left, top, "Joints: turns add up", "Begin at the shoulder,", "Begin again at the elbow");

	const float shoulderX = left + 60.0f;
	const float shoulderY = PictureY(top) - 30.0f;
	const float shoulderAngle = -20.0f + 35.0f * sinf(seconds * 1.3f);
	const float elbowAngle = 60.0f + 45.0f * sinf(seconds * 2.1f);

	// The body the arm hangs from, not turned
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(110, 120, 150, 255));
	CP_Settings_RectMode(CP_POSITION_CENTER);
	CP_Graphics_DrawRect(shoulderX - 22.0f, shoulderY + 50.0f, 40.0f, 130.0f);

	BeginRotation(shoulderX, shoulderY, shoulderAngle);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(shapeColor);
	CP_Graphics_DrawRect(-10.0f, -10.0f, 90.0f, 20.0f); // upper arm: the pivot 10 in from its end

	// The elbow is 70 along the upper arm. Inside the shoulder's turn, so
	// the forearm turns with the upper arm, and then by its own angle too.
	BeginRotation(70.0f, 0.0f, elbowAngle);
	CP_Settings_Fill(CP_Color_Create(255, 150, 110, 255));
	CP_Graphics_DrawRect(-8.0f, -8.0f, 78.0f, 16.0f);
	CP_Settings_Fill(CP_Color_Create(255, 200, 160, 255));
	CP_Graphics_DrawCircle(70.0f, 0.0f, 22.0f); // the hand, at the end
	DrawPivot(0.0f, 0.0f);
	EndRotation();

	DrawPivot(0.0f, 0.0f);
	EndRotation();
}

void DrawSprite(float left, float top, float seconds)
{
	DrawBox(left, top, "A sprite on its feet", "ImageMode CORNER", "DrawSubImage(-w/2, -h, ...)");

	const float size = frameSize * 2.0f; // drawn twice as big
	const float groundY = PictureY(top) + 60.0f;
	const float wobble = 25.0f * sinf(seconds * 3.0f);

	CP_Settings_StrokeWeight(2.0f);
	CP_Settings_Stroke(CP_Color_Create(90, 96, 120, 255));
	CP_Graphics_DrawLine(left + 20.0f, groundY, left + BOX_WIDTH - 20.0f, groundY);

	// The frame to show, 8 a second
	int frame = (int)(seconds * 8.0f) % 8;
	if (frame < 0)
	{
		frame += 8; // below 0 after turning back by hand
	}
	const float frameLeft = (float)frame * frameSize;

	// The pivot is the middle of the bottom edge, so draw the corner half
	// the width to the left and the whole height up
	BeginRotation(PictureX(left), groundY, wobble);
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_Image_DrawSubImage(slimeSheet, -size / 2.0f, -size, size, size, frameLeft, 0.0f, frameLeft + frameSize, frameSize, 255);
	CP_Settings_NoFill(); // the outline of the frame: its corner is where it's drawn
	CP_Settings_StrokeWeight(1.0f);
	CP_Settings_Stroke(CP_Color_Create(90, 96, 120, 255));
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Graphics_DrawRect(-size / 2.0f, -size, size, size);
	DrawOffset(-size / 2.0f, -size);
	DrawPivot(0.0f, 0.0f);
	EndRotation();
}

void DrawCannon(float left, float top)
{
	DrawBox(left, top, "Facing the mouse", "a = Degrees(atan2f(dy, dx))", "DrawRect(0, -h/2, len, h)");

	const float x = PictureX(left);
	const float y = PictureY(top) + 40.0f;

	// The angle from the cannon to the mouse. Degrees go clockwise, because
	// y goes down the screen, and so does atan2f with y going down.
	const float dx = CP_Input_GetMouseX() - x;
	const float dy = CP_Input_GetMouseY() - y;
	const float aim = CP_Math_Degrees(atan2f(dy, dx));

	// The barrel starts at the pivot and points along x, the way 0 degrees
	// points, so it faces wherever it's turned
	BeginRotation(x, y, aim);
	CP_Settings_NoStroke();
	CP_Settings_Fill(shapeColor);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Graphics_DrawRect(0.0f, -9.0f, 80.0f, 18.0f);
	DrawOffset(0.0f, -9.0f);
	EndRotation();

	// The base, on top of the barrel, not turned
	CP_Settings_Fill(CP_Color_Create(110, 120, 150, 255));
	CP_Graphics_DrawCircle(x, y, 44.0f);
	DrawPivot(x, y);
}

//---------------------------------------------------------

void game_init(void)
{
	CP_System_SetWindowSize((int)(COLUMNS * (BOX_WIDTH + MARGIN) + MARGIN), 720);
	CP_System_SetWindowTitle("Rotation and pivots");
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);
	slimeSheet = CP_Image_Load("assets/SlimeSheet.png");

	shapeColor = CP_Color_Create(255, 120, 90, 255);
	pivotColor = CP_Color_Create(255, 220, 80, 255);
	offsetColor = CP_Color_Create(255, 170, 60, 255);
}

void game_update(void)
{
	// Left and Right turn back and forward at the usual speed. Without them,
	// it turns forward on its own, until Space stops it.
	if (CP_Input_KeyTriggered(CP_KEY_SPACE))
	{
		turningOnItsOwn = !turningOnItsOwn;
	}
	if (CP_Input_KeyDown(CP_KEY_LEFT))
	{
		turnTime -= CP_System_GetDt();
	}
	else if (CP_Input_KeyDown(CP_KEY_RIGHT) || turningOnItsOwn)
	{
		turnTime += CP_System_GetDt();
	}
	const float angle = turnTime * turnSpeed;

	CP_Graphics_ClearBackground(CP_Color_Create(24, 26, 36, 255));

	const float column[COLUMNS] = { MARGIN, MARGIN + (BOX_WIDTH + MARGIN), MARGIN + 2.0f * (BOX_WIDTH + MARGIN), MARGIN + 3.0f * (BOX_WIDTH + MARGIN) };
	const float topRow = MARGIN;
	const float bottomRow = MARGIN * 2.0f + BOX_HEIGHT;

	DrawAroundCenter(column[0], topRow, angle);
	DrawAroundCorner(column[1], topRow, angle);
	DrawAroundAnyPoint(column[2], topRow, angle);
	DrawChosenPivot(column[3], topRow, angle);

	DrawOrbit(column[0], bottomRow, angle);
	DrawArm(column[1], bottomRow, turnTime);
	DrawSprite(column[2], bottomRow, turnTime);
	DrawCannon(column[3], bottomRow);

	CP_Settings_Fill(CP_Color_Create(170, 170, 190, 255));
	CP_Settings_TextSize(20.0f);
	char help[100];
	snprintf(help, sizeof(help), "Space: %s    Left/Right: turn back/forward    Drag the fourth box", turningOnItsOwn ? "stop" : "turn on its own");
	CP_Font_DrawText(help, MARGIN, 705.0f);
}

void game_exit(void)
{
	CP_Image_Free(&slimeSheet);
}

int main(void)
{
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
