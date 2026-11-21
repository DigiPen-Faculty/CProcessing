//---------------------------------------------------------
// file:	text_effects.c
//
// Example: glowing text, shadows, and the space between letters and lines.
//
// Shows how to:
// - make a neon sign whose glow pulses, with the text drawn blurred and then
//   sharp on top
// - give a title a soft drop shadow, and small text a dark halo, so they
//   stay readable over a busy background
// - bring a title in from wide letter spacing, and underline text with
//   CP_Font_GetTextWidth, which counts the spacing
// - fit a dialog box around its text with CP_Font_GetTextBoxSize as the
//   line spacing changes
//
// Three settings do it. Like the other settings, each stays until it's
// changed, and Save and Restore keep them:
// - CP_Settings_TextBlur(blur) blurs text by that many pixels, in whole
//   steps up to 20. 0, the default, is sharp. A glow or a shadow is the
//   text drawn twice: blurred in the glow or shadow color, then sharp.
// - CP_Settings_TextLetterSpacing(spacing) adds pixels between letters.
// - CP_Settings_TextLineSpacing(spacing) adds pixels between the lines of a
//   text box. Both are 0 by default, and below 0 brings things closer.
//
// A glow pulses smoothly when its color's alpha changes. Changing the blur
// instead goes in steps, since it's in whole pixels.
//
// Left and Right change the letter spacing, and Up and Down the line
// spacing.
//
// Try this:
// - Make the sign another color, or another word, or flicker more often.
// - Glow a button when the mouse is over it.
// - Bring the title in from blurred to sharp as well as from wide to narrow.
//
// To run it, copy this file over the template's main.c.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include <math.h>
#include <stdio.h>

#define WINDOW_WIDTH  800.0f
#define WINDOW_HEIGHT 600.0f

// The four panels: left and right columns, top and bottom rows
#define PANEL_LEFT_X   30.0f
#define PANEL_RIGHT_X  415.0f
#define PANEL_TOP_Y    30.0f
#define PANEL_BOTTOM_Y 250.0f
#define PANEL_WIDTH    355.0f
#define PANEL_HEIGHT   200.0f
#define PANEL_TALL     290.0f

// How long the title takes to come in, and how long it stays
#define TITLE_IN_TIME    1.2f
#define TITLE_CYCLE_TIME 4.0f

static float letterSpacing = 6.0f;
static float lineSpacing = 4.0f;
static float titleTime = 0.0f;

// Draws text with a glow around it: blurred in the glow color, once wide
// and soft and once tight and bright, then sharp on top in the core color
static void DrawGlowingText(const char* text, float x, float y, float blur, CP_Color glow, CP_Color core)
{
	CP_Settings_Save();
	CP_Settings_Fill(glow);
	CP_Settings_TextBlur(blur);
	CP_Font_DrawText(text, x, y);
	CP_Font_DrawText(text, x, y);
	CP_Settings_TextBlur(blur / 3.0f);
	CP_Font_DrawText(text, x, y);
	CP_Settings_Restore(); // the blur goes back to what it was: sharp
	CP_Settings_Fill(core);
	CP_Font_DrawText(text, x, y);
}

// Draws text with a soft shadow: blurred and dark, moved down and right by
// the offset, then the text itself on top. With an offset of 0 the shadow
// is a dark halo all around.
static void DrawShadowedText(const char* text, float x, float y, float offset, float blur, CP_Color color)
{
	CP_Settings_Save();
	CP_Settings_TextBlur(blur);
	CP_Settings_Fill(CP_Color_Create(0, 0, 0, 230));
	CP_Font_DrawText(text, x + offset, y + offset);
	CP_Font_DrawText(text, x + offset, y + offset);
	CP_Settings_Restore();
	CP_Settings_Fill(color);
	CP_Font_DrawText(text, x, y);
}

static void DrawPanel(float x, float y, float w, float h)
{
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(30, 30, 44, 255));
	CP_Graphics_DrawRoundedRect(x, y, w, h, 12.0f);
}

static void DrawNeonSign(void)
{
	DrawPanel(PANEL_LEFT_X, PANEL_TOP_Y, PANEL_WIDTH, PANEL_HEIGHT);
	float x = PANEL_LEFT_X + PANEL_WIDTH / 2.0f;
	float y = PANEL_TOP_Y + PANEL_HEIGHT / 2.0f;
	CP_Settings_TextSize(90.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);

	// Every 5 seconds the sign flickers off twice, briefly
	float seconds = CP_System_GetSeconds();
	float cycle = fmodf(seconds, 5.0f);
	if ((cycle > 4.0f && cycle < 4.08f) || (cycle > 4.16f && cycle < 4.22f))
	{
		CP_Settings_Fill(CP_Color_Create(70, 30, 55, 255)); // the unlit tube
		CP_Font_DrawText("ARCADE", x, y);
		return;
	}

	// The glow pulses through its alpha
	int glowAlpha = (int)(200.0f + 55.0f * sinf(seconds * 3.0f));
	DrawGlowingText("ARCADE", x, y, 16.0f, CP_Color_Create(255, 40, 170, glowAlpha), CP_Color_Create(255, 225, 245, 255));
}

static void DrawBusyBackground(float x, float y, float w, float h)
{
	CP_Settings_Clip(x, y, w, h);
	CP_Color stripes[3] = { CP_Color_Create(255, 230, 120, 255), CP_Color_Create(120, 210, 255, 255), CP_Color_Create(250, 250, 255, 255) };
	CP_Settings_NoStroke();
	for (int i = 0; i < 12; ++i)
	{
		CP_Settings_Fill(stripes[i % 3]);
		CP_Graphics_DrawRect(x + (float)i * 32.0f - 8.0f, y, 32.0f, h);
	}
	CP_Settings_NoClip();
}

