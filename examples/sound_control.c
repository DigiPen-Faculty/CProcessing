//---------------------------------------------------------
// file:	sound_control.c
//
// Example: controlling sound in a game: a pause menu with its own music,
// a radio that gets quieter far away, and pausing when the window is left.
//
// Shows how to:
// - pause the game's sounds for a pause menu (CP_Sound_PauseGroup), while
//   the menu's own music and clicks play in a group of their own
// - bring them back with CP_Sound_ResumeGroup, then pause again what the
//   player had paused on purpose: resuming brings back everything in the
//   group
// - pause every sound while the window isn't in front (CP_Sound_PauseAll),
//   and resume them when it is again (CP_Sound_ResumeAll)
// - change one playing sound's volume and pitch (CP_Sound_SetVolume and
//   CP_Sound_SetPitch on its instance): a radio that's quieter far away,
//   and slows down while Shift is held
// - raise a sound's pitch for each coin in a quick row, a combo
// - stop a whole group at once with CP_Sound_StopGroup
// - play a sound the simple way, with CP_Sound_Play
//
// Keys: WASD or the arrow keys move. Space jumps. R turns the radio off or
// on, and holding Shift slows it down. P opens and closes the pause menu: Up and
// Down choose, Left and Right change a volume, Enter picks Resume.
// Click another window and everything pauses.
//
// Try this:
// - Turn the radio off, open the pause menu, and close it: the radio stays
//   off. Then delete the line in ApplyPauses that keeps it off, and try
//   again.
// - Give the menu a volume for its own group, CP_SOUND_GROUP_2.
// - Make the radio quieter behind a wall, not only far away.
//
// See also: music_and_sounds.c, for loading and playing music and
// effects; state_effects.c, for music that eases down under a pause;
// lesson 8.
//
// To run it, copy this file over the template's main.c. It needs Music.wav,
// Coin.wav, Jump.wav, HiHat.wav, Cowbell.wav and the notes folder in the
// assets folder, which the template includes.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

// The room the player walks around in
#define ROOM_LEFT   20.0f
#define ROOM_TOP    60.0f
#define ROOM_WIDTH  840.0f
#define ROOM_HEIGHT 640.0f

// The groups. The game's sounds are in the music and effects groups, and the
// pause menu has a group of its own, so pausing the game doesn't pause it.
#define GAME_MUSIC CP_SOUND_GROUP_MUSIC
#define GAME_SFX   CP_SOUND_GROUP_SFX
#define MENU_GROUP CP_SOUND_GROUP_2

CP_Sound music;
CP_Sound coinSound;
CP_Sound jumpSound;
CP_Sound stepSound;
CP_Sound clickSound;
CP_Sound notes[3];                         // the menu's music: C, E and G, one note at a time
const int notePattern[4] = { 0, 1, 2, 1 }; // C E G E, around and around

// The radio: one play of the music, kept so it can change while it plays
CP_SoundInstance radio = 0;
CP_Vector radioPosition;
CP_BOOL radioOn = TRUE;
float radioVolume = 1.0f;
float radioPitch = 1.0f;

// The player, and the coins
CP_Vector player;
float jumpTime = 0.0f; // seconds left in the air
float stepTimer = 0.0f;
#define COIN_COUNT 5
CP_Vector coins[COIN_COUNT];
int combo = 0; // coins in a row, each within a second of the last
float comboTimer = 0.0f;

// The pause menu
CP_BOOL menuOpen = FALSE;
int menuChoice = 0;
#define MENU_ITEMS 3
float noteTimer = 0.0f;
int noteIndex = 0;

// Whether the window was in front last frame
CP_BOOL wasFocused = TRUE;

//---------------------------------------------------------
// Pausing

// Resuming a group, or everything, brings back every sound in it, even one
// that was paused on purpose. So after any resume, this pauses again what
// should stay paused.
void ApplyPauses(void)
{
	if (menuOpen)
	{
		CP_Sound_PauseGroup(GAME_MUSIC);
		CP_Sound_PauseGroup(GAME_SFX);
	}
	if (!radioOn)
	{
		CP_Sound_Pause(radio);
	}
}

void OpenMenu(void)
{
	menuOpen = TRUE;
	menuChoice = 0;
	noteTimer = 0.0f;
	noteIndex = 0;
	ApplyPauses();
}

