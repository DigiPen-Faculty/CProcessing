//---------------------------------------------------------
// file:	transitions.c
//
// Example: animated changes between game states, written with ordinary
// CProcessing functions.
//
// CProcessing changes state at once. A transition is a picture you draw on
// top for a moment: a black cover that fades, or a picture of the old state
// that fades or slides away while the new state runs underneath it.
//
// The part marked TRANSITION HELPER does the work, and can be copied into
// another program as it is:
// - Transition_SetNext, Transition_Push and Transition_Pop ask for a change
//   of state with a transition, instead of CP_Engine_SetNextGameState and
//   the rest
// - Transition_Draw draws it: call it from the function you give
//   CP_Engine_SetPostUpdateFunction, which runs after every state has drawn
//
// How it works:
// - for a fade, the helper draws a black cover that darkens, asks for the
//   change when the screen is black, then lightens the cover again
// - for the others, it takes a screenshot of the finished frame
//   (CP_Image_Screenshot), asks for the change, and draws the screenshot over
//   the new state, fading it or sliding it away
// The old state's exit runs before the new state's init, as always: the
// picture of the old state is only a picture, so it can't run any more. That
// keeps the rule from lesson 9 true, and the two states never share their
// values at the same time.
//
// Keys: on the menu, 1 to 7 choose the transition and Enter plays. In the
// game: P pauses, M goes back to the menu.
//
// Try this:
// - Write another drawn transition: shrink the old picture into the middle,
//   or slide it away with a bounce (CP_Math_EaseOutBounce).
// - Give the pause a slide of its own, and the menu a fade.
// - Change EaseInOutCubic in Transition_Progress for another easing.
//
// To run it, copy this file over the template's main.c.
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

//=========================================================
// TRANSITION HELPER
//=========================================================

typedef enum TransitionKind
{
	TRANSITION_FADE,		// to black, then the new state comes in from black
	TRANSITION_CROSSFADE,	// the old picture fades away over the new state
	TRANSITION_SLIDE_LEFT,	// the old picture slides away to the left
	TRANSITION_SLIDE_RIGHT,
	TRANSITION_SLIDE_UP,
	TRANSITION_SLIDE_DOWN,
	TRANSITION_DRAWN		// a function of yours draws the old picture
} TransitionKind;

// Draws the picture of the old state (`before`) over the new one, as it
// should look at `progress`, from 0 (all old) to 1 (all new)
typedef void (*TransitionDrawFunction)(CP_Image before, float progress);

typedef enum TransitionChange { CHANGE_SET, CHANGE_PUSH, CHANGE_POP } TransitionChange;

typedef struct Transition
{
	CP_BOOL playing;
	CP_BOOL asked;			// the change has been asked for
	TransitionKind kind;
	TransitionDrawFunction draw;
	float seconds;
	float time;
	TransitionChange change;
	FunctionPtr init, update, exit;
	CP_Image before;		// the old picture (not for a fade)
} Transition;

Transition transition;

static void Transition_Start(TransitionChange change, TransitionKind kind, float seconds,
	FunctionPtr init, FunctionPtr update, FunctionPtr exit)
{
	if (transition.playing)
	{
		return;		// one at a time
	}
	transition.playing = TRUE;
	transition.asked = FALSE;
	transition.kind = kind;
	transition.seconds = seconds > 0.0f ? seconds : 0.01f;
	transition.time = 0.0f;
	transition.change = change;
	transition.init = init;
	transition.update = update;
	transition.exit = exit;
}

// Like CP_Engine_SetNextGameState, CP_Engine_PushGameState and
// CP_Engine_PopGameState, with a transition
void Transition_SetNext(TransitionKind kind, float seconds, FunctionPtr init, FunctionPtr update, FunctionPtr exit)
{
	Transition_Start(CHANGE_SET, kind, seconds, init, update, exit);
}

void Transition_Push(TransitionKind kind, float seconds, FunctionPtr init, FunctionPtr update, FunctionPtr exit)
{
	Transition_Start(CHANGE_PUSH, kind, seconds, init, update, exit);
}

// (It takes off the top state, which is the one that asks in most programs)
void Transition_Pop(TransitionKind kind, float seconds)
{
	Transition_Start(CHANGE_POP, kind, seconds, NULL, NULL, NULL);
}

void Transition_SetDrawFunction(TransitionDrawFunction draw)
{
	transition.draw = draw;
}

CP_BOOL Transition_IsPlaying(void)
{
	return transition.playing;
}

