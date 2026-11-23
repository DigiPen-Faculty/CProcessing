//---------------------------------------------------------
// file:	curves.c
//
// Example: arcs, pies and Bezier curves, and a car that drives along them.
//
// Shows how to:
// - draw a pie chart with CP_Graphics_DrawPie, one slice per value
// - draw a spinning loading ring with CP_Graphics_DrawArc
// - draw a curve with CP_Graphics_DrawBezier, and move its control points
// - make a smooth road of Bezier curves through any points, with no corners
//   where one curve meets the next
// - find a point on a curve (BezierPoint) and the way it heads there
//   (BezierDirection), to move something along it and turn it to face ahead
//
// Angles are in degrees. 0 points to the right, and a positive sweep goes
// clockwise (y points down), the same as CP_Vector_FromAngle. A negative
// sweep goes the other way.
//
// A Bezier curve goes from its start (t = 0) to its end (t = 1), but equal
// steps of t aren't equal distances: they bunch up where the curve bends
// tight and spread out where it runs straight. The white dots on the road
// are equal steps of t. Moving t evenly makes the car speed up and slow
// down; dividing by the length of the direction keeps its speed steady.
//
// Drag the round handles of the curve, and the points of the road, with the
// mouse. Space switches the car between a steady speed and moving t evenly.
//
// Try this:
// - Turn the pie into a ring chart by drawing a smaller circle over its middle.
// - Make the pie slice under the mouse stick out.
// - Send an enemy along a curve, the way ships fly in at the start of a wave.
// - Add a second car that drives faster, and make it slow down behind the first.
//
// See also: shapes.c, for curves inside your own shapes; rotation.c, for
// turning things; easing.c, for moving along a line with style; noise.c,
// for hills and other wavy lines.
//
// To run it, copy this file over the template's main.c.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include <math.h>
#include <stdio.h>

#define SLICE_COUNT   4
#define HANDLE_COUNT  4 // the curve's two ends and its two control points
#define HANDLE_RADIUS 12.0f
#define ROAD_POINTS   8 // the road is a loop through these points
#define ROAD_WIDTH    26.0f

float sliceValues[SLICE_COUNT] = { 40.0f, 25.0f, 20.0f, 15.0f };
CP_Color sliceColors[SLICE_COUNT];

// The curve: end, control, control, end
CP_Vector handles[HANDLE_COUNT];

// The road goes through these, back to the first
CP_Vector roadPoints[ROAD_POINTS];

// The point the mouse holds (a handle or a road point), or NULL
CP_Vector* grabbed = NULL;

// Where the car is: the whole part is which curve of the road it's on, and
// the rest is how far along that curve (its t). 2.5 is halfway along the
// third curve.
float carPosition = 0.0f;
bool steadySpeed = true;
const float carSpeed = 180.0f;      // pixels per second, at a steady speed
const float curvesPerSecond = 0.9f; // when t moves evenly

void DrawPieChart(float x, float y, float diameter)
{
	float total = 0.0f;
	for (int i = 0; i < SLICE_COUNT; ++i)
	{
		total += sliceValues[i];
	}

	CP_Settings_Stroke(CP_Color_Create(30, 30, 50, 255));
	CP_Settings_StrokeWeight(2.0f);
	float start = -90.0f; // start at the top
	for (int i = 0; i < SLICE_COUNT; ++i)
	{
		const float sweep = sliceValues[i] / total * 360.0f;
		CP_Settings_Fill(sliceColors[i]);
		CP_Graphics_DrawPie(x, y, diameter, start, sweep);
		start += sweep;
	}
}

