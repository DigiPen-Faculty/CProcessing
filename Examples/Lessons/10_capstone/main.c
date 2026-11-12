//---------------------------------------------------------
// file:	main.c
//
// Lesson 10: Capstone. Slime Snacks.
// https://github.com/DigiPen-Faculty/CProcessing/wiki/Lesson-10-Capstone
//
// A small, finished game that uses everything in the lessons: you are a
// slime, collect the coins, and don't touch the red balls.
//
//   WASD or the arrow keys   move
//   P                        pause
//   Enter                    start, and play again
//
// Where each lesson is used:
//   1, 2, 3   shapes, coordinates and color: the arena, the HUD, the balls
//   4         input: movement keys, pause, the menu
//   5         vectors and dt: the slime and the balls move in pixels per second
//   6         collision: slime and coin, slime and ball, balls bounce
//   7         images: the animated slime; easing: a coin pops in, the score
//             text bumps
//   8         sound: music, a chime that rises with the combo, and a hit
//             that dips the music
//   9         game states: menu, play, game over
//
// To run it, copy this file over the template's main.c. It needs
// SlimeSheet.png, Music.wav, Coin.wav and Hit.wav in the Assets folder,
// which the template includes.
//
// Try this:
// - Add a power-up: a star that makes you safe for five seconds.
// - Give coins different values by color.
// - Add a second level: after 20 coins the arena gets a wall (lesson 6).
// - Save the best score to a file, so it is still there next time.
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

void menu_init(void);
void menu_update(void);
void menu_exit(void);
void play_init(void);
void play_update(void);
void play_exit(void);
void over_init(void);
void over_update(void);
void over_exit(void);

//---------------------------------------------------------
// SHARED BY EVERY STATE
//---------------------------------------------------------
#define FRAME_COUNT 8
const float frameSize = 64.0f;

int score = 0;
int bestScore = 0;
int coinsCollected = 0;

CP_Color background;
CP_Color white;

float stateTime = 0.0f;		// seconds since this state began

void DrawCenteredText(const char* text, float y, float size)
{
	CP_Settings_TextSize(size);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Settings_Fill(white);
	CP_Font_DrawText(text, CP_System_GetWindowWidth() / 2.0f, y);
}

// Draws frame number `frame` of the slime sheet, centered on a point. The
// slime faces left when `faceLeft` is true. `alpha` is how solid it is.
void DrawSlime(CP_Image sheet, CP_Vector position, int frame, CP_BOOL faceLeft, float size, int alpha)
{
	const float u0 = frame * frameSize;
	const float u1 = u0 + frameSize;
	if (faceLeft)
	{
		CP_Image_DrawSubImage(sheet, position.x, position.y, size, size, u1, 0.0f, u0, frameSize, alpha);
	}
	else
	{
		CP_Image_DrawSubImage(sheet, position.x, position.y, size, size, u0, 0.0f, u1, frameSize, alpha);
	}
}

//---------------------------------------------------------
// MENU STATE
//---------------------------------------------------------
CP_Image menuSheet;

void menu_init(void)
{
	stateTime = 0.0f;
	menuSheet = CP_Image_Load("Assets/SlimeSheet.png");
}

void menu_update(void)
{
	stateTime += CP_System_GetDt();

	CP_Graphics_ClearBackground(background);
	DrawCenteredText("SLIME SNACKS", 150.0f, 80.0f);

	// The slime hops on the menu, its frame chosen from the time: 10 frames
	// a second, round and round the 8 frames
	const int frame = (int)(stateTime * 10.0f) % FRAME_COUNT;
	DrawSlime(menuSheet, CP_Vector_Set(400.0f, 290.0f), frame, FALSE, 160.0f, 255);

	DrawCenteredText("Collect the coins. Keep away from the red balls.", 410.0f, 24.0f);
	DrawCenteredText("WASD or the arrow keys move you, P pauses", 445.0f, 24.0f);
	DrawCenteredText("Press Enter to play", 510.0f, 30.0f);
	if (bestScore > 0)
	{
		char text[60];
		snprintf(text, sizeof(text), "Best score: %d", bestScore);
		DrawCenteredText(text, 560.0f, 22.0f);
	}

	if (CP_Input_KeyTriggered(KEY_ENTER))
	{
		CP_Engine_SetNextGameState(play_init, play_update, play_exit);
	}
}

void menu_exit(void)
{
	// Whoever loads it, frees it
	CP_Image_Free(&menuSheet);
}

//---------------------------------------------------------
// PLAY STATE
//---------------------------------------------------------
CP_Image slimeSheet;
CP_Sound music;
CP_Sound coinSound;
CP_Sound hitSound;
CP_SoundInstance musicInstance = 0;

