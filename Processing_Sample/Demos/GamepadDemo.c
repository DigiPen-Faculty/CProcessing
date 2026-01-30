//---------------------------------------------------------
// file:	GamepadDemo.c
// 
// CProcessing features in this demo:
// - Check for connected gamepads
// - Find the default gamepad index (this changes as devices are added removed)
// - Check and visualize all analog and digital inputs from each gamepad
//---------------------------------------------------------

#include "cprocessing.h"
#include "GamepadDemo.h"
#include "DemoManager.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void DrawGamepadData(int posX, int posY, int i)
{
	// if the gamepad isn't connected then don't render the input state
	if (!CP_Input_GamepadConnectedAdvanced(i))
	{
		return;
	}

	// find default gamepad
	int defaultId = -1;
	for (int id = 0; id < CP_MAX_GAMEPADS; ++id)
	{
		if (CP_Input_GamepadConnectedAdvanced(id))
		{
			defaultId = id;
			break;
		}
	}

	CP_Settings_Save();
	CP_Settings_Translate((float)posX, (float)posY);	// move to the specified quadrant

	//-------------------------
	// INDEX and DEFAULT
	char buffer[20] = { 0 };
	sprintf_s(buffer, _countof(buffer), "Gamepad: %d", i + 1);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Font_DrawText(buffer, 10, 30);
	if (defaultId == i)
	{
		CP_Font_DrawText("Default", 10, 50);
	}

	//-------------------------
	// STICKS
	float circleDiameter = 60.0f;
	float movementOffset = 30.0f;
	CP_Vector rightVec = CP_Vector_Set(1.0f, 0);
	CP_Vector stick = CP_Vector_Zero();
	CP_BOOL stickClick = FALSE;

	// left stick
	stick = CP_Input_GamepadLeftStickAdvanced(i);
	stickClick = CP_Input_GamepadDownAdvanced(GAMEPAD_LEFT_THUMB, i) ? 80 : 50;
	CP_Settings_Fill(CP_Color_FromColorHSL(CP_ColorHSL_Create(stick.y < 0 ? (int)CP_Vector_Angle(stick, rightVec) : 360 - (int)CP_Vector_Angle(stick, rightVec), (int)(CP_Vector_Length(stick) * 100), stickClick, (int)(CP_Vector_Length(stick) * 255) + (CP_Input_GamepadDownAdvanced(GAMEPAD_LEFT_THUMB, i) ? 255 : 0))));
	CP_Graphics_DrawCircle(100.0f + (stick.x * movementOffset), 160.0f + (-stick.y * movementOffset), circleDiameter);
	// right stick
	stick = CP_Input_GamepadRightStickAdvanced(i);
	stickClick = CP_Input_GamepadDownAdvanced(GAMEPAD_RIGHT_THUMB, i) ? 80 : 50;
	CP_Settings_Fill(CP_Color_FromColorHSL(CP_ColorHSL_Create(stick.y < 0 ? (int)CP_Vector_Angle(stick, rightVec) : 360 - (int)CP_Vector_Angle(stick, rightVec), (int)(CP_Vector_Length(stick) * 100), stickClick, (int)(CP_Vector_Length(stick) * 255) + (CP_Input_GamepadDownAdvanced(GAMEPAD_RIGHT_THUMB, i) ? 255 : 0))));
	CP_Graphics_DrawCircle(300.0f + (stick.x * movementOffset), 160.0f + (-stick.y * movementOffset), circleDiameter);

	//-------------------------
	// TRIGGERS
	CP_Settings_RectMode(CP_POSITION_CORNER);
	float rectWidth = 80.0f;
	CP_Input_GamepadDownAdvanced(GAMEPAD_LEFT_SHOULDER, i) ? CP_Settings_Fill(CP_Color_Create(200, 200, 200, 255)) : CP_Settings_NoFill();
	CP_Graphics_DrawRect(60.0f, 80.0f, rectWidth, 20.0f);
	CP_Input_GamepadDownAdvanced(GAMEPAD_RIGHT_SHOULDER, i) ? CP_Settings_Fill(CP_Color_Create(200, 200, 200, 255)) : CP_Settings_NoFill();
	CP_Graphics_DrawRect(260.0f, 80.0f, rectWidth, 20.0f);
	float trigger = CP_Input_GamepadLeftTriggerAdvanced(i);
	CP_Settings_Fill(CP_Color_Create(255 - (int)(trigger * 255), 0, (int)(trigger * 255), 255));
	CP_Graphics_DrawRect(60.0f, 80.0f, trigger * rectWidth, 20.0f);
	trigger = CP_Input_GamepadRightTriggerAdvanced(i);
	CP_Settings_Fill(CP_Color_Create((int)(trigger * 255), 255, 0, 255));
	CP_Graphics_DrawRect(260.0f, 80.0f, trigger * rectWidth, 20.0f);

	//-------------------------
	// DPAD
	CP_Settings_RectMode(CP_POSITION_CENTER);
	float buttonSize = 30.0f;
	CP_Color fillcolor = CP_Color_FromColorHSL(CP_ColorHSL_Create((int)(CP_System_GetSeconds() * 60.0f) % 360, 100, 50, 255));
	CP_Input_GamepadDownAdvanced(GAMEPAD_DPAD_UP, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawRect(100.0f, 240.0f, buttonSize, buttonSize);
	CP_Input_GamepadDownAdvanced(GAMEPAD_DPAD_LEFT, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawRect(60.0f, 280.0f, buttonSize, buttonSize);
	CP_Input_GamepadDownAdvanced(GAMEPAD_DPAD_RIGHT, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawRect(140.0f, 280.0f, buttonSize, buttonSize);
	CP_Input_GamepadDownAdvanced(GAMEPAD_DPAD_DOWN, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawRect(100.0f, 320.0f, buttonSize, buttonSize);

	//-------------------------
	// ABXY
	CP_Input_GamepadDownAdvanced(GAMEPAD_Y, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawCircle(300.0f, 240.0f, buttonSize);
	CP_Input_GamepadDownAdvanced(GAMEPAD_X, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawCircle(260.0f, 280.0f, buttonSize);
	CP_Input_GamepadDownAdvanced(GAMEPAD_B, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawCircle(340.0f, 280.0f, buttonSize);
	CP_Input_GamepadDownAdvanced(GAMEPAD_A, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawCircle(300.0f, 320.0f, buttonSize);

	CP_Settings_Restore();
}

void GamepadDemoInit(void)
{
	CP_Settings_Save();
	CP_Settings_TextSize(25);
	CP_Settings_Stroke(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_StrokeWeight(3);
}

void GamepadDemoUpdate(void)
{
	// clear background and draw separation lines
	CP_Graphics_ClearBackground(GetCommonBackgoundColor());
	CP_Graphics_DrawLine(CP_System_GetWindowWidth() * 0.5f, 0, CP_System_GetWindowWidth() * 0.5f, (float)CP_System_GetWindowHeight());
	CP_Graphics_DrawLine(0, CP_System_GetWindowHeight() * 0.5f, (float)CP_System_GetWindowWidth(), CP_System_GetWindowHeight() * 0.5f);
	
	// if no gamepad is connected let the user know to add one
	if (!CP_Input_GamepadConnected())
	{
		CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
		CP_Font_DrawText("Connect a gamepad to see visualization data.", 50, 50);
	}
		
	// calculate positioning information
	int gamepadVisualizationWidth = 400;
	int windowWidth = CP_System_GetWindowWidth();
	int halfWidth = windowWidth / 2;
	int halfHeight = CP_System_GetWindowHeight() / 2;
	int leftSideX = (halfWidth - gamepadVisualizationWidth) / 2;
	
	// render all the gamepad data
	DrawGamepadData(leftSideX, 0, 0);							// top left
	DrawGamepadData(halfWidth + leftSideX, 0, 1);				// top right
	DrawGamepadData(leftSideX, halfHeight, 2);					// bottom left
	DrawGamepadData(halfWidth + leftSideX, halfHeight, 3);		// bottom right
}

void GamepadDemoExit(void)
{
	CP_Settings_Restore();
}