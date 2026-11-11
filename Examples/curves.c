//---------------------------------------------------------
// file:	curves.c
//
// Example: arcs, pies, Bezier curves and layered noise.
//
// Shows how to:
// - draw a pie chart with CP_Graphics_DrawPie, one slice per value
// - draw a spinning loading ring with CP_Graphics_DrawArc
// - draw a curve with CP_Graphics_DrawBezier, and move its control points
// - draw hills with CP_Random_NoiseOctaves, and add or remove detail
//
// Angles are in degrees. 0 points to the right, and a positive sweep goes
// clockwise (y points down), the same as CP_Vector_FromAngle. A negative
// sweep goes the other way.
//
// Drag the round handles of the curve with the mouse. Press 1 to 6 to choose
// how many layers (octaves) of noise make up the hills.
//
// Try this:
// - Turn the pie into a ring chart by drawing a smaller circle over its middle.
// - Make the pie slice under the mouse stick out.
// - Make the hills scroll as time passes (the z of the noise).
//
// To run it, copy this file over the template's main.c.
//---------------------------------------------------------

#include "cprocessing.h"
#include <math.h>
#include <stdio.h>

#define SLICE_COUNT 4
#define HANDLE_COUNT 4		// the curve's two ends and its two control points
#define HANDLE_RADIUS 12.0f

float sliceValues[SLICE_COUNT] = { 40.0f, 25.0f, 20.0f, 15.0f };
CP_Color sliceColors[SLICE_COUNT];

// The curve: end, control, control, end
CP_Vector handles[HANDLE_COUNT];
int grabbed = -1;				// which handle the mouse holds, or -1

int octaves = 4;

void DrawPieChart(float x, float y, float diameter)
{
	float total = 0.0f;
	for (int i = 0; i < SLICE_COUNT; ++i)
	{
		total += sliceValues[i];
	}

	CP_Settings_Stroke(CP_Color_Create(30, 30, 50, 255));
	CP_Settings_StrokeWeight(2.0f);
	float start = -90.0f;		// start at the top
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
	CP_Graphics_DrawArc(x, y, diameter, 0.0f, 360.0f);		// the track

	CP_Settings_Stroke(CP_Color_Create(255, 200, 60, 255));
	CP_Settings_LineCapMode(CP_LINE_CAP_ROUND);
	CP_Graphics_DrawArc(x, y, diameter, start, sweep);
	CP_Settings_LineCapMode(CP_LINE_CAP_BUTT);
}

void UpdateHandles(void)
{
	const float mouseX = CP_Input_GetMouseX();
	const float mouseY = CP_Input_GetMouseY();

	if (CP_Input_MouseTriggered(MOUSE_BUTTON_LEFT))
	{
		grabbed = -1;
		for (int i = 0; i < HANDLE_COUNT; ++i)
		{
			if (CP_Math_Distance(mouseX, mouseY, handles[i].x, handles[i].y) <= HANDLE_RADIUS * 1.5f)
			{
				grabbed = i;
			}
		}
	}
	if (!CP_Input_MouseDown(MOUSE_BUTTON_LEFT))
	{
		grabbed = -1;
	}
	if (grabbed >= 0)
	{
		handles[grabbed].x = mouseX;
		handles[grabbed].y = mouseY;
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

// Each hill height is layered noise: the first octave is the big shape, and
// every octave after it adds smaller bumps
void DrawHills(float top, float height)
{
	const float width = (float)CP_System_GetWindowWidth();
	CP_Settings_Stroke(CP_Color_Create(60, 200, 120, 255));
	CP_Settings_StrokeWeight(3.0f);
	float previousY = 0.0f;
	for (float x = 0.0f; x <= width; x += 4.0f)
	{
		const float n = CP_Random_NoiseOctaves(x * 0.006f, 0.5f, 0.0f, octaves, 0.5f);
		const float y = top + (1.0f - n) * height;
		if (x > 0.0f)
		{
			CP_Graphics_DrawLine(x - 4.0f, previousY, x, y);
		}
		previousY = y;
	}
}

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Arcs, curves and noise");
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_MIDDLE);

	sliceColors[0] = CP_Color_Create(235, 90, 90, 255);
	sliceColors[1] = CP_Color_Create(250, 190, 70, 255);
	sliceColors[2] = CP_Color_Create(90, 190, 130, 255);
	sliceColors[3] = CP_Color_Create(90, 140, 240, 255);

	handles[0] = CP_Vector_Set(540.0f, 250.0f);
	handles[1] = CP_Vector_Set(560.0f, 50.0f);
	handles[2] = CP_Vector_Set(740.0f, 50.0f);
	handles[3] = CP_Vector_Set(770.0f, 250.0f);

	CP_Random_NoiseSeed(7);
}

void game_update(void)
{
	for (int i = 0; i < 6; ++i)
	{
		if (CP_Input_KeyTriggered((CP_KEY)(KEY_1 + i)))
		{
			octaves = i + 1;
		}
	}
	UpdateHandles();

	CP_Graphics_ClearBackground(CP_Color_Create(30, 30, 50, 255));

	DrawPieChart(170.0f, 160.0f, 230.0f);
	DrawLoadingRing(400.0f, 160.0f, 120.0f);
	DrawCurve();

	DrawHills(320.0f, 220.0f);

	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(200, 200, 200, 255));
	CP_Settings_TextSize(22.0f);
	CP_Font_DrawText("Pie", 50.0f, 300.0f);
	CP_Font_DrawText("Arc", 370.0f, 300.0f);
	CP_Font_DrawText("Bezier: drag the handles", 520.0f, 300.0f);
	char text[40];
	snprintf(text, sizeof(text), "Noise octaves (keys 1-6): %d", octaves);
	CP_Font_DrawText(text, 20.0f, 570.0f);
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
