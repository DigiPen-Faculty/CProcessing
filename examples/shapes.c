//---------------------------------------------------------
// file:	shapes.c
//
// Example: your own shapes, with straight sides, curves and holes.
//
// Shows how to:
// - make a star and regular polygons from a loop, with CP_Graphics_AddVertex
// - give a shape curved edges: CP_Graphics_AddQuadraticVertex bends toward
//   one control point, CP_Graphics_AddBezierVertex has two, for S curves
// - draw a smooth curve through a list of points (the AddSmoothVertices
//   function below: copy it into your game)
// - cut holes in a shape with CP_Graphics_BeginHole and CP_Graphics_EndHole
// - choose how an outline turns its corners with CP_Settings_LineJointMode:
//   sharp (miter), cut off (bevel, the default) or round
// - draw a dot with CP_Graphics_DrawPoint
//
// A shape always goes in this order:
//
//     CP_Graphics_BeginShape();
//     ...the whole outline: AddVertex and the curves...
//     CP_Graphics_BeginHole();      // any holes, each between
//     ...the hole's outline...      // BeginHole and EndHole
//     CP_Graphics_EndHole();
//     CP_Graphics_EndShape();       // fills and outlines it
//
// You don't need to go back to the first point: EndShape closes the outline
// with a straight edge back to it. A curve starts from the point before it,
// so start the outline, and each hole, with AddVertex. The points of a hole
// can go either way round. Keep holes inside the shape and apart from each
// other: where a hole pokes out, or two holes overlap, it's filled again.
//
// Press Space to see the points: the ones the shape goes through (filled)
// and the control points the curves bend toward (hollow). J changes the
// star's corners.
//
// Try this:
// - Give the star more points, or make its inner radius change with time.
// - Make a cloud from a blob with more points and a slower wobble.
// - Cut a keyhole in a door: a circle and a trapezoid as one hole.
//
// See also: curves.c, for arcs, pies and Bezier curves; tiled_fill.c and
// gradients.c, to fill your shapes.
//
// To run it, copy this file over the template's main.c.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include <math.h>

#define WINDOW_WIDTH  800.0f
#define WINDOW_HEIGHT 600.0f
#define BLOB_POINTS   8

bool showPoints = false;

// How the star's outline turns its corners (J changes it)
#define JOINT_COUNT 3
const CP_LINE_JOINT_MODE joints[JOINT_COUNT] = { CP_LINE_JOINT_MITER, CP_LINE_JOINT_BEVEL, CP_LINE_JOINT_ROUND };
const char* jointNames[JOINT_COUNT] = { "J: sharp corners (miter)", "J: cut-off corners (bevel)", "J: round corners" };
int jointIndex = 0;

//---------------------------------------------------------
// Showing the points (Space)

