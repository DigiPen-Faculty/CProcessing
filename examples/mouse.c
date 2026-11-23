//---------------------------------------------------------
// file:	mouse.c
//
// Example: the mouse: buttons, clicks, double clicks, dragging, the wheel.
//
// Shows how to:
// - make a button that lights up under the mouse and clicks the way buttons
//   do everywhere: pressed and let go over it (DoButton, a recipe to copy)
// - tell pressed this frame (CP_Input_MouseTriggered), held
//   (CP_Input_MouseDown) and let go this frame (CP_Input_MouseReleased)
//   apart, for the left, right and middle buttons
// - tell a click from a double click (CP_Input_MouseDoubleClicked): click
//   to choose, double click to open
// - drag something and drop it in a slot, or send it back where it came
//   from
// - read the wheel (CP_Input_MouseWheel)
// - know the mouse moved (CP_Input_MouseMoved), how far
//   (CP_Input_GetMouseDeltaX and Y), and how fast
//
// Each panel says what to do in it.
//
// Try this:
// - Give DoButton a disabled look, for a button that can't be pressed yet.
// - Let a right click on a gem send it back to the bag.
// - Make the wheel scroll faster while Shift is held.
//
// See also: collision.c, for the mouse over a circle or a box;
// color_picker.c, for sliders the mouse drags; clipping.c, for a list
// dragged and scrolled; camera.c, for the mouse in a moved world.
//
// To run it, copy this file over the template's main.c.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

// Six panels, three across and two down
#define PANEL_WIDTH  400.0f
#define PANEL_HEIGHT 315.0f
const float panelLeft[6] = { 20.0f, 440.0f, 860.0f, 20.0f, 440.0f, 860.0f };
const float panelTop[6] = { 60.0f, 60.0f, 60.0f, 385.0f, 385.0f, 385.0f };

//---------------------------------------------------------
// Helpers

bool MouseIsOver(float left, float top, float width, float height)
{
	float x = CP_Input_GetMouseX();
	float y = CP_Input_GetMouseY();
	return x >= left && x < left + width && y >= top && y < top + height;
}

void DrawMousePanel(int panel, const char* title, const char* hint)
{
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(40, 44, 60, 255));
	CP_Graphics_DrawRect(panelLeft[panel], panelTop[panel], PANEL_WIDTH, PANEL_HEIGHT);
	CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
	CP_Settings_TextSize(22.0f);
	CP_Font_DrawText(title, panelLeft[panel] + 14.0f, panelTop[panel] + 30.0f);
	CP_Settings_Fill(CP_Color_Create(150, 200, 255, 255));
	CP_Settings_TextSize(15.0f);
	CP_Font_DrawText(hint, panelLeft[panel] + 14.0f, panelTop[panel] + 52.0f);
}

void DrawSmallText(const char* text, float x, float y)
{
	CP_Settings_Fill(CP_Color_Create(200, 200, 215, 255));
	CP_Settings_TextSize(16.0f);
	CP_Font_DrawText(text, x, y);
}

//---------------------------------------------------------
// 1. A button to copy

// The button the left button went down on, or -1. A click counts only if
// the button is let go over the same button it was pressed on, so a player
// who presses, changes their mind and slides off doesn't click.
int pressedButton = -1;

