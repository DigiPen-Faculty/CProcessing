//---------------------------------------------------------
// file:	fonts.c
//
// Example: fonts: loading one, choosing it, measuring text, and freeing it.
//
// Shows how to:
// - load a font from the assets folder with CP_Font_Load, and check for
//   NULL, which means it wasn't found
// - choose the font for the text drawn after it with CP_Font_Set, and get
//   the built-in one back with CP_Font_GetDefault
// - change the font and size in a function and put them back afterwards,
//   with CP_Font_GetCurrent and CP_Settings_GetTextSize
// - measure text with CP_Font_GetTextSize and CP_Font_GetTextWidth, after
//   choosing the font and size: another font is another width
// - size a button to fit its label, and shrink a long name to fit a space
// - wrap text in a box with CP_Font_DrawTextBox, with a background that
//   fits it from CP_Font_GetTextBoxSize
// - free a loaded font in game_exit with CP_Font_Free
//
// Keys: F switches the headings and the measured lines between the two
// fonts. Space changes the button's label and the name. Left and Right
// change the width of the wrapped box.
//
// Try this:
// - Put a font of your own in the assets folder and load it. Free fonts are
//   on Google Fonts; keep the license file with it.
// - Make the name plate wrap onto two lines before it shrinks.
// - Add a third font for numbers, and use it for a score.
//
// See also: score_and_text.c, for text placed by its left, center or right
// edge; text_effects.c, for glowing text and the space between letters and
// lines; text_input.c, for typing.
//
// To run it, copy this file over the template's main.c. It needs
// Exo2-LightItalic.ttf in the assets folder, which the template includes.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

#define PANEL_WIDTH  610.0f
#define PANEL_HEIGHT 310.0f
const float panelLeft[4] = { 20.0f, 650.0f, 20.0f, 650.0f };
const float panelTop[4] = { 60.0f, 60.0f, 390.0f, 390.0f };

// The fonts. The built-in font, Exo 2, is always there: CP_Font_GetDefault
// gives it. Other fonts are loaded from files.
CP_Font builtIn;
CP_Font italic;
CP_Font headingFont; // the one F chooses

// What Space goes through
const char* labels[] = { "OK", "Play", "Options", "Quit to desktop" };
const char* names[] = { "Al", "Robin", "Maximilian", "Bartholomew Fizzlewick III" };
int labelIndex = 0;

float boxWidth = 360.0f;

//---------------------------------------------------------
// Helpers

// Draws text in a font and size, then puts back the font and size that were
// set before, so the code after it isn't surprised
void DrawInFont(CP_Font font, float size, const char* text, float x, float y)
{
	CP_Font fontBefore = CP_Font_GetCurrent();
	float sizeBefore = CP_Settings_GetTextSize();
	CP_Font_Set(font);
	CP_Settings_TextSize(size);
	CP_Font_DrawText(text, x, y);
	CP_Font_Set(fontBefore);
	CP_Settings_TextSize(sizeBefore);
}

void DrawFontPanel(int panel, const char* heading)
{
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(40, 44, 60, 255));
	CP_Graphics_DrawRect(panelLeft[panel], panelTop[panel], PANEL_WIDTH, PANEL_HEIGHT);
	CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
	DrawInFont(headingFont, 26.0f, heading, panelLeft[panel] + 16.0f, panelTop[panel] + 36.0f);
}

//---------------------------------------------------------
// 1. Loading and choosing

