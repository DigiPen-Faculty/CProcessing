//---------------------------------------------------------
// file:	state_effects.c
//
// Example: effects around game states, written with ordinary CProcessing
// functions. The engine gives you the state stack (CP_Engine_PushGameState and
// CP_Engine_PopGameState); slow motion and turning the sound down are a few
// lines of your own code, and you can change them however you like.
//
// Shows how to:
// - slow the game down with your own time scale: the game multiplies
//   CP_System_GetDt by timeScale, so changing one number slows everything
// - ease into slow motion and back out of it when a level-up box opens and
//   closes (CP_Math_LerpFloat with an easing function)
// - let the game keep running under the level-up box, in slow motion, with
//   CP_Engine_SetGameStatePassThrough(CP_STATE_PASSIVE)
// - slow the sound down with the time (CP_Sound_SetGroupPitch)
// - turn the music down under the pause menu, instead of stopping it
//   (CP_Sound_SetGroupVolume), and put it back when the pause ends
// - show the frozen game through frosted glass under the pause: a screenshot
//   in the pause's init is the picture below it, and drawing that small and
//   then stretching it back up blurs it (or pixelates it), without shaders
//
// Keys: the arrow keys move, P pauses. Every 5 coins you level up: pick an
// upgrade with 1 or 2. In the pause, B switches between blurred and
// pixelated.
//
// Try this:
// - Make the slow motion slower or quicker to start (LEVEL_UP_SLOWEST,
//   EASE_SECONDS).
// - Ease the music back up after the pause, in the game's update, instead of
//   setting it back at once.
// - Make the glass more or less frosted (FROST_STEPS), or tint it.
//
// To run it, copy this file over the template's main.c. It needs Music.wav,
// Coin.wav and Jump.wav in the Assets folder, which the template includes.
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

void game_init(void);
void game_update(void);
void game_exit(void);
void pause_init(void);
void pause_update(void);
void pause_exit(void);
void level_up_init(void);
void level_up_update(void);

#define EASE_SECONDS     0.4f  // how long easing in or out takes
#define LEVEL_UP_SLOWEST 0.15f // the time scale under the level-up box
#define PAUSE_VOLUME     0.4f  // the music volume under the pause
#define FROST_STEPS      3     // how many times the paused game is halved: more is blurrier

CP_Color white;
CP_Sound music;
CP_Sound coinSound;
CP_Sound levelSound;

// How fast time goes in the game: 1 is normal, 0.5 is half speed. Only this
// program's code uses it; the engine doesn't know about it.
float timeScale = 1.0f;

// Go from `from` to `to` over `seconds`, easing out: quick at first, then
// slowing as it arrives. `time` is how long it has been going.
float EaseTo(float from, float to, float time, float seconds)
{
	const float progress = CP_Math_ClampFloat(time / seconds, 0.0f, 1.0f);
	return CP_Math_LerpFloat(from, to, CP_Math_EaseOutCubic(progress));
}

// The time scale changes the sound too: slower time, lower sounds. (A pitch
// as low as the time scale sounds too strange, so it only goes half as far.)
void SetTimeScale(float scale)
{
	timeScale = scale;
	const float pitch = CP_Math_LerpFloat(0.5f, 1.0f, scale);
	CP_Sound_SetGroupPitch(CP_SOUND_GROUP_MUSIC, pitch);
	CP_Sound_SetGroupPitch(CP_SOUND_GROUP_SFX, pitch);
}

void DrawCenteredText(const char* text, float y, float size)
{
	CP_Settings_TextSize(size);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Settings_Fill(white);
	CP_Font_DrawText(text, CP_System_GetWindowWidth() / 2.0f, y);
}

void DrawPanel(float width, float height)
{
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CENTER);
	CP_Settings_Fill(CP_Color_Create(40, 40, 70, 230));
	CP_Graphics_DrawRect(CP_System_GetWindowWidth() / 2.0f, CP_System_GetWindowHeight() / 2.0f, width, height);
}

//-------------------------
// GAME: collect the coins that drift around

#define COIN_COUNT 8
CP_Vector player;
CP_Vector coins[COIN_COUNT];
CP_Vector coinVelocities[COIN_COUNT];
int coinsCollected;
int level;
float playerSpeed;
float reach; // how close a coin has to be to collect it

void PlaceCoin(int i)
{
	coins[i] = CP_Vector_Set(CP_Random_RangeFloat(30.0f, 770.0f), CP_Random_RangeFloat(30.0f, 570.0f));
	coinVelocities[i] = CP_Vector_Scale(CP_Vector_FromAngle(CP_Random_RangeFloat(0.0f, 360.0f)), 120.0f);
}

