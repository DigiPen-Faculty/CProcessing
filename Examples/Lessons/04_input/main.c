//---------------------------------------------------------
// file:	main.c
//
// Lesson 4: Input.
// https://github.com/DigiPen-Faculty/CProcessing/wiki/Lesson-04-Input
//
// This is the finished program from the lesson. The window has four panels,
// one for each way a player talks to your game:
//   top left      keyboard: Triggered, Down and Released (press the space bar)
//   top right     mouse: position, buttons, clicks and the wheel (use the mouse in the panel)
//   bottom left   typing: the letters the player types (type, Backspace, Enter)
//   bottom right  gamepad: sticks, buttons and triggers (plug in a gamepad)
//
// To run it, copy this file over the template's main.c.
//
// Try this:
// - Add lamps for a second key.
// - Make a click leave a ring in a different color for the right mouse button.
// - Show the name of the gamepad button that was pressed last.
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>
#include <string.h>

// The window is divided into four panels, each this big
const float cellWidth = 400.0f;
const float cellHeight = 300.0f;

//---------------------------------------------------------
// Helpers
//---------------------------------------------------------

// Text centered on x. CP_Font_GetTextWidth says how wide the text will be.
void DrawCenteredText(const char* text, float x, float y)
{
	CP_Font_DrawText(text, x - CP_Font_GetTextWidth(text) / 2.0f, y);
}

// A panel's dark background and its title
void DrawPanel(const char* title, float cellX, float cellY, CP_Color color)
{
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_NoStroke();
	CP_Settings_Fill(color);
	CP_Graphics_DrawRect(cellX, cellY, cellWidth, cellHeight);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Font_DrawText(title, cellX + 12.0f, cellY + 22.0f);
}

// Is the point (x, y) inside a panel? The mouse only does things in its panel.
CP_BOOL IsInside(float x, float y, float cellX, float cellY)
{
	return x >= cellX && x < cellX + cellWidth && y >= cellY && y < cellY + cellHeight;
}

// A round lamp with a label under it, bright when `lit`
void DrawLamp(const char* label, float x, float y, CP_BOOL lit, CP_Color color)
{
	CP_Settings_Stroke(CP_Color_Create(200, 200, 200, 255));
	CP_Settings_StrokeWeight(2.0f);
	CP_Settings_Fill(lit ? color : CP_Color_Create(50, 50, 60, 255));
	CP_Graphics_DrawCircle(x, y, 44.0f);
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	DrawCenteredText(label, x, y + 44.0f);
}

//---------------------------------------------------------
// Keyboard: a key has three questions you can ask about it each frame.
//   KeyTriggered: did it go down THIS frame?  (true for one frame)
//   KeyDown:      is it held down right now?  (true for every frame it is held)
//   KeyReleased:  did it come back up THIS frame?  (true for one frame)
//---------------------------------------------------------
int triggeredCount = 0;
int framesDown = 0;
int releasedCount = 0;
float triggeredLampUntil = 0.0f;		// a frame is too short to see, so a lamp
float releasedLampUntil = 0.0f;			// stays lit for a moment after it happens

void DoKeyboard(float cellX, float cellY)
{
	const float now = CP_System_GetSeconds();

	if (CP_Input_KeyTriggered(KEY_SPACE))
	{
		triggeredCount++;
		triggeredLampUntil = now + 0.25f;
	}
	if (CP_Input_KeyDown(KEY_SPACE))
	{
		framesDown++;
	}
	if (CP_Input_KeyReleased(KEY_SPACE))
	{
		releasedCount++;
		releasedLampUntil = now + 0.25f;
	}

	DrawPanel("Keyboard: hold and tap the space bar", cellX, cellY, CP_Color_Create(30, 40, 60, 255));

	DrawLamp("Triggered", cellX + 80.0f, cellY + 100.0f, now < triggeredLampUntil, CP_Color_Create(90, 220, 120, 255));
	DrawLamp("Down", cellX + 200.0f, cellY + 100.0f, CP_Input_KeyDown(KEY_SPACE), CP_Color_Create(250, 200, 60, 255));
	DrawLamp("Released", cellX + 320.0f, cellY + 100.0f, now < releasedLampUntil, CP_Color_Create(240, 90, 90, 255));

	// Counting shows the difference: hold the key and "frames down" races
	// ahead, while "triggered" only goes up by one.
	char text[60];
	snprintf(text, sizeof(text), "Triggered: %d times", triggeredCount);
	CP_Font_DrawText(text, cellX + 40.0f, cellY + 190.0f);
	snprintf(text, sizeof(text), "Down: %d frames", framesDown);
	CP_Font_DrawText(text, cellX + 40.0f, cellY + 220.0f);
	snprintf(text, sizeof(text), "Released: %d times", releasedCount);
	CP_Font_DrawText(text, cellX + 40.0f, cellY + 250.0f);
}