// The slime
const float slimeRadius = 24.0f;
const float slimeSpeed = 270.0f;
CP_Vector slimePosition;
CP_BOOL facingLeft;
int currentFrame;
float frameTimer;
int lives;
float safeTimer;			// seconds of safety left after a hit
const float safeTime = 1.6f;

// The coin
const float coinRadius = 14.0f;
CP_Vector coinPosition;
float coinAge;				// seconds since it appeared, for its pop in

// The balls
#define MAX_BALLS 7
const float ballRadius = 18.0f;
CP_Vector ballPositions[MAX_BALLS];
CP_Vector ballVelocities[MAX_BALLS];
int ballCount;

// The combo: coins taken in quick succession
int combo;
float comboTimer;
const float comboWindow = 2.0f;

float scorePop;				// 1 right after a coin, easing to 0
float musicDip;				// 1 right after a hit, easing to 0
CP_BOOL paused;

const float hudHeight = 60.0f;	// the arena is below the HUD

// A random place in the arena that is at least `distance` from `away`
CP_Vector RandomSpot(CP_Vector away, float distance, float margin)
{
	CP_Vector spot;
	do
	{
		spot = CP_Vector_Set(CP_Random_RangeFloat(margin, CP_System_GetWindowWidth() - margin),
			CP_Random_RangeFloat(hudHeight + margin, CP_System_GetWindowHeight() - margin));
	} while (CP_Vector_Distance(spot, away) < distance);
	return spot;
}

void PlaceCoin(void)
{
	coinPosition = RandomSpot(slimePosition, 160.0f, 40.0f);
	coinAge = 0.0f;
}

void AddBall(void)
{
	ballPositions[ballCount] = RandomSpot(slimePosition, 300.0f, ballRadius + 4.0f);
	// A random direction (an angle), at a random speed
	const CP_Vector direction = CP_Vector_FromAngle(CP_Random_RangeFloat(0.0f, 360.0f));
	ballVelocities[ballCount] = CP_Vector_Scale(direction, CP_Random_RangeFloat(140.0f, 220.0f));
	ballCount++;
}

void play_init(void)
{
	stateTime = 0.0f;

	// Load what the game needs. play_exit frees all of it.
	slimeSheet = CP_Image_Load("Assets/SlimeSheet.png");
	music = CP_Sound_LoadStream("Assets/Music.wav");
	coinSound = CP_Sound_Load("Assets/Coin.wav");
	hitSound = CP_Sound_Load("Assets/Hit.wav");
	musicInstance = CP_Sound_PlayAdvanced(music, 1.0f, 1.0f, TRUE, CP_SOUND_GROUP_MUSIC);
	CP_Sound_SetGroupVolume(CP_SOUND_GROUP_MUSIC, 0.5f);

	// Start fresh. Every value the game changes is set here, so playing
	// again, or restarting, always begins the same way.
	slimePosition = CP_Vector_Set(400.0f, 360.0f);
	facingLeft = FALSE;
	currentFrame = 0;
	frameTimer = 0.0f;
	lives = 3;
	safeTimer = 0.0f;
	score = 0;
	coinsCollected = 0;
	combo = 0;
	comboTimer = 0.0f;
	scorePop = 0.0f;
	musicDip = 0.0f;
	paused = FALSE;
	ballCount = 0;
	AddBall();
	PlaceCoin();
}

// The slime took the coin: score it, make the sound, and put out the next
void CollectCoin(void)
{
	coinsCollected++;
	combo++;
	comboTimer = comboWindow;

	// More points for a longer combo, but no more than 5
	score += combo < 5 ? combo : 5;
	scorePop = 1.0f;

	// The chime rises with the combo, up to the tenth coin in a row
	const float pitch = 1.0f + 0.07f * (float)((combo > 10 ? 10 : combo) - 1);
	CP_Sound_PlayAdvanced(coinSound, 1.0f, pitch, FALSE, CP_SOUND_GROUP_SFX);

	// A new ball every 4 coins
	if (coinsCollected % 4 == 0 && ballCount < MAX_BALLS)
	{
		AddBall();
	}
	PlaceCoin();
}

