//------------------------------------------------------------------------------
// file:	CP_Input.c
// author:	Daniel Hamilton
// brief:	Handle all input and querying
//
// Copyright © 2019 DigiPen, All rights reserved.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include Files:
//------------------------------------------------------------------------------

#include <math.h>
#include "cprocessing.h"
#include "Internal_System.h"
#include <timeapi.h>
#include <xinput.h>

//------------------------------------------------------------------------------
// Defines and Internal Variables:
//------------------------------------------------------------------------------

#define CP_NUM_KEYS          GLFW_KEY_LAST + 1
#define CP_NUM_MOUSE_BUTTONS GLFW_MOUSE_BUTTON_LAST
#define CP_VALID_KEY_MAX     120 // this must match valid_keys array below
#define DOUBLE_CLICK_TIME    500 // in milliseconds

#define CP_GAMEPAD_TRIGGER_THRESHOLD	XINPUT_GAMEPAD_TRIGGER_THRESHOLD
#define CP_GAMEPAD_THUMB_DEADZONE		XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE
#define CP_GAMEPAD_TRIGGER_RANGE		255.0f
#define CP_GAMEPAD_THUMB_RANGE			32767.0f

//-------------------------------------
// Keyboard

static int valid_keys[CP_VALID_KEY_MAX] = {
	32, 39, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 59, 61, 65,
	66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82,	83,	84,
	85,	86,	87,	88,	89,	90,	91,	92,	93,	96,	161, 162, 256, 257, 258, 259, 260,
	261, 262, 263, 264, 265, 266, 267, 268, 269, 280, 281, 282, 283, 284, 290,
	291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305,
	306, 307, 308, 309, 310, 311, 312, 313, 314, 320, 321, 322, 323, 324, 325,
	326, 327, 328, 329, 330, 331, 332, 333, 334, 335, 336, 340, 341, 342, 343,
	344, 345, 346, 347, 348
};
static bool valid_keys_sparse[CP_NUM_KEYS] = { false };

// Track keyboard states
static int key_states_previous[CP_NUM_KEYS] = { 0 };
static int key_states_current[CP_NUM_KEYS]  = { 0 };
static int key_states_realtime[CP_NUM_KEYS] = { 0 };
static bool key_any_triggered = false;
static bool key_any_down = false;
static bool key_any_released = false;

//-------------------------------------
// Mouse

// Track mouse states
static int mouse_states_previous[CP_NUM_MOUSE_BUTTONS] = { 0 };
static int mouse_states_current[CP_NUM_MOUSE_BUTTONS]  = { 0 };
static int mouse_states_realtime[CP_NUM_MOUSE_BUTTONS] = { 0 };

// Mouse Wheel
static int   mouse_wheel_captured  = FALSE;
static float mouse_wheelx_previous = 0.0f;
static float mouse_wheely_previous = 0.0f;
static float mouse_wheelx_current  = 0.0f;
static float mouse_wheely_current  = 0.0f;
static float mouse_wheelx_realtime = 0.0f;
static float mouse_wheely_realtime = 0.0f;

static DWORD previous_click_time = 0;
static DWORD current_click_time  = 0;

static int	mouse_double_clicked_current  = FALSE;
static int	mouse_double_clicked_realtime = FALSE;

// Mouse Information
static float _mouseX = 0;
static float _mouseY = 0;
static float _pmouseX = 0;
static float _pmouseY = 0;
static float _worldMouseX = 0;
static float _worldMouseY = 0;
static bool _worldMouseIsDirty = TRUE;

//-------------------------------------
// Gamepad

static XINPUT_STATE gamepad_curr_states[XUSER_MAX_COUNT] = { 0 };
static XINPUT_STATE gamepad_prev_states[XUSER_MAX_COUNT] = { 0 };
static CP_GAMEPAD_ANALOG_STATE gamepad_curr_analog_states[XUSER_MAX_COUNT] = { 0 };
static CP_GAMEPAD_ANALOG_STATE gamepad_prev_analog_states[XUSER_MAX_COUNT] = { 0 };
static bool gamepad_connected[XUSER_MAX_COUNT] = { false };
static int _defaultGamepadId = -1;
static const float _deadzone = CP_GAMEPAD_THUMB_DEADZONE / CP_GAMEPAD_THUMB_RANGE;