//---------------------------------------------------------
// Mouse: where it is, which buttons are held, and when it was clicked.
//---------------------------------------------------------
#define RING_COUNT 8
CP_Vector ringPosition[RING_COUNT];
float ringAge[RING_COUNT];				// seconds since the click, or -1 for unused
float ballSize = 60.0f;

void DoMouse(float cellX, float cellY)
{
	const float mouseX = CP_Input_GetMouseX();
	const float mouseY = CP_Input_GetMouseY();
	const CP_BOOL inPanel = IsInside(mouseX, mouseY, cellX, cellY);

	if (inPanel)
	{
		// The wheel gives how far it turned this frame: up is positive
		ballSize = CP_Math_ClampFloat(ballSize + CP_Input_MouseWheel() * 6.0f, 20.0f, 140.0f);

		// A click starts a ring in the first unused place
		if (CP_Input_MouseTriggered(MOUSE_BUTTON_LEFT))
		{
			for (int i = 0; i < RING_COUNT; i++)
			{
				if (ringAge[i] < 0.0f)
				{
					ringPosition[i] = CP_Vector_Set(mouseX, mouseY);
					ringAge[i] = 0.0f;
					break;
				}
			}
		}
	}

	DrawPanel("Mouse: move, click and turn the wheel", cellX, cellY, CP_Color_Create(40, 50, 40, 255));

	// The rings grow and fade for half a second, then their place is free again
	for (int i = 0; i < RING_COUNT; i++)
	{
		if (ringAge[i] < 0.0f)
		{
			continue;
		}
		ringAge[i] += CP_System_GetDt();
		if (ringAge[i] > 0.5f)
		{
			ringAge[i] = -1.0f;
			continue;
		}
		const float t = ringAge[i] / 0.5f;
		CP_Settings_NoFill();
		CP_Settings_Stroke(CP_Color_Create(255, 255, 255, (int)((1.0f - t) * 255.0f)));
		CP_Settings_StrokeWeight(4.0f);
		CP_Graphics_DrawCircle(ringPosition[i].x, ringPosition[i].y, 20.0f + t * 120.0f);
	}

	// The ball follows the mouse. It is solid while the left button is held
	// down, and only an outline when it is not.
	if (inPanel)
	{
		CP_Settings_Stroke(CP_Color_Create(250, 200, 60, 255));
		CP_Settings_StrokeWeight(4.0f);
		if (CP_Input_MouseDown(MOUSE_BUTTON_LEFT))
		{
			CP_Settings_Fill(CP_Color_Create(250, 200, 60, 255));
		}
		else
		{
			CP_Settings_NoFill();
		}
		CP_Graphics_DrawCircle(mouseX, mouseY, ballSize);
	}

	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	char text[60];
	snprintf(text, sizeof(text), "Mouse at (%.0f, %.0f)", mouseX, mouseY);
	CP_Font_DrawText(text, cellX + 12.0f, cellY + cellHeight - 14.0f);
}

//---------------------------------------------------------
// Typing: CP_Input_GetTypedText gives the letters typed this frame. It is
// usually empty, so add what it gives to your own text. Backspace and Enter
// are not letters, so ask for them as keys.
//---------------------------------------------------------
char typedText[32] = "";

void DoTyping(float cellX, float cellY)
{
	const char* typed = CP_Input_GetTypedText();
	const size_t length = strlen(typedText);
	if (length + strlen(typed) < sizeof(typedText))
	{
		snprintf(typedText + length, sizeof(typedText) - length, "%s", typed);
	}
	if (CP_Input_KeyTriggered(KEY_BACKSPACE) && strlen(typedText) > 0)
	{
		typedText[strlen(typedText) - 1] = '\0';
	}
	if (CP_Input_KeyTriggered(KEY_ENTER))
	{
		typedText[0] = '\0';
	}

	DrawPanel("Typing: type something", cellX, cellY, CP_Color_Create(60, 40, 50, 255));

	CP_Settings_TextSize(34.0f);
	CP_Font_DrawText(typedText, cellX + 30.0f, cellY + 130.0f);
	CP_Settings_TextSize(18.0f);

	CP_Settings_Fill(CP_Color_Create(200, 200, 200, 255));
	CP_Font_DrawText("Backspace takes one off. Enter clears.", cellX + 30.0f, cellY + 230.0f);
}

