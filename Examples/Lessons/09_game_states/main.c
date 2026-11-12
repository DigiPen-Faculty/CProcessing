//---------------------------------------------------------
// file:	main.c
//
// Lesson 9: Game states.
// https://github.com/DigiPen-Faculty/CProcessing/wiki/Lesson-09-Game-States
//
// This is the finished program from the lesson: a game called Pop! with a
// menu, the game, a pause and a game over. Bubbles grow, and you click them
// before they burst.
//
//   Menu        click or Enter to play
//   Playing     click a bubble to pop it (bigger bubbles score more)
//               P pauses, R starts over
//   Paused      P keeps playing, R starts over, M goes to the menu
//   Game over   Enter plays again, M goes to the menu
//
// The ideas:
//   a state        three functions: init (once, on the way in), update (every
//                  frame) and exit (once, on the way out)
//   switching      CP_Engine_SetNextGameState, and the Forced version to
//                  restart the state you're already in
//   shared values  global variables outlive every state (the best score)
//   one drawing    play and pause both call DrawGame, so pause shows the
//                  frozen game
//
// To run it, copy this file over the template's main.c.
//
// Try this:
// - Add a "How to play" state, reached from the menu with H.
// - Give each bubble its own growth speed, and score the fast ones more.
// - Make the bubbles grow faster the longer the game lasts.
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

// Each state is three functions. They're declared here, so that any state can
// switch to any other, whatever order they are written in below.
void menu_init(void);
void menu_update(void);
void menu_exit(void);
void play_init(void);
void play_update(void);
void play_exit(void);
void pause_init(void);
void pause_update(void);
void pause_exit(void);
void over_init(void);
void over_update(void);
void over_exit(void);

//---------------------------------------------------------
// VALUES EVERY STATE CAN USE. A global lives as long as the program, so it is
// how one state hands something to the next, like the score to game over.
//---------------------------------------------------------
int score = 0;
int bestScore = 0;
int lives = 3;

CP_Color background;
CP_Color white;

// Seconds since this state began. Each state's init sets it to 0, and the
// fade-in uses it.
float stateTime = 0.0f;

void DrawCenteredText(const char* text, float y, float size)
{
	CP_Settings_TextSize(size);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Settings_Fill(white);
	CP_Font_DrawText(text, CP_System_GetWindowWidth() / 2.0f, y);
}

// A black cover that fades away over the first half second of a state, so
// a switch is not a sudden jump (lesson 7's easing)
void DrawFadeIn(void)
{
	const float progress = CP_Math_ClampFloat(stateTime / 0.5f, 0.0f, 1.0f);
	if (progress < 1.0f)
	{
		CP_Settings_NoStroke();
		CP_Settings_Fill(CP_Color_Create(0, 0, 0, (int)(255.0f * (1.0f - CP_Math_EaseOutQuad(progress)))));
		CP_Graphics_DrawRect(CP_System_GetWindowWidth() / 2.0f, CP_System_GetWindowHeight() / 2.0f,
			(float)CP_System_GetWindowWidth(), (float)CP_System_GetWindowHeight());
	}
}

//---------------------------------------------------------
// THE GAME'S DATA
//---------------------------------------------------------
#define BUBBLE_COUNT 8

typedef struct Bubble
{
	CP_BOOL active;
	CP_Vector position;
	float radius;
} Bubble;

Bubble bubbles[BUBBLE_COUNT];
float spawnTimer;
float spawnGap;			// seconds between new bubbles, shorter as you go
const float burstRadius = 55.0f;
const float growSpeed = 22.0f;		// pixels of radius per second

// Set when we leave play for the pause, so play_init knows this is not a
// new game
CP_BOOL resuming = FALSE;

void SpawnBubble(void)
{
	for (int i = 0; i < BUBBLE_COUNT; i++)
	{
		if (!bubbles[i].active)
		{
			bubbles[i].active = TRUE;
			bubbles[i].position = CP_Vector_Set(CP_Random_RangeFloat(70.0f, 730.0f), CP_Random_RangeFloat(120.0f, 540.0f));
			bubbles[i].radius = 6.0f;
			return;
		}
	}
}