//------------------------------------------------------------------------------
// Internal Functions:
//------------------------------------------------------------------------------

void CP_Input_KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    UNREFERENCED_PARAMETER(mods);
    UNREFERENCED_PARAMETER(scancode);
    UNREFERENCED_PARAMETER(window);
    
    switch (action)
    {
    case GLFW_PRESS:
        key_states_realtime[key] = TRUE;
        break;
    case GLFW_RELEASE:
        key_states_realtime[key] = FALSE;
        break;
    case GLFW_REPEAT:
        break;
    default:
        break;
    }
}

void CP_Input_MouseCallback(GLFWwindow* window, int button, int action, int mods)
{
    UNREFERENCED_PARAMETER(mods);
    UNREFERENCED_PARAMETER(window);

    switch (action)
    {
    case GLFW_PRESS:
        mouse_states_realtime[button] = TRUE;
        break;
    case GLFW_RELEASE:
        mouse_states_realtime[button] = FALSE;

        // Update click times
        if (button == MOUSE_BUTTON_1)
        {
            previous_click_time = current_click_time;
            current_click_time = timeGetTime();

            DWORD dt = current_click_time - previous_click_time;
            if (dt <= DOUBLE_CLICK_TIME)
            {
                mouse_double_clicked_realtime = TRUE;
            }
        }
        break;
    case GLFW_REPEAT:
        break;
    default:
        break;
    }
}

void CP_Input_MouseWheelCallback(GLFWwindow * window, double xoffset, double yoffset)
{
    UNREFERENCED_PARAMETER(window);

    mouse_wheelx_realtime = (float)xoffset;
    mouse_wheely_realtime = (float)yoffset;

    // Mark that the wheel was captured this frame
    mouse_wheel_captured  = TRUE;
}

void CP_Input_Init(void)
{
	// setup sparse vector for keyboard valid keys
	for (unsigned i = 0; i < CP_VALID_KEY_MAX; ++i)
	{
		valid_keys_sparse[valid_keys[i]] = true;
	}

    CP_CorePtr CORE = GetCPCore();
    glfwSetInputMode(CORE->window, GLFW_STICKY_MOUSE_BUTTONS, 1);

	CP_Input_MouseUpdate();
	CP_Input_MouseUpdate(); // intentionally called twice to setup curr and prev mouse values
}

void CP_Input_Update(void)
{
	if (CP_System_GetWindowFocus())
	{
		CP_Input_KeyboardUpdate();
		CP_Input_MouseUpdate();
		CP_Input_GamepadUpdate();
	}
}

void CP_Input_KeyboardUpdate(void)
{
	// Move current  -> previous
	//      realtime -> current
	unsigned size = sizeof(key_states_previous[0]) * CP_NUM_KEYS;
	memcpy_s(key_states_previous, size, key_states_current, size);
	memcpy_s(key_states_current, size, key_states_realtime, size);
	// track values for ANY key
	key_any_triggered = false;
	key_any_down = false;
	key_any_released = false;
	for (unsigned keyCode = 0; keyCode < CP_NUM_KEYS; ++keyCode)
	{
		if (!key_any_triggered && key_states_current[keyCode] && !key_states_previous[keyCode])
		{
			key_any_triggered = true;
		}
		if (!key_any_down && key_states_current[keyCode])
		{
			key_any_down = true;
		}
		if (!key_any_released && !key_states_current[keyCode] && key_states_previous[keyCode])
		{
			key_any_released = true;
		}
	}
}