// Draws a button and returns true on the frame it's clicked. Give each
// button its own id number. Corner x, y.
bool DoButton(int id, float x, float y, float width, float height, const char* label)
{
	bool over = MouseIsOver(x, y, width, height);
	if (over && CP_Input_MouseTriggered(CP_MOUSE_LEFT))
	{
		pressedButton = id;
	}
	bool held = pressedButton == id && CP_Input_MouseDown(CP_MOUSE_LEFT);
	bool clicked = pressedButton == id && over && CP_Input_MouseReleased(CP_MOUSE_LEFT);

	// darker while held down over it, lighter under the mouse
	CP_Color color = CP_Color_Create(70, 120, 210, 255);
	if (held && over)
	{
		color = CP_Color_Create(40, 80, 160, 255);
	}
	else if (over)
	{
		color = CP_Color_Create(110, 160, 250, 255);
	}
	CP_Settings_NoStroke();
	CP_Settings_Fill(color);
	CP_Graphics_DrawRoundedRect(x, y, width, height, 8.0f);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_TextSize(20.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Font_DrawText(label, x + width / 2.0f, y + height / 2.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);
	return clicked;
}

int count = 0;
bool soundOn = true;

void DoButtons(void)
{
	float x = panelLeft[0] + 14.0f;
	float y = panelTop[0];
	DrawMousePanel(0, "A button to copy: DoButton", "Press and let go on a button. Or press, slide off, let go.");

	if (DoButton(1, x, y + 80.0f, 170.0f, 50.0f, "Add 1"))
	{
		++count;
	}
	if (DoButton(2, x + 190.0f, y + 80.0f, 170.0f, 50.0f, "Reset"))
	{
		count = 0;
	}
	if (DoButton(3, x, y + 145.0f, 360.0f, 50.0f, soundOn ? "Sound: on" : "Sound: off"))
	{
		soundOn = !soundOn;
	}

	char text[64];
	snprintf(text, sizeof(text), "Count: %d", count);
	CP_Settings_Fill(CP_Color_Create(255, 210, 60, 255));
	CP_Settings_TextSize(30.0f);
	CP_Font_DrawText(text, x, y + 250.0f);
	DrawSmallText(pressedButton != -1 ? "Held on a button: let go over it to click" : "", x, y + 290.0f);
}

//---------------------------------------------------------
// 2. Pressed, held, let go

const CP_MOUSE mouseButtons[3] = { CP_MOUSE_LEFT, CP_MOUSE_RIGHT, CP_MOUSE_MIDDLE };
const char* mouseButtonNames[3] = { "Left", "Right", "Middle" };
int presses[3];
int letGoes[3];
float pressRings[3]; // 1 when pressed, down to 0
float letGoRings[3];
int clicks = 0;

void DoPressHoldLetGo(float dt)
{
	float x = panelLeft[1] + 14.0f;
	float y = panelTop[1];
	DrawMousePanel(1, "Pressed, held, let go", "Press the left, right and middle buttons anywhere.");

	for (int i = 0; i < 3; ++i)
	{
		if (CP_Input_MouseTriggered(mouseButtons[i]))
		{
			++presses[i];
			pressRings[i] = 1.0f;
		}
		if (CP_Input_MouseReleased(mouseButtons[i]))
		{
			++letGoes[i];
			letGoRings[i] = 1.0f;
		}
		pressRings[i] = CP_Math_ClampFloat(pressRings[i] - dt * 2.0f, 0.0f, 1.0f);
		letGoRings[i] = CP_Math_ClampFloat(letGoRings[i] - dt * 2.0f, 0.0f, 1.0f);

		// filled while held; a green ring grows when it's pressed and a red
		// one when it's let go
		float cx = x + 60.0f + i * 130.0f;
		float cy = y + 135.0f;
		CP_Settings_StrokeWeight(3.0f);
		CP_Settings_Stroke(CP_Color_Create(200, 200, 215, 255));
		CP_Settings_Fill(CP_Input_MouseDown(mouseButtons[i]) ? CP_Color_Create(255, 210, 60, 255) : CP_Color_Create(30, 32, 44, 255));
		CP_Graphics_DrawCircle(cx, cy, 50.0f);
		CP_Settings_NoFill();
		CP_Settings_Stroke(CP_Color_Create(120, 230, 120, (int)(255.0f * pressRings[i])));
		CP_Graphics_DrawCircle(cx, cy, 50.0f + (1.0f - pressRings[i]) * 40.0f);
		CP_Settings_Stroke(CP_Color_Create(255, 110, 100, (int)(255.0f * letGoRings[i])));
		CP_Graphics_DrawCircle(cx, cy, 50.0f + (1.0f - letGoRings[i]) * 40.0f);
		CP_Settings_NoStroke();

		char text[48];
		CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
		CP_Settings_TextSize(18.0f);
		CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_BASELINE);
		CP_Font_DrawText(mouseButtonNames[i], cx, cy + 60.0f);
		snprintf(text, sizeof(text), "pressed %d", presses[i]);
		CP_Settings_TextSize(15.0f);
		CP_Font_DrawText(text, cx, cy + 82.0f);
		snprintf(text, sizeof(text), "let go %d", letGoes[i]);
		CP_Font_DrawText(text, cx, cy + 100.0f);
		CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);
	}

	// CP_Input_MouseClicked is the left button let go, anywhere
	if (CP_Input_MouseClicked())
	{
		++clicks;
	}
	char text[64];
	snprintf(text, sizeof(text), "CP_Input_MouseClicked (left let go): %d", clicks);
	DrawSmallText(text, x, y + 295.0f);
}

