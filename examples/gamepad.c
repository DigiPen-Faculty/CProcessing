//---------------------------------------------------------
// file:	gamepad.c
//
// Example: gamepads: every button, stick and trigger, for up to 4 players.
//
// Shows how to:
// - read one gamepad by its index with the Advanced functions: the same
//   functions with an index from 0 to 3 at the end
// - know which gamepad the functions without Advanced read (the default)
// - tell pressed this frame (Triggered), held (Down) and let go this frame
//   (Released) apart
// - read the sticks (-1 to 1, y positive up) and the triggers (0 to 1)
// - notice when a gamepad is connected or disconnected
// - give each gamepad its own player
//
// Controls: connect up to 4 gamepads. Each panel shows one. A button
// lights up while it's held and rings when it's pressed. In the box on the
// bottom right, each gamepad moves its own player with the left stick, and
// the right trigger makes it faster.
//
// Try this:
// - Give each player a jump on A (Triggered), and a higher one when A is
//   held longer.
// - Move through a menu with the left stick: move one item when the stick
//   goes past 0.5, and not again until it comes back under 0.5.
// - Let players join by pressing Start, so a player isn't added for a
//   gamepad nobody is holding. When a gamepad disconnects, pause and ask
//   everyone to press Start again.
//
// See also: movement.c, for moving with the keyboard or a gamepad's stick;
// controls.c, for keys a player can change; lesson 4.
//
// To run it, copy this file over the template's main.c.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>
#include <string.h>

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

// The 2 by 2 panels, one for each gamepad index
#define PANEL_WIDTH  400.0f
#define PANEL_HEIGHT 315.0f
const float panelLeft[CP_MAX_GAMEPADS] = { 20.0f, 440.0f, 20.0f, 440.0f };
const float panelTop[CP_MAX_GAMEPADS] = { 60.0f, 60.0f, 385.0f, 385.0f };

// The box on the right where the players move
#define ARENA_LEFT   860.0f
#define ARENA_TOP    385.0f
#define ARENA_WIDTH  400.0f
#define ARENA_HEIGHT 315.0f

// The buttons are numbered from CP_GAMEPAD_DPAD_UP (0) to CP_GAMEPAD_Y, so
// a loop can check them all
#define BUTTON_COUNT (CP_GAMEPAD_Y + 1)
const char* buttonNames[BUTTON_COUNT] = {
	"D-pad up",
	"D-pad down",
	"D-pad left",
	"D-pad right",
	"Start",
	"Back",
	"Left stick (pushed in)",
	"Right stick (pushed in)",
	"Left bumper",
	"Right bumper",
	"A",
	"B",
	"X",
	"Y",
};

// What each gamepad did, by index
float ringTimers[CP_MAX_GAMEPADS][BUTTON_COUNT]; // 1 when pressed, down to 0
int lastPressed[CP_MAX_GAMEPADS];                // a button, or -1 for none yet
int lastReleased[CP_MAX_GAMEPADS];
bool wasConnected[CP_MAX_GAMEPADS];

// A on the default gamepad, counted with the functions without Advanced
int defaultPresses = 0;
int defaultReleases = 0;

// Connected and disconnected messages, newest first
#define LOG_SIZE 5
char logLines[LOG_SIZE][64];
int logCount = 0;

// The players, one for each gamepad index
CP_Vector players[CP_MAX_GAMEPADS];
const float playerSize = 30.0f;

//---------------------------------------------------------
// Helpers

CP_Color PlayerColor(int pad)
{
	switch (pad)
	{
	case 0:  return CP_Color_Create(90, 170, 255, 255);
	case 1:  return CP_Color_Create(255, 120, 90, 255);
	case 2:  return CP_Color_Create(120, 220, 120, 255);
	default: return CP_Color_Create(240, 200, 80, 255);
	}
}