void CP_Input_MouseUpdate(void)
{
	double mx, my;

	// Update mouse position
	_pmouseX = _mouseX;
	_pmouseY = _mouseY;

	glfwGetCursorPos(GetCPCore()->window, &mx, &my);
	_mouseX = (float)mx;
	_mouseY = (float)my;

    CP_Input_WorldMouseUpdate();

    // Update Mouse Buttons
    int size = sizeof(mouse_states_previous[0]) * CP_NUM_MOUSE_BUTTONS;
    memcpy_s(mouse_states_previous, size, mouse_states_current, size);
    memcpy_s(mouse_states_current, size, mouse_states_realtime, size);

    // Update mouse wheel
    mouse_wheelx_previous = mouse_wheelx_current;
    mouse_wheely_previous = mouse_wheely_current;

    if (mouse_wheel_captured)
    {
        mouse_wheelx_current = mouse_wheelx_realtime;
        mouse_wheely_current = mouse_wheely_realtime;
        mouse_wheel_captured = FALSE;
    }
    else
    {
        mouse_wheelx_current = 0.0f;
        mouse_wheely_current = 0.0f;

        mouse_wheelx_realtime = 0.0f;
        mouse_wheely_realtime = 0.0f;
    }

    // Update double clicks
    mouse_double_clicked_current = mouse_double_clicked_realtime;
    mouse_double_clicked_realtime = FALSE;
}

void CP_Input_GamepadUpdate(void)
{
	_defaultGamepadId = -1;
	memset(gamepad_connected, 0, sizeof(bool) * XUSER_MAX_COUNT);

	for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		// copy to previous structures
		memcpy(&gamepad_prev_states[i], &gamepad_curr_states[i], sizeof(XINPUT_STATE));
		memcpy(&gamepad_prev_analog_states[i], &gamepad_curr_analog_states[i], sizeof(CP_GAMEPAD_ANALOG_STATE));
		
		// zero out new structures
		memset(&gamepad_curr_states[i], 0, sizeof(XINPUT_STATE));
		memset(&gamepad_curr_analog_states[i], 0, sizeof(CP_GAMEPAD_ANALOG_STATE));

		if (XInputGetState(i, &gamepad_curr_states[i]) == 0)
		{
			// mark connected and keep track of one default gamepad for basic function access
			gamepad_connected[i] = true;
			if (_defaultGamepadId < 0)
			{
				_defaultGamepadId = i;
			}

			// handle deadzones and store analog values in range 0 - 1.0f

			// triggers
			gamepad_curr_analog_states[i].left_trigger = CP_Math_ClampFloat((float)(gamepad_curr_states[i].Gamepad.bLeftTrigger - CP_GAMEPAD_TRIGGER_THRESHOLD) / (CP_GAMEPAD_TRIGGER_RANGE - CP_GAMEPAD_TRIGGER_THRESHOLD), 0, 1.0f);
			gamepad_curr_analog_states[i].right_trigger = CP_Math_ClampFloat((float)(gamepad_curr_states[i].Gamepad.bRightTrigger - CP_GAMEPAD_TRIGGER_THRESHOLD) / (CP_GAMEPAD_TRIGGER_RANGE - CP_GAMEPAD_TRIGGER_THRESHOLD), 0, 1.0f);

			// sticks
			short currStick = 0;
			float normStick = 0;
			// left X
			currStick = gamepad_curr_states[i].Gamepad.sThumbLX;
			normStick = fmaxf(-1.0f, (float)currStick / CP_GAMEPAD_THUMB_RANGE);
			gamepad_curr_analog_states[i].left_stick.x = (fabsf(normStick) < _deadzone ? 0 : (fabsf(normStick) - _deadzone) * (normStick / fabsf(normStick))) / (1.0f - _deadzone);
			// left Y
			currStick = gamepad_curr_states[i].Gamepad.sThumbLY;
			normStick = fmaxf(-1.0f, (float)currStick / CP_GAMEPAD_THUMB_RANGE);
			gamepad_curr_analog_states[i].left_stick.y = (fabsf(normStick) < _deadzone ? 0 : (fabsf(normStick) - _deadzone) * (normStick / fabsf(normStick))) / (1.0f - _deadzone);
			// left X
			currStick = gamepad_curr_states[i].Gamepad.sThumbRX;
			normStick = fmaxf(-1.0f, (float)currStick / CP_GAMEPAD_THUMB_RANGE);
			gamepad_curr_analog_states[i].right_stick.x = (fabsf(normStick) < _deadzone ? 0 : (fabsf(normStick) - _deadzone) * (normStick / fabsf(normStick))) / (1.0f - _deadzone);
			// left Y
			currStick = gamepad_curr_states[i].Gamepad.sThumbRY;
			normStick = fmaxf(-1.0f, (float)currStick / CP_GAMEPAD_THUMB_RANGE);
			gamepad_curr_analog_states[i].right_stick.y = (fabsf(normStick) < _deadzone ? 0 : (fabsf(normStick) - _deadzone) * (normStick / fabsf(normStick))) / (1.0f - _deadzone);		
		}
	}
}

