//---------------------------------------------------------
// file:	state_stack.c
//
// Example: game states on a stack, transitions, and drawing into an image.
//
// Shows how to:
// - push a state on top of the game (a pause menu) with
//   CP_Engine_PushGameState, and take it off with CP_Engine_PopGameState: the
//   game freezes behind it, its last picture stays visible, and it carries on
//   where it was, without its init running again
// - stack more on top: options over the pause, "really quit?" over that
// - push with CP_Engine_PushGameStateAdvanced and CP_BELOW_RUNNING, for a
//   hint that sits over a game that keeps moving (it gets no input), or
//   CP_BELOW_HIDDEN, for a full-screen options page
// - animate every change of state with CP_Engine_SetTransition, or with a
//   transition you draw yourself (CP_Engine_SetTransitionFunction)
// - draw into an image (CP_Image_Create, CP_Image_BeginDrawing) for a trail
//   that fades
//
// Keys: on the menu, Enter plays and T changes the transition.
// In the game: arrow keys move, P pauses, H shows a hint.
// The pause, options and quit screens list their own keys.
//
// Try this:
// - Push a "level complete" state when the player reaches a corner.
// - Write another transition function: a circle that closes, or a wipe.
//
// To run it, copy this file over the template's main.c.
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

void menu_init(void);
void menu_update(void);
void game_init(void);
void game_update(void);
void game_exit(void);
void pause_update(void);
void options_update(void);
void confirm_update(void);
void hint_update(void);

CP_Color white;

void DrawCenteredText(const char* text, float y, float size)
{
	CP_Settings_TextSize(size);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Settings_Fill(white);
	CP_Font_DrawText(text, CP_System_GetWindowWidth() / 2.0f, y);
}

// A box in the middle of the window, for the menus over the game
void DrawPanel(float width, float height)
{
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CENTER);
	CP_Settings_Fill(CP_Color_Create(40, 40, 70, 240));
	CP_Graphics_DrawRect(CP_System_GetWindowWidth() / 2.0f, CP_System_GetWindowHeight() / 2.0f, width, height);
}