void game_init(void)
{
	player = CP_Vector_Set(400.0f, 300.0f);
	for (int i = 0; i < COIN_COUNT; ++i)
	{
		PlaceCoin(i);
	}
	coinsCollected = 0;
	level = 1;
	playerSpeed = 220.0f;
	reach = 30.0f;
	SetTimeScale(1.0f);

	music = CP_Sound_LoadStream("Assets/Music.wav");
	coinSound = CP_Sound_Load("Assets/Coin.wav");
	levelSound = CP_Sound_Load("Assets/Jump.wav");
	CP_Sound_PlayAdvanced(music, 1.0f, 1.0f, TRUE, CP_SOUND_GROUP_MUSIC);
	CP_Sound_SetGroupVolume(CP_SOUND_GROUP_MUSIC, 1.0f);
}

void DrawHUD(void)
{
	char text[60];
	snprintf(text, sizeof(text), "Level %d   Coins %d   Time x%.2f", level, coinsCollected, timeScale);
	CP_Settings_Fill(white);
	CP_Settings_TextSize(20.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
	CP_Font_DrawText(text, 10.0f, 10.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_RIGHT, CP_TEXT_ALIGN_V_TOP);
	CP_Font_DrawText("Arrows: move   P: pause", CP_System_GetWindowWidth() - 10.0f, 10.0f);
}

void game_update(void)
{
	// Everything that moves uses dt, so the time scale slows all of it
	const float dt = CP_System_GetDt() * timeScale;

	// (Under the level-up box, keys read as not pressed: only the top state
	// gets input)
	if (CP_Input_KeyTriggered(KEY_P))
	{
		CP_Engine_PushGameState(pause_init, pause_update, pause_exit);
	}
	CP_Vector move = CP_Vector_Zero();
	if (CP_Input_KeyDown(KEY_LEFT)) move.x -= 1.0f;
	if (CP_Input_KeyDown(KEY_RIGHT)) move.x += 1.0f;
	if (CP_Input_KeyDown(KEY_UP)) move.y -= 1.0f;
	if (CP_Input_KeyDown(KEY_DOWN)) move.y += 1.0f;
	if (move.x != 0.0f || move.y != 0.0f)
	{
		player = CP_Vector_Add(player, CP_Vector_Scale(CP_Vector_Normalize(move), playerSpeed * dt));
	}

	for (int i = 0; i < COIN_COUNT; ++i)
	{
		coins[i] = CP_Vector_Add(coins[i], CP_Vector_Scale(coinVelocities[i], dt));
		if (coins[i].x < 10.0f || coins[i].x > CP_System_GetWindowWidth() - 10.0f) coinVelocities[i].x = -coinVelocities[i].x;
		if (coins[i].y < 10.0f || coins[i].y > CP_System_GetWindowHeight() - 10.0f) coinVelocities[i].y = -coinVelocities[i].y;

		if (CP_Vector_Distance(coins[i], player) < reach)
		{
			PlaceCoin(i);
			++coinsCollected;
			CP_Sound_PlayAdvanced(coinSound, 0.6f, 1.0f, FALSE, CP_SOUND_GROUP_SFX);
			if (coinsCollected % 5 == 0)
			{
				CP_Engine_PushGameState(level_up_init, level_up_update, NULL);
			}
		}
	}

	CP_Graphics_ClearBackground(CP_Color_Create(25, 35, 55, 255));
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(255, 210, 60, 255));
	for (int i = 0; i < COIN_COUNT; ++i)
	{
		CP_Graphics_DrawCircle(coins[i].x, coins[i].y, 20.0f);
	}
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 40));
	CP_Graphics_DrawCircle(player.x, player.y, reach * 2.0f);
	CP_Settings_RectMode(CP_POSITION_CENTER);
	CP_Settings_Fill(white);
	CP_Graphics_DrawRect(player.x, player.y, 24.0f, 24.0f);

	DrawHUD();
}

void game_exit(void)
{
	CP_Sound_Free(&music);
	CP_Sound_Free(&coinSound);
	CP_Sound_Free(&levelSound);
}

//-------------------------
// PAUSE: the game freezes behind it (the default), the music gets quieter,
// and the game shows through frosted glass

CP_Image frosted;  // the frozen game, small: made once, when the pause starts
CP_BOOL pixelated; // B: stretch it back up blocky instead of smooth

// A copy of an image at half the size. With the linear filter, each pixel of
// the copy is the average of the 2 x 2 pixels it covers. (Halving a few times
// averages properly; shrinking 8 times in one go would only average a few of
// the 64 pixels each one covers, and look speckled.)
CP_Image Halve(CP_Image image)
{
	const int width = CP_Image_GetWidth(image) / 2;
	const int height = CP_Image_GetHeight(image) / 2;
	CP_Image half = CP_Image_Create(width, height);
	CP_Image_BeginDrawing(half);
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_LINEAR);
	CP_Image_Draw(image, 0.0f, 0.0f, (float)width, (float)height, 255);
	CP_Image_EndDrawing();
	return half;
}