void CloseMenu(void)
{
	menuOpen = FALSE;
	CP_Sound_StopGroup(MENU_GROUP); // the menu's notes that are still ringing
	CP_Sound_ResumeGroup(GAME_MUSIC);
	CP_Sound_ResumeGroup(GAME_SFX);
	ApplyPauses(); // the radio stays off if the player turned it off
}

//---------------------------------------------------------
// Helpers

CP_Vector RandomSpotInRoom(void)
{
	return CP_Vector_Set(CP_Random_RangeFloat(ROOM_LEFT + 40.0f, ROOM_LEFT + ROOM_WIDTH - 40.0f),
		CP_Random_RangeFloat(ROOM_TOP + 40.0f, ROOM_TOP + ROOM_HEIGHT - 40.0f));
}

void DrawBar(const char* label, float value, float x, float y, CP_Color color)
{
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(30, 32, 44, 255));
	CP_Graphics_DrawRect(x, y, 200.0f, 14.0f);
	CP_Settings_Fill(color);
	CP_Graphics_DrawRect(x, y, 200.0f * value, 14.0f);

	char text[64];
	snprintf(text, sizeof(text), "%s %.2f", label, value);
	CP_Settings_Fill(CP_Color_Create(200, 200, 215, 255));
	CP_Settings_TextSize(16.0f);
	CP_Font_DrawText(text, x + 210.0f, y + 13.0f);
}

//---------------------------------------------------------
// The game

void UpdateGame(float dt)
{
	// Move
	CP_Vector move = CP_Vector_Zero();
	if (CP_Input_KeyDown(CP_KEY_W) || CP_Input_KeyDown(CP_KEY_UP))
	{
		move.y -= 1.0f;
	}
	if (CP_Input_KeyDown(CP_KEY_S) || CP_Input_KeyDown(CP_KEY_DOWN))
	{
		move.y += 1.0f;
	}
	if (CP_Input_KeyDown(CP_KEY_A) || CP_Input_KeyDown(CP_KEY_LEFT))
	{
		move.x -= 1.0f;
	}
	if (CP_Input_KeyDown(CP_KEY_D) || CP_Input_KeyDown(CP_KEY_RIGHT))
	{
		move.x += 1.0f;
	}
	if (CP_Vector_Length(move) > 0.0f)
	{
		player = CP_Vector_Add(player, CP_Vector_Scale(CP_Vector_Normalize(move), 260.0f * dt));
		player.x = CP_Math_ClampFloat(player.x, ROOM_LEFT + 20.0f, ROOM_LEFT + ROOM_WIDTH - 20.0f);
		player.y = CP_Math_ClampFloat(player.y, ROOM_TOP + 20.0f, ROOM_TOP + ROOM_HEIGHT - 20.0f);

		// a quiet step every 0.3 seconds while walking on the ground
		stepTimer -= dt;
		if (stepTimer <= 0.0f && jumpTime <= 0.0f)
		{
			CP_Sound_PlayAdvanced(stepSound, 0.25f, CP_Random_RangeFloat(0.9f, 1.1f), FALSE, GAME_SFX);
			stepTimer = 0.3f;
		}
	}

	// Jump: CP_Sound_Play plays a loaded sound once, at full volume, in the
	// effects group
	if (CP_Input_KeyTriggered(CP_KEY_SPACE) && jumpTime <= 0.0f)
	{
		CP_Sound_Play(jumpSound);
		jumpTime = 0.5f;
	}
	jumpTime -= dt;

	// Coins: each one in a quick row sounds a little higher, up to twice the
	// pitch
	comboTimer -= dt;
	if (comboTimer <= 0.0f)
	{
		combo = 0;
	}
	for (int i = 0; i < COIN_COUNT; ++i)
	{
		if (CP_Vector_Distance(player, coins[i]) < 34.0f)
		{
			float pitch = CP_Math_ClampFloat(1.0f + 0.12f * (float)combo, 1.0f, 2.0f);
			CP_Sound_PlayAdvanced(coinSound, 0.7f, pitch, FALSE, GAME_SFX);
			++combo;
			comboTimer = 1.0f;
			coins[i] = RandomSpotInRoom();
		}
	}

	// The radio: R turns it off or on. Pause keeps its place in the song.
	if (CP_Input_KeyTriggered(CP_KEY_R))
	{
		radioOn = !radioOn;
		if (radioOn)
		{
			CP_Sound_Resume(radio);
		}
		else
		{
			CP_Sound_Pause(radio);
		}
	}

	// Its volume falls from 1 next to it to 0 far away. The group's volume
	// multiplies this.
	float distance = CP_Vector_Distance(player, radioPosition);
	radioVolume = CP_Math_ClampFloat(1.0f - distance / 800.0f, 0.0f, 1.0f);
	CP_Sound_SetVolume(radio, radioVolume);

	// Holding Shift slows it down, like a tape running out, and it speeds
	// up again when Shift is let go
	float target = CP_Input_KeyDown(CP_KEY_LEFT_SHIFT) || CP_Input_KeyDown(CP_KEY_RIGHT_SHIFT) ? 0.5f : 1.0f;
	radioPitch += (target - radioPitch) * CP_Math_ClampFloat(dt * 3.0f, 0.0f, 1.0f);
	CP_Sound_SetPitch(radio, radioPitch);
}