//---------------------------------------------------------
// 3. Click or double click

bool folderChosen = false;
bool folderOpen = false;

void DoDoubleClick(void)
{
	float x = panelLeft[2] + 14.0f;
	float y = panelTop[2];
	DrawMousePanel(2, "Click or double click", "Click the folder to choose it, double click to open it.");

	float folderX = x + 30.0f;
	float folderY = y + 100.0f;
	bool over = MouseIsOver(folderX, folderY - 15.0f, 90.0f, 85.0f);

	// A double click is two clicks close together: the first one has already
	// chosen the folder by the time the second one opens it. That's why
	// "click to choose, double click to open" works well, and why a single
	// click shouldn't do something big.
	if (CP_Input_MouseClicked())
	{
		folderChosen = over;
	}
	if (CP_Input_MouseDoubleClicked() && over)
	{
		folderOpen = !folderOpen;
	}

	if (folderChosen)
	{
		CP_Settings_Fill(CP_Color_Create(80, 120, 200, 120));
		CP_Graphics_DrawRoundedRect(folderX - 10.0f, folderY - 25.0f, 110.0f, 105.0f, 8.0f);
	}
	CP_Settings_Fill(CP_Color_Create(230, 180, 70, 255));
	CP_Graphics_DrawRoundedRect(folderX, folderY - 12.0f, 40.0f, 20.0f, 4.0f);
	CP_Graphics_DrawRoundedRect(folderX, folderY, 90.0f, 65.0f, 6.0f);
	CP_Settings_Fill(CP_Color_Create(240, 240, 250, 255));
	CP_Settings_TextSize(16.0f);
	CP_Font_DrawText("Levels", folderX + 18.0f, folderY + 95.0f);

	if (folderOpen)
	{
		const char* files[] = { "level_1.txt", "level_2.txt", "boss.txt" };
		CP_Settings_Fill(CP_Color_Create(30, 32, 44, 255));
		CP_Graphics_DrawRoundedRect(x + 160.0f, y + 80.0f, 200.0f, 110.0f, 8.0f);
		for (int i = 0; i < 3; ++i)
		{
			DrawSmallText(files[i], x + 175.0f, y + 110.0f + i * 28.0f);
		}
	}

	DrawSmallText(folderChosen ? "Chosen. Click somewhere else to let go of it." : "Not chosen", x, y + 250.0f);
	DrawSmallText(folderOpen ? "Open: double click again to close it" : "Closed", x, y + 275.0f);
}

//---------------------------------------------------------
// 4. Drag and drop

#define GEM_COUNT  3
#define SLOT_COUNT 3
CP_Vector gems[GEM_COUNT];
CP_Vector gemHomes[GEM_COUNT]; // where each starts, in the bag
int gemInSlot[SLOT_COUNT];     // which gem is in each slot, or -1
int draggedGem = -1;
CP_Vector grabOffset; // from the mouse to the gem's center

CP_Vector SlotCenter(int slot)
{
	return CP_Vector_Set(panelLeft[3] + 90.0f + slot * 110.0f, panelTop[3] + 235.0f);
}

CP_Color GemColor(int gem)
{
	switch (gem)
	{
	case 0:  return CP_Color_Create(240, 80, 90, 255);
	case 1:  return CP_Color_Create(80, 200, 120, 255);
	default: return CP_Color_Create(90, 150, 255, 255);
	}
}