void CP_Input_WorldMouseUpdate(void)
{
	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg) return;

	float worldToScreen[6] = { 0 };
	float screenToWorld[6] = { 0 };
	nvgCurrentTransform(CORE->nvg, worldToScreen); // world to screen
	nvgTransformInverse(screenToWorld, worldToScreen);         // screen to world
	nvgTransformPoint(&_worldMouseX, &_worldMouseY, screenToWorld, _mouseX, _mouseY);
	_worldMouseIsDirty = FALSE;
}

void CP_Input_SetWorldMouseDirty(void)
{
	_worldMouseIsDirty = TRUE;
}

CP_BOOL CP_Input_IsValidKey(CP_KEY key)
{
	if (key < 0 || key >= CP_NUM_KEYS)
	{
		return FALSE;
	}

	return valid_keys_sparse[key];
}

CP_BOOL CP_Input_IsValidMouse(CP_MOUSE button)
{
    return (button >= 0 && button <= MOUSE_BUTTON_LAST);
}

CP_BOOL CP_Input_IsValidGamepad(CP_GAMEPAD button)
{
	return (button >= 0 && button <= GAMEPAD_Y);
}

CP_BOOL CP_Input_IsValidGamepadIndex(unsigned index)
{
	return index >= 0 && index < 4;
}

int CP_Input_ConvertGamepadToXInput(CP_GAMEPAD button)
{
	static int buttonConverter[] = {
		XINPUT_GAMEPAD_DPAD_UP,
		XINPUT_GAMEPAD_DPAD_DOWN,
		XINPUT_GAMEPAD_DPAD_LEFT,
		XINPUT_GAMEPAD_DPAD_RIGHT,
		XINPUT_GAMEPAD_START,
		XINPUT_GAMEPAD_BACK,
		XINPUT_GAMEPAD_LEFT_THUMB,
		XINPUT_GAMEPAD_RIGHT_THUMB,
		XINPUT_GAMEPAD_LEFT_SHOULDER,
		XINPUT_GAMEPAD_RIGHT_SHOULDER,
		XINPUT_GAMEPAD_A,
		XINPUT_GAMEPAD_B,
		XINPUT_GAMEPAD_X,
		XINPUT_GAMEPAD_Y };
	return buttonConverter[button];
}

//------------------------------------------------------------------------------
// Library Functions:
//------------------------------------------------------------------------------

//-------------------------------------
// Keyboard

CP_API CP_BOOL CP_Input_KeyTriggered(CP_KEY keyCode)
{
	if (keyCode == KEY_ANY)
	{
		return key_any_triggered;
	}
    if (CP_Input_IsValidKey(keyCode))
    {
        // Wasn't pressed last frame and is pressed this frame
        return (key_states_current[keyCode] && !key_states_previous[keyCode]);
    }

    return FALSE;
}

