//---------------------------------------------------------
// file:	controls.c
//
// Example: letting the player choose their own keys.
//
// Shows how to:
// - keep the keys for each action in an array, instead of writing CP_KEY_A
//   and CP_KEY_SPACE all through the game
// - find out which key the player pressed with CP_Input_GetKeyTriggered
// - show a key's name on the screen
//
// Click an action, then press the key you want for it. Escape cancels. A key
// that another action already uses swaps over, so no two actions share a
// key. Then try your keys on the square at the bottom.
//
// CP_Input_GetKeyTriggered gives the key pressed this frame, or CP_KEY_NONE
// when there wasn't one. If two keys go down in the same frame, it gives the
// one pressed first.
//
// Try this:
// - Add a fourth action, like a dash that moves the square quickly.
// - Give each action two keys, so it works with WASD and the arrows.
// - Add a "Reset" button that puts back the keys the game started with.
// - Save the keys to a file, and load them when the game starts.
//
// To run it, copy this file over the template's main.c.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

//---------------------------------------------------------
// The actions and their keys. The game asks for keys[ACTION_JUMP] instead
// of CP_KEY_SPACE, so changing an entry here changes the controls.

#define ACTION_LEFT  0
#define ACTION_RIGHT 1
#define ACTION_JUMP  2
#define ACTION_COUNT 3

const char* actionNames[ACTION_COUNT] = { "Move left", "Move right", "Jump" };
CP_KEY keys[ACTION_COUNT] = { CP_KEY_A, CP_KEY_D, CP_KEY_SPACE };

// The action waiting for a key, or -1 when none is
int waitingFor = -1;

// The last key pressed, to show at the bottom
CP_KEY lastKey = CP_KEY_NONE;

//---------------------------------------------------------
// Key names. Letters and digits are their own characters (CP_KEY_A is 'A'
// and CP_KEY_0 is '0'), and the other keys need a name each.

void DrawKeyName(CP_KEY key, float x, float y)
{
	char name[24];
	if ((key >= CP_KEY_A && key <= CP_KEY_Z) || (key >= CP_KEY_0 && key <= CP_KEY_9))
	{
		snprintf(name, sizeof(name), "%c", (char)key);
	}
	else if (key >= CP_KEY_F1 && key <= CP_KEY_F12)
	{
		snprintf(name, sizeof(name), "F%d", key - CP_KEY_F1 + 1);
	}
	else
	{
		switch (key)
		{
		case CP_KEY_NONE:          snprintf(name, sizeof(name), "-"); break;
		case CP_KEY_SPACE:         snprintf(name, sizeof(name), "Space"); break;
		case CP_KEY_ENTER:         snprintf(name, sizeof(name), "Enter"); break;
		case CP_KEY_TAB:           snprintf(name, sizeof(name), "Tab"); break;
		case CP_KEY_BACKSPACE:     snprintf(name, sizeof(name), "Backspace"); break;
		case CP_KEY_ESCAPE:        snprintf(name, sizeof(name), "Escape"); break;
		case CP_KEY_LEFT:          snprintf(name, sizeof(name), "Left arrow"); break;
		case CP_KEY_RIGHT:         snprintf(name, sizeof(name), "Right arrow"); break;
		case CP_KEY_UP:            snprintf(name, sizeof(name), "Up arrow"); break;
		case CP_KEY_DOWN:          snprintf(name, sizeof(name), "Down arrow"); break;
		case CP_KEY_LEFT_SHIFT:    snprintf(name, sizeof(name), "Left Shift"); break;
		case CP_KEY_RIGHT_SHIFT:   snprintf(name, sizeof(name), "Right Shift"); break;
		case CP_KEY_LEFT_CONTROL:  snprintf(name, sizeof(name), "Left Ctrl"); break;
		case CP_KEY_RIGHT_CONTROL: snprintf(name, sizeof(name), "Right Ctrl"); break;
		case CP_KEY_LEFT_ALT:      snprintf(name, sizeof(name), "Left Alt"); break;
		case CP_KEY_RIGHT_ALT:     snprintf(name, sizeof(name), "Right Alt"); break;
		default:                   snprintf(name, sizeof(name), "Key %d", (int)key); break;
		}
	}
	CP_Font_DrawText(name, x, y);
}

//---------------------------------------------------------
// The menu: one row per action

#define ROW_LEFT   40.0f
#define ROW_TOP    90.0f
#define ROW_WIDTH  420.0f
#define ROW_HEIGHT 50.0f
#define ROW_GAP    14.0f

float RowTop(int action)
{
	return ROW_TOP + (float)action * (ROW_HEIGHT + ROW_GAP);
}

CP_BOOL MouseIsOverRow(int action)
{
	const float mouseX = CP_Input_GetMouseX();
	const float mouseY = CP_Input_GetMouseY();
	return mouseX >= ROW_LEFT && mouseX < ROW_LEFT + ROW_WIDTH && mouseY >= RowTop(action) && mouseY < RowTop(action) + ROW_HEIGHT;
}

// Give the waiting action its new key. If another action had that key, it
// gets the waiting action's old key, so every action keeps a key of its own.
void ChooseKey(CP_KEY key)
{
	for (int other = 0; other < ACTION_COUNT; ++other)
	{
		if (other != waitingFor && keys[other] == key)
		{
			keys[other] = keys[waitingFor];
		}
	}
	keys[waitingFor] = key;
	waitingFor = -1;
}

