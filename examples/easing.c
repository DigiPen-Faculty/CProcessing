//---------------------------------------------------------
// file:	easing.c
//
// Example: easing functions, drawn and compared.
//
// An easing function bends the progress of a movement. It takes how far
// through the TIME you are (t, from 0 to 1) and gives back how far along the
// PATH to be (also 0 to 1 at the ends, but not at the same pace in between).
// Use it between the time and a lerp:
//
//     float t = CP_Math_ClampFloat(timer / duration, 0.0f, 1.0f);
//     x = CP_Math_LerpFloat(startX, endX, EaseOutCubic(t));
//
// Each one is a line or two of math, so they are not part of CProcessing:
// copy the ones you want from this file into your game. Each box below draws
// one: its curve (time across, path up), a dot riding the curve, and a ball
// sliding along at the eased pace.
//
// Press Space to start the movement again, and 1 to 4 to make it last 1 to
// 4 seconds.
//
// Try this:
// - Ease a size instead of a position, so a coin pops in (EaseOutBack).
// - Ease a color with CP_Color_Lerp, or an alpha to fade something out.
// - Write your own: what does t * t * t * t look like? And 1 - t?
// - Combine two: ease in for the first half, bounce for the second.
//
// See also: transitions.c and state_effects.c, which use easing for slides
// and slow motion.
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
// The easing functions. t goes from 0 to 1; each returns 0 at t = 0 and 1 at
// t = 1. Quad, cubic and sine are gentle, cubic is a stronger quad, and back,
// elastic and bounce go past the end or come back from it.

// Starts slow, ends fast: something leaving, or speeding up
float EaseInQuad(float t)
{
	return t * t;
}

// Starts fast, ends slow: something arriving. The most natural default.
float EaseOutQuad(float t)
{
	return 1.0f - (1.0f - t) * (1.0f - t);
}

// Slow, fast, slow: moving between two resting places
float EaseInOutQuad(float t)
{
	if (t < 0.5f)
	{
		return 2.0f * t * t;
	}
	const float u = -2.0f * t + 2.0f;
	return 1.0f - u * u / 2.0f;
}

float EaseInCubic(float t)
{
	return t * t * t;
}

float EaseOutCubic(float t)
{
	const float u = 1.0f - t;
	return 1.0f - u * u * u;
}

float EaseInOutCubic(float t)
{
	if (t < 0.5f)
	{
		return 4.0f * t * t * t;
	}
	const float u = -2.0f * t + 2.0f;
	return 1.0f - u * u * u / 2.0f;
}

// A quarter of a sine wave: the gentlest of all
float EaseInSine(float t)
{
	return 1.0f - cosf(t * CP_HALF_PI);
}

float EaseOutSine(float t)
{
	return sinf(t * CP_HALF_PI);
}

// Goes a little past the end and settles back: a button popping in
float EaseOutBack(float t)
{
	const float overshoot = 1.70158f; // how far past: bigger goes further
	const float u = t - 1.0f;
	return 1.0f + (overshoot + 1.0f) * u * u * u + overshoot * u * u;
}

// Springs past the end and back a few times, like a rubber band
float EaseOutElastic(float t)
{
	if (t <= 0.0f || t >= 1.0f)
	{
		return t <= 0.0f ? 0.0f : 1.0f;
	}
	return powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * (CP_TWO_PI / 3.0f)) + 1.0f;
}

// Arrives and bounces to a stop: something dropped
float EaseOutBounce(float t)
{
	const float n = 7.5625f;
	const float d = 2.75f;
	if (t < 1.0f / d)
	{
		return n * t * t;
	}
	if (t < 2.0f / d)
	{
		t -= 1.5f / d;
		return n * t * t + 0.75f;
	}
	if (t < 2.5f / d)
	{
		t -= 2.25f / d;
		return n * t * t + 0.9375f;
	}
	t -= 2.625f / d;
	return n * t * t + 0.984375f;
}

//---------------------------------------------------------
// Choosing one by its number, so the boxes can be drawn in a loop

#define EASING_COUNT 12
const char* easingNames[EASING_COUNT] = {
	"linear (none)", "EaseInQuad", "EaseOutQuad", "EaseInOutQuad",
	"EaseInCubic", "EaseOutCubic", "EaseInOutCubic", "EaseInSine",
	"EaseOutSine", "EaseOutBack", "EaseOutElastic", "EaseOutBounce"
};