// A point the shape goes through is a white dot; a control point is a
// yellow ring, with a thin line to the point it belongs to.
// CP_Graphics_DrawPoint draws a dot in the stroke color, as wide as the
// stroke weight: round with round line caps, square with the others.
void DrawPointMarker(float x, float y)
{
	if (!showPoints)
	{
		return;
	}
	CP_Settings_Save();
	CP_Settings_LineCapMode(CP_LINE_CAP_ROUND);
	CP_Settings_Stroke(CP_Color_Create(20, 20, 30, 255));
	CP_Settings_StrokeWeight(11.0f);
	CP_Graphics_DrawPoint(x, y);
	CP_Settings_Stroke(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_StrokeWeight(7.0f);
	CP_Graphics_DrawPoint(x, y);
	CP_Settings_Restore();
}

void DrawControlMarker(float x, float y, float fromX, float fromY)
{
	if (!showPoints)
	{
		return;
	}
	CP_Settings_Save();
	CP_Settings_Stroke(CP_Color_Create(255, 210, 60, 160));
	CP_Settings_StrokeWeight(1.5f);
	CP_Graphics_DrawLine(fromX, fromY, x, y);
	CP_Settings_NoFill();
	CP_Settings_Stroke(CP_Color_Create(255, 210, 60, 255));
	CP_Settings_StrokeWeight(2.5f);
	CP_Graphics_DrawCircle(x, y, 9.0f);
	CP_Settings_Restore();
}

//---------------------------------------------------------
// Straight sides: a star and polygons, with a point every so many degrees

// A star with `points` points. Its outline goes out to the outer radius and
// back in to the inner one, all the way round.
void DrawStar(float x, float y, float outerRadius, float innerRadius, int points, float degrees)
{
	CP_Graphics_BeginShape();
	for (int i = 0; i < points * 2; ++i)
	{
		const float radius = i % 2 == 0 ? outerRadius : innerRadius;
		const CP_Vector corner = CP_Vector_Scale(CP_Vector_FromAngle(degrees - 90.0f + i * 180.0f / points), radius);
		CP_Graphics_AddVertex(x + corner.x, y + corner.y);
	}
	CP_Graphics_EndShape();
}

// A polygon with every side the same length: 3 sides is a triangle, 6 a hexagon
void DrawPolygon(float x, float y, float radius, int sides, float degrees)
{
	CP_Graphics_BeginShape();
	for (int i = 0; i < sides; ++i)
	{
		const CP_Vector corner = CP_Vector_Scale(CP_Vector_FromAngle(degrees - 90.0f + i * 360.0f / sides), radius);
		CP_Graphics_AddVertex(x + corner.x, y + corner.y);
	}
	CP_Graphics_EndShape();
}

//---------------------------------------------------------
// Quadratic curves: each bends toward one control point. The corners of the
// bubble are curves whose control point is the corner a square would have.

void DrawSpeechBubble(float left, float top, float width, float height, const char* text)
{
	const float r = 20.0f; // how far each rounded corner reaches
	const float right = left + width;
	const float bottom = top + height;

	CP_Settings_Fill(CP_Color_Create(250, 250, 255, 255));
	CP_Settings_Stroke(CP_Color_Create(20, 20, 30, 255));
	CP_Settings_StrokeWeight(3.0f);
	CP_Graphics_BeginShape();
	CP_Graphics_AddVertex(left + r, top);
	CP_Graphics_AddVertex(right - r, top);
	CP_Graphics_AddQuadraticVertex(right, top, right, top + r);
	CP_Graphics_AddVertex(right, bottom - r);
	CP_Graphics_AddQuadraticVertex(right, bottom, right - r, bottom);
	// The tail: out to its tip and back, each side bending
	CP_Graphics_AddVertex(left + 80.0f, bottom);
	CP_Graphics_AddQuadraticVertex(left + 70.0f, bottom + 25.0f, left + 30.0f, bottom + 45.0f);
	CP_Graphics_AddQuadraticVertex(left + 50.0f, bottom + 22.0f, left + 45.0f, bottom);
	CP_Graphics_AddVertex(left + r, bottom);
	CP_Graphics_AddQuadraticVertex(left, bottom, left, bottom - r);
	CP_Graphics_AddVertex(left, top + r);
	CP_Graphics_AddQuadraticVertex(left, top, left + r, top);
	CP_Graphics_EndShape();

	CP_Settings_Fill(CP_Color_Create(20, 20, 30, 255));
	CP_Settings_TextSize(26.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Font_DrawText(text, left + width / 2.0f, top + height / 2.0f);

	DrawControlMarker(left + 70.0f, bottom + 25.0f, left + 80.0f, bottom);
	DrawControlMarker(left + 50.0f, bottom + 22.0f, left + 30.0f, bottom + 45.0f);
	DrawPointMarker(left + 80.0f, bottom);
	DrawPointMarker(left + 30.0f, bottom + 45.0f);
	DrawPointMarker(left + 45.0f, bottom);
}

// A flag whose top and bottom edges are two curves each. Moving their
// control points up and down makes it wave.
void DrawFlag(float poleX, float top, float seconds)
{
	const float width = 170.0f;
	const float height = 100.0f;
	const float wave = 16.0f * sinf(seconds * 4.0f);

	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(150, 150, 160, 255));
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Graphics_DrawRect(poleX - 8.0f, top - 10.0f, 8.0f, 190.0f);

	CP_Settings_FillLinearGradient(poleX, 0.0f, CP_Color_Create(230, 60, 70, 255), poleX + width, 0.0f, CP_Color_Create(250, 150, 60, 255));
	CP_Graphics_BeginShape();
	CP_Graphics_AddVertex(poleX, top);
	CP_Graphics_AddQuadraticVertex(poleX + width * 0.25f, top - wave, poleX + width * 0.5f, top);
	CP_Graphics_AddQuadraticVertex(poleX + width * 0.75f, top + wave, poleX + width, top);
	CP_Graphics_AddVertex(poleX + width, top + height);
	CP_Graphics_AddQuadraticVertex(poleX + width * 0.75f, top + height + wave, poleX + width * 0.5f, top + height);
	CP_Graphics_AddQuadraticVertex(poleX + width * 0.25f, top + height - wave, poleX, top + height);
	CP_Graphics_EndShape();

	DrawControlMarker(poleX + width * 0.25f, top - wave, poleX, top);
	DrawControlMarker(poleX + width * 0.75f, top + wave, poleX + width * 0.5f, top);
	DrawControlMarker(poleX + width * 0.75f, top + height + wave, poleX + width, top + height);
	DrawControlMarker(poleX + width * 0.25f, top + height - wave, poleX + width * 0.5f, top + height);
	DrawPointMarker(poleX, top);
	DrawPointMarker(poleX + width * 0.5f, top);
	DrawPointMarker(poleX + width, top);
	DrawPointMarker(poleX + width, top + height);
	DrawPointMarker(poleX + width * 0.5f, top + height);
	DrawPointMarker(poleX, top + height);
}

//---------------------------------------------------------
// Bezier curves: two control points each. The heart's numbers come from a
// heart 100 across; each row is one curve: control 1, control 2, end point.

const float heartStart[2] = { 50.0f, 30.0f };
const float heartCurves[6][6] = {
	{ 50.0f,  27.0f, 45.0f,  15.0f, 25.0f,  15.0f },
	{ 0.0f,   15.0f, 0.0f,   42.5f, 0.0f,   42.5f },
	{ 0.0f,   60.0f, 20.0f,  77.0f, 50.0f,  95.0f },
	{ 80.0f,  77.0f, 100.0f, 60.0f, 100.0f, 42.5f },
	{ 100.0f, 42.5f, 100.0f, 15.0f, 75.0f,  15.0f },
	{ 60.0f,  15.0f, 50.0f,  27.0f, 50.0f,  30.0f },
};

// The heart, `size` across, centered on (x, y)
void DrawHeart(float x, float y, float size)
{
	const float scale = size / 100.0f;
	const float left = x - 50.0f * scale;
	const float top = y - 55.0f * scale;

	CP_Settings_NoStroke();
	CP_Settings_FillRadialGradient(x - 0.2f * size, y - 0.2f * size, 0.0f, CP_Color_Create(255, 140, 160, 255), size * 1.2f, CP_Color_Create(200, 20, 60, 255));
	CP_Graphics_BeginShape();
	CP_Graphics_AddVertex(left + heartStart[0] * scale, top + heartStart[1] * scale);
	for (int i = 0; i < 6; ++i)
	{
		const float* c = heartCurves[i];
		CP_Graphics_AddBezierVertex(left + c[0] * scale, top + c[1] * scale, left + c[2] * scale, top + c[3] * scale, left + c[4] * scale, top + c[5] * scale);
	}
	CP_Graphics_EndShape();

	// Each control point with a line to the end of the curve it's nearer
	float fromX = left + heartStart[0] * scale;
	float fromY = top + heartStart[1] * scale;
	for (int i = 0; i < 6; ++i)
	{
		const float* c = heartCurves[i];
		const float endX = left + c[4] * scale;
		const float endY = top + c[5] * scale;
		DrawControlMarker(left + c[0] * scale, top + c[1] * scale, fromX, fromY);
		DrawControlMarker(left + c[2] * scale, top + c[3] * scale, endX, endY);
		DrawPointMarker(endX, endY);
		fromX = endX;
		fromY = endY;
	}
}

//---------------------------------------------------------
// A smooth curve through points, and a blob made with it

// Adds a smooth closed curve through every point, in order, as Bezier
// curves (a Catmull-Rom spline). For the curve from p1 to p2, the control
// points are p1 + (p2 - p0) / 6 and p2 - (p3 - p1) / 6, where p0 is the
// point before p1 and p3 the point after p2: so the curve passes through
// each point heading the way its two neighbors lie. Call it between
// BeginShape and EndShape.
void AddSmoothVertices(const CP_Vector* points, int count)
{
	CP_Graphics_AddVertex(points[0].x, points[0].y);
	for (int i = 0; i < count; ++i)
	{
		const CP_Vector p0 = points[(i + count - 1) % count];
		const CP_Vector p1 = points[i];
		const CP_Vector p2 = points[(i + 1) % count];
		const CP_Vector p3 = points[(i + 2) % count];
		CP_Graphics_AddBezierVertex(p1.x + (p2.x - p0.x) / 6.0f, p1.y + (p2.y - p0.y) / 6.0f,
			p2.x - (p3.x - p1.x) / 6.0f, p2.y - (p3.y - p1.y) / 6.0f,
			p2.x, p2.y);
	}
}

// A blob: points around a circle, each in and out with noise, joined smoothly
void DrawBlob(float x, float y, float radius, float seconds)
{
	CP_Vector points[BLOB_POINTS];
	for (int i = 0; i < BLOB_POINTS; ++i)
	{
		const float wobble = 0.75f + 0.5f * CP_Random_Noise(i * 0.9f, seconds * 0.6f, 0.0f);
		points[i] = CP_Vector_Add(CP_Vector_Set(x, y), CP_Vector_Scale(CP_Vector_FromAngle(i * 360.0f / BLOB_POINTS), radius * wobble));
	}

	CP_Settings_Stroke(CP_Color_Create(20, 60, 30, 255));
	CP_Settings_StrokeWeight(4.0f);
	CP_Settings_Fill(CP_Color_Create(90, 200, 110, 255));
	CP_Graphics_BeginShape();
	AddSmoothVertices(points, BLOB_POINTS);
	CP_Graphics_EndShape();

	// Its eyes
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Graphics_DrawCircle(x - 18.0f, y - 10.0f, 22.0f);
	CP_Graphics_DrawCircle(x + 18.0f, y - 10.0f, 22.0f);
	CP_Settings_Fill(CP_Color_Create(20, 20, 30, 255));
	CP_Graphics_DrawCircle(x - 15.0f, y - 8.0f, 10.0f);
	CP_Graphics_DrawCircle(x + 21.0f, y - 8.0f, 10.0f);

	for (int i = 0; i < BLOB_POINTS; ++i)
	{
		DrawPointMarker(points[i].x, points[i].y);
	}
}

//---------------------------------------------------------
// Holes. Whatever was drawn before shows through them.

// A circle as four Bezier curves, a quarter each. 0.5523 times the radius is
// how far the control points go for the curve to stay round.
void AddCircleVertices(float x, float y, float diameter)
{
	const float r = diameter / 2.0f;
	const float k = 0.5523f * r;
	CP_Graphics_AddVertex(x + r, y);
	CP_Graphics_AddBezierVertex(x + r, y + k, x + k, y + r, x, y + r);
	CP_Graphics_AddBezierVertex(x - k, y + r, x - r, y + k, x - r, y);
	CP_Graphics_AddBezierVertex(x - r, y - k, x - k, y - r, x, y - r);
	CP_Graphics_AddBezierVertex(x + k, y - r, x + r, y - k, x + r, y);
}

// A rectangle's corners, for an outline or a hole
void AddRectVertices(float left, float top, float width, float height)
{
	CP_Graphics_AddVertex(left, top);
	CP_Graphics_AddVertex(left + width, top);
	CP_Graphics_AddVertex(left + width, top + height);
	CP_Graphics_AddVertex(left, top + height);
}

// The outline of an arched window: up the left side, the arch (its middle
// `arch` above the top corners), down the right side
void AddArchVertices(float left, float top, float width, float height, float arch)
{
	CP_Graphics_AddVertex(left, top + height);
	CP_Graphics_AddVertex(left, top);
	CP_Graphics_AddQuadraticVertex(left + width / 2.0f, top - arch * 2.0f, left + width, top);
	CP_Graphics_AddVertex(left + width, top + height);
}

// An arched window: the frame is one shape, with a hole for each pane, so
// the sky drawn behind it shows through. The sky is the same outline with
// no holes.
void DrawWindow(float left, float top, float width, float height)
{
	const float arch = 40.0f; // how far the top curves up
	const float right = left + width;

	CP_Settings_NoStroke();
	CP_Settings_FillLinearGradient(0.0f, top - arch, CP_Color_Create(80, 140, 230, 255), 0.0f, top + height, CP_Color_Create(200, 230, 255, 255));
	CP_Graphics_BeginShape();
	AddArchVertices(left, top, width, height, arch);
	CP_Graphics_EndShape();
	CP_Settings_Fill(CP_Color_Create(255, 230, 120, 255));
	CP_Graphics_DrawCircle(left + width * 0.65f, top + 10.0f, 34.0f);

	CP_Settings_Fill(CP_Color_Create(130, 80, 45, 255));
	CP_Settings_Stroke(CP_Color_Create(60, 35, 20, 255));
	CP_Settings_StrokeWeight(2.0f);
	CP_Graphics_BeginShape();
	AddArchVertices(left, top, width, height, arch);
	// The arched pane at the top, then four square ones
	const float bar = 10.0f;
	CP_Graphics_BeginHole();
	CP_Graphics_AddVertex(left + bar, top);
	CP_Graphics_AddQuadraticVertex(left + width / 2.0f, top - arch * 2.0f + bar * 2.0f, right - bar, top);
	CP_Graphics_EndHole();
	const float paneWidth = (width - bar * 3.0f) / 2.0f;
	const float paneHeight = (height - bar * 3.0f) / 2.0f;
	for (int row = 0; row < 2; ++row)
	{
		for (int column = 0; column < 2; ++column)
		{
			CP_Graphics_BeginHole();
			AddRectVertices(left + bar + column * (paneWidth + bar), top + bar + row * (paneHeight + bar), paneWidth, paneHeight);
			CP_Graphics_EndHole();
		}
	}
	CP_Graphics_EndShape();
}

// A donut: a circle with a circle as its hole, and icing the same way
void DrawDonut(float x, float y, float diameter)
{
	CP_Settings_Stroke(CP_Color_Create(110, 60, 30, 255));
	CP_Settings_StrokeWeight(3.0f);
	CP_Settings_Fill(CP_Color_Create(220, 160, 90, 255));
	CP_Graphics_BeginShape();
	AddCircleVertices(x, y, diameter);
	CP_Graphics_BeginHole();
	AddCircleVertices(x, y, diameter * 0.36f);
	CP_Graphics_EndHole();
	CP_Graphics_EndShape();

	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(250, 120, 180, 255));
	CP_Graphics_BeginShape();
	AddCircleVertices(x, y, diameter * 0.84f);
	CP_Graphics_BeginHole();
	AddCircleVertices(x, y, diameter * 0.46f);
	CP_Graphics_EndHole();
	CP_Graphics_EndShape();
}