void DrawGame(void)
{
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(40, 44, 60, 255));
	CP_Graphics_DrawRect(ROOM_LEFT, ROOM_TOP, ROOM_WIDTH, ROOM_HEIGHT);

	// The radio, with rings that grow louder the closer the player is
	if (radioOn && !menuOpen)
	{
		float ring = CP_System_GetSeconds() * radioPitch;
		CP_Settings_NoFill();
		CP_Settings_StrokeWeight(3.0f);
		for (int i = 0; i < 3; ++i)
		{
			float grow = (ring + i / 3.0f) - (float)(int)(ring + i / 3.0f); // 0 to 1, over and over
			CP_Settings_Stroke(CP_Color_Create(150, 200, 255, (int)(255.0f * (1.0f - grow) * radioVolume)));
			CP_Graphics_DrawCircle(radioPosition.x, radioPosition.y, 60.0f + grow * 120.0f);
		}
	}
	CP_Settings_NoStroke();
	CP_Settings_Fill(radioOn ? CP_Color_Create(200, 120, 70, 255) : CP_Color_Create(110, 90, 80, 255));
	CP_Graphics_DrawRoundedRect(radioPosition.x - 40.0f, radioPosition.y - 25.0f, 80.0f, 50.0f, 8.0f);
	CP_Settings_Fill(CP_Color_Create(40, 30, 30, 255));
	CP_Graphics_DrawCircle(radioPosition.x - 18.0f, radioPosition.y, 26.0f);
	CP_Graphics_DrawCircle(radioPosition.x + 18.0f, radioPosition.y, 26.0f);

	// The coins
	CP_Settings_Fill(CP_Color_Create(255, 210, 60, 255));
	for (int i = 0; i < COIN_COUNT; ++i)
	{
		CP_Graphics_DrawCircle(coins[i].x, coins[i].y, 22.0f);
	}

	// The player, bigger while in the air
	float size = jumpTime > 0.0f ? 44.0f : 34.0f;
	CP_Settings_Fill(CP_Color_Create(120, 220, 140, 255));
	CP_Graphics_DrawCircle(player.x, player.y, size);
}

//---------------------------------------------------------
// The pause menu

const char* menuNames[MENU_ITEMS] = { "Resume", "Music volume", "Effects volume" };

void UpdateMenu(float dt)
{
	// The menu's music: a note every 0.35 seconds, around and around, in the
	// menu's group, so it plays while the game's groups are paused
	noteTimer -= dt;
	if (noteTimer <= 0.0f)
	{
		CP_Sound_PlayAdvanced(notes[notePattern[noteIndex]], 0.35f, 1.0f, FALSE, MENU_GROUP);
		noteIndex = (noteIndex + 1) % 4;
		noteTimer = 0.35f;
	}

	if (CP_Input_KeyTriggered(CP_KEY_UP))
	{
		menuChoice = (menuChoice + MENU_ITEMS - 1) % MENU_ITEMS;
		CP_Sound_PlayAdvanced(clickSound, 0.4f, 1.5f, FALSE, MENU_GROUP);
	}
	if (CP_Input_KeyTriggered(CP_KEY_DOWN))
	{
		menuChoice = (menuChoice + 1) % MENU_ITEMS;
		CP_Sound_PlayAdvanced(clickSound, 0.4f, 1.5f, FALSE, MENU_GROUP);
	}

	// Left and Right change the chosen group's volume. It's paused, so the
	// change is heard when the game comes back.
	CP_SOUND_GROUP group = menuChoice == 1 ? GAME_MUSIC : GAME_SFX;
	float change = 0.0f;
	if (CP_Input_KeyTriggered(CP_KEY_LEFT))
	{
		change = -0.1f;
	}
	if (CP_Input_KeyTriggered(CP_KEY_RIGHT))
	{
		change = 0.1f;
	}
	if (menuChoice > 0 && change != 0.0f)
	{
		CP_Sound_SetGroupVolume(group, CP_Math_ClampFloat(CP_Sound_GetGroupVolume(group) + change, 0.0f, 1.0f));
		CP_Sound_PlayAdvanced(clickSound, 0.4f, 1.0f, FALSE, MENU_GROUP);
	}

	if (CP_Input_KeyTriggered(CP_KEY_ENTER) && menuChoice == 0)
	{
		CloseMenu();
	}
}