void DrawLoading(void)
{
	float x = panelLeft[0] + 16.0f;
	float y = panelTop[0];
	DrawFontPanel(0, "Load a font, then choose it");

	CP_Settings_Fill(CP_Color_Create(200, 200, 215, 255));
	DrawInFont(builtIn, 24.0f, "Exo 2, built in: CP_Font_GetDefault()", x, y + 90.0f);
	DrawInFont(italic, 24.0f, "Exo 2 Light Italic: CP_Font_Load(\"assets/...\")", x, y + 130.0f);

	// A font that didn't load is NULL. CP_Font_Set(NULL) changes nothing,
	// so text would quietly stay in the font chosen before: check, and say
	// so on the screen, where it's easy to see
	CP_Settings_TextSize(18.0f);
	if (italic == NULL)
	{
		CP_Settings_Fill(CP_Color_Create(255, 140, 120, 255));
		CP_Font_DrawText("Exo2-LightItalic.ttf didn't load: CP_Font_Load gave NULL.", x, y + 180.0f);
		CP_Font_DrawText("Check the name, and that it's in the assets folder.", x, y + 204.0f);
	}
	else
	{
		CP_Settings_Fill(CP_Color_Create(170, 170, 185, 255));
		CP_Font_DrawText("If a file isn't found, CP_Font_Load gives NULL, so check", x, y + 180.0f);
		CP_Font_DrawText("before using the font. This one loaded.", x, y + 204.0f);
	}

	char text[96];
	snprintf(text, sizeof(text), "F: the headings use %s", headingFont == italic ? "Light Italic" : "the built-in font");
	CP_Settings_Fill(CP_Color_Create(150, 200, 255, 255));
	CP_Font_DrawText(text, x, y + 270.0f);
}

//---------------------------------------------------------
// 2. Sizes and measuring

void DrawSizes(void)
{
	float x = panelLeft[1] + 16.0f;
	float y = panelTop[1] + 60.0f;
	DrawFontPanel(1, "Sizes, and measuring");

	// Choose the font and size first, then measure: the measurement is for
	// the font and size chosen now
	const float sizes[] = { 14.0f, 20.0f, 28.0f, 40.0f, 56.0f };
	CP_Font_Set(headingFont);
	for (int i = 0; i < 5; ++i)
	{
		char text[64];
		snprintf(text, sizeof(text), "Size %.0f", sizes[i]);
		CP_Settings_TextSize(sizes[i]);
		CP_Vector size = CP_Font_GetTextSize(text); // width, and the line's height
		y += size.y + 10.0f;                        // the line's height, and a little room

		CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
		CP_Font_DrawText(text, x, y);

		// a bar as wide as the text, and the numbers beside it
		CP_Settings_Fill(CP_Color_Create(255, 180, 60, 255));
		CP_Graphics_DrawRect(x, y + 4.0f, size.x, 3.0f);
		snprintf(text, sizeof(text), "%.0f wide, %.0f high", size.x, size.y);
		CP_Settings_Fill(CP_Color_Create(200, 200, 215, 255));
		DrawInFont(builtIn, 16.0f, text, x + 360.0f, y);
	}
	CP_Font_Set(builtIn);
}

//---------------------------------------------------------
// 3. Fitting text in a space

void DrawFitting(void)
{
	float x = panelLeft[2] + 16.0f;
	float y = panelTop[2];
	DrawFontPanel(2, "Fit text in a space");
	CP_Font_Set(builtIn);

	// A button that fits its label: measure the label, add some room
	// around it, and draw the label in the middle
	const char* label = labels[labelIndex];
	CP_Settings_TextSize(28.0f);
	CP_Vector labelSize = CP_Font_GetTextSize(label);
	float buttonWidth = labelSize.x + 48.0f;
	float buttonHeight = labelSize.y + 20.0f;
	float centerX = x + 150.0f;
	float centerY = y + 110.0f;
	CP_Settings_Fill(CP_Color_Create(80, 140, 230, 255));
	CP_Graphics_DrawRoundedRect(centerX - buttonWidth / 2.0f, centerY - buttonHeight / 2.0f, buttonWidth, buttonHeight, 10.0f);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Font_DrawText(label, centerX, centerY);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);

	// A name plate with room for 250 pixels: the size goes down until the
	// name fits, from 36 to 12 at the smallest
	const char* name = names[labelIndex];
	const float room = 250.0f;
	float size = 36.0f;
	CP_Settings_TextSize(size);
	while (CP_Font_GetTextWidth(name) > room && size > 12.0f)
	{
		size -= 1.0f;
		CP_Settings_TextSize(size);
	}
	float plateX = x + 310.0f;
	CP_Settings_Fill(CP_Color_Create(70, 60, 50, 255));
	CP_Graphics_DrawRect(plateX, y + 85.0f, room + 20.0f, 50.0f);
	CP_Settings_Fill(CP_Color_Create(255, 220, 150, 255));
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Font_DrawText(name, plateX + (room + 20.0f) / 2.0f, y + 110.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);

	char text[64];
	CP_Settings_Fill(CP_Color_Create(200, 200, 215, 255));
	CP_Settings_TextSize(16.0f);
	snprintf(text, sizeof(text), "%.0f x %.0f, plus room around it", labelSize.x, labelSize.y);
	CP_Font_DrawText(text, x, y + 175.0f);
	snprintf(text, sizeof(text), "size %.0f fits in %.0f pixels", size, room);
	CP_Font_DrawText(text, plateX, y + 175.0f);
	CP_Settings_Fill(CP_Color_Create(150, 200, 255, 255));
	CP_Font_DrawText("Space: another label and name", x, y + 270.0f);
}

