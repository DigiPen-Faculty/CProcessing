//---------------------------------------------------------
// file:	main.c
//
// Lesson 11: Build your own state manager.
// https://github.com/DigiPen-Faculty/CProcessing/wiki/Lesson-11-Your-Own-State-Manager
//
// This is the finished program from the lesson: a small catching game
// with a menu, the game, a pause, a "really quit?" box on top of the pause,
// and a game over. The states are not CProcessing's. A manager written in
// this file keeps a stack of "screens", with the same ideas as the engine's
// Push and Pop, in about sixty lines.
//
//   Menu        Enter plays
//   Playing     move the mouse to catch the stars. P pauses.
//   Paused      P keeps playing, Q asks about quitting
//   Quit box    Y goes to the menu, N goes back to the pause
//   Game over   Enter plays again
//
// The ideas:
//   a screen       init, update, draw and exit, in a struct
//   a stack        an array of pointers to screens, and how many there are
//   requests       Set, Push and Pop only ask. The change happens at the
//                  start of the next frame, as with the engine's.
//   update         only the top screen's update runs, so only the top gets
//                  input, and the ones under it are frozen
//   draw           every screen's draw runs, bottom first, so a pause is
//                  drawn over the game
//   the stack list the names of the screens are drawn in the corner: it
//                  is how you can see the stack working
//
// To run it, copy this file over the template's main.c.
//
// Try this:
// - Let a screen say whether the one under it keeps updating, so a box can
//   float over a game that carries on.
// - Give every screen a time since it started, and fade the pause in.
// - Write a Replace: take off the top screen and put another in its place.
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>
#include <string.h>

//---------------------------------------------------------
// THE MANAGER
//---------------------------------------------------------

// A screen is four functions and a name. Update changes the game, draw only
// draws it. init and exit can be NULL, for a screen that doesn't need them.
typedef struct Screen
{
	const char* name;
	CP_FunctionPtr init;
	CP_FunctionPtr update;
	CP_FunctionPtr draw;
	CP_FunctionPtr exit;
} Screen;

#define MAX_SCREENS 8

const Screen* stack[MAX_SCREENS]; // the screens, the bottom one first
int depth = 0;                    // how many are on the stack

// What was asked for this frame, and when it happens (the start of the next)
typedef enum Request
{
	REQUEST_NONE,
	REQUEST_SET,
	REQUEST_PUSH,
	REQUEST_POP
} Request;

Request request = REQUEST_NONE;
const Screen* requested = NULL;

// These only ask. The last one asked in a frame wins.
void Screens_Set(const Screen* screen)
{
	request = REQUEST_SET;
	requested = screen;
}

void Screens_Push(const Screen* screen)
{
	request = REQUEST_PUSH;
	requested = screen;
}

void Screens_Pop(void)
{
	request = REQUEST_POP;
	requested = NULL;
}

void Screens_Enter(const Screen* screen)
{
	stack[depth] = screen;
	++depth;
	if (screen->init)
	{
		screen->init();
	}
}

void Screens_Leave(void)
{
	const Screen* screen = stack[depth - 1];
	if (screen->exit)
	{
		screen->exit();
	}
	--depth;
}

// Do what was asked. This is the only place the stack changes.
void Screens_Apply(void)
{
	const Request asked = request;
	request = REQUEST_NONE;

	if (asked == REQUEST_SET)
	{
		// Clear the table: every screen ends, the top one first
		while (depth > 0)
		{
			Screens_Leave();
		}
		Screens_Enter(requested);
	}
	else if (asked == REQUEST_PUSH)
	{
		if (depth < MAX_SCREENS)
		{
			Screens_Enter(requested);
		}
		else
		{
			printf("Too many screens on the stack.\n");
		}
	}
	else if (asked == REQUEST_POP)
	{
		if (depth > 1)
		{
			Screens_Leave();
		}
		else
		{
			printf("Pop ignored: it would leave no screen.\n");
		}
	}
}

// The names of the screens, bottom to top, in the corner
void Screens_DrawList(void)
{
	char text[100] = "stack:";
	for (int i = 0; i < depth; ++i)
	{
		snprintf(text + strlen(text), sizeof(text) - strlen(text), " %s", stack[i]->name);
	}
	CP_Settings_TextSize(16.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BOTTOM);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 160));
	CP_Font_DrawText(text, 10.0f, CP_System_GetWindowHeight() - 8.0f);
}

// Once a frame: change the stack if it was asked to, update the top screen,
// and then draw every screen from the bottom up
void Screens_Update(void)
{
	Screens_Apply();

	const Screen* top = stack[depth - 1];
	if (top->update)
	{
		top->update();
	}

	for (int i = 0; i < depth; ++i)
	{
		// Each draw starts with the settings it was called with, and can
		// change them (fill, text size) without upsetting the next one
		CP_Settings_Save();
		stack[i]->draw();
		CP_Settings_Restore();
	}
	Screens_DrawList();
}