void play_update(void)
{
	const float dt = CP_System_GetDt();
	stateTime += dt;

	if (CP_Input_KeyTriggered(KEY_P))
	{
		paused = !paused;
		if (paused)
		{
			CP_Sound_Pause(musicInstance);
		}
		else
		{
			CP_Sound_Resume(musicInstance);
		}
	}

	if (!paused)
	{
		//-------------------------------------------------
		// The slime moves (lesson 5) and animates (lesson 7)
		CP_Vector direction = CP_Vector_Zero();
		if (CP_Input_KeyDown(KEY_A) || CP_Input_KeyDown(KEY_LEFT))
		{
			direction.x -= 1.0f;
		}
		if (CP_Input_KeyDown(KEY_D) || CP_Input_KeyDown(KEY_RIGHT))
		{
			direction.x += 1.0f;
		}
		if (CP_Input_KeyDown(KEY_W) || CP_Input_KeyDown(KEY_UP))
		{
			direction.y -= 1.0f;
		}
		if (CP_Input_KeyDown(KEY_S) || CP_Input_KeyDown(KEY_DOWN))
		{
			direction.y += 1.0f;
		}
		if (direction.x != 0.0f || direction.y != 0.0f)
		{
			direction = CP_Vector_Normalize(direction);
			slimePosition = CP_Vector_Add(slimePosition, CP_Vector_Scale(direction, slimeSpeed * dt));
			slimePosition.x = CP_Math_ClampFloat(slimePosition.x, slimeRadius, CP_System_GetWindowWidth() - slimeRadius);
			slimePosition.y = CP_Math_ClampFloat(slimePosition.y, hudHeight + slimeRadius, CP_System_GetWindowHeight() - slimeRadius);
			if (direction.x != 0.0f)
			{
				facingLeft = direction.x < 0.0f;
			}

			frameTimer += dt;
			while (frameTimer >= 1.0f / 12.0f)
			{
				frameTimer -= 1.0f / 12.0f;
				currentFrame = (currentFrame + 1) % FRAME_COUNT;
			}
		}
		else
		{
			currentFrame = 0;
			frameTimer = 0.0f;
		}

		//-------------------------------------------------
		// Timers that count down by dt
		coinAge += dt;
		if (safeTimer > 0.0f)
		{
			safeTimer -= dt;
		}
		if (comboTimer > 0.0f)
		{
			comboTimer -= dt;
			if (comboTimer <= 0.0f)
			{
				combo = 0;
			}
		}
		scorePop = CP_Math_ClampFloat(scorePop - dt / 0.35f, 0.0f, 1.0f);
		musicDip = CP_Math_ClampFloat(musicDip - dt / 0.9f, 0.0f, 1.0f);
		CP_Sound_SetGroupPitch(CP_SOUND_GROUP_MUSIC, 1.0f - 0.35f * CP_Math_EaseOutQuad(musicDip));

		//-------------------------------------------------
		// Collision (lesson 6): the coin, then the balls
		if (CP_Vector_Distance(slimePosition, coinPosition) <= slimeRadius + coinRadius)
		{
			CollectCoin();
		}

		for (int i = 0; i < ballCount; i++)
		{
			ballPositions[i] = CP_Vector_Add(ballPositions[i], CP_Vector_Scale(ballVelocities[i], dt));

			// Bounce off the edges of the arena: turn round across or down
			if (ballPositions[i].x < ballRadius || ballPositions[i].x > CP_System_GetWindowWidth() - ballRadius)
			{
				ballPositions[i].x = CP_Math_ClampFloat(ballPositions[i].x, ballRadius, CP_System_GetWindowWidth() - ballRadius);
				ballVelocities[i].x = -ballVelocities[i].x;
			}
			if (ballPositions[i].y < hudHeight + ballRadius || ballPositions[i].y > CP_System_GetWindowHeight() - ballRadius)
			{
				ballPositions[i].y = CP_Math_ClampFloat(ballPositions[i].y, hudHeight + ballRadius, CP_System_GetWindowHeight() - ballRadius);
				ballVelocities[i].y = -ballVelocities[i].y;
			}

			// A hit costs a life, unless the slime is still safe from the
			// last one
			if (safeTimer <= 0.0f && CP_Vector_Distance(slimePosition, ballPositions[i]) <= slimeRadius + ballRadius)
			{
				lives--;
				safeTimer = safeTime;
				combo = 0;
				musicDip = 1.0f;
				CP_Sound_PlayAdvanced(hitSound, 1.0f, CP_Random_RangeFloat(0.95f, 1.05f), FALSE, CP_SOUND_GROUP_SFX);
			}
		}
	}

	//-----------------------------------------------------
	// Draw
	CP_Graphics_ClearBackground(background);
	CP_Settings_NoStroke();

	// The arena
	CP_Settings_Fill(CP_Color_Create(40, 52, 66, 255));
	CP_Graphics_DrawRect(CP_System_GetWindowWidth() / 2.0f, (hudHeight + CP_System_GetWindowHeight()) / 2.0f,
		(float)CP_System_GetWindowWidth(), CP_System_GetWindowHeight() - hudHeight);

	// The coin pops in: it grows to full size with a bounce
	const float coinScale = CP_Math_EaseOutBounce(coinAge / 0.5f);
	CP_Settings_Fill(CP_Color_Create(250, 210, 60, 255));
	CP_Graphics_DrawCircle(coinPosition.x, coinPosition.y, coinRadius * 2.0f * coinScale);
	CP_Settings_Fill(CP_Color_Create(255, 245, 190, 255));
	CP_Graphics_DrawCircle(coinPosition.x - 4.0f, coinPosition.y - 4.0f, coinRadius * 0.6f * coinScale);

	CP_Settings_Fill(CP_Color_Create(235, 80, 80, 255));
	for (int i = 0; i < ballCount; i++)
	{
		CP_Graphics_DrawCircle(ballPositions[i].x, ballPositions[i].y, ballRadius * 2.0f);
	}

	// The slime blinks while it is safe after a hit
	const int slimeAlpha = (safeTimer > 0.0f && (int)(safeTimer * 12.0f) % 2 == 0) ? 90 : 255;
	DrawSlime(slimeSheet, CP_Vector_Set(slimePosition.x, slimePosition.y - 18.0f), currentFrame, facingLeft, 84.0f, slimeAlpha);

	// The HUD: the score text is bumped bigger after a coin
	CP_Settings_Fill(CP_Color_Create(24, 24, 40, 255));
	CP_Graphics_DrawRect(CP_System_GetWindowWidth() / 2.0f, hudHeight / 2.0f, (float)CP_System_GetWindowWidth(), hudHeight);
	char text[60];
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Settings_Fill(white);
	CP_Settings_TextSize(26.0f + 12.0f * CP_Math_EaseOutQuad(scorePop));
	snprintf(text, sizeof(text), "Score: %d", score);
	CP_Font_DrawText(text, 20.0f, hudHeight / 2.0f);
	CP_Settings_TextSize(24.0f);
	if (combo > 1)
	{
		snprintf(text, sizeof(text), "Combo x%d", combo);
		CP_Settings_Fill(CP_Color_Create(250, 210, 60, 255));
		CP_Font_DrawText(text, 300.0f, hudHeight / 2.0f);
	}
	// A heart for each life left
	CP_Settings_Fill(CP_Color_Create(235, 80, 80, 255));
	for (int i = 0; i < lives; i++)
	{
		CP_Graphics_DrawCircle(700.0f + i * 30.0f, hudHeight / 2.0f, 20.0f);
	}

	if (paused)
	{
		CP_Settings_Fill(CP_Color_Create(0, 0, 0, 150));
		CP_Graphics_DrawRect(CP_System_GetWindowWidth() / 2.0f, CP_System_GetWindowHeight() / 2.0f,
			(float)CP_System_GetWindowWidth(), (float)CP_System_GetWindowHeight());
		DrawCenteredText("PAUSED", 270.0f, 80.0f);
		DrawCenteredText("P to keep playing", 350.0f, 26.0f);
	}

	if (lives <= 0)
	{
		CP_Engine_SetNextGameState(over_init, over_update, over_exit);
	}
}