// A ring that turns: the arc is long when the sine is high, and it moves
// around once every two seconds
void DrawLoadingRing(float x, float y, float diameter)
{
	const float seconds = CP_System_GetSeconds();
	const float start = seconds * 180.0f;
	const float sweep = 90.0f + 120.0f * (1.0f + sinf(seconds * 3.0f)) * 0.5f;

	CP_Settings_Stroke(CP_Color_Create(80, 80, 110, 255));
	CP_Settings_StrokeWeight(12.0f);
	CP_Graphics_DrawArc(x, y, diameter, 0.0f, 360.0f); // the track

	CP_Settings_Stroke(CP_Color_Create(255, 200, 60, 255));
	CP_Settings_LineCapMode(CP_LINE_CAP_ROUND);
	CP_Graphics_DrawArc(x, y, diameter, start, sweep);
	CP_Settings_LineCapMode(CP_LINE_CAP_BUTT);
}

//---------------------------------------------------------
// Dragging points

// The point within reach of the mouse, or NULL
CP_Vector* PointUnderMouse(CP_Vector points[], int count)
{
	const float mouseX = CP_Input_GetMouseX();
	const float mouseY = CP_Input_GetMouseY();
	for (int i = 0; i < count; ++i)
	{
		if (CP_Math_Distance(mouseX, mouseY, points[i].x, points[i].y) <= HANDLE_RADIUS * 1.5f)
		{
			return &points[i];
		}
	}
	return NULL;
}

void UpdateDragging(void)
{
	if (CP_Input_MouseTriggered(CP_MOUSE_LEFT))
	{
		grabbed = PointUnderMouse(handles, HANDLE_COUNT);
		if (grabbed == NULL)
		{
			grabbed = PointUnderMouse(roadPoints, ROAD_POINTS);
		}
	}
	if (!CP_Input_MouseDown(CP_MOUSE_LEFT))
	{
		grabbed = NULL;
	}
	if (grabbed != NULL)
	{
		grabbed->x = CP_Input_GetMouseX();
		grabbed->y = CP_Input_GetMouseY();
	}
}