void DrawMenu(void)
{
	float x = ROOM_LEFT + ROOM_WIDTH / 2.0f - 200.0f;
	float y = ROOM_TOP + 180.0f;
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(0, 0, 0, 150));
	CP_Graphics_DrawRect(ROOM_LEFT, ROOM_TOP, ROOM_WIDTH, ROOM_HEIGHT);
	CP_Settings_Fill(CP_Color_Create(30, 32, 48, 240));
	CP_Graphics_DrawRoundedRect(x, y, 400.0f, 250.0f, 14.0f);

	CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
	CP_Settings_TextSize(34.0f);
	CP_Font_DrawText("Paused", x + 30.0f, y + 50.0f);

	for (int i = 0; i < MENU_ITEMS; ++i)
	{
		float rowY = y + 100.0f + i * 45.0f;
		CP_Settings_Fill(i == menuChoice ? CP_Color_Create(255, 210, 60, 255) : CP_Color_Create(190, 190, 205, 255));
		CP_Settings_TextSize(22.0f);
		CP_Font_DrawText(menuNames[i], x + 30.0f, rowY);
		if (i > 0)
		{
			CP_SOUND_GROUP group = i == 1 ? GAME_MUSIC : GAME_SFX;
			CP_Settings_Fill(CP_Color_Create(60, 64, 84, 255));
			CP_Graphics_DrawRect(x + 210.0f, rowY - 14.0f, 160.0f, 12.0f);
			CP_Settings_Fill(CP_Color_Create(150, 200, 255, 255));
			CP_Graphics_DrawRect(x + 210.0f, rowY - 14.0f, 160.0f * CP_Sound_GetGroupVolume(group), 12.0f);
		}
	}
}

//---------------------------------------------------------
// The column on the right

void DrawInfo(void)
{
	float x = ROOM_LEFT + ROOM_WIDTH + 20.0f;
	float y = ROOM_TOP + 20.0f;
	CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
	CP_Settings_TextSize(22.0f);
	CP_Font_DrawText("The groups", x, y);

	CP_Settings_TextSize(16.0f);
	CP_Settings_Fill(CP_Color_Create(190, 190, 205, 255));
	char text[96];
	snprintf(text, sizeof(text), "Music (the radio): %s", menuOpen ? "paused" : "playing");
	CP_Font_DrawText(text, x, y + 30.0f);
	DrawBar("volume", CP_Sound_GetGroupVolume(GAME_MUSIC), x, y + 40.0f, CP_Color_Create(150, 200, 255, 255));
	snprintf(text, sizeof(text), "Effects (steps, jumps, coins): %s", menuOpen ? "paused" : "playing");
	CP_Font_DrawText(text, x, y + 80.0f);
	DrawBar("volume", CP_Sound_GetGroupVolume(GAME_SFX), x, y + 90.0f, CP_Color_Create(255, 180, 60, 255));
	snprintf(text, sizeof(text), "Menu (its notes and clicks): %s", menuOpen ? "playing" : "quiet");
	CP_Font_DrawText(text, x, y + 130.0f);

	CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
	CP_Settings_TextSize(22.0f);
	CP_Font_DrawText("The radio's instance", x, y + 185.0f);
	CP_Settings_TextSize(16.0f);
	CP_Settings_Fill(CP_Color_Create(190, 190, 205, 255));
	snprintf(text, sizeof(text), "R: %s", radioOn ? "on" : "off (paused on purpose)");
	CP_Font_DrawText(text, x, y + 215.0f);
	DrawBar("volume, by distance", radioVolume, x, y + 225.0f, CP_Color_Create(150, 200, 255, 255));
	DrawBar("pitch (hold Shift)", radioPitch, x, y + 250.0f, CP_Color_Create(200, 150, 255, 255));
	snprintf(text, sizeof(text), "Heard at: %.2f (instance x group volume)", radioVolume * CP_Sound_GetGroupVolume(GAME_MUSIC));
	CP_Font_DrawText(text, x, y + 290.0f);

	CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
	CP_Settings_TextSize(22.0f);
	CP_Font_DrawText("Coins", x, y + 345.0f);
	CP_Settings_TextSize(16.0f);
	CP_Settings_Fill(CP_Color_Create(190, 190, 205, 255));
	snprintf(text, sizeof(text), "In a row: %d, next pitch %.2f", combo, CP_Math_ClampFloat(1.0f + 0.12f * (float)combo, 1.0f, 2.0f));
	CP_Font_DrawText(text, x, y + 375.0f);

	const char* keys[] = {
		"WASD or arrows: move   Space: jump",
		"R: radio off or on   Shift (hold): slow it",
		"P: pause menu",
		"Click another window: everything pauses",
	};
	CP_Settings_Fill(CP_Color_Create(150, 200, 255, 255));
	for (int i = 0; i < 4; ++i)
	{
		CP_Font_DrawText(keys[i], x, y + 460.0f + i * 22.0f);
	}
}