static void Transition_AskForChange(void)
{
	// From the post-update function: a pop takes off the top state
	switch (transition.change)
	{
	case CHANGE_SET:  CP_Engine_SetNextGameState(transition.init, transition.update, transition.exit); break;
	case CHANGE_PUSH: CP_Engine_PushGameState(transition.init, transition.update, transition.exit); break;
	case CHANGE_POP:  CP_Engine_PopGameState(); break;
	}
	transition.asked = TRUE;
}

// From 0 to 1, eased so it starts and ends gently
static float Transition_Progress(void)
{
	return CP_Math_EaseInOutCubic(CP_Math_ClampFloat(transition.time / transition.seconds, 0.0f, 1.0f));
}

// Call this from the post-update function, every frame
void Transition_Draw(void)
{
	if (!transition.playing)
	{
		return;
	}
	const float width = (float)CP_System_GetWindowWidth();
	const float height = (float)CP_System_GetWindowHeight();

	if (transition.kind != TRANSITION_FADE && !transition.asked)
	{
		// The frame is finished: take its picture, then ask for the change.
		// It happens at the start of the next frame, and this picture is
		// drawn over the new state from then on.
		transition.before = CP_Image_Screenshot(0, 0, (int)width, (int)height);
		Transition_AskForChange();
		return;
	}

	transition.time += CP_System_GetDt();
	const float progress = Transition_Progress();

	// Draw over whatever the states drew, without changing their settings
	CP_Settings_Save();
	CP_Settings_ResetMatrix();
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_ImageMode(CP_POSITION_CORNER);

	switch (transition.kind)
	{
	case TRANSITION_FADE:
	{
		// Darker until the middle, where the change is asked for, then lighter
		if (!transition.asked && progress >= 0.5f)
		{
			Transition_AskForChange();
		}
		const float darkness = progress < 0.5f ? progress * 2.0f : (1.0f - progress) * 2.0f;
		CP_Settings_Fill(CP_Color_Create(0, 0, 0, (int)(255.0f * darkness)));
		CP_Graphics_DrawRect(0.0f, 0.0f, width, height);
		break;
	}
	case TRANSITION_CROSSFADE:
		CP_Image_Draw(transition.before, 0.0f, 0.0f, width, height, (int)(255.0f * (1.0f - progress)));
		break;
	case TRANSITION_SLIDE_LEFT:
		CP_Image_Draw(transition.before, -width * progress, 0.0f, width, height, 255);
		break;
	case TRANSITION_SLIDE_RIGHT:
		CP_Image_Draw(transition.before, width * progress, 0.0f, width, height, 255);
		break;
	case TRANSITION_SLIDE_UP:
		CP_Image_Draw(transition.before, 0.0f, -height * progress, width, height, 255);
		break;
	case TRANSITION_SLIDE_DOWN:
		CP_Image_Draw(transition.before, 0.0f, height * progress, width, height, 255);
		break;
	case TRANSITION_DRAWN:
		if (transition.draw)
		{
			transition.draw(transition.before, progress);
		}
		break;
	}
	CP_Settings_Restore();

	if (transition.time >= transition.seconds)
	{
		transition.playing = FALSE;
		CP_Image_Free(&transition.before);		// (nothing to free after a fade)
	}
}

//=========================================================
// THE PROGRAM
//=========================================================

void menu_init(void);
void menu_update(void);
void menu_exit(void);
void game_init(void);
void game_update(void);
void game_exit(void);
void pause_update(void);

CP_Color white;

void DrawCenteredText(const char* text, float y, float size)
{
	CP_Settings_TextSize(size);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Settings_Fill(white);
	CP_Font_DrawText(text, CP_System_GetWindowWidth() / 2.0f, y);
}

// A transition drawn by this program: the old picture splits down the
// middle, and its halves slide apart to show the new state behind them
void SplitApart(CP_Image before, float progress)
{
	const float width = (float)CP_System_GetWindowWidth();
	const float height = (float)CP_System_GetWindowHeight();
	const float half = width / 2.0f;
	const float gap = half * progress;
	// the screenshot's pixels are the window's: from (0, 0) to (width, height)
	CP_Image_DrawSubImage(before, -gap, 0.0f, half, height, 0.0f, 0.0f, half, height, 255);
	CP_Image_DrawSubImage(before, half + gap, 0.0f, half, height, half, 0.0f, width, height, 255);
}