// When the program ends, the screens on the stack end, from the top down
void Screens_ExitAll(void)
{
	while (depth > 0)
	{
		Screens_Leave();
	}
}

//---------------------------------------------------------
// THE ENGINE'S ONE STATE. CProcessing still runs the program with one
// state. All it does is hand the frame to the manager.
//---------------------------------------------------------
void game_init(void);
void game_update(void);
void game_exit(void);

//---------------------------------------------------------
// THE SCREENS, declared first so that any can ask for any other
//---------------------------------------------------------
void menu_update(void);
void menu_draw(void);
void play_init(void);
void play_update(void);
void play_draw(void);
void pause_update(void);
void pause_draw(void);
void confirm_update(void);
void confirm_draw(void);
void over_update(void);
void over_draw(void);

const Screen MENU = { "menu", NULL, menu_update, menu_draw, NULL };
const Screen PLAY = { "play", play_init, play_update, play_draw, NULL };
const Screen PAUSE = { "pause", NULL, pause_update, pause_draw, NULL };
const Screen CONFIRM = { "confirm", NULL, confirm_update, confirm_draw, NULL };
const Screen OVER = { "over", NULL, over_update, over_draw, NULL };

//---------------------------------------------------------
// VALUES THE SCREENS SHARE
//---------------------------------------------------------
int score = 0;
int bestScore = 0;
int lives = 3;

CP_Color background;
CP_Color white;

void DrawCenteredText(const char* text, float y, float size)
{
	CP_Settings_TextSize(size);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Settings_Fill(white);
	CP_Font_DrawText(text, CP_System_GetWindowWidth() / 2.0f, y);
}

// A see-through dark cover over everything drawn so far
void DrawVeil(int alpha)
{
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(CP_Color_Create(0, 0, 0, alpha));
	CP_Graphics_DrawRect(0.0f, 0.0f, (float)CP_System_GetWindowWidth(), (float)CP_System_GetWindowHeight());
}

// A panel in the middle of the window
void DrawPanel(float width, float height)
{
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CENTER);
	CP_Settings_Fill(CP_Color_Create(40, 40, 80, 255));
	CP_Graphics_DrawRect(CP_System_GetWindowWidth() / 2.0f, CP_System_GetWindowHeight() / 2.0f, width, height);
}

//---------------------------------------------------------
// THE GAME'S DATA: stars fall, and a basket under the mouse catches them
//---------------------------------------------------------
#define STAR_COUNT   6
#define BASKET_Y     540.0f
#define BASKET_WIDTH 110.0f

typedef struct Star
{
	CP_BOOL active;
	CP_Vector position;
	float speed;
} Star;

Star stars[STAR_COUNT];
float spawnTimer;
float basketX;

void SpawnStar(void)
{
	for (int i = 0; i < STAR_COUNT; ++i)
	{
		if (!stars[i].active)
		{
			stars[i].active = TRUE;
			stars[i].position = CP_Vector_Set(CP_Random_RangeFloat(40.0f, 760.0f), -20.0f);
			stars[i].speed = CP_Random_RangeFloat(120.0f, 220.0f) + score * 4.0f;
			return;
		}
	}
}

//---------------------------------------------------------
// MENU
//---------------------------------------------------------
void menu_update(void)
{
	if (CP_Input_KeyTriggered(CP_KEY_ENTER))
	{
		Screens_Set(&PLAY);
	}
}

void menu_draw(void)
{
	CP_Graphics_ClearBackground(background);
	DrawCenteredText("CATCH", 200.0f, 110.0f);
	DrawCenteredText("A game with a state manager of its own", 310.0f, 26.0f);
	DrawCenteredText("Press Enter to play", 400.0f, 24.0f);
	if (bestScore > 0)
	{
		char text[60];
		snprintf(text, sizeof(text), "Best score: %d", bestScore);
		DrawCenteredText(text, 470.0f, 24.0f);
	}
}

//---------------------------------------------------------
// PLAY. Update changes the game, and draw only draws it.
//---------------------------------------------------------
void play_init(void)
{
	// Every value the game changes is set here, as in lesson 9. Coming back
	// from the pause doesn't run it: the pause was pushed, not set.
	for (int i = 0; i < STAR_COUNT; ++i)
	{
		stars[i].active = FALSE;
	}
	score = 0;
	lives = 3;
	spawnTimer = 0.0f;
	basketX = CP_Input_GetMouseX();
}