void play_exit(void)
{
	CP_Sound_StopAll();
	CP_Sound_SetGroupPitch(CP_SOUND_GROUP_MUSIC, 1.0f);
	CP_Image_Free(&slimeSheet);
	CP_Sound_Free(&music);
	CP_Sound_Free(&coinSound);
	CP_Sound_Free(&hitSound);
}

//---------------------------------------------------------
// GAME OVER STATE
//---------------------------------------------------------
CP_BOOL newBest;

void over_init(void)
{
	stateTime = 0.0f;
	newBest = score > bestScore;
	if (newBest)
	{
		bestScore = score;
	}
}

void over_update(void)
{
	stateTime += CP_System_GetDt();

	CP_Graphics_ClearBackground(background);

	// The title drops in from above and bounces to a stop
	const float drop = CP_Math_EaseOutBounce(stateTime / 0.9f);
	DrawCenteredText("GAME OVER", -60.0f + 250.0f * drop, 90.0f);

	char text[60];
	snprintf(text, sizeof(text), "Score: %d    Coins: %d", score, coinsCollected);
	DrawCenteredText(text, 320.0f, 34.0f);
	if (newBest)
	{
		DrawCenteredText("A new best score!", 375.0f, 26.0f);
	}
	else
	{
		snprintf(text, sizeof(text), "Best score: %d", bestScore);
		DrawCenteredText(text, 375.0f, 26.0f);
	}
	DrawCenteredText("Enter to play again, M for the menu", 470.0f, 24.0f);

	if (CP_Input_KeyTriggered(KEY_ENTER))
	{
		CP_Engine_SetNextGameState(play_init, play_update, play_exit);
	}
	if (CP_Input_KeyTriggered(KEY_M))
	{
		CP_Engine_SetNextGameState(menu_init, menu_update, menu_exit);
	}
}

void over_exit(void)
{
}

int main(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Lesson 10: Slime Snacks");
	background = CP_Color_Create(30, 30, 50, 255);
	white = CP_Color_Create(255, 255, 255, 255);

	CP_Engine_SetNextGameState(menu_init, menu_update, menu_exit);
	CP_Engine_Run();
	return 0;
}