void DrawCurve(void)
{
	// The lines from each end to its control point show where the curve is pulled
	CP_Settings_Stroke(CP_Color_Create(120, 120, 150, 255));
	CP_Settings_StrokeWeight(1.0f);
	CP_Graphics_DrawLine(handles[0].x, handles[0].y, handles[1].x, handles[1].y);
	CP_Graphics_DrawLine(handles[3].x, handles[3].y, handles[2].x, handles[2].y);

	CP_Settings_Stroke(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_StrokeWeight(4.0f);
	CP_Graphics_DrawBezier(handles[0].x, handles[0].y, handles[1].x, handles[1].y,
		handles[2].x, handles[2].y, handles[3].x, handles[3].y);

	CP_Settings_NoStroke();
	for (int i = 0; i < HANDLE_COUNT; ++i)
	{
		const CP_Color color = (i == 0 || i == 3) ? CP_Color_Create(255, 200, 60, 255) : CP_Color_Create(90, 200, 255, 255);
		CP_Settings_Fill(color);
		CP_Graphics_DrawCircle(handles[i].x, handles[i].y, HANDLE_RADIUS * 2.0f);
	}
}

//---------------------------------------------------------
// Following a curve

// The point at t on a Bezier curve from start to end, pulled by its two
// control points: t = 0 is the start, 1 the end. Each of the four points
// pulls on the curve, the start hardest near t = 0 and the end near t = 1.
CP_Vector BezierPoint(CP_Vector start, CP_Vector control1, CP_Vector control2, CP_Vector end, float t)
{
	const float u = 1.0f - t;
	const float a = u * u * u;
	const float b = 3.0f * u * u * t;
	const float c = 3.0f * u * t * t;
	const float d = t * t * t;
	return CP_Vector_Set(a * start.x + b * control1.x + c * control2.x + d * end.x,
		a * start.y + b * control1.y + c * control2.y + d * end.y);
}

// The way the curve heads at t. Its length is how many pixels the point
// moves for each 1 of t there, so it's long where the steps of t spread out.
CP_Vector BezierDirection(CP_Vector start, CP_Vector control1, CP_Vector control2, CP_Vector end, float t)
{
	const float u = 1.0f - t;
	const CP_Vector toControl1 = CP_Vector_Subtract(control1, start);
	const CP_Vector between = CP_Vector_Subtract(control2, control1);
	const CP_Vector toEnd = CP_Vector_Subtract(end, control2);
	CP_Vector direction = CP_Vector_Scale(toControl1, 3.0f * u * u);
	direction = CP_Vector_Add(direction, CP_Vector_Scale(between, 6.0f * u * t));
	direction = CP_Vector_Add(direction, CP_Vector_Scale(toEnd, 3.0f * t * t));
	return direction;
}

// The road point i, counting round the loop: after the last comes the first
CP_Vector RoadPoint(int i)
{
	return roadPoints[(i + ROAD_POINTS) % ROAD_POINTS];
}

// The four points of the road's curve from road point i to the next (a
// Catmull-Rom spline, the same as AddSmoothVertices in shapes.c). Each
// control point sits on a line parallel to the one between the points before
// and after, so two curves leave a point heading the same way: no corner.
// (Dividing by 6 makes the curve run close to that line; try 3 or 12.)
void RoadCurve(int i, CP_Vector curve[4])
{
	const CP_Vector before = RoadPoint(i - 1);
	const CP_Vector start = RoadPoint(i);
	const CP_Vector end = RoadPoint(i + 1);
	const CP_Vector after = RoadPoint(i + 2);
	curve[0] = start;
	curve[1] = CP_Vector_Add(start, CP_Vector_Scale(CP_Vector_Subtract(end, before), 1.0f / 6.0f));
	curve[2] = CP_Vector_Subtract(end, CP_Vector_Scale(CP_Vector_Subtract(after, start), 1.0f / 6.0f));
	curve[3] = end;
}

void DrawRoad(void)
{
	CP_Settings_LineCapMode(CP_LINE_CAP_ROUND);
	CP_Vector curve[4];

	// An edge, then the road over it
	for (int pass = 0; pass < 2; ++pass)
	{
		CP_Settings_Stroke(pass == 0 ? CP_Color_Create(200, 200, 210, 255) : CP_Color_Create(70, 70, 85, 255));
		CP_Settings_StrokeWeight(pass == 0 ? ROAD_WIDTH + 6.0f : ROAD_WIDTH);
		for (int i = 0; i < ROAD_POINTS; ++i)
		{
			RoadCurve(i, curve);
			CP_Graphics_DrawBezier(curve[0].x, curve[0].y, curve[1].x, curve[1].y,
				curve[2].x, curve[2].y, curve[3].x, curve[3].y);
		}
	}
	CP_Settings_LineCapMode(CP_LINE_CAP_BUTT);

	// Equal steps of t, which aren't equal distances
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(230, 230, 230, 255));
	for (int i = 0; i < ROAD_POINTS; ++i)
	{
		RoadCurve(i, curve);
		for (int step = 0; step < 8; ++step)
		{
			const CP_Vector dot = BezierPoint(curve[0], curve[1], curve[2], curve[3], step / 8.0f);
			CP_Graphics_DrawCircle(dot.x, dot.y, 4.0f);
		}
	}

	// The points to drag
	CP_Settings_Fill(CP_Color_Create(90, 200, 255, 160));
	for (int i = 0; i < ROAD_POINTS; ++i)
	{
		CP_Graphics_DrawCircle(roadPoints[i].x, roadPoints[i].y, HANDLE_RADIUS * 2.0f);
	}
}

