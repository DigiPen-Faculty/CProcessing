//---------------------------------------------------------
// file:	main.c
//
// Lesson 8: Sound.
// https://github.com/DigiPen-Faculty/CProcessing/wiki/Lesson-08-Sound
//
// This is the finished program from the lesson: catch the falling coins, jump
// the falling rocks, and listen to what the game tells you.
//
//   A and D, or Left and Right   move
//   Space                        jump
//   M                            music on or off
//   1 and 2                      music volume down and up
//   3 and 4                      effects volume down and up
//
// Sound is used in four ways:
//   music           one long loop, streamed, in the music group
//   effects         short sounds, played at the moment something happens
//   pitch           a combo makes each coin chime higher; a hit dips the music
//   groups          music and effects each have their own volume
//
// To run it, copy this file over the template's main.c. It needs Music.wav,
// Coin.wav, Jump.wav and Hit.wav in the Assets folder, which the template
// includes.
//
// Try this:
// - Play a soft sound when a coin lands on the ground and is missed.
// - Make the music play faster as the combo grows (CP_Sound_SetGroupPitch).
// - Pause the music, with its instance, while the game is paused.
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

//---------------------------------------------------------
// SOUNDS. Loaded once, in init, and freed in exit.
//---------------------------------------------------------
CP_Sound music;
CP_Sound coinSound;
CP_Sound jumpSound;
CP_Sound hitSound;
CP_SoundInstance musicInstance = 0;		// this one play of the music

CP_BOOL musicOn = TRUE;
float musicVolume = 0.6f;				// the volume of each group, from 0 to 1
float effectsVolume = 0.8f;
float musicDip = 0.0f;					// 1 right after a hit, easing back to 0
const float dipTime = 0.8f;				// seconds for the music to recover

// Play an effect in the effects group, at a pitch near 1 so repeats of the
// same sound don't sound exactly alike
void PlayEffect(CP_Sound sound)
{
	CP_Sound_PlayAdvanced(sound, 1.0f, CP_Random_RangeFloat(0.95f, 1.05f), FALSE, CP_SOUND_GROUP_SFX);
}

//---------------------------------------------------------
// THE GAME: things fall; coins are good, rocks are not.
//---------------------------------------------------------
#define MAX_THINGS 12

typedef struct Thing
{
	CP_BOOL active;
	CP_BOOL isRock;
	CP_Vector position;
	float speed;			// pixels per second, downward
} Thing;

Thing things[MAX_THINGS];
float spawnTimer = 0.0f;

const float groundY = 520.0f;
const float playerRadius = 22.0f;
const float thingRadius = 14.0f;
const float playerSpeed = 300.0f;
const float gravity = 1800.0f;
const float jumpSpeed = 700.0f;

CP_Vector player;
float playerVelocityY = 0.0f;
CP_BOOL onGround = TRUE;

int score = 0;
int combo = 0;			// coins caught in a row
int bestCombo = 0;
int hits = 0;

void Spawn(void)
{
	for (int i = 0; i < MAX_THINGS; i++)
	{
		if (!things[i].active)
		{
			things[i].active = TRUE;
			things[i].isRock = CP_Random_RangeInt(0, 3) == 0;	// about one in four
			things[i].position = CP_Vector_Set(CP_Random_RangeFloat(40.0f, 760.0f), -20.0f);
			things[i].speed = CP_Random_RangeFloat(170.0f, 260.0f);
			return;
		}
	}
}

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Lesson 8: Sound");
	CP_Settings_TextSize(20.0f);

	// Long music is STREAMED: read from the file a little at a time as it
	// plays. Short effects are loaded whole, so they start at once.
	music = CP_Sound_LoadStream("Assets/Music.wav");
	coinSound = CP_Sound_Load("Assets/Coin.wav");
	jumpSound = CP_Sound_Load("Assets/Jump.wav");
	hitSound = CP_Sound_Load("Assets/Hit.wav");

	// Start the music once, looping, in the music group. Keep the instance
	// that comes back: it is this play of the music, to stop or pause later.
	musicInstance = CP_Sound_PlayAdvanced(music, 1.0f, 1.0f, TRUE, CP_SOUND_GROUP_MUSIC);
	CP_Sound_SetGroupVolume(CP_SOUND_GROUP_MUSIC, musicVolume);
	CP_Sound_SetGroupVolume(CP_SOUND_GROUP_SFX, effectsVolume);

	player = CP_Vector_Set(400.0f, groundY - playerRadius);
}