void AddLogLine(const char* text)
{
	// move every line down one row (the last one drops off), then put the
	// new line on top. memmove is the copy for when the two places overlap.
	memmove(logLines[1], logLines[0], sizeof(logLines[0]) * (LOG_SIZE - 1));
	snprintf(logLines[0], sizeof(logLines[0]), "%s", text);
	if (logCount < LOG_SIZE)
	{
		++logCount;
	}
}

// A button centered at x, y: filled while it's held, outlined when it's
// not, with a ring that grows and fades after it's pressed. Corners round
// by half the height, so a square is a circle and a wide one is a pill.
void DrawPadButton(int pad, CP_GAMEPAD button, float x, float y, float width, float height, CP_Color color)
{
	float corner = height / 2.0f;
	CP_Settings_StrokeWeight(2.0f);
	CP_Settings_Stroke(color);
	if (CP_Input_GamepadDownAdvanced(button, pad))
	{
		CP_Settings_Fill(color);
	}
	else
	{
		CP_Settings_Fill(CP_Color_Create(30, 32, 44, 255));
	}
	CP_Graphics_DrawRoundedRect(x - width / 2.0f, y - height / 2.0f, width, height, corner);

	float ring = ringTimers[pad][button];
	if (ring > 0.0f)
	{
		float grow = (1.0f - ring) * 14.0f;
		CP_Settings_NoFill();
		CP_Settings_Stroke(CP_Color_Create(255, 255, 255, (int)(ring * 255.0f)));
		CP_Graphics_DrawRoundedRect(x - width / 2.0f - grow, y - height / 2.0f - grow, width + grow * 2.0f, height + grow * 2.0f, corner + grow);
	}
}

// A stick: the dot moves inside the circle, and the stick is also a button
// when it's pushed in. Sticks have y positive up and the screen has y
// positive down, so y flips.
void DrawStick(int pad, CP_GAMEPAD button, CP_Vector stick, float x, float y)
{
	const float reach = 18.0f;
	CP_Settings_NoFill();
	CP_Settings_StrokeWeight(2.0f);
	CP_Settings_Stroke(CP_Color_Create(120, 125, 150, 255));
	CP_Graphics_DrawCircle(x, y, 64.0f);
	DrawPadButton(pad, button, x + stick.x * reach, y - stick.y * reach, 28.0f, 28.0f, CP_Color_Create(220, 220, 235, 255));
}

// A trigger: a bar that fills from 0 to 1, with its number
void DrawTrigger(const char* name, float value, float x, float y)
{
	const float width = 140.0f;
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(30, 32, 44, 255));
	CP_Graphics_DrawRect(x, y, width, 14.0f);
	if (value > 0.0f)
	{
		CP_Settings_Fill(CP_Color_Create(255, 180, 60, 255));
		CP_Graphics_DrawRect(x, y, width * value, 14.0f);
	}

	char text[32];
	snprintf(text, sizeof(text), "%s %.2f", name, value);
	CP_Settings_Fill(CP_Color_Create(200, 200, 215, 255));
	CP_Settings_TextSize(15.0f);
	CP_Font_DrawText(text, x, y - 6.0f);
}

//---------------------------------------------------------
// One gamepad's panel