//---------------------------------------------------------

void DrawLabel(const char* text, float x, float y)
{
	CP_Settings_Fill(CP_Color_Create(220, 220, 235, 255));
	CP_Settings_TextSize(20.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_TOP);
	CP_Font_DrawText(text, x, y);
}

void game_init(void)
{
	CP_System_SetWindowSize((int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
	CP_System_SetWindowTitle("Shapes");
}

void game_update(void)
{
	if (CP_Input_KeyTriggered(CP_KEY_SPACE))
	{
		showPoints = !showPoints;
	}
	if (CP_Input_KeyTriggered(CP_KEY_J))
	{
		jointIndex = (jointIndex + 1) % JOINT_COUNT;
	}
	const float seconds = CP_System_GetSeconds();

	CP_Graphics_ClearBackground(CP_Color_Create(35, 35, 55, 255));

	// Top row
	CP_Settings_Stroke(CP_Color_Create(120, 80, 0, 255));
	CP_Settings_StrokeWeight(3.0f);
	CP_Settings_Fill(CP_Color_Create(255, 210, 60, 255));
	// The star's thick outline shows its corners. The joint mode is a
	// setting like the others: it stays until it's changed, so it's put
	// back to the default after the star.
	CP_Settings_StrokeWeight(7.0f);
	CP_Settings_LineJointMode(joints[jointIndex]);
	DrawStar(110.0f, 140.0f, 64.0f, 28.0f, 5, seconds * 30.0f);
	CP_Settings_LineJointMode(CP_LINE_JOINT_BEVEL);
	CP_Settings_StrokeWeight(3.0f);
	CP_Settings_Fill(CP_Color_Create(100, 180, 255, 255));
	DrawPolygon(205.0f, 100.0f, 32.0f, 6, 0.0f);
	CP_Settings_Fill(CP_Color_Create(170, 120, 255, 255));
	DrawPolygon(205.0f, 185.0f, 32.0f, 3, -seconds * 45.0f);
	DrawLabel("A loop of AddVertex", 150.0f, 250.0f);
	CP_Settings_Fill(CP_Color_Create(150, 150, 170, 255));
	CP_Settings_TextSize(16.0f);
	CP_Font_DrawText(jointNames[jointIndex], 150.0f, 274.0f);

	DrawSpeechBubble(300.0f, 70.0f, 200.0f, 100.0f, "Hello!");
	DrawLabel("AddQuadraticVertex", 400.0f, 250.0f);

	DrawHeart(660.0f, 145.0f, 150.0f * (1.0f + 0.06f * sinf(seconds * 6.0f)));
	DrawLabel("AddBezierVertex", 660.0f, 250.0f);

	// Bottom row
	DrawFlag(70.0f, 340.0f, seconds);
	DrawLabel("Curves that move", 150.0f, 540.0f);

	DrawBlob(400.0f, 420.0f, 85.0f, seconds);
	DrawLabel("Smooth through points", 400.0f, 540.0f);

	DrawWindow(572.0f, 370.0f, 100.0f, 130.0f);
	DrawDonut(740.0f, 450.0f, 90.0f);
	DrawLabel("BeginHole, EndHole", 660.0f, 540.0f);

	CP_Settings_Fill(CP_Color_Create(150, 150, 170, 255));
	CP_Settings_TextSize(18.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_BOTTOM);
	CP_Font_DrawText(showPoints ? "Space: hide the points" : "Space: show the points and control points", WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT - 12.0f);
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
