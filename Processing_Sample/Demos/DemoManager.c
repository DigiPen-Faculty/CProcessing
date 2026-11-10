//---------------------------------------------------------
// file:	DemoManager.c
//
// Switches between the demos, and handles the keys that work in all of
// them. The keys are listed in a bar at the bottom of the window.
//---------------------------------------------------------

#include <stdio.h>
#include "cprocessing.h"
#include "DemoManager.h"

// demo headers
#include "GridWaveDemo.h"
#include "DancingLinesDemo.h"
#include "GamepadSoundDemo.h"
#include "NoiseDemo.h"
#include "ScreenshotDemo.h"
#include "ColorPickerDemo.h"

// window size and fullscreen
static const int windowWidth = 1280;
static const int windowHeight = 720;
static CP_BOOL isFullscreen = FALSE;

// background color to be used in all demos for consistency
static const CP_Color commonBackground = { 30, 30, 30, 255 };

CP_Color GetCommonBackgroundColor(void)
{
	return commonBackground;
}

// Each demo is a game state: its init, update and exit functions, and the
// name shown in the key bar
typedef struct Demo
{
	const char* name;
	FunctionPtr init;
	FunctionPtr update;
	FunctionPtr exit;
} Demo;

static const Demo demos[] =
{
	{ "Grid wave",         GridWaveDemoInit,     GridWaveDemoUpdate,     GridWaveDemoExit },
	{ "Dancing lines",     DancingLinesDemoInit, DancingLinesDemoUpdate, DancingLinesDemoExit },
	{ "Gamepad and sound", GamepadSoundDemoInit, GamepadSoundDemoUpdate, GamepadSoundDemoExit },
	{ "Noise",             NoiseDemoInit,        NoiseDemoUpdate,        NoiseDemoExit },
	{ "Screenshots",       ScreenshotDemoInit,   ScreenshotDemoUpdate,   ScreenshotDemoExit },
	{ "Color picker",      ColorPickerDemoInit,  ColorPickerDemoUpdate,  ColorPickerDemoExit },
};
#define DEMO_COUNT (int)(sizeof(demos) / sizeof(demos[0]))

static int currentDemo = 0;
static CP_BOOL showKeyBar = TRUE;

static const char* cursorModeNames[] = { "normal", "hidden", "captured", "disabled" };

static void SwitchToDemo(int index)
{
	currentDemo = index;
	CP_Engine_SetNextGameState(demos[index].init, demos[index].update, demos[index].exit);
}

// Runs every frame before the demo's update: the keys that work in every demo
static void DemoManagerPreUpdate(void)
{
	// Escape closes the program
	if (CP_Input_KeyTriggered(KEY_ESCAPE))
	{
		CP_Engine_Terminate();
	}

	// F toggles fullscreen
	if (CP_Input_KeyTriggered(KEY_F))
	{
		if (isFullscreen)
		{
			CP_System_SetWindowSize(windowWidth, windowHeight);
			isFullscreen = FALSE;
		}
		else
		{
			CP_System_FullscreenAdvanced(windowWidth, windowHeight);
			isFullscreen = TRUE;
		}
	}

	// M cycles the mouse cursor mode: normal, hidden, captured, disabled
	if (CP_Input_KeyTriggered(KEY_M))
	{
		CP_CURSOR_MODE mode = (CP_CURSOR_MODE)((CP_System_GetCursorMode() + 1) % 4);
		CP_System_SetCursorMode(mode);
		printf("Cursor mode: %s\n", cursorModeNames[mode]);
	}

	// C shows or hides the console, where printf output appears
	if (CP_Input_KeyTriggered(KEY_C))
	{
		CP_System_ShowConsole(!CP_System_GetConsoleVisible());
	}

	// H shows or hides the key bar
	if (CP_Input_KeyTriggered(KEY_H))
	{
		showKeyBar = !showKeyBar;
	}

	// The number keys switch demos
	for (int i = 0; i < DEMO_COUNT && i < 9; ++i)
	{
		if (CP_Input_KeyTriggered(KEY_1 + i) && i != currentDemo)
		{
			printf("Switching to demo %d: %s\n", i + 1, demos[i].name);
			SwitchToDemo(i);
		}
	}
}

// Runs every frame after the demo's update, so the key bar is drawn on top
static void DemoManagerPostUpdate(void)
{
	if (!showKeyBar)
	{
		return;
	}

	const float barHeight = 30.0f;
	const float width = (float)CP_System_GetWindowWidth();
	const float top = CP_System_GetWindowHeight() - barHeight;
	const float middle = top + barHeight * 0.5f;
	char text[160];

	// Start from known settings, whatever the demo changed, and put the
	// demo's settings back afterwards
	CP_Settings_Save();
	CP_Settings_ResetMatrix();
	CP_Settings_BlendMode(CP_BLEND_ALPHA);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(0, 0, 0, 200));
	CP_Graphics_DrawRect(0.0f, top, width, barHeight);

	CP_Settings_TextSize(18.0f);

	// the demo's number and name on the left
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Settings_Fill(CP_Color_Create(255, 180, 30, 255));
	snprintf(text, sizeof(text), "%d/%d  %s", currentDemo + 1, DEMO_COUNT, demos[currentDemo].name);
	CP_Font_DrawText(text, 12.0f, middle);

	// the keys on the right
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_RIGHT, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Settings_Fill(CP_Color_Create(230, 230, 230, 255));
	snprintf(text, sizeof(text), "1-%d demos     F fullscreen     M mouse: %s     C console     H hide keys     Esc quit",
		DEMO_COUNT, cursorModeNames[CP_System_GetCursorMode()]);
	CP_Font_DrawText(text, width - 12.0f, middle);

	CP_Settings_Restore();
}

void DemoManagerInit(void)
{
	// Set the window size, the keys for every demo, and the first demo
	CP_System_SetWindowSize(windowWidth, windowHeight);
	CP_System_SetWindowTitle("CProcessing demos");
	CP_Engine_SetPreUpdateFunction(DemoManagerPreUpdate);
	CP_Engine_SetPostUpdateFunction(DemoManagerPostUpdate);
	SwitchToDemo(0);
}