void DrawGamepad(int pad)
{
	float left = panelLeft[pad];
	float top = panelTop[pad];
	bool connected = CP_Input_GamepadConnectedAdvanced(pad);

	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(40, 44, 60, 255));
	CP_Graphics_DrawRect(left, top, PANEL_WIDTH, PANEL_HEIGHT);

	char text[96];
	snprintf(text, sizeof(text), "Gamepad %d", pad);
	CP_Settings_Fill(PlayerColor(pad));
	CP_Settings_TextSize(22.0f);
	CP_Font_DrawText(text, left + 14.0f, top + 28.0f);

	CP_Settings_TextSize(16.0f);
	if (!connected)
	{
		CP_Settings_Fill(CP_Color_Create(130, 130, 145, 255));
		CP_Font_DrawText("not connected", left + 140.0f, top + 27.0f);
		return;
	}

	// Indexes have no gaps (see game_update), so whenever a gamepad is
	// connected, index 0 is the default
	CP_Settings_Fill(CP_Color_Create(140, 220, 140, 255));
	CP_Font_DrawText(pad == 0 ? "connected, the default" : "connected", left + 140.0f, top + 27.0f);

	// The body, laid out like an Xbox controller
	float x = left + 40.0f;
	float y = top + 66.0f;
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(60, 65, 88, 255));
	CP_Graphics_DrawRoundedRect(x, y, 320.0f, 165.0f, 50.0f);

	CP_Color gray = CP_Color_Create(200, 200, 215, 255);
	DrawPadButton(pad, CP_GAMEPAD_LEFT_SHOULDER, x + 60.0f, y - 12.0f, 80.0f, 16.0f, gray);
	DrawPadButton(pad, CP_GAMEPAD_RIGHT_SHOULDER, x + 260.0f, y - 12.0f, 80.0f, 16.0f, gray);
	DrawPadButton(pad, CP_GAMEPAD_BACK, x + 135.0f, y + 45.0f, 24.0f, 12.0f, gray);
	DrawPadButton(pad, CP_GAMEPAD_START, x + 185.0f, y + 45.0f, 24.0f, 12.0f, gray);

	DrawStick(pad, CP_GAMEPAD_LEFT_THUMB, CP_Input_GamepadLeftStickAdvanced(pad), x + 65.0f, y + 50.0f);
	DrawStick(pad, CP_GAMEPAD_RIGHT_THUMB, CP_Input_GamepadRightStickAdvanced(pad), x + 205.0f, y + 115.0f);

	float dpadX = x + 115.0f;
	float dpadY = y + 115.0f;
	DrawPadButton(pad, CP_GAMEPAD_DPAD_UP, dpadX, dpadY - 22.0f, 20.0f, 20.0f, gray);
	DrawPadButton(pad, CP_GAMEPAD_DPAD_DOWN, dpadX, dpadY + 22.0f, 20.0f, 20.0f, gray);
	DrawPadButton(pad, CP_GAMEPAD_DPAD_LEFT, dpadX - 22.0f, dpadY, 20.0f, 20.0f, gray);
	DrawPadButton(pad, CP_GAMEPAD_DPAD_RIGHT, dpadX + 22.0f, dpadY, 20.0f, 20.0f, gray);

	float faceX = x + 255.0f;
	float faceY = y + 50.0f;
	DrawPadButton(pad, CP_GAMEPAD_Y, faceX, faceY - 24.0f, 24.0f, 24.0f, CP_Color_Create(240, 200, 60, 255));
	DrawPadButton(pad, CP_GAMEPAD_A, faceX, faceY + 24.0f, 24.0f, 24.0f, CP_Color_Create(90, 200, 90, 255));
	DrawPadButton(pad, CP_GAMEPAD_X, faceX - 24.0f, faceY, 24.0f, 24.0f, CP_Color_Create(80, 140, 255, 255));
	DrawPadButton(pad, CP_GAMEPAD_B, faceX + 24.0f, faceY, 24.0f, 24.0f, CP_Color_Create(240, 80, 80, 255));

	DrawTrigger("Left trigger", CP_Input_GamepadLeftTriggerAdvanced(pad), left + 40.0f, top + 256.0f);
	DrawTrigger("Right trigger", CP_Input_GamepadRightTriggerAdvanced(pad), left + 220.0f, top + 256.0f);

	CP_Vector leftStick = CP_Input_GamepadLeftStickAdvanced(pad);
	CP_Vector rightStick = CP_Input_GamepadRightStickAdvanced(pad);
	snprintf(text, sizeof(text), "Left stick %.2f, %.2f    Right stick %.2f, %.2f", leftStick.x, leftStick.y, rightStick.x, rightStick.y);
	CP_Settings_Fill(CP_Color_Create(200, 200, 215, 255));
	CP_Settings_TextSize(15.0f);
	CP_Font_DrawText(text, left + 14.0f, top + 286.0f);

	snprintf(text, sizeof(text), "Pressed: %s    Let go: %s", lastPressed[pad] < 0 ? "-" : buttonNames[lastPressed[pad]],
		lastReleased[pad] < 0 ? "-" : buttonNames[lastReleased[pad]]);
	CP_Font_DrawText(text, left + 14.0f, top + 305.0f);
}