void UpdateMenu(CP_KEY pressed)
{
	if (waitingFor >= 0)
	{
		// Waiting: the next key pressed is the one, unless it's Escape
		if (pressed == CP_KEY_ESCAPE)
		{
			waitingFor = -1;
		}
		else if (pressed != CP_KEY_NONE)
		{
			ChooseKey(pressed);
		}
		return;
	}

	// Not waiting: a click on a row starts waiting for that action's key
	if (CP_Input_MouseClicked())
	{
		for (int action = 0; action < ACTION_COUNT; ++action)
		{
			if (MouseIsOverRow(action))
			{
				waitingFor = action;
			}
		}
	}
}

void DrawMenu(void)
{
	CP_Settings_TextSize(26.0f);
	CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
	CP_Font_DrawText("Controls: click one to change it", ROW_LEFT, 60.0f);

	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	for (int action = 0; action < ACTION_COUNT; ++action)
	{
		const float top = RowTop(action);
		if (action == waitingFor)
		{
			CP_Settings_Fill(CP_Color_Create(255, 170, 60, 255));
		}
		else if (waitingFor < 0 && MouseIsOverRow(action))
		{
			CP_Settings_Fill(CP_Color_Create(70, 78, 110, 255));
		}
		else
		{
			CP_Settings_Fill(CP_Color_Create(45, 50, 70, 255));
		}
		CP_Graphics_DrawRoundedRect(ROW_LEFT, top, ROW_WIDTH, ROW_HEIGHT, 10.0f);

		const float textY = top + ROW_HEIGHT / 2.0f + 8.0f;
		CP_Settings_TextSize(24.0f);
		CP_Settings_Fill(action == waitingFor ? CP_Color_Create(30, 30, 40, 255) : CP_Color_Create(230, 230, 240, 255));
		CP_Font_DrawText(actionNames[action], ROW_LEFT + 18.0f, textY);
		if (action == waitingFor)
		{
			CP_Font_DrawText("Press a key...", ROW_LEFT + 220.0f, textY);
		}
		else
		{
			DrawKeyName(keys[action], ROW_LEFT + 220.0f, textY);
		}
	}

	CP_Settings_TextSize(20.0f);
	CP_Settings_Fill(CP_Color_Create(170, 170, 190, 255));
	CP_Font_DrawText(waitingFor >= 0 ? "Escape cancels" : "Then try your keys on the square below", ROW_LEFT, RowTop(ACTION_COUNT) + 20.0f);
}

//---------------------------------------------------------
// A square to try the keys on

const float floorY = 520.0f;
const float playerSize = 40.0f;
const float walkSpeed = 300.0f; // pixels per second
const float jumpSpeed = 600.0f; // pixels per second, upwards
const float gravity = 1600.0f;  // pixels per second, per second
float playerX = 400.0f;
float playerY = 520.0f; // the bottom of the square
float playerSpeedY = 0.0f;

void UpdatePlayer(void)
{
	const float dt = CP_System_GetDt();

	// While a key is being chosen, the keys don't move the square
	if (waitingFor < 0)
	{
		if (CP_Input_KeyDown(keys[ACTION_LEFT]))
		{
			playerX -= walkSpeed * dt;
		}
		if (CP_Input_KeyDown(keys[ACTION_RIGHT]))
		{
			playerX += walkSpeed * dt;
		}
		if (CP_Input_KeyTriggered(keys[ACTION_JUMP]) && playerY >= floorY)
		{
			playerSpeedY = -jumpSpeed;
		}
	}
	playerX = CP_Math_ClampFloat(playerX, playerSize / 2.0f, (float)CP_System_GetWindowWidth() - playerSize / 2.0f);

	playerSpeedY += gravity * dt;
	playerY += playerSpeedY * dt;
	if (playerY > floorY)
	{
		playerY = floorY;
		playerSpeedY = 0.0f;
	}
}

void DrawPlayer(void)
{
	CP_Settings_Fill(CP_Color_Create(60, 66, 90, 255));
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Graphics_DrawRect(0.0f, floorY, (float)CP_System_GetWindowWidth(), 80.0f);

	CP_Settings_Fill(CP_Color_Create(255, 120, 90, 255));
	CP_Graphics_DrawRect(playerX - playerSize / 2.0f, playerY - playerSize, playerSize, playerSize);
}

//---------------------------------------------------------

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Choosing your controls");
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);
}

void game_update(void)
{
	// Ask once a frame, and give the answer to everything that needs it
	const CP_KEY pressed = CP_Input_GetKeyTriggered();
	if (pressed != CP_KEY_NONE)
	{
		lastKey = pressed;
	}

	// The square first: a key chosen in the menu this frame (Space for Jump,
	// say) shouldn't also make the square jump
	UpdatePlayer();
	UpdateMenu(pressed);

	CP_Graphics_ClearBackground(CP_Color_Create(24, 26, 36, 255));
	DrawMenu();
	DrawPlayer();

	CP_Settings_TextSize(20.0f);
	CP_Settings_Fill(CP_Color_Create(200, 200, 215, 255));
	CP_Font_DrawText("Last key pressed:", 20.0f, 585.0f);
	DrawKeyName(lastKey, 190.0f, 585.0f);
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