// Draws the game as it is now. Play and pause both use it: the pause state
// does not run the game, but it still shows it.
void DrawGame(void)
{
	CP_Graphics_ClearBackground(background);
	CP_Settings_NoStroke();
	for (int i = 0; i < BUBBLE_COUNT; i++)
	{
		if (!bubbles[i].active)
		{
			continue;
		}
		// Blue until about half grown, then redder the closer to bursting
		const float danger = CP_Math_ClampFloat((bubbles[i].radius / burstRadius - 0.5f) * 2.0f, 0.0f, 1.0f);
		CP_Settings_Fill(CP_Color_Lerp(CP_Color_Create(90, 200, 255, 255), CP_Color_Create(255, 60, 60, 255), danger));
		CP_Graphics_DrawCircle(bubbles[i].position.x, bubbles[i].position.y, bubbles[i].radius * 2.0f);
		CP_Settings_Fill(CP_Color_Create(255, 255, 255, 90));
		CP_Graphics_DrawCircle(bubbles[i].position.x - bubbles[i].radius * 0.35f,
			bubbles[i].position.y - bubbles[i].radius * 0.35f, bubbles[i].radius * 0.5f);
	}

	char text[60];
	CP_Settings_TextSize(24.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Settings_Fill(white);
	snprintf(text, sizeof(text), "Score: %d", score);
	CP_Font_DrawText(text, 20.0f, 30.0f);
	snprintf(text, sizeof(text), "Lives: %d", lives);
	CP_Font_DrawText(text, 680.0f, 30.0f);
}

//---------------------------------------------------------
// MENU STATE
//---------------------------------------------------------
void menu_init(void)
{
	stateTime = 0.0f;
}

void menu_update(void)
{
	stateTime += CP_System_GetDt();

	CP_Graphics_ClearBackground(background);
	DrawCenteredText("POP!", 190.0f, 110.0f);
	DrawCenteredText("Click the bubbles before they burst", 310.0f, 26.0f);
	DrawCenteredText("Click or press Enter to play", 400.0f, 24.0f);
	if (bestScore > 0)
	{
		char text[60];
		snprintf(text, sizeof(text), "Best score: %d", bestScore);
		DrawCenteredText(text, 470.0f, 24.0f);
	}
	DrawFadeIn();

	if (CP_Input_MouseTriggered(MOUSE_BUTTON_LEFT) || CP_Input_KeyTriggered(KEY_ENTER))
	{
		// The change happens at the start of the NEXT frame: menu_exit runs,
		// then play_init, then play_update every frame
		CP_Engine_SetNextGameState(play_init, play_update, play_exit);
	}
}

void menu_exit(void)
{
}

//---------------------------------------------------------
// PLAY STATE
//---------------------------------------------------------
void play_init(void)
{
	stateTime = 0.0f;

	// init runs every time we ENTER play, which includes coming back from the
	// pause. Then the game must carry on, not start again.
	if (resuming)
	{
		resuming = FALSE;
		return;
	}

	for (int i = 0; i < BUBBLE_COUNT; i++)
	{
		bubbles[i].active = FALSE;
	}
	score = 0;
	lives = 3;
	spawnGap = 1.4f;
	spawnTimer = 0.3f;
}

void play_update(void)
{
	const float dt = CP_System_GetDt();
	stateTime += dt;

	if (CP_Input_KeyTriggered(KEY_P))
	{
		resuming = TRUE;
		CP_Engine_SetNextGameState(pause_init, pause_update, pause_exit);
		return;		// this frame is over: nothing else may change the game
	}

	// R starts over. The state we want is the one we're already in, so the
	// plain SetNextGameState would do nothing. The Forced version runs
	// play_exit and play_init anyway.
	if (CP_Input_KeyTriggered(KEY_R))
	{
		resuming = FALSE;
		CP_Engine_SetNextGameStateForced(play_init, play_update, play_exit);
		return;
	}

	// New bubbles, a little faster as the score goes up
	spawnTimer -= dt;
	if (spawnTimer <= 0.0f)
	{
		SpawnBubble();
		spawnGap = CP_Math_ClampFloat(1.4f - score * 0.02f, 0.5f, 1.4f);
		spawnTimer = spawnGap;
	}

	const CP_Vector mouse = CP_Vector_Set(CP_Input_GetMouseX(), CP_Input_GetMouseY());
	for (int i = 0; i < BUBBLE_COUNT; i++)
	{
		if (!bubbles[i].active)
		{
			continue;
		}
		bubbles[i].radius += growSpeed * dt;

		// A click inside the bubble pops it. Bigger bubbles are worth more.
		if (CP_Input_MouseTriggered(MOUSE_BUTTON_LEFT) && CP_Vector_Distance(mouse, bubbles[i].position) <= bubbles[i].radius)
		{
			score += 1 + (int)(bubbles[i].radius / 10.0f);
			bubbles[i].active = FALSE;
		}
		else if (bubbles[i].radius >= burstRadius)
		{
			// Burst: it cost a life
			bubbles[i].active = FALSE;
			lives--;
		}
	}

	DrawGame();
	DrawFadeIn();

	if (lives <= 0)
	{
		CP_Engine_SetNextGameState(over_init, over_update, over_exit);
	}
}

void play_exit(void)
{
}

//---------------------------------------------------------
// PAUSE STATE. Nothing moves, because pause_update never changes the game.
//---------------------------------------------------------
void pause_init(void)
{
	stateTime = 0.0f;
}

void pause_update(void)
{
	stateTime += CP_System_GetDt();

	DrawGame();
	// A dark cover over the frozen game, then the words
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(0, 0, 0, 150));
	CP_Graphics_DrawRect(CP_System_GetWindowWidth() / 2.0f, CP_System_GetWindowHeight() / 2.0f,
		(float)CP_System_GetWindowWidth(), (float)CP_System_GetWindowHeight());
	DrawCenteredText("PAUSED", 230.0f, 80.0f);
	DrawCenteredText("P to keep playing", 320.0f, 26.0f);
	DrawCenteredText("R to start over, M for the menu", 360.0f, 26.0f);

	if (CP_Input_KeyTriggered(KEY_P))
	{
		// resuming is already TRUE, so play_init will carry on
		CP_Engine_SetNextGameState(play_init, play_update, play_exit);
	}
	if (CP_Input_KeyTriggered(KEY_R))
	{
		// A new game, so not resuming
		resuming = FALSE;
		CP_Engine_SetNextGameState(play_init, play_update, play_exit);
	}
	if (CP_Input_KeyTriggered(KEY_M))
	{
		resuming = FALSE;
		CP_Engine_SetNextGameState(menu_init, menu_update, menu_exit);
	}
}

void pause_exit(void)
{
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
	char text[60];
	DrawCenteredText("GAME OVER", 190.0f, 90.0f);
	snprintf(text, sizeof(text), "Score: %d", score);
	DrawCenteredText(text, 290.0f, 36.0f);
	if (newBest)
	{
		DrawCenteredText("A new best score!", 345.0f, 26.0f);
	}
	else
	{
		snprintf(text, sizeof(text), "Best score: %d", bestScore);
		DrawCenteredText(text, 345.0f, 26.0f);
	}
	DrawCenteredText("Enter to play again, M for the menu", 440.0f, 24.0f);
	DrawFadeIn();

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
	CP_System_SetWindowTitle("Lesson 9: Game states");
	background = CP_Color_Create(30, 30, 50, 255);
	white = CP_Color_Create(255, 255, 255, 255);

	// The game starts at the menu
	CP_Engine_SetNextGameState(menu_init, menu_update, menu_exit);
	CP_Engine_Run();
	return 0;
}