//---------------------------------------------------------

void game_init(void)
{
	CP_System_SetWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	CP_System_SetWindowTitle("Sound control");
	CP_Settings_RectMode(CP_POSITION_CORNER);

	music = CP_Sound_LoadStream("assets/Music.wav");
	coinSound = CP_Sound_Load("assets/Coin.wav");
	jumpSound = CP_Sound_Load("assets/Jump.wav");
	stepSound = CP_Sound_Load("assets/HiHat.wav");
	clickSound = CP_Sound_Load("assets/Cowbell.wav");
	notes[0] = CP_Sound_Load("assets/notes/c.wav");
	notes[1] = CP_Sound_Load("assets/notes/e.wav");
	notes[2] = CP_Sound_Load("assets/notes/g.wav");

	player = CP_Vector_Set(ROOM_LEFT + ROOM_WIDTH / 2.0f, ROOM_TOP + ROOM_HEIGHT / 2.0f);
	radioPosition = CP_Vector_Set(ROOM_LEFT + ROOM_WIDTH - 160.0f, ROOM_TOP + 160.0f);
	for (int i = 0; i < COIN_COUNT; ++i)
	{
		coins[i] = RandomSpotInRoom();
	}

	// The radio loops in the music group. Keeping its instance lets the
	// game change its volume and pitch while it plays.
	radio = CP_Sound_PlayAdvanced(music, 1.0f, 1.0f, TRUE, GAME_MUSIC);
}

void game_update(void)
{
	float dt = CP_System_GetDt();

	// Leaving the window pauses every sound, and coming back resumes them
	// all. Resuming everything also resumes what the menu or the player
	// paused, so ApplyPauses pauses those again.
	CP_BOOL focused = CP_System_GetWindowFocus();
	if (!focused && wasFocused)
	{
		CP_Sound_PauseAll();
	}
	if (focused && !wasFocused)
	{
		CP_Sound_ResumeAll();
		ApplyPauses();
	}
	wasFocused = focused;

	if (focused)
	{
		if (CP_Input_KeyTriggered(CP_KEY_P))
		{
			if (menuOpen)
			{
				CloseMenu();
			}
			else
			{
				OpenMenu();
			}
		}
		else if (menuOpen)
		{
			UpdateMenu(dt);
		}
		else
		{
			UpdateGame(dt);
		}
	}

	CP_Graphics_ClearBackground(CP_Color_Create(24, 26, 36, 255));
	CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
	CP_Settings_TextSize(26.0f);
	CP_Font_DrawText("Sound control: groups, instances and pausing", ROOM_LEFT, 38.0f);

	DrawGame();
	if (menuOpen)
	{
		DrawMenu();
	}
	DrawInfo();

	if (!focused)
	{
		CP_Settings_NoStroke();
		CP_Settings_Fill(CP_Color_Create(0, 0, 0, 170));
		CP_Graphics_DrawRect(0.0f, 0.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT);
		CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
		CP_Settings_TextSize(30.0f);
		CP_Font_DrawText("All sound paused: click the window to come back", 330.0f, 360.0f);
	}
}

void game_exit(void)
{
	CP_Sound_StopAll();
	CP_Sound_Free(&music);
	CP_Sound_Free(&coinSound);
	CP_Sound_Free(&jumpSound);
	CP_Sound_Free(&stepSound);
	CP_Sound_Free(&clickSound);
	for (int i = 0; i < 3; ++i)
	{
		CP_Sound_Free(&notes[i]);
	}
}

int main(void)
{
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