CP_API CP_BOOL CP_Input_KeyReleased(CP_KEY keyCode)
{
	if (keyCode == KEY_ANY)
	{
		return key_any_released;
	}
    if (CP_Input_IsValidKey(keyCode))
    {
        // Was pressed last frame and isn't pressed this frame
        return (!key_states_current[keyCode] && key_states_previous[keyCode]);
    }

    return FALSE;
}

CP_API CP_BOOL CP_Input_KeyDown(CP_KEY keyCode)
{
	if (keyCode == KEY_ANY)
	{
		return key_any_down;
	}
    if (CP_Input_IsValidKey(keyCode))
    {
        // Is the key down?
        return key_states_current[keyCode];
    }

    return FALSE;
}

//-------------------------------------
// Mouse

CP_API CP_BOOL CP_Input_MouseTriggered(CP_MOUSE button)
{
	if (!CP_Input_IsValidMouse(button))
	{
		return FALSE;
	}

    return mouse_states_current[button] && !mouse_states_previous[button];
}

CP_API CP_BOOL CP_Input_MouseReleased(CP_MOUSE button)
{
	if (!CP_Input_IsValidMouse(button))
	{
		return FALSE;
	}

    return !mouse_states_current[button] && mouse_states_previous[button];
}

CP_API CP_BOOL CP_Input_MouseDown(CP_MOUSE button)
{
	if (!CP_Input_IsValidMouse(button))
	{
		return FALSE;
	}

    return mouse_states_current[button];
}

CP_API CP_BOOL CP_Input_MouseMoved(void)
{
    return ((CP_Input_GetMouseX() != CP_Input_GetMousePreviousX()) || (CP_Input_GetMouseY() != CP_Input_GetMousePreviousY()));
}

CP_API CP_BOOL CP_Input_MouseClicked(void)
{
    return CP_Input_MouseReleased(MOUSE_BUTTON_LEFT);
}

CP_API CP_BOOL CP_Input_MouseDoubleClicked(void)
{
    return mouse_double_clicked_current;
}

CP_API CP_BOOL CP_Input_MouseDragged(CP_MOUSE button)
{
    if (!CP_Input_IsValidMouse(button))
	{
		return FALSE;
	}

    return (mouse_states_current[button] && mouse_states_previous[button]) && CP_Input_MouseMoved();
}

CP_API float CP_Input_MouseWheel(void)
{
	return mouse_wheely_current;
}

CP_API float CP_Input_GetMouseX(void)
{
	return _mouseX;
}

CP_API float CP_Input_GetMouseY(void)
{
	return _mouseY;
}

CP_API float CP_Input_GetMousePreviousX(void)
{
	return _pmouseX;
}

CP_API float CP_Input_GetMousePreviousY(void)
{
	return _pmouseY;
}

CP_API float CP_Input_GetMouseDeltaX(void)
{
	return _mouseX - _pmouseX;
}

CP_API float CP_Input_GetMouseDeltaY(void)
{
	return _mouseY - _pmouseY;
}

CP_API float CP_Input_GetMouseWorldX(void)
{
	if (_worldMouseIsDirty)
	{
		CP_Input_WorldMouseUpdate();
	}

	return _worldMouseX;
}

CP_API float CP_Input_GetMouseWorldY(void)
{
	if (_worldMouseIsDirty)
	{
		CP_Input_WorldMouseUpdate();
	}

	return _worldMouseY;
}

//-------------------------------------
// Gamepad

CP_API CP_BOOL CP_Input_GamepadTriggered(CP_GAMEPAD button)
{
	return CP_Input_GamepadTriggeredAdvanced(button, _defaultGamepadId);
}

CP_API CP_BOOL CP_Input_GamepadTriggeredAdvanced(CP_GAMEPAD button, unsigned gamepadIndex)
{
	if (CP_Input_IsValidGamepad(button) && CP_Input_IsValidGamepadIndex(gamepadIndex))
	{
		// Wasn't pressed last frame and is pressed this frame
		int convertedButton = CP_Input_ConvertGamepadToXInput(button);
		return (gamepad_curr_states[gamepadIndex].Gamepad.wButtons & convertedButton) != 0 && (gamepad_prev_states[gamepadIndex].Gamepad.wButtons & convertedButton) == 0;
	}

	return FALSE;
}