//---------------------------------------------------------
// What happened this frame

void UpdateGamepads(float dt)
{
	for (int pad = 0; pad < CP_MAX_GAMEPADS; ++pad)
	{
		// Connected or disconnected since the last frame
		bool connected = CP_Input_GamepadConnectedAdvanced(pad);
		if (connected != wasConnected[pad])
		{
			char text[64];
			snprintf(text, sizeof(text), "%.1f s: gamepad %d %s", CP_System_GetSeconds(), pad, connected ? "connected" : "disconnected");
			AddLogLine(text);
			wasConnected[pad] = connected;
			lastPressed[pad] = -1;
			lastReleased[pad] = -1;
		}

		for (int button = 0; button < BUTTON_COUNT; ++button)
		{
			if (CP_Input_GamepadTriggeredAdvanced((CP_GAMEPAD)button, pad))
			{
				ringTimers[pad][button] = 1.0f;
				lastPressed[pad] = button;
			}
			if (CP_Input_GamepadReleasedAdvanced((CP_GAMEPAD)button, pad))
			{
				lastReleased[pad] = button;
			}
			ringTimers[pad][button] -= dt * 2.0f; // gone in half a second
			if (ringTimers[pad][button] < 0.0f)
			{
				ringTimers[pad][button] = 0.0f;
			}
		}
	}

	// The functions without Advanced read the default gamepad, index 0. A
	// game for one player can use them and never think about indexes.
	if (CP_Input_GamepadTriggered(CP_GAMEPAD_A))
	{
		++defaultPresses;
	}
	if (CP_Input_GamepadReleased(CP_GAMEPAD_A))
	{
		++defaultReleases;
	}
}

// Each gamepad moves its own player: the stick gives the direction, and
// the trigger, from 0 to 1, gives extra speed
void UpdatePlayers(float dt)
{
	for (int pad = 0; pad < CP_MAX_GAMEPADS; ++pad)
	{
		if (!CP_Input_GamepadConnectedAdvanced(pad))
		{
			continue;
		}
		CP_Vector stick = CP_Input_GamepadLeftStickAdvanced(pad);
		float speed = 150.0f + 300.0f * CP_Input_GamepadRightTriggerAdvanced(pad);
		players[pad].x += stick.x * speed * dt;
		players[pad].y -= stick.y * speed * dt; // the stick's y is up
		players[pad].x = CP_Math_ClampFloat(players[pad].x, ARENA_LEFT + playerSize / 2.0f, ARENA_LEFT + ARENA_WIDTH - playerSize / 2.0f);
		players[pad].y = CP_Math_ClampFloat(players[pad].y, ARENA_TOP + 50.0f + playerSize / 2.0f, ARENA_TOP + ARENA_HEIGHT - playerSize / 2.0f);
	}
}

//---------------------------------------------------------
// The column on the right

void DrawInfo(void)
{
	int count = 0;
	for (int pad = 0; pad < CP_MAX_GAMEPADS; ++pad)
	{
		if (CP_Input_GamepadConnectedAdvanced(pad))
		{
			++count;
		}
	}

	float x = ARENA_LEFT;
	float y = 84.0f;
	char text[64];
	snprintf(text, sizeof(text), "Connected: %d of %d", count, CP_MAX_GAMEPADS);
	CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
	CP_Settings_TextSize(22.0f);
	CP_Font_DrawText(text, x, y);

	const char* lines[] = {
		"The functions without Advanced, like",
		"CP_Input_GamepadTriggered(CP_GAMEPAD_A),",
		"read the default: the first gamepad.",
		"Indexes have no gaps. Unplug gamepad 0",
		"and gamepad 1 becomes gamepad 0.",
		"The index that goes missing is 1.",
	};
	CP_Settings_TextSize(16.0f);
	CP_Settings_Fill(CP_Color_Create(190, 190, 205, 255));
	y += 30.0f;
	for (int i = 0; i < 6; ++i)
	{
		CP_Font_DrawText(lines[i], x, y);
		y += 20.0f;
	}
	snprintf(text, sizeof(text), "The default's A: pressed %d, let go %d", defaultPresses, defaultReleases);
	CP_Settings_Fill(PlayerColor(0));
	CP_Font_DrawText(text, x, y);
	y += 20.0f;

	y += 14.0f;
	CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
	CP_Font_DrawText("Connected and disconnected:", x, y);
	CP_Settings_Fill(CP_Color_Create(150, 200, 255, 255));
	for (int i = 0; i < logCount; ++i)
	{
		y += 20.0f;
		CP_Font_DrawText(logLines[i], x, y);
	}
}

