//------------------------------------------------------------------------------
// file:	Internal_Input.h
// author:	Daniel Hamilton
// brief:	Handle input and querying
//
// INTERNAL USE ONLY, DO NOT DISTRIBUTE
//
// Copyright © 2019 DigiPen, All rights reserved.
//------------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// GLFW includes needed for input
struct GLFWwindow;
typedef struct GLFWwindow GLFWwindow;

typedef struct CP_GAMEPAD_ANALOG_STATE
{
	CP_Vector left_stick;
	CP_Vector right_stick;
	float left_trigger;
	float right_trigger;
} CP_GAMEPAD_ANALOG_STATE;

////////////////////////////////////////////////////////////////////////////////
// INTERNAL USE
void CP_Input_KeyboardCallback(GLFWwindow* win, int key, int scancode, int action, int mods);
void CP_Input_MouseCallback(GLFWwindow* window, int button, int action, int mods);
void CP_Input_MouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset);
void CP_Input_Init(void);
void CP_Input_Update(void);
void CP_Input_KeyboardUpdate(void);
void CP_Input_MouseUpdate(void);
void CP_Input_GamepadUpdate(void);
void CP_Input_WorldMouseUpdate(void);
void CP_Input_SetWorldMouseDirty(void);
CP_BOOL  CP_Input_IsValidKey(CP_KEY key);
CP_BOOL  CP_Input_IsValidMouse(CP_MOUSE button);
CP_BOOL  CP_Input_IsValidGamepad(CP_GAMEPAD button);
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