// How deep the stack is, in a corner, so you can watch it change
void DrawDepth(void)
{
	char text[40];
	snprintf(text, sizeof(text), "States on the stack: %d", CP_Engine_GetGameStateDepth());
	CP_Settings_TextSize(18.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
	CP_Settings_Fill(white);
	CP_Font_DrawText(text, 10.0f, 10.0f);
}

//-------------------------
// TRANSITIONS

// A transition drawn by the program: the old picture splits down the middle
// and its halves slide apart, showing the new one behind
void SplitTransition(float progress, CP_Image from, CP_Image to)
{
	const float width = (float)CP_System_GetWindowWidth();
	const float height = (float)CP_System_GetWindowHeight();
	const float half = width / 2.0f;
	const float gap = half * CP_Math_EaseInCubic(progress);

	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_Image_Draw(to, 0.0f, 0.0f, width, height, 255);
	// the image coordinates are the window's: from (0, 0) to (width, height)
	CP_Image_DrawSubImage(from, -gap, 0.0f, half, height, 0.0f, 0.0f, half, height, 255);
	CP_Image_DrawSubImage(from, half + gap, 0.0f, half, height, half, 0.0f, width, height, 255);
}

#define TRANSITION_CHOICES 8
const char* transitionNames[TRANSITION_CHOICES] = {
	"none", "fade", "crossfade", "slide left", "slide right", "slide up", "slide down", "split (drawn by this program)"
};
int transitionChoice = 2;

void ApplyTransitionChoice(void)
{
	const float seconds = 0.6f;
	switch (transitionChoice)
	{
	case 0: CP_Engine_SetTransition(CP_TRANSITION_NONE, 0.0f); break;
	case 1: CP_Engine_SetTransition(CP_TRANSITION_FADE, seconds); break;
	case 2: CP_Engine_SetTransition(CP_TRANSITION_CROSSFADE, seconds); break;
	case 3: CP_Engine_SetTransition(CP_TRANSITION_SLIDE_LEFT, seconds); break;
	case 4: CP_Engine_SetTransition(CP_TRANSITION_SLIDE_RIGHT, seconds); break;
	case 5: CP_Engine_SetTransition(CP_TRANSITION_SLIDE_UP, seconds); break;
	case 6: CP_Engine_SetTransition(CP_TRANSITION_SLIDE_DOWN, seconds); break;
	default: CP_Engine_SetTransitionFunction(SplitTransition, 0.8f); break;
	}
}

//-------------------------
// MENU

void menu_init(void)
{
}

void menu_update(void)
{
	if (CP_Input_KeyTriggered(KEY_T))
	{
		transitionChoice = (transitionChoice + 1) % TRANSITION_CHOICES;
		ApplyTransitionChoice();
	}
	if (CP_Input_KeyTriggered(KEY_ENTER))
	{
		CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	}

	CP_Graphics_ClearBackground(CP_Color_Create(30, 30, 50, 255));
	DrawCenteredText("STATE STACK", 180.0f, 70.0f);
	DrawCenteredText("Enter to play", 300.0f, 28.0f);
	char text[80];
	snprintf(text, sizeof(text), "T: transition is %s", transitionNames[transitionChoice]);
	DrawCenteredText(text, 360.0f, 24.0f);
	DrawDepth();
}

//-------------------------
// GAME: a square to move, balls that bounce, and a fading trail

#define BALL_COUNT 6
CP_Vector player;
CP_Vector balls[BALL_COUNT];
CP_Vector ballVelocities[BALL_COUNT];
CP_Image trail;		// drawn into every frame: the path the player took

void game_init(void)
{
	// init runs once per game: not again after a pause
	player = CP_Vector_Set(400.0f, 300.0f);
	for (int i = 0; i < BALL_COUNT; ++i)
	{
		balls[i] = CP_Vector_Set(CP_Random_RangeFloat(50.0f, 750.0f), CP_Random_RangeFloat(50.0f, 550.0f));
		ballVelocities[i] = CP_Vector_Scale(CP_Vector_FromAngle(CP_Random_RangeFloat(0.0f, 360.0f)), 150.0f);
	}
	trail = CP_Image_Create(CP_System_GetWindowWidth(), CP_System_GetWindowHeight());
}

void game_update(void)
{
	const float dt = CP_System_GetDt();

	// Keys only reach the top state: under the hint, which runs the game
	// below it, these all read as not pressed
	if (CP_Input_KeyTriggered(KEY_P))
	{
		CP_Engine_PushGameState(NULL, pause_update, NULL);
	}
	if (CP_Input_KeyTriggered(KEY_H))
	{
		CP_Engine_PushGameStateAdvanced(NULL, hint_update, NULL, CP_BELOW_RUNNING);
	}
	CP_Vector move = CP_Vector_Zero();
	if (CP_Input_KeyDown(KEY_LEFT)) move.x -= 1.0f;
	if (CP_Input_KeyDown(KEY_RIGHT)) move.x += 1.0f;
	if (CP_Input_KeyDown(KEY_UP)) move.y -= 1.0f;
	if (CP_Input_KeyDown(KEY_DOWN)) move.y += 1.0f;
	if (move.x != 0.0f || move.y != 0.0f)
	{
		player = CP_Vector_Add(player, CP_Vector_Scale(CP_Vector_Normalize(move), 250.0f * dt));
	}

	for (int i = 0; i < BALL_COUNT; ++i)
	{
		balls[i] = CP_Vector_Add(balls[i], CP_Vector_Scale(ballVelocities[i], dt));
		if (balls[i].x < 15.0f || balls[i].x > CP_System_GetWindowWidth() - 15.0f) ballVelocities[i].x = -ballVelocities[i].x;
		if (balls[i].y < 15.0f || balls[i].y > CP_System_GetWindowHeight() - 15.0f) ballVelocities[i].y = -ballVelocities[i].y;
	}

	// The trail: each frame, the image fades a little (a nearly clear
	// rectangle over it), and gets a dot where the player is
	CP_Image_BeginDrawing(trail);
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_BlendMode(CP_BLEND_SUBTRACT);
	CP_Settings_Fill(CP_Color_Create(0, 0, 0, 6));
	CP_Graphics_DrawRect(0.0f, 0.0f, (float)CP_Image_GetWidth(trail), (float)CP_Image_GetHeight(trail));
	CP_Settings_BlendMode(CP_BLEND_ALPHA);
	CP_Settings_Fill(CP_Color_Create(120, 220, 255, 255));
	CP_Graphics_DrawCircle(player.x, player.y, 18.0f);
	CP_Image_EndDrawing();

	CP_Graphics_ClearBackground(CP_Color_Create(20, 40, 40, 255));
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_Image_Draw(trail, 0.0f, 0.0f, (float)CP_System_GetWindowWidth(), (float)CP_System_GetWindowHeight(), 255);
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(240, 90, 80, 255));
	for (int i = 0; i < BALL_COUNT; ++i)
	{
		CP_Graphics_DrawCircle(balls[i].x, balls[i].y, 30.0f);
	}
	CP_Settings_RectMode(CP_POSITION_CENTER);
	CP_Settings_Fill(white);
	CP_Graphics_DrawRect(player.x, player.y, 24.0f, 24.0f);

	char text[60];
	snprintf(text, sizeof(text), "Playing for %.1f seconds (not counting pauses)", CP_Engine_GetGameStateTime());
	CP_Settings_TextSize(18.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BOTTOM);
	CP_Font_DrawText(text, 10.0f, CP_System_GetWindowHeight() - 10.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_RIGHT, CP_TEXT_ALIGN_V_BOTTOM);
	CP_Font_DrawText("P: pause   H: hint", CP_System_GetWindowWidth() - 10.0f, CP_System_GetWindowHeight() - 10.0f);
	DrawDepth();
}

