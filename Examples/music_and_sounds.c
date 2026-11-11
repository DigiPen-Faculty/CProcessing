//---------------------------------------------------------
// file:	music_and_sounds.c
//
// Example: background music and sound effects.
//
// Shows how to:
// - stream music with CP_Sound_LoadStream, and load short sounds with
//   CP_Sound_Load
// - play music on a loop in the music group, and sound effects in the
//   effects group, so each group has its own volume
// - keep the handle that CP_Sound_PlayAdvanced returns, to pause, resume or
//   stop that one sound (and to ask if it is still playing)
// - vary a sound effect's pitch so repeats don't sound the same
// - load sounds once, in init, and free them in exit
//
// Keys: 1, 2 and 3 play sound effects. M pauses and resumes the music, and S
// stops it or starts it again.
// Up and Down change the music volume, Left and Right the effects volume.
//
// Try this:
// - Play the coin sound when the player picks something up in one of the
//   other examples.
// - Slow the music down (CP_Sound_SetGroupPitch) while a key is held.
//
// To run it, copy this file over the template's main.c. It needs Music.wav,
// Coin.wav, Jump.wav and Hit.wav in the Assets folder, which the template
// includes.
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

CP_Sound music;
CP_Sound coinSound;
CP_Sound jumpSound;
CP_Sound hitSound;

// The music that is playing now. Playing a sound gives you a handle for that
// one play, so you can pause, resume or stop it. (0 is "no sound".)
CP_SoundHandle musicHandle = 0;
CP_BOOL musicPaused = FALSE;
const char* lastEffect = "none yet";
float lastPitch = 1.0f;

// Play an effect at full volume, a random pitch between 0.9 and 1.1, not
// looping, in the effects group
void PlayEffect(CP_Sound sound, const char* name)
{
	lastPitch = CP_Random_RangeFloat(0.9f, 1.1f);
	lastEffect = name;
	CP_Sound_PlayAdvanced(sound, 1.0f, lastPitch, FALSE, CP_SOUND_GROUP_SFX);
}

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Music and sounds");

	// Streaming reads the file a little at a time while it plays, which
	// suits long music. Short effects load all at once, so they start
	// instantly.
	music = CP_Sound_LoadStream("Assets/Music.wav");
	coinSound = CP_Sound_Load("Assets/Coin.wav");
	jumpSound = CP_Sound_Load("Assets/Jump.wav");
	hitSound = CP_Sound_Load("Assets/Hit.wav");

	// Start the music, looping, in the music group
	musicHandle = CP_Sound_PlayAdvanced(music, 1.0f, 1.0f, TRUE, CP_SOUND_GROUP_MUSIC);
	CP_Sound_SetGroupVolume(CP_SOUND_GROUP_MUSIC, 0.6f);
}

void game_update(void)
{
	if (CP_Input_KeyTriggered(KEY_1))
	{
		PlayEffect(coinSound, "coin");
	}
	if (CP_Input_KeyTriggered(KEY_2))
	{
		PlayEffect(jumpSound, "jump");
	}
	if (CP_Input_KeyTriggered(KEY_3))
	{
		PlayEffect(hitSound, "hit");
	}

	// M pauses or resumes the music, only while it's playing
	if (CP_Input_KeyTriggered(KEY_M) && CP_Sound_IsPlaying(musicHandle))
	{
		if (musicPaused)
		{
			CP_Sound_Resume(musicHandle);
		}
		else
		{
			CP_Sound_Pause(musicHandle);
		}
		musicPaused = !musicPaused;
	}

	// S stops the music, or starts it again from the beginning
	if (CP_Input_KeyTriggered(KEY_S))
	{
		if (CP_Sound_IsPlaying(musicHandle))
		{
			CP_Sound_Stop(musicHandle);
		}
		else
		{
			musicHandle = CP_Sound_PlayAdvanced(music, 1.0f, 1.0f, TRUE, CP_SOUND_GROUP_MUSIC);
		}
		musicPaused = FALSE;
	}

	// Each group's volume, from 0 (silent) to 1 (full), in steps of 0.1
	float musicVolume = CP_Sound_GetGroupVolume(CP_SOUND_GROUP_MUSIC);
	float effectsVolume = CP_Sound_GetGroupVolume(CP_SOUND_GROUP_SFX);
	if (CP_Input_KeyTriggered(KEY_UP))
	{
		musicVolume += 0.1f;
	}
	if (CP_Input_KeyTriggered(KEY_DOWN))
	{
		musicVolume -= 0.1f;
	}
	if (CP_Input_KeyTriggered(KEY_RIGHT))
	{
		effectsVolume += 0.1f;
	}
	if (CP_Input_KeyTriggered(KEY_LEFT))
	{
		effectsVolume -= 0.1f;
	}
	CP_Sound_SetGroupVolume(CP_SOUND_GROUP_MUSIC, CP_Math_ClampFloat(musicVolume, 0.0f, 1.0f));
	CP_Sound_SetGroupVolume(CP_SOUND_GROUP_SFX, CP_Math_ClampFloat(effectsVolume, 0.0f, 1.0f));

	// Show what's going on, with a bar for each volume
	CP_Graphics_ClearBackground(CP_Color_Create(30, 30, 50, 255));
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_NoStroke();
	CP_Settings_TextSize(24.0f);

	char text[100];
	const char* musicState = !CP_Sound_IsPlaying(musicHandle) ? "stopped" : (musicPaused ? "paused" : "playing");
	snprintf(text, sizeof(text), "Music: %s, volume %.1f", musicState, CP_Sound_GetGroupVolume(CP_SOUND_GROUP_MUSIC));
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Font_DrawText(text, 60.0f, 120.0f);
	CP_Settings_Fill(CP_Color_Create(80, 200, 255, 255));
	CP_Graphics_DrawRect(60.0f, 140.0f, 400.0f * CP_Sound_GetGroupVolume(CP_SOUND_GROUP_MUSIC), 20.0f);

	snprintf(text, sizeof(text), "Effects: volume %.1f    Last played: %s, pitch %.2f", CP_Sound_GetGroupVolume(CP_SOUND_GROUP_SFX), lastEffect, lastPitch);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Font_DrawText(text, 60.0f, 240.0f);
	CP_Settings_Fill(CP_Color_Create(255, 200, 60, 255));
	CP_Graphics_DrawRect(60.0f, 260.0f, 400.0f * CP_Sound_GetGroupVolume(CP_SOUND_GROUP_SFX), 20.0f);

	CP_Settings_Fill(CP_Color_Create(200, 200, 200, 255));
	CP_Font_DrawText("1, 2, 3: coin, jump, hit", 60.0f, 380.0f);
	CP_Font_DrawText("M: pause or resume the music    S: stop or start it", 60.0f, 420.0f);
	CP_Font_DrawText("Up and Down: music volume", 60.0f, 460.0f);
	CP_Font_DrawText("Left and Right: effects volume", 60.0f, 500.0f);
}

void game_exit(void)
{
	// Stop everything, then free each sound that init loaded
	CP_Sound_StopAll();
	CP_Sound_Free(&music);
	CP_Sound_Free(&coinSound);
	CP_Sound_Free(&jumpSound);
	CP_Sound_Free(&hitSound);
}

int main(void)
{
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