//---------------------------------------------------------
// Gamepad: sticks give a number from -1 to 1 on each of two axes. Right is
// +x and UP is +y (the opposite of the screen), and a stick that is let go
// of still wobbles a little around the middle, so CProcessing gives exactly
// 0 until it is pushed past a small "dead zone", about a quarter of the way.
//---------------------------------------------------------
const float stickRadius = 60.0f;
const float deadZoneRadius = 0.27f;		// as a share of the way out: about right

void DrawStick(const char* label, CP_Vector stick, float x, float y)
{
	CP_Settings_StrokeWeight(2.0f);
	CP_Settings_Stroke(CP_Color_Create(200, 200, 200, 255));
	CP_Settings_Fill(CP_Color_Create(30, 30, 40, 255));
	CP_Graphics_DrawCircle(x, y, stickRadius * 2.0f);
	CP_Settings_Stroke(CP_Color_Create(110, 110, 130, 255));
	CP_Settings_NoFill();
	CP_Graphics_DrawCircle(x, y, stickRadius * 2.0f * deadZoneRadius);

	// Pushing a stick up gives a positive y, but y grows DOWN the screen, so
	// the dot is the stick's y above the middle: the middle MINUS the stick
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(250, 200, 60, 255));
	CP_Graphics_DrawCircle(x + stick.x * stickRadius, y - stick.y * stickRadius, 18.0f);

	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	DrawCenteredText(label, x, y + stickRadius + 22.0f);
}

void DrawButton(const char* label, CP_BOOL down, float x, float y)
{
	DrawLamp(label, x, y, down, CP_Color_Create(90, 180, 250, 255));
}

void DoGamepad(float cellX, float cellY)
{
	DrawPanel("Gamepad: plug one in", cellX, cellY, CP_Color_Create(45, 40, 65, 255));

	if (!CP_Input_GamepadConnected())
	{
		CP_Settings_Fill(CP_Color_Create(200, 200, 200, 255));
		DrawCenteredText("No gamepad connected.", cellX + cellWidth / 2.0f, cellY + 140.0f);
		DrawCenteredText("Plug one in (or turn it on).", cellX + cellWidth / 2.0f, cellY + 170.0f);
		return;
	}

	DrawStick("Left stick", CP_Input_GamepadLeftStick(), cellX + 100.0f, cellY + 110.0f);
	DrawStick("Right stick", CP_Input_GamepadRightStick(), cellX + 300.0f, cellY + 110.0f);

	// The buttons are like keys: Triggered, Down and Released
	DrawButton("A", CP_Input_GamepadDown(GAMEPAD_A), cellX + 70.0f, cellY + 250.0f);
	DrawButton("B", CP_Input_GamepadDown(GAMEPAD_B), cellX + 130.0f, cellY + 250.0f);
	DrawButton("X", CP_Input_GamepadDown(GAMEPAD_X), cellX + 190.0f, cellY + 250.0f);
	DrawButton("Y", CP_Input_GamepadDown(GAMEPAD_Y), cellX + 250.0f, cellY + 250.0f);

	// The triggers are not on or off: they give 0.0 to 1.0, how far they are pressed
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(90, 180, 250, 255));
	CP_Graphics_DrawRect(cellX + 300.0f, cellY + 235.0f, 80.0f * CP_Input_GamepadLeftTrigger(), 12.0f);
	CP_Graphics_DrawRect(cellX + 300.0f, cellY + 255.0f, 80.0f * CP_Input_GamepadRightTrigger(), 12.0f);
}

//---------------------------------------------------------

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Lesson 4: Input");
	CP_Settings_TextSize(18.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);

	for (int i = 0; i < RING_COUNT; i++)
	{
		ringAge[i] = -1.0f;
	}
}

void game_update(void)
{
	CP_Graphics_ClearBackground(CP_Color_Create(20, 20, 30, 255));

	DoKeyboard(0.0f, 0.0f);
	DoMouse(cellWidth, 0.0f);
	DoTyping(0.0f, cellHeight);
	DoGamepad(cellWidth, cellHeight);
}

void game_exit(void)
{
	// This program loaded nothing to free.
}

int main(void)
{
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