void game_exit(void)
{
	// runs when the game really ends: going to the menu, or quitting, even
	// from the pause
	CP_Image_Free(&trail);
}

//-------------------------
// PAUSE: on top of the frozen game

void pause_update(void)
{
	// A pushed state starts with a clear layer over the one below. Clearing
	// it with a see-through color darkens the frozen game without building up.
	CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 140));
	DrawPanel(420.0f, 300.0f);
	DrawCenteredText("PAUSED", 200.0f, 56.0f);
	DrawCenteredText("P: keep playing", 270.0f, 24.0f);
	DrawCenteredText("O: options", 305.0f, 24.0f);
	DrawCenteredText("M: menu", 340.0f, 24.0f);
	DrawCenteredText("Q: quit", 375.0f, 24.0f);
	DrawDepth();

	if (CP_Input_KeyTriggered(KEY_P))
	{
		CP_Engine_PopGameState();		// the game carries on where it was
	}
	if (CP_Input_KeyTriggered(KEY_O))
	{
		CP_Engine_PushGameStateAdvanced(NULL, options_update, NULL, CP_BELOW_HIDDEN);
	}
	if (CP_Input_KeyTriggered(KEY_M))
	{
		CP_Engine_SetNextGameState(menu_init, menu_update, NULL);	// ends the pause, then the game
	}
	if (CP_Input_KeyTriggered(KEY_Q))
	{
		CP_Engine_PushGameState(NULL, confirm_update, NULL);
	}
}

//-------------------------
// OPTIONS: pushed with CP_BELOW_HIDDEN, so it has the window to itself

void options_update(void)
{
	if (CP_Input_KeyTriggered(KEY_T))
	{
		transitionChoice = (transitionChoice + 1) % TRANSITION_CHOICES;
		ApplyTransitionChoice();
	}
	if (CP_Input_KeyTriggered(KEY_B))
	{
		CP_Engine_PopGameState();		// back to the pause
	}

	CP_Graphics_ClearBackground(CP_Color_Create(50, 30, 60, 255));
	DrawCenteredText("OPTIONS", 180.0f, 56.0f);
	char text[80];
	snprintf(text, sizeof(text), "T: transition is %s", transitionNames[transitionChoice]);
	DrawCenteredText(text, 280.0f, 24.0f);
	DrawCenteredText("B: back", 340.0f, 24.0f);
	DrawDepth();
}

//-------------------------
// QUIT? on top of the pause, on top of the game

void confirm_update(void)
{
	CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 100));
	DrawPanel(360.0f, 160.0f);
	DrawCenteredText("Really quit?", 280.0f, 36.0f);
	DrawCenteredText("Y: yes   N: no", 330.0f, 24.0f);
	DrawDepth();

	if (CP_Input_KeyTriggered(KEY_Y))
	{
		CP_Engine_Terminate();			// every state on the stack exits, top first
	}
	if (CP_Input_KeyTriggered(KEY_N))
	{
		CP_Engine_PopGameState();
	}
}

//-------------------------
// HINT: pushed with CP_BELOW_RUNNING, so the game keeps moving under it.
// It closes itself after 4 seconds.

void hint_update(void)
{
	const float shown = CP_Engine_GetGameStateTime();
	CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 0));
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(CP_Color_Create(250, 220, 120, 230));
	CP_Graphics_DrawRect(150.0f, 40.0f, 500.0f, 80.0f);
	CP_Settings_Fill(CP_Color_Create(60, 40, 0, 255));
	CP_Graphics_DrawRect(150.0f, 114.0f, 500.0f * (1.0f - shown / 4.0f), 6.0f);
	CP_Settings_TextSize(22.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Font_DrawText("The game keeps running under this hint,", 400.0f, 65.0f);
	CP_Font_DrawText("but your keys come to me: try the arrows", 400.0f, 92.0f);

	if (shown >= 4.0f || CP_Input_KeyTriggered(KEY_H))
	{
		CP_Engine_PopGameState();
	}
}

int main(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("State stack");
	white = CP_Color_Create(255, 255, 255, 255);
	ApplyTransitionChoice();

	CP_Engine_SetNextGameState(menu_init, menu_update, NULL);
	CP_Engine_Run();
	return 0;
}
