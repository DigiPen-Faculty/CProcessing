//---------------------------------------------------------
// file:	text_input.c
//
// Example: typing text, for a name on a high score list.
//
// Shows how to:
// - read the characters the player types with CP_Input_GetTypedText
// - delete the last character when Backspace is pressed
// - finish with Enter
// - find where the text ends with CP_Font_GetTextWidth, to put a blinking
//   cursor after it
//
// CP_Input_GetTypedText gives the text typed during the last frame: usually
// nothing, sometimes a letter, and several if the player types fast. The
// keys that aren't letters, like Backspace and Enter, are read with
// CP_Input_KeyTriggered as always.
//
// Try this:
// - Only accept letters and digits.
// - Show the name in the middle of the window, using the text's width.
// - Keep a list of the last five names entered.
//
// To run it, copy this file over the template's main.c.
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>
#include <string.h>

#define MAX_NAME_BYTES 24		// room for 23 bytes of text and the ending 0

char name[MAX_NAME_BYTES] = "";
CP_BOOL entered = FALSE;		// TRUE once the player has pressed Enter

// Remove the last character. The name is UTF-8, in which characters such as
// "é" take more than one byte: bytes that start with the bits 10 are the
// later bytes of a character, so go back past them to its first byte.
void DeleteLastCharacter(void)
{
	int length = (int)strlen(name);
	if (length == 0)
	{
		return;
	}
	length--;
	while (length > 0 && (name[length] & 0xC0) == 0x80)
	{
		length--;
	}
	name[length] = '\0';
}

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Text input");
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_MIDDLE);
}

void game_update(void)
{
	if (!entered)
	{
		// Add what was typed this frame, if it fits. (A character that doesn't
		// fit is left out, and so are the rest of this frame's letters.)
		const char* typed = CP_Input_GetTypedText();
		size_t length = strlen(name);
		if (length + strlen(typed) < MAX_NAME_BYTES)
		{
			snprintf(name + length, MAX_NAME_BYTES - length, "%s", typed);
		}

		if (CP_Input_KeyTriggered(KEY_BACKSPACE))
		{
			DeleteLastCharacter();
		}

		// Enter finishes, as long as there is a name
		if (CP_Input_KeyTriggered(KEY_ENTER) && strlen(name) > 0)
		{
			entered = TRUE;
		}
	}
	else if (CP_Input_KeyTriggered(KEY_ENTER))
	{
		// Enter again starts over
		name[0] = '\0';
		entered = FALSE;
	}

	CP_Graphics_ClearBackground(CP_Color_Create(30, 30, 50, 255));
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_TextSize(40.0f);

	if (entered)
	{
		char text[100];
		snprintf(text, sizeof(text), "Welcome, %s!", name);
		CP_Font_DrawText(text, 100.0f, 260.0f);
		CP_Settings_TextSize(24.0f);
		CP_Settings_Fill(CP_Color_Create(200, 200, 200, 255));
		CP_Font_DrawText("Press Enter to type another name", 100.0f, 340.0f);
		return;
	}

	CP_Font_DrawText("Your name for the high score list:", 100.0f, 180.0f);

	// A box for the name, and the name in it
	const float boxLeft = 100.0f;
	const float boxTop = 240.0f;
	const float boxHeight = 70.0f;
	const float textLeft = boxLeft + 20.0f;
	CP_Settings_Fill(CP_Color_Create(50, 50, 80, 255));
	CP_Graphics_DrawRect(boxLeft, boxTop, 600.0f, boxHeight);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Font_DrawText(name, textLeft, boxTop + boxHeight / 2.0f);

	// A cursor after the last character that blinks twice a second: on
	// for half of every second
	if ((int)(CP_System_GetSeconds() * 2.0f) % 2 == 0)
	{
		float cursorX = textLeft + CP_Font_GetTextWidth(name) + 2.0f;
		CP_Graphics_DrawRect(cursorX, boxTop + 12.0f, 3.0f, boxHeight - 24.0f);
	}

	CP_Settings_TextSize(24.0f);
	CP_Settings_Fill(CP_Color_Create(200, 200, 200, 255));
	CP_Font_DrawText("Type your name, Backspace to fix a mistake, Enter when you're done", 100.0f, 360.0f);
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