CP_API CP_BOOL CP_Input_GamepadReleased(CP_GAMEPAD button)
{
	return CP_Input_GamepadReleasedAdvanced(button, _defaultGamepadId);
}

CP_API CP_BOOL CP_Input_GamepadReleasedAdvanced(CP_GAMEPAD button, unsigned gamepadIndex)
{
	if (CP_Input_IsValidGamepad(button) && CP_Input_IsValidGamepadIndex(gamepadIndex))
	{
		// Was pressed last frame and isn't pressed this frame
		int convertedButton = CP_Input_ConvertGamepadToXInput(button);
		return (gamepad_curr_states[gamepadIndex].Gamepad.wButtons & convertedButton) == 0 && (gamepad_prev_states[gamepadIndex].Gamepad.wButtons & convertedButton) != 0;
	}

	return FALSE;
}

CP_API CP_BOOL CP_Input_GamepadDown(CP_GAMEPAD button)
{
	return CP_Input_GamepadDownAdvanced(button, _defaultGamepadId);
}

CP_API CP_BOOL CP_Input_GamepadDownAdvanced(CP_GAMEPAD button, unsigned gamepadIndex)
{
	if (CP_Input_IsValidGamepad(button) && CP_Input_IsValidGamepadIndex(gamepadIndex))
	{
		// Is the button down?
		int convertedButton = CP_Input_ConvertGamepadToXInput(button);
		return (gamepad_curr_states[gamepadIndex].Gamepad.wButtons & convertedButton) != 0;
	}

	return FALSE;
}

CP_API float CP_Input_GamepadRightTrigger(void)
{
	return CP_Input_GamepadRightTriggerAdvanced(_defaultGamepadId);
}

CP_API float CP_Input_GamepadRightTriggerAdvanced(unsigned gamepadIndex)
{
	if (CP_Input_IsValidGamepadIndex(gamepadIndex))
	{
		return gamepad_curr_analog_states[gamepadIndex].right_trigger;
	}

	return 0;
}

CP_API float CP_Input_GamepadLeftTrigger(void)
{
	return CP_Input_GamepadLeftTriggerAdvanced(_defaultGamepadId);
}

CP_API float CP_Input_GamepadLeftTriggerAdvanced(unsigned gamepadIndex)
{
	if (CP_Input_IsValidGamepadIndex(gamepadIndex))
	{
		return gamepad_curr_analog_states[gamepadIndex].left_trigger;
	}

	return 0;
}

CP_API CP_Vector CP_Input_GamepadRightStick(void)
{
	return CP_Input_GamepadRightStickAdvanced(_defaultGamepadId);
}

CP_API CP_Vector CP_Input_GamepadRightStickAdvanced(unsigned gamepadIndex)
{
	if (CP_Input_IsValidGamepadIndex(gamepadIndex))
	{
		return gamepad_curr_analog_states[gamepadIndex].right_stick;
	}

	return CP_Vector_Zero();
}

CP_API CP_Vector CP_Input_GamepadLeftStick(void)
{
	return CP_Input_GamepadLeftStickAdvanced(_defaultGamepadId);
}

CP_API CP_Vector CP_Input_GamepadLeftStickAdvanced(unsigned gamepadIndex)
{
	if (CP_Input_IsValidGamepadIndex(gamepadIndex))
	{
		return gamepad_curr_analog_states[gamepadIndex].left_stick;
	}

	return CP_Vector_Zero();
}

CP_API CP_BOOL CP_Input_GamepadConnected(void)
{
	return _defaultGamepadId >= 0;
}

CP_API CP_BOOL CP_Input_GamepadConnectedAdvanced(unsigned gamepadIndex)
{
	return CP_Input_IsValidGamepadIndex(gamepadIndex) && gamepad_connected[gamepadIndex];
}