void game_update(void)
{
	const float dt = CP_System_GetDt();

	//-----------------------------------------------------
	// The sound controls. M pauses and resumes this play of the music, and
	// each group has a volume of its own.
	if (CP_Input_KeyTriggered(KEY_M))
	{
		musicOn = !musicOn;
		if (musicOn)
		{
			CP_Sound_Resume(musicInstance);
		}
		else
		{
			CP_Sound_Pause(musicInstance);
		}
	}
	if (CP_Input_KeyTriggered(KEY_1)) { musicVolume -= 0.1f; }
	if (CP_Input_KeyTriggered(KEY_2)) { musicVolume += 0.1f; }
	if (CP_Input_KeyTriggered(KEY_3)) { effectsVolume -= 0.1f; }
	if (CP_Input_KeyTriggered(KEY_4)) { effectsVolume += 0.1f; }
	musicVolume = CP_Math_ClampFloat(musicVolume, 0.0f, 1.0f);
	effectsVolume = CP_Math_ClampFloat(effectsVolume, 0.0f, 1.0f);
	CP_Sound_SetGroupVolume(CP_SOUND_GROUP_MUSIC, musicVolume);
	CP_Sound_SetGroupVolume(CP_SOUND_GROUP_SFX, effectsVolume);

	// A hit dips the pitch of the whole music group, and it eases back up.
	// Pitch 1 is normal, 0.5 an octave lower and slower.
	if (musicDip > 0.0f)
	{
		musicDip = CP_Math_ClampFloat(musicDip - dt / dipTime, 0.0f, 1.0f);
	}
	CP_Sound_SetGroupPitch(CP_SOUND_GROUP_MUSIC, 1.0f - 0.35f * CP_Math_EaseOutQuad(musicDip));

	//-----------------------------------------------------
	// The player: across, and a jump with gravity
	float across = 0.0f;
	if (CP_Input_KeyDown(KEY_A) || CP_Input_KeyDown(KEY_LEFT))
	{
		across -= 1.0f;
	}
	if (CP_Input_KeyDown(KEY_D) || CP_Input_KeyDown(KEY_RIGHT))
	{
		across += 1.0f;
	}
	player.x = CP_Math_ClampFloat(player.x + across * playerSpeed * dt, playerRadius, 800.0f - playerRadius);

	if (onGround && CP_Input_KeyTriggered(KEY_SPACE))
	{
		playerVelocityY = -jumpSpeed;
		onGround = FALSE;
		PlayEffect(jumpSound);			// at the moment of the jump, once
	}
	playerVelocityY += gravity * dt;
	player.y += playerVelocityY * dt;
	if (player.y >= groundY - playerRadius)
	{
		player.y = groundY - playerRadius;
		playerVelocityY = 0.0f;
		onGround = TRUE;
	}

	//-----------------------------------------------------
	// Things fall, and a coin or a rock touching the player is an EVENT: the
	// moment to play its sound, once.
	spawnTimer -= dt;
	if (spawnTimer <= 0.0f)
	{
		Spawn();
		spawnTimer = CP_Random_RangeFloat(0.35f, 0.8f);
	}

	for (int i = 0; i < MAX_THINGS; i++)
	{
		if (!things[i].active)
		{
			continue;
		}
		things[i].position.y += things[i].speed * dt;

		if (CP_Vector_Distance(things[i].position, player) <= playerRadius + thingRadius)
		{
			things[i].active = FALSE;
			if (things[i].isRock)
			{
				hits++;
				combo = 0;
				musicDip = 1.0f;
				PlayEffect(hitSound);
			}
			else
			{
				score++;
				combo++;
				if (combo > bestCombo)
				{
					bestCombo = combo;
				}
				// Each coin in a row is a little higher: up a notch for
				// every coin, until the tenth, so a combo sounds like a run
				const float pitch = 1.0f + 0.07f * (float)((combo > 10 ? 10 : combo) - 1);
				CP_Sound_PlayAdvanced(coinSound, 1.0f, pitch, FALSE, CP_SOUND_GROUP_SFX);
			}
		}
		else if (things[i].position.y > groundY + thingRadius)
		{
			// On the ground: a missed coin breaks the combo
			if (!things[i].isRock)
			{
				combo = 0;
			}
			things[i].active = FALSE;
		}
	}

	//-----------------------------------------------------
	// Draw
	CP_Graphics_ClearBackground(CP_Color_Create(30, 30, 50, 255));
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(45, 60, 70, 255));
	CP_Graphics_DrawRect(400.0f, (groundY + 600.0f) / 2.0f, 800.0f, 600.0f - groundY);

	for (int i = 0; i < MAX_THINGS; i++)
	{
		if (!things[i].active)
		{
			continue;
		}
		CP_Settings_Fill(things[i].isRock ? CP_Color_Create(235, 80, 80, 255) : CP_Color_Create(250, 210, 60, 255));
		CP_Graphics_DrawCircle(things[i].position.x, things[i].position.y, thingRadius * 2.0f);
	}

	CP_Settings_Fill(CP_Color_Create(80, 200, 255, 255));
	CP_Graphics_DrawCircle(player.x, player.y, playerRadius * 2.0f);

	// The sound panel: a bar for each group's volume
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Font_DrawText(musicOn ? "Music (1, 2, M)" : "Music: paused (M)", 20.0f, 30.0f);
	CP_Font_DrawText("Effects (3, 4)", 20.0f, 70.0f);
	CP_Settings_Fill(CP_Color_Create(70, 70, 100, 255));
	CP_Graphics_DrawRect(190.0f, 16.0f, 150.0f, 16.0f);
	CP_Graphics_DrawRect(190.0f, 56.0f, 150.0f, 16.0f);
	CP_Settings_Fill(musicOn ? CP_Color_Create(80, 200, 255, 255) : CP_Color_Create(120, 120, 140, 255));
	CP_Graphics_DrawRect(190.0f, 16.0f, 150.0f * musicVolume, 16.0f);
	CP_Settings_Fill(CP_Color_Create(255, 200, 60, 255));
	CP_Graphics_DrawRect(190.0f, 56.0f, 150.0f * effectsVolume, 16.0f);
	CP_Settings_RectMode(CP_POSITION_CENTER);

	char text[80];
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	snprintf(text, sizeof(text), "Coins: %d    Combo: %d (best %d)    Hits: %d", score, combo, bestCombo, hits);
	CP_Font_DrawText(text, 380.0f, 30.0f);
	snprintf(text, sizeof(text), "Music pitch: %.2f", CP_Sound_GetGroupPitch(CP_SOUND_GROUP_MUSIC));
	CP_Font_DrawText(text, 380.0f, 70.0f);
	CP_Settings_Fill(CP_Color_Create(200, 200, 200, 255));
	CP_Font_DrawText("Catch the gold, jump the red. A and D move, Space jumps.", 20.0f, 580.0f);
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