void DrawPlayers(void)
{
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(40, 44, 60, 255));
	CP_Graphics_DrawRect(ARENA_LEFT, ARENA_TOP, ARENA_WIDTH, ARENA_HEIGHT);

	CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
	CP_Settings_TextSize(20.0f);
	CP_Font_DrawText("A player for each gamepad", ARENA_LEFT + 12.0f, ARENA_TOP + 24.0f);
	CP_Settings_Fill(CP_Color_Create(190, 190, 205, 255));
	CP_Settings_TextSize(15.0f);
	CP_Font_DrawText("Left stick: move.  Right trigger: faster.", ARENA_LEFT + 12.0f, ARENA_TOP + 43.0f);

	bool anyone = false;
	for (int pad = 0; pad < CP_MAX_GAMEPADS; ++pad)
	{
		if (!CP_Input_GamepadConnectedAdvanced(pad))
		{
			continue;
		}
		anyone = true;
		CP_Settings_Fill(PlayerColor(pad));
		CP_Graphics_DrawCircle(players[pad].x, players[pad].y, playerSize);

		char number[4];
		snprintf(number, sizeof(number), "%d", pad);
		CP_Settings_Fill(CP_Color_Create(20, 20, 30, 255));
		CP_Settings_TextSize(18.0f);
		CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
		CP_Font_DrawText(number, players[pad].x, players[pad].y);
		CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);
	}

	if (!anyone)
	{
		CP_Settings_Fill(CP_Color_Create(130, 130, 145, 255));
		CP_Settings_TextSize(16.0f);
		CP_Font_DrawText("Connect a gamepad to add a player.", ARENA_LEFT + 12.0f, ARENA_TOP + 170.0f);
	}
}

//---------------------------------------------------------

void game_init(void)
{
	CP_System_SetWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	CP_System_SetWindowTitle("Gamepads");
	CP_Settings_RectMode(CP_POSITION_CORNER);

	for (int pad = 0; pad < CP_MAX_GAMEPADS; ++pad)
	{
		lastPressed[pad] = -1;
		lastReleased[pad] = -1;
		wasConnected[pad] = false;
		players[pad] = CP_Vector_Set(ARENA_LEFT + 80.0f + pad * 80.0f, ARENA_TOP + 190.0f);
	}
}

void game_update(void)
{
	float dt = CP_System_GetDt();

	// A gamepad's index is its place among the gamepads connected: the
	// first is 0, the next 1, and so on, with no gaps. So when gamepad 0 is
	// unplugged, gamepad 1 becomes gamepad 0 and takes over player 0, and
	// the log says "gamepad 1 disconnected", the index that's now missing.
	// Many games pause when a gamepad disconnects and ask each player to
	// press Start again.
	UpdateGamepads(dt);
	UpdatePlayers(dt);

	CP_Graphics_ClearBackground(CP_Color_Create(24, 26, 36, 255));
	CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
	CP_Settings_TextSize(26.0f);
	CP_Font_DrawText("Gamepads: up to 4 at once", 20.0f, 38.0f);

	for (int pad = 0; pad < CP_MAX_GAMEPADS; ++pad)
	{
		DrawGamepad(pad);
	}
	DrawInfo();
	DrawPlayers();
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