//---------------------------------------------------------
// 4. A wrapped box

void DrawWrapped(void)
{
	float x = panelLeft[3] + 16.0f;
	float y = panelTop[3] + 60.0f;
	DrawFontPanel(3, "Wrap text in a box");

	const char* story = "The old map showed a path through the forest, past the river and up to the "
						"tower. Nobody had walked it in a hundred years.";
	CP_Font_Set(builtIn);
	CP_Settings_TextSize(20.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);

	// Measure first, to draw the background the size the text will be
	CP_Vector size = CP_Font_GetTextBoxSize(story, boxWidth);
	CP_Settings_Fill(CP_Color_Create(240, 230, 200, 255));
	CP_Graphics_DrawRoundedRect(x - 10.0f, y - 10.0f, size.x + 20.0f, size.y + 20.0f, 8.0f);
	CP_Settings_Fill(CP_Color_Create(50, 40, 30, 255));
	CP_Font_DrawTextBox(story, x, y, boxWidth);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);

	char text[64];
	snprintf(text, sizeof(text), "Left and Right: %.0f wide, so %.0f high", boxWidth, size.y);
	CP_Settings_Fill(CP_Color_Create(150, 200, 255, 255));
	CP_Settings_TextSize(16.0f);
	CP_Font_DrawText(text, x, panelTop[3] + 290.0f);
}

//---------------------------------------------------------

void game_init(void)
{
	CP_System_SetWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	CP_System_SetWindowTitle("Fonts");
	CP_Settings_RectMode(CP_POSITION_CORNER);

	// Load fonts once, in init, like images and sounds
	builtIn = CP_Font_GetDefault();
	italic = CP_Font_Load("assets/Exo2-LightItalic.ttf");
	if (italic == NULL)
	{
		printf("Couldn't load assets/Exo2-LightItalic.ttf\n");
	}
	headingFont = builtIn;
}

void game_update(void)
{
	if (CP_Input_KeyTriggered(CP_KEY_F) && italic != NULL)
	{
		headingFont = headingFont == builtIn ? italic : builtIn;
	}
	if (CP_Input_KeyTriggered(CP_KEY_SPACE))
	{
		labelIndex = (labelIndex + 1) % 4;
	}
	if (CP_Input_KeyDown(CP_KEY_LEFT))
	{
		boxWidth = CP_Math_ClampFloat(boxWidth - 200.0f * CP_System_GetDt(), 150.0f, 560.0f);
	}
	if (CP_Input_KeyDown(CP_KEY_RIGHT))
	{
		boxWidth = CP_Math_ClampFloat(boxWidth + 200.0f * CP_System_GetDt(), 150.0f, 560.0f);
	}

	CP_Graphics_ClearBackground(CP_Color_Create(24, 26, 36, 255));
	CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
	DrawInFont(headingFont, 30.0f, "Fonts: load, choose, measure, free", 20.0f, 40.0f);

	DrawLoading();
	DrawSizes();
	DrawFitting();
	DrawWrapped();
}

void game_exit(void)
{
	// Free what CP_Font_Load loaded. The built-in font can't be freed (Free
	// leaves it alone), and freeing NULL does nothing, so this is safe even
	// if the font didn't load.
	CP_Font_Free(&italic);
}

int main(void)
{
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