void DoDragAndDrop(void)
{
	DrawMousePanel(3, "Drag and drop", "Drag a gem into a slot. Anywhere else sends it back.");
	CP_Vector mouse = CP_Vector_Set(CP_Input_GetMouseX(), CP_Input_GetMouseY());

	// Pick up the gem under the mouse. Keeping the offset from the mouse to
	// its center stops it jumping to center itself on the mouse.
	if (CP_Input_MouseTriggered(CP_MOUSE_LEFT))
	{
		for (int i = 0; i < GEM_COUNT; ++i)
		{
			if (CP_Vector_Distance(mouse, gems[i]) < 28.0f)
			{
				draggedGem = i;
				grabOffset = CP_Vector_Subtract(gems[i], mouse);
				for (int s = 0; s < SLOT_COUNT; ++s)
				{
					if (gemInSlot[s] == i)
					{
						gemInSlot[s] = -1; // it's out of its slot while it's carried
					}
				}
			}
		}
	}

	// Carry it while the button is held
	if (draggedGem >= 0 && CP_Input_MouseDown(CP_MOUSE_LEFT))
	{
		gems[draggedGem] = CP_Vector_Add(mouse, grabOffset);
	}

	// Drop it: into an empty slot it's over, or else back to its home
	if (draggedGem >= 0 && CP_Input_MouseReleased(CP_MOUSE_LEFT))
	{
		int dropSlot = -1;
		for (int s = 0; s < SLOT_COUNT; ++s)
		{
			if (gemInSlot[s] < 0 && CP_Vector_Distance(gems[draggedGem], SlotCenter(s)) < 45.0f)
			{
				dropSlot = s;
			}
		}
		if (dropSlot >= 0)
		{
			gemInSlot[dropSlot] = draggedGem;
			gems[draggedGem] = SlotCenter(dropSlot);
		}
		else
		{
			gems[draggedGem] = gemHomes[draggedGem];
		}
		draggedGem = -1;
	}

	// The slots, lit when the carried gem is over an empty one
	for (int s = 0; s < SLOT_COUNT; ++s)
	{
		CP_Vector c = SlotCenter(s);
		bool target = draggedGem >= 0 && gemInSlot[s] < 0 && CP_Vector_Distance(gems[draggedGem], c) < 45.0f;
		CP_Settings_StrokeWeight(3.0f);
		CP_Settings_Stroke(target ? CP_Color_Create(255, 210, 60, 255) : CP_Color_Create(110, 115, 140, 255));
		CP_Settings_Fill(CP_Color_Create(30, 32, 44, 255));
		CP_Graphics_DrawRoundedRect(c.x - 40.0f, c.y - 40.0f, 80.0f, 80.0f, 10.0f);
	}
	CP_Settings_NoStroke();

	// The gems, the carried one last so it's on top
	for (int i = 0; i < GEM_COUNT; ++i)
	{
		if (i != draggedGem)
		{
			CP_Settings_Fill(GemColor(i));
			CP_Graphics_DrawCircle(gems[i].x, gems[i].y, 50.0f);
		}
	}
	if (draggedGem >= 0)
	{
		CP_Settings_Fill(GemColor(draggedGem));
		CP_Graphics_DrawCircle(gems[draggedGem].x, gems[draggedGem].y, 58.0f);
	}
}

//---------------------------------------------------------
// 5. The wheel

float wheelValue = 50.0f;
float lastWheel = 0.0f;

void DoWheel(void)
{
	float x = panelLeft[4] + 14.0f;
	float y = panelTop[4];
	DrawMousePanel(4, "The wheel", "Turn the wheel anywhere.");

	// How far it turned this frame: positive away from you, 0 when it
	// didn't. A touchpad gives small fractions; a wheel, whole steps.
	float wheel = CP_Input_MouseWheel();
	if (wheel != 0.0f)
	{
		lastWheel = wheel;
	}
	wheelValue = CP_Math_ClampFloat(wheelValue + wheel * 5.0f, 10.0f, 150.0f);

	CP_Settings_Fill(CP_Color_Create(200, 150, 255, 255));
	CP_Graphics_DrawCircle(x + 280.0f, y + 175.0f, wheelValue);

	char text[64];
	snprintf(text, sizeof(text), "Size: %.0f", wheelValue);
	DrawSmallText(text, x, y + 110.0f);
	snprintf(text, sizeof(text), "This frame: %+.2f", wheel);
	DrawSmallText(text, x, y + 140.0f);
	snprintf(text, sizeof(text), "Last turn: %+.2f", lastWheel);
	DrawSmallText(text, x, y + 170.0f);
}