#define KIND_COUNT 7
const char* kindNames[KIND_COUNT] = {
	"fade", "crossfade", "slide left", "slide right", "slide up", "slide down", "split apart (drawn by this program)"
};
TransitionKind chosen = TRANSITION_CROSSFADE;
const float transitionSeconds = 0.6f;

//-------------------------
// MENU

void menu_init(void)
{
	printf("menu_init\n");
}

void menu_update(void)
{
	for (int i = 0; i < KIND_COUNT; ++i)
	{
		if (CP_Input_KeyTriggered((CP_KEY)(KEY_1 + i)))
		{
			chosen = (TransitionKind)i;
		}
	}
	if (CP_Input_KeyTriggered(KEY_ENTER))
	{
		Transition_SetNext(chosen, transitionSeconds, game_init, game_update, game_exit);
	}

	CP_Graphics_ClearBackground(CP_Color_Create(40, 30, 70, 255));
	DrawCenteredText("TRANSITIONS", 140.0f, 64.0f);
	for (int i = 0; i < KIND_COUNT; ++i)
	{
		char text[80];
		snprintf(text, sizeof(text), "%s%d: %s", i == (int)chosen ? "> " : "", i + 1, kindNames[i]);
		DrawCenteredText(text, 230.0f + 32.0f * i, 24.0f);
	}
	DrawCenteredText("Enter to play", 500.0f, 28.0f);
}

void menu_exit(void)
{
	printf("menu_exit\n");
}

//-------------------------
// GAME: balls that keep moving, so you can see the new state run under the
// old picture as it goes

#define BALL_COUNT 8
CP_Vector balls[BALL_COUNT];
CP_Vector ballVelocities[BALL_COUNT];

void game_init(void)
{
	printf("game_init\n");
	for (int i = 0; i < BALL_COUNT; ++i)
	{
		balls[i] = CP_Vector_Set(CP_Random_RangeFloat(50.0f, 750.0f), CP_Random_RangeFloat(50.0f, 550.0f));
		ballVelocities[i] = CP_Vector_Scale(CP_Vector_FromAngle(CP_Random_RangeFloat(0.0f, 360.0f)), 200.0f);
	}
}

void game_update(void)
{
	const float dt = CP_System_GetDt();
	if (CP_Input_KeyTriggered(KEY_P))
	{
		// The pause is pushed on top: a crossfade makes its dark cover fade in
		Transition_Push(TRANSITION_CROSSFADE, 0.25f, NULL, pause_update, NULL);
	}
	if (CP_Input_KeyTriggered(KEY_M))
	{
		Transition_SetNext(chosen, transitionSeconds, menu_init, menu_update, menu_exit);
	}

	for (int i = 0; i < BALL_COUNT; ++i)
	{
		balls[i] = CP_Vector_Add(balls[i], CP_Vector_Scale(ballVelocities[i], dt));
		if (balls[i].x < 20.0f || balls[i].x > CP_System_GetWindowWidth() - 20.0f) ballVelocities[i].x = -ballVelocities[i].x;
		if (balls[i].y < 20.0f || balls[i].y > CP_System_GetWindowHeight() - 20.0f) ballVelocities[i].y = -ballVelocities[i].y;
	}

	CP_Graphics_ClearBackground(CP_Color_Create(20, 60, 50, 255));
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(250, 200, 80, 255));
	for (int i = 0; i < BALL_COUNT; ++i)
	{
		CP_Graphics_DrawCircle(balls[i].x, balls[i].y, 40.0f);
	}
	DrawCenteredText("P: pause   M: menu", 560.0f, 22.0f);
}

void game_exit(void)
{
	printf("game_exit\n");
}

//-------------------------
// PAUSE: pushed on top of the frozen game

void pause_update(void)
{
	CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 150));
	DrawCenteredText("PAUSED", 260.0f, 64.0f);
	DrawCenteredText("P to keep playing", 340.0f, 26.0f);
	if (CP_Input_KeyTriggered(KEY_P))
	{
		Transition_Pop(TRANSITION_CROSSFADE, 0.25f);
	}
}

int main(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Transitions");
	white = CP_Color_Create(255, 255, 255, 255);

	Transition_SetDrawFunction(SplitApart);
	// Transition_Draw runs after every state has drawn, each frame
	CP_Engine_SetPostUpdateFunction(Transition_Draw);

	CP_Engine_SetNextGameState(menu_init, menu_update, menu_exit);
	CP_Engine_Run();
	return 0;
}
