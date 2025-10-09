//---------------------------------------------------------
// file:	DemoManager.c
// 
// Utilities to organize multiple demos
//---------------------------------------------------------

#include "cprocessing.h"
#include "DemoManager.h"

// demo headers
#include "GridWaveDemo.h"
#include "DancingLinesDemo.h"
#include "GamepadDemo.h"
#include "NoiseDemo.h"

// window size and fullscreen
int windowWidth = 1280;
int windowHeight = 720;
CP_BOOL isFullscreen = FALSE;

// background color to be used in all demos for consitency
CP_Color commonBackground = { 30, 30, 30, 255 };

CP_Color GetCommonBackgoundColor(void)
{
	return commonBackground;
}

// organize gamestate functions into an array and enum to be easily navigated
typedef struct GamestateFuncs
{
	FunctionPtr init;
	FunctionPtr update;
	FunctionPtr exit;
} GamestateFuncs;

#define GAMESTATE_FUNCS(name) {name ## Init, name ## Update, name ## Exit}

GamestateFuncs demoGamestateArray[] =
{
	GAMESTATE_FUNCS(GridWaveDemo),
	GAMESTATE_FUNCS(DancingLinesDemo),
	GAMESTATE_FUNCS(GamepadDemo),
	GAMESTATE_FUNCS(NoiseDemo),
};

enum GAMESTATE_ENUM
{
	GridWaveDemo,
	DancingLinesDemo,
	GamepadDemo,
	NoiseDemo,
	GAMESTATE_MAX,
};

void DemoStateSwitcher(GamestateFuncs state)
{
	CP_Engine_SetNextGameState(state.init, state.update, state.exit);
}

// preupdate function for global input checking
void DemoManagerPreupdate(void)
{
	// Escape will close the demo program at any time
	if (CP_Input_KeyTriggered(KEY_ESCAPE))
	{
		CP_Engine_Terminate();
	}

	// F will toggle fullscreen
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

	// Demo states can be chosen by pressing different number keys 1-9
	for (int key = KEY_1; key <= KEY_9; ++key)
	{
		if (CP_Input_KeyTriggered(key))
		{
			int key_value = key - KEY_1;
			if (key_value < GAMESTATE_MAX)	// only valid demo numbers work
			{
				DemoStateSwitcher(demoGamestateArray[key - KEY_1]);
			}
		}
	}
}

void DemoManagerInit(void)
{
	// Set the window size and starting demo
	CP_System_SetWindowSize(windowWidth, windowHeight);
	CP_Engine_SetPreUpdateFunction(DemoManagerPreupdate);
	DemoStateSwitcher(demoGamestateArray[0]);
}