float Ease(int which, float t)
{
	switch (which)
	{
	case 1:  return EaseInQuad(t);
	case 2:  return EaseOutQuad(t);
	case 3:  return EaseInOutQuad(t);
	case 4:  return EaseInCubic(t);
	case 5:  return EaseOutCubic(t);
	case 6:  return EaseInOutCubic(t);
	case 7:  return EaseInSine(t);
	case 8:  return EaseOutSine(t);
	case 9:  return EaseOutBack(t);
	case 10: return EaseOutElastic(t);
	case 11: return EaseOutBounce(t);
	default: return t; // linear: no change
	}
}

//---------------------------------------------------------
// Drawing

#define COLUMNS    4
#define BOX_WIDTH  230.0f
#define BOX_HEIGHT 200.0f
#define MARGIN     18.0f

float duration = 2.0f; // seconds the movement takes
float timer = 0.0f;    // seconds since it started

// One box: the curve, a dot on it at the current time, and a sliding ball
void DrawEasingBox(int which, float left, float top, float t)
{
	const float graphLeft = left + 20.0f;
	const float graphWidth = BOX_WIDTH - 40.0f;
	const float graphBottom = top + BOX_HEIGHT - 45.0f;
	const float graphHeight = 80.0f; // 0 to 1, with room above for back and elastic

	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(40, 44, 60, 255));
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Graphics_DrawRect(left, top, BOX_WIDTH, BOX_HEIGHT);

	// The lines for 0 and 1
	CP_Settings_StrokeWeight(1.0f);
	CP_Settings_Stroke(CP_Color_Create(80, 86, 110, 255));
	CP_Graphics_DrawLine(graphLeft, graphBottom, graphLeft + graphWidth, graphBottom);
	CP_Graphics_DrawLine(graphLeft, graphBottom - graphHeight, graphLeft + graphWidth, graphBottom - graphHeight);

	// The curve: time across, how far along the path up
	CP_Settings_StrokeWeight(2.5f);
	CP_Settings_Stroke(CP_Color_Create(120, 200, 255, 255));
	const int steps = 60;
	float previousX = graphLeft;
	float previousY = graphBottom - Ease(which, 0.0f) * graphHeight;
	for (int i = 1; i <= steps; ++i)
	{
		const float stepT = (float)i / (float)steps;
		const float x = graphLeft + stepT * graphWidth;
		const float y = graphBottom - Ease(which, stepT) * graphHeight;
		CP_Graphics_DrawLine(previousX, previousY, x, y);
		previousX = x;
		previousY = y;
	}

	// The dot riding the curve, and the ball sliding at the eased pace
	const float eased = Ease(which, t);
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(255, 200, 80, 255));
	CP_Graphics_DrawCircle(graphLeft + t * graphWidth, graphBottom - eased * graphHeight, 10.0f);
	CP_Settings_Fill(CP_Color_Create(255, 120, 90, 255));
	CP_Graphics_DrawCircle(graphLeft + eased * graphWidth, top + BOX_HEIGHT - 18.0f, 18.0f);

	CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
	CP_Settings_TextSize(20.0f);
	CP_Font_DrawText(easingNames[which], left + 12.0f, top + 24.0f);
}

void game_init(void)
{
	CP_System_SetWindowSize((int)(COLUMNS * (BOX_WIDTH + MARGIN) + MARGIN), 720);
	CP_System_SetWindowTitle("Easing functions");
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);
}

void game_update(void)
{
	if (CP_Input_KeyTriggered(CP_KEY_SPACE))
	{
		timer = 0.0f;
	}
	for (int seconds = 1; seconds <= 4; ++seconds)
	{
		if (CP_Input_KeyTriggered((CP_KEY)(CP_KEY_0 + seconds)))
		{
			duration = (float)seconds;
			timer = 0.0f;
		}
	}

	// Run the movement, then hold at the end for a moment and start again
	timer += CP_System_GetDt();
	if (timer > duration + 1.0f)
	{
		timer = 0.0f;
	}
	const float t = CP_Math_ClampFloat(timer / duration, 0.0f, 1.0f);

	CP_Graphics_ClearBackground(CP_Color_Create(24, 26, 36, 255));
	for (int i = 0; i < EASING_COUNT; ++i)
	{
		const float left = MARGIN + (float)(i % COLUMNS) * (BOX_WIDTH + MARGIN);
		const float top = MARGIN + (float)(i / COLUMNS) * (BOX_HEIGHT + MARGIN);
		DrawEasingBox(i, left, top, t);
	}

	char text[80];
	snprintf(text, sizeof(text), "Space: start again    1-4: %.0f second%s", duration, duration == 1.0f ? "" : "s");
	CP_Settings_Fill(CP_Color_Create(170, 170, 190, 255));
	CP_Settings_TextSize(20.0f);
	CP_Font_DrawText(text, MARGIN, 705.0f);
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