void play_update(void)
{
	const float dt = CP_System_GetDt();

	if (CP_Input_KeyTriggered(CP_KEY_P))
	{
		Screens_Push(&PAUSE);
	}

	basketX = CP_Math_ClampFloat(CP_Input_GetMouseX(), BASKET_WIDTH / 2.0f, CP_System_GetWindowWidth() - BASKET_WIDTH / 2.0f);

	spawnTimer -= dt;
	if (spawnTimer <= 0.0f)
	{
		SpawnStar();
		spawnTimer = CP_Math_ClampFloat(1.1f - score * 0.03f, 0.35f, 1.1f);
	}

	for (int i = 0; i < STAR_COUNT; ++i)
	{
		if (!stars[i].active)
		{
			continue;
		}
		stars[i].position.y += stars[i].speed * dt;

		const float y = stars[i].position.y;
		const float across = stars[i].position.x - basketX;
		const CP_BOOL inBasket = across > -(BASKET_WIDTH / 2.0f + 10.0f) && across < BASKET_WIDTH / 2.0f + 10.0f;
		if (y > BASKET_Y - 20.0f && y < BASKET_Y + 10.0f && inBasket)
		{
			stars[i].active = FALSE;
			++score;
		}
		else if (y > CP_System_GetWindowHeight() + 20.0f)
		{
			stars[i].active = FALSE;
			--lives;
		}
	}

	if (lives <= 0)
	{
		if (score > bestScore)
		{
			bestScore = score;
		}
		Screens_Set(&OVER);
	}
}

void play_draw(void)
{
	CP_Graphics_ClearBackground(background);

	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(255, 215, 70, 255));
	for (int i = 0; i < STAR_COUNT; ++i)
	{
		if (stars[i].active)
		{
			CP_Graphics_DrawCircle(stars[i].position.x, stars[i].position.y, 28.0f);
		}
	}

	CP_Settings_RectMode(CP_POSITION_CENTER);
	CP_Settings_Fill(CP_Color_Create(90, 200, 255, 255));
	CP_Graphics_DrawRect(basketX, BASKET_Y + 14.0f, BASKET_WIDTH, 22.0f);

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
// PAUSE. It draws a cover over play's picture, which is drawn first every
// frame. Play's update doesn't run, so nothing moves.
//---------------------------------------------------------
void pause_update(void)
{
	if (CP_Input_KeyTriggered(CP_KEY_P))
	{
		Screens_Pop();
	}
	if (CP_Input_KeyTriggered(CP_KEY_Q))
	{
		Screens_Push(&CONFIRM);
	}
}

void pause_draw(void)
{
	DrawVeil(150);
	DrawCenteredText("PAUSED", 240.0f, 80.0f);
	DrawCenteredText("P: keep playing", 320.0f, 26.0f);
	DrawCenteredText("Q: quit to the menu", 360.0f, 26.0f);
}

//---------------------------------------------------------
// CONFIRM: a box on top of the pause, which is on top of the game
//---------------------------------------------------------
void confirm_update(void)
{
	if (CP_Input_KeyTriggered(CP_KEY_Y))
	{
		Screens_Set(&MENU);
	}
	if (CP_Input_KeyTriggered(CP_KEY_N))
	{
		Screens_Pop();
	}
}

void confirm_draw(void)
{
	DrawPanel(440.0f, 280.0f);
	DrawCenteredText("Quit this game?", 255.0f, 44.0f);
	DrawCenteredText("Y: yes     N: no", 345.0f, 28.0f);
}

//---------------------------------------------------------
// GAME OVER
//---------------------------------------------------------
void over_update(void)
{
	if (CP_Input_KeyTriggered(CP_KEY_ENTER))
	{
		Screens_Set(&PLAY);
	}
	if (CP_Input_KeyTriggered(CP_KEY_M))
	{
		Screens_Set(&MENU);
	}
}

void over_draw(void)
{
	char text[60];
	CP_Graphics_ClearBackground(background);
	DrawCenteredText("GAME OVER", 200.0f, 90.0f);
	snprintf(text, sizeof(text), "Score: %d     Best: %d", score, bestScore);
	DrawCenteredText(text, 310.0f, 30.0f);
	DrawCenteredText("Enter: play again     M: menu", 400.0f, 24.0f);
}

//---------------------------------------------------------
// The engine's one state
//---------------------------------------------------------
void game_init(void)
{
	Screens_Set(&MENU);
}

void game_update(void)
{
	Screens_Update();
}

void game_exit(void)
{
	Screens_ExitAll();
}

int main(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Lesson 11: Your own state manager");
	background = CP_Color_Create(30, 30, 50, 255);
	white = CP_Color_Create(255, 255, 255, 255);

	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