//---------------------------------------------------------
// 6. Movement

// The trail: where the mouse was on each of the last 30 frames it moved,
// newest first, and whether it was over the panel then
#define TRAIL_LENGTH 30
CP_Vector trail[TRAIL_LENGTH];
bool trailOverPanel[TRAIL_LENGTH];

void DoMovement(float dt)
{
	float x = panelLeft[5] + 14.0f;
	float y = panelTop[5];
	DrawMousePanel(5, "Movement", "Move the mouse over this panel.");

	// How far it moved since the last frame, and so how fast
	float dx = CP_Input_GetMouseDeltaX();
	float dy = CP_Input_GetMouseDeltaY();
	float speed = dt > 0.0f ? CP_Vector_Length(CP_Vector_Set(dx, dy)) / dt : 0.0f;
	bool moved = CP_Input_MouseMoved();

	// The trail moves on one point on every frame the mouse moves, wherever
	// it is, so it fades as the mouse moves away and starts fresh when it
	// comes back. Only the parts over the panel are drawn.
	if (moved)
	{
		for (int i = TRAIL_LENGTH - 1; i > 0; --i)
		{
			trail[i] = trail[i - 1];
			trailOverPanel[i] = trailOverPanel[i - 1];
		}
		trail[0] = CP_Vector_Set(CP_Input_GetMouseX(), CP_Input_GetMouseY());
		trailOverPanel[0] = MouseIsOver(panelLeft[5], panelTop[5] + 60.0f, PANEL_WIDTH, PANEL_HEIGHT - 60.0f);
	}
	CP_Settings_StrokeWeight(4.0f);
	for (int i = 1; i < TRAIL_LENGTH; ++i)
	{
		if (trailOverPanel[i - 1] && trailOverPanel[i])
		{
			CP_Settings_Stroke(CP_Color_Create(120, 230, 160, 255 - i * 255 / TRAIL_LENGTH));
			CP_Graphics_DrawLine(trail[i - 1].x, trail[i - 1].y, trail[i].x, trail[i].y);
		}
	}
	CP_Settings_NoStroke();

	char text[64];
	CP_Settings_Fill(moved ? CP_Color_Create(120, 230, 120, 255) : CP_Color_Create(70, 74, 90, 255));
	CP_Graphics_DrawCircle(x + 10.0f, y + 85.0f, 16.0f);
	DrawSmallText(moved ? "Moved this frame" : "Still", x + 26.0f, y + 91.0f);
	snprintf(text, sizeof(text), "Moved by %.0f, %.0f", dx, dy);
	DrawSmallText(text, x, y + 120.0f);
	snprintf(text, sizeof(text), "Speed: %.0f pixels a second", speed);
	DrawSmallText(text, x, y + 145.0f);
}

//---------------------------------------------------------

void game_init(void)
{
	CP_System_SetWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	CP_System_SetWindowTitle("Mouse");
	CP_Settings_RectMode(CP_POSITION_CORNER);

	for (int i = 0; i < GEM_COUNT; ++i)
	{
		gemHomes[i] = CP_Vector_Set(panelLeft[3] + 90.0f + i * 110.0f, panelTop[3] + 110.0f);
		gems[i] = gemHomes[i];
	}
	for (int s = 0; s < SLOT_COUNT; ++s)
	{
		gemInSlot[s] = -1;
	}
}

void game_update(void)
{
	float dt = CP_System_GetDt();

	CP_Graphics_ClearBackground(CP_Color_Create(24, 26, 36, 255));
	CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
	CP_Settings_TextSize(26.0f);
	CP_Font_DrawText("The mouse", 20.0f, 38.0f);

	DoButtons();
	DoPressHoldLetGo(dt);
	DoDoubleClick();
	DoDragAndDrop();
	DoWheel();
	DoMovement(dt);

	// Once the left button is let go, no button is held any more
	if (CP_Input_MouseReleased(CP_MOUSE_LEFT))
	{
		pressedButton = -1;
	}
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