void UpdateCar(void)
{
	CP_Vector curve[4];
	const int which = (int)carPosition;
	RoadCurve(which, curve);
	const float t = carPosition - which;
	const CP_Vector direction = BezierDirection(curve[0], curve[1], curve[2], curve[3], t);

	if (steadySpeed)
	{
		// Pixels to go, divided by pixels per 1 of t here, is how far t moves
		const float pixelsPerT = CP_Vector_Length(direction);
		if (pixelsPerT > 0.001f)
		{
			carPosition += carSpeed * CP_System_GetDt() / pixelsPerT;
		}
	}
	else
	{
		carPosition += curvesPerSecond * CP_System_GetDt();
	}
	while (carPosition >= ROAD_POINTS)
	{
		carPosition -= ROAD_POINTS;
	}
}

void DrawCar(void)
{
	CP_Vector curve[4];
	const int which = (int)carPosition;
	RoadCurve(which, curve);
	const float t = carPosition - which;
	const CP_Vector position = BezierPoint(curve[0], curve[1], curve[2], curve[3], t);
	const CP_Vector direction = BezierDirection(curve[0], curve[1], curve[2], curve[3], t);
	const float angle = CP_Math_Degrees(atan2f(direction.y, direction.x));

	CP_Settings_Save();
	CP_Settings_Translate(position.x, position.y);
	CP_Settings_Rotate(angle);
	CP_Settings_RectMode(CP_POSITION_CENTER);
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(235, 90, 90, 255));
	CP_Graphics_DrawRect(0.0f, 0.0f, 34.0f, 18.0f);
	CP_Settings_Fill(CP_Color_Create(160, 220, 255, 255)); // the windshield, at the front
	CP_Graphics_DrawRect(7.0f, 0.0f, 6.0f, 14.0f);
	CP_Settings_Restore();
}

//---------------------------------------------------------

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Arcs, pies and curves");
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_MIDDLE);

	sliceColors[0] = CP_Color_Create(235, 90, 90, 255);
	sliceColors[1] = CP_Color_Create(250, 190, 70, 255);
	sliceColors[2] = CP_Color_Create(90, 190, 130, 255);
	sliceColors[3] = CP_Color_Create(90, 140, 240, 255);

	handles[0] = CP_Vector_Set(540.0f, 250.0f);
	handles[1] = CP_Vector_Set(560.0f, 50.0f);
	handles[2] = CP_Vector_Set(740.0f, 50.0f);
	handles[3] = CP_Vector_Set(770.0f, 250.0f);

	roadPoints[0] = CP_Vector_Set(90.0f, 450.0f);
	roadPoints[1] = CP_Vector_Set(230.0f, 365.0f);
	roadPoints[2] = CP_Vector_Set(400.0f, 430.0f);
	roadPoints[3] = CP_Vector_Set(570.0f, 365.0f);
	roadPoints[4] = CP_Vector_Set(720.0f, 440.0f);
	roadPoints[5] = CP_Vector_Set(650.0f, 540.0f);
	roadPoints[6] = CP_Vector_Set(430.0f, 510.0f);
	roadPoints[7] = CP_Vector_Set(200.0f, 545.0f);
}

void game_update(void)
{
	if (CP_Input_KeyTriggered(CP_KEY_SPACE))
	{
		steadySpeed = !steadySpeed;
	}
	UpdateDragging();
	UpdateCar();

	CP_Graphics_ClearBackground(CP_Color_Create(30, 30, 50, 255));

	DrawPieChart(170.0f, 160.0f, 230.0f);
	DrawLoadingRing(400.0f, 160.0f, 120.0f);
	DrawCurve();
	DrawRoad();
	DrawCar();

	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(200, 200, 200, 255));
	CP_Settings_TextSize(22.0f);
	CP_Font_DrawText("Pie", 50.0f, 300.0f);
	CP_Font_DrawText("Arc", 370.0f, 300.0f);
	CP_Font_DrawText("Bezier: drag the handles", 520.0f, 300.0f);
	char text[80];
	snprintf(text, sizeof(text), "Road: drag its points.   Car: %s (Space switches)", steadySpeed ? "steady speed" : "t moves evenly");
	CP_Font_DrawText(text, 20.0f, 585.0f);
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