void pause_init(void)
{
	// This state's layer starts clear, so a screenshot now is the picture
	// below it: the game. The game is frozen, so one picture is enough.
	frosted = CP_Image_Screenshot(0, 0, CP_System_GetWindowWidth(), CP_System_GetWindowHeight());
	for (int i = 0; i < FROST_STEPS; ++i)
	{
		CP_Image half = Halve(frosted);
		CP_Image_Free(&frosted);
		frosted = half;
	}
	pixelated = FALSE;
}

void pause_update(void)
{
	// Ease the music down over the first moments of the pause. The pause's
	// own time (CP_Engine_GetGameStateTime) starts at 0 when it is pushed.
	const float time = CP_Engine_GetGameStateTime();
	CP_Sound_SetGroupVolume(CP_SOUND_GROUP_MUSIC, EaseTo(1.0f, PAUSE_VOLUME, time, EASE_SECONDS));

	if (CP_Input_KeyTriggered(KEY_B))
	{
		pixelated = !pixelated;
	}

	// Stretch the small picture over the window: the linear filter blends
	// between its pixels (a blur), nearest keeps them as blocks. It fades in,
	// so the glass fogs up, and a little white makes it look frosted.
	CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 0));
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_Settings_ImageFilterMode(pixelated ? CP_IMAGE_FILTER_NEAREST : CP_IMAGE_FILTER_LINEAR);
	const int fog = (int)EaseTo(0.0f, 255.0f, time, EASE_SECONDS);
	CP_Image_Draw(frosted, 0.0f, 0.0f, (float)CP_System_GetWindowWidth(), (float)CP_System_GetWindowHeight(), fog);
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, fog / 8));
	CP_Graphics_DrawRect(0.0f, 0.0f, (float)CP_System_GetWindowWidth(), (float)CP_System_GetWindowHeight());

	DrawPanel(380.0f, 220.0f);
	DrawCenteredText("PAUSED", 260.0f, 56.0f);
	DrawCenteredText("P: keep playing", 320.0f, 24.0f);
	DrawCenteredText(pixelated ? "B: blurred" : "B: pixelated", 355.0f, 24.0f);

	if (CP_Input_KeyTriggered(KEY_P))
	{
		CP_Engine_PopGameState();
	}
}

void pause_exit(void)
{
	// Put the music back as the pause ends, and free the picture
	CP_Sound_SetGroupVolume(CP_SOUND_GROUP_MUSIC, 1.0f);
	CP_Image_Free(&frosted);
}

//-------------------------
// LEVEL UP: the game keeps running under it, in slow motion. When the
// player picks an upgrade, time speeds back up, and then the box goes.

CP_BOOL levelUpClosing;
float levelUpClosedAt; // the box's time when the upgrade was picked

void level_up_init(void)
{
	// The game under this box keeps running and drawing (without input)
	CP_Engine_SetGameStatePassThrough(CP_STATE_PASSIVE);
	levelUpClosing = FALSE;
	levelUpClosedAt = 0.0f;
	++level;
	CP_Sound_PlayAdvanced(levelSound, 0.8f, 1.0f, FALSE, CP_SOUND_GROUP_SFX);
}

void level_up_update(void)
{
	const float time = CP_Engine_GetGameStateTime();

	if (!levelUpClosing)
	{
		// Ease into slow motion
		SetTimeScale(EaseTo(1.0f, LEVEL_UP_SLOWEST, time, EASE_SECONDS));

		if (CP_Input_KeyTriggered(KEY_1))
		{
			playerSpeed += 60.0f;
			levelUpClosing = TRUE;
		}
		if (CP_Input_KeyTriggered(KEY_2))
		{
			reach += 12.0f;
			levelUpClosing = TRUE;
		}
		if (levelUpClosing)
		{
			levelUpClosedAt = time;
		}
	}
	else
	{
		// Ease back to full speed, then take the box off
		const float closing = time - levelUpClosedAt;
		SetTimeScale(EaseTo(LEVEL_UP_SLOWEST, 1.0f, closing, EASE_SECONDS));
		if (closing >= EASE_SECONDS)
		{
			SetTimeScale(1.0f);
			CP_Engine_PopGameState();
		}
	}

	// This state's layer starts clear, and the game shows through it. Clear
	// it with a clear color each frame, so the box can go away while time
	// is still easing back.
	CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 0));
	if (!levelUpClosing)
	{
		char text[40];
		snprintf(text, sizeof(text), "LEVEL %d!", level);
		DrawPanel(420.0f, 220.0f);
		DrawCenteredText(text, 230.0f, 56.0f);
		DrawCenteredText("1: move faster", 300.0f, 24.0f);
		DrawCenteredText("2: reach further", 340.0f, 24.0f);
	}
}

int main(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("State effects");
	white = CP_Color_Create(255, 255, 255, 255);

	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