static void DrawShadows(void)
{
	float x = PANEL_RIGHT_X;
	float y = PANEL_TOP_Y;
	DrawBusyBackground(x, y, PANEL_WIDTH, PANEL_HEIGHT);

	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Settings_TextSize(58.0f);
	DrawShadowedText("TREASURE", x + PANEL_WIDTH / 2.0f, y + 70.0f, 5.0f, 6.0f, CP_Color_Create(255, 190, 40, 255));

	// The same small text without and with a halo
	CP_Settings_TextSize(22.0f);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Font_DrawText("Hard to read", x + PANEL_WIDTH * 0.27f, y + 150.0f);
	DrawShadowedText("Easy to read", x + PANEL_WIDTH * 0.73f, y + 150.0f, 0.0f, 3.0f, CP_Color_Create(255, 255, 255, 255));
}

static void DrawLetterSpacing(void)
{
	float x = PANEL_LEFT_X;
	float y = PANEL_BOTTOM_Y;
	DrawPanel(x, y, PANEL_WIDTH, PANEL_TALL);

	// The title comes in from wide letter spacing, slowing as it settles,
	// and fades in as it comes. The clip keeps its wide start in the panel.
	float t = CP_Math_ClampFloat(titleTime / TITLE_IN_TIME, 0.0f, 1.0f);
	float eased = 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);
	CP_Settings_Save();
	CP_Settings_Clip(x, y, PANEL_WIDTH, PANEL_TALL);
	CP_Settings_TextLetterSpacing(CP_Math_LerpFloat(40.0f, 4.0f, eased));
	CP_Settings_TextSize(54.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	DrawGlowingText("LEVEL 3", x + PANEL_WIDTH / 2.0f, y + 70.0f, 8.0f, CP_Color_Create(80, 200, 255, (int)(160.0f * eased)), CP_Color_Create(235, 250, 255, (int)(255.0f * eased)));
	CP_Settings_Restore();

	// Text with the spacing set with the arrow keys, underlined to its
	// measured width: the measure counts the spacing
	char label[64];
	snprintf(label, sizeof(label), "Letter spacing: %.0f", letterSpacing);
	CP_Settings_TextSize(18.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);
	CP_Settings_Fill(CP_Color_Create(150, 150, 175, 255));
	CP_Font_DrawText(label, x + 25.0f, y + 160.0f);

	CP_Settings_Save();
	CP_Settings_TextLetterSpacing(letterSpacing);
	CP_Settings_TextSize(34.0f);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Font_DrawText("HIGH SCORE", x + 25.0f, y + 215.0f);
	float width = CP_Font_GetTextWidth("HIGH SCORE");
	CP_Settings_Restore();
	CP_Settings_Stroke(CP_Color_Create(255, 190, 40, 255));
	CP_Settings_StrokeWeight(3.0f);
	CP_Graphics_DrawLine(x + 25.0f, y + 228.0f, x + 25.0f + width, y + 228.0f);
}

static void DrawDialog(void)
{
	float x = PANEL_RIGHT_X;
	float y = PANEL_BOTTOM_Y;
	const char* speech = "The cave splits in two ahead. The left path is quicker, but I hear something big breathing down there.";
	float rowWidth = PANEL_WIDTH - 40.0f;

	// The box fits around the text, however far apart the lines are
	CP_Settings_TextSize(20.0f);
	CP_Settings_TextLineSpacing(lineSpacing);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
	CP_Vector textSize = CP_Font_GetTextBoxSize(speech, rowWidth);

	DrawPanel(x, y, PANEL_WIDTH, textSize.y + 70.0f);
	CP_Settings_Fill(CP_Color_Create(255, 190, 40, 255));
	CP_Font_DrawText("Old Sage", x + 20.0f, y + 16.0f);
	CP_Settings_Fill(CP_Color_Create(235, 235, 245, 255));
	CP_Font_DrawTextBox(speech, x + 20.0f, y + 48.0f, rowWidth);

	char label[64];
	snprintf(label, sizeof(label), "Line spacing: %.0f", lineSpacing);
	CP_Settings_TextSize(18.0f);
	CP_Settings_Fill(CP_Color_Create(150, 150, 175, 255));
	CP_Font_DrawText(label, x + 20.0f, y + textSize.y + 82.0f);
}

void game_init(void)
{
	CP_System_SetWindowSize((int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
	CP_Settings_RectMode(CP_POSITION_CORNER);
}

void game_update(void)
{
	if (CP_Input_KeyTriggered(CP_KEY_RIGHT)) letterSpacing += 2.0f;
	if (CP_Input_KeyTriggered(CP_KEY_LEFT)) letterSpacing -= 2.0f;
	if (CP_Input_KeyTriggered(CP_KEY_UP)) lineSpacing += 2.0f;
	if (CP_Input_KeyTriggered(CP_KEY_DOWN)) lineSpacing -= 2.0f;
	letterSpacing = CP_Math_ClampFloat(letterSpacing, -4.0f, 20.0f);
	lineSpacing = CP_Math_ClampFloat(lineSpacing, -8.0f, 24.0f);
	titleTime += CP_System_GetDt();
	if (titleTime > TITLE_CYCLE_TIME)
	{
		titleTime = 0.0f;
	}

	CP_Graphics_ClearBackground(CP_Color_Create(14, 14, 22, 255));
	DrawNeonSign();
	DrawShadows();
	DrawLetterSpacing();
	DrawDialog();

	CP_Settings_Fill(CP_Color_Create(150, 150, 170, 255));
	CP_Settings_TextSize(18.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_BOTTOM);
	CP_Font_DrawText("Left / Right: letter spacing     Up / Down: line spacing", WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT - 14.0f);
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
