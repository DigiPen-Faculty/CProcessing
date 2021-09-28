//------------------------------------------------------------------------------
// File:	Internal_Input.h
// Author:	Daniel Hamilton
// Brief:	Handle input and querying
//
// GitHub Repository:
// https://github.com/DigiPen-Faculty/CProcessing
//
//---------------------------------------------------------
// MIT License
//
// Copyright (C) 2021 DigiPen Institute of Technology
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//---------------------------------------------------------

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
	void 		CP_Input_KeyboardCallback(GLFWwindow* win, int key, int scancode, int action, int mods);
	void 		CP_Input_MouseCallback(GLFWwindow* window, int button, int action, int mods);
	void 		CP_Input_MouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset);
	void 		CP_Input_Init(void);
	void		CP_Input_Update(void);
	void		CP_Input_KeyboardUpdate(void);
	void		CP_Input_MouseUpdate(void);
	void		CP_Input_GamepadUpdate(void);
	void		CP_Input_WorldMouseUpdate(void);
	void		CP_Input_SetWorldMouseDirty(void);
	CP_BOOL		CP_Input_IsValidKey(CP_KEY key);
	CP_BOOL		CP_Input_IsValidMouse(CP_MOUSE button);
	CP_BOOL		CP_Input_IsValidGamepad(CP_GAMEPAD button);
	////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
