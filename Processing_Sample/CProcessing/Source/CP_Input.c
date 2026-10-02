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
#include "Internal_InputLogic.h"

//------------------------------------------------------------------------------
// Defines and Internal Variables:
//------------------------------------------------------------------------------

#define CP_NUM_KEYS          (GLFW_KEY_LAST + 1)
#define CP_NUM_MOUSE_BUTTONS (GLFW_MOUSE_BUTTON_LAST + 1)
#define CP_VALID_KEY_MAX     120 // this must match valid_keys array below
#define CP_NUM_GAMEPADS      4   // matches CP_MAX_GAMEPADS in cprocessing_common.h

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
static bool key_pressed_since_update[CP_NUM_KEYS] = { false }; // see CP_InputLogic_SampleButton
static bool key_any_triggered = false;
static bool key_any_down = false;
static bool key_any_released = false;

//-------------------------------------
// Mouse

// Track mouse states
static int mouse_states_previous[CP_NUM_MOUSE_BUTTONS] = { 0 };
static int mouse_states_current[CP_NUM_MOUSE_BUTTONS]  = { 0 };
static int mouse_states_realtime[CP_NUM_MOUSE_BUTTONS] = { 0 };
static bool mouse_pressed_since_update[CP_NUM_MOUSE_BUTTONS] = { false };

// Mouse Wheel
static int   mouse_wheel_captured  = FALSE;
static float mouse_wheelx_previous = 0.0f;
static float mouse_wheely_previous = 0.0f;
static float mouse_wheelx_current  = 0.0f;
static float mouse_wheely_current  = 0.0f;
static float mouse_wheelx_realtime = 0.0f;
static float mouse_wheely_realtime = 0.0f;

static double previous_click_time = 0;
static double current_click_time  = 0;

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

// Gamepads come from GLFW's gamepad API (SDL_GameControllerDB mappings), which
// covers XInput controllers on Windows as well as Linux and macOS devices.
// CProcessing gamepad index N is the Nth connected GLFW joystick that has a
// gamepad mapping, in GLFW joystick order.
static unsigned gamepad_curr_buttons[CP_NUM_GAMEPADS] = { 0 };
static unsigned gamepad_prev_buttons[CP_NUM_GAMEPADS] = { 0 };
static CP_GAMEPAD_ANALOG_STATE gamepad_curr_analog_states[CP_NUM_GAMEPADS] = { 0 };
static CP_GAMEPAD_ANALOG_STATE gamepad_prev_analog_states[CP_NUM_GAMEPADS] = { 0 };
static bool gamepad_connected[CP_NUM_GAMEPADS] = { false };
static int _defaultGamepadId = -1;

//------------------------------------------------------------------------------
// Internal Functions:
//------------------------------------------------------------------------------

void CP_Input_KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    UNREFERENCED_PARAMETER(mods);
    UNREFERENCED_PARAMETER(scancode);
    UNREFERENCED_PARAMETER(window);

    // GLFW reports keys it can't map (media keys, some layouts) as
    // GLFW_KEY_UNKNOWN (-1); ignore anything outside the tracked range
    if (key < 0 || key >= CP_NUM_KEYS)
    {
        return;
    }

    switch (action)
    {
    case GLFW_PRESS:
        key_states_realtime[key] = TRUE;
        key_pressed_since_update[key] = true;
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

    if (button < 0 || button >= CP_NUM_MOUSE_BUTTONS)
    {
        return;
    }

    switch (action)
    {
    case GLFW_PRESS:
        mouse_states_realtime[button] = TRUE;
        mouse_pressed_since_update[button] = true;
        break;
    case GLFW_RELEASE:
        mouse_states_realtime[button] = FALSE;

        // Update click times
        if (button == MOUSE_BUTTON_1)
        {
            previous_click_time = current_click_time;
            current_click_time = glfwGetTime();

            if (CP_InputLogic_IsDoubleClick(previous_click_time, current_click_time))
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
	CP_Input_KeyboardUpdate();
	CP_Input_MouseUpdate();
	CP_Input_GamepadUpdate();
}

void CP_Input_KeyboardUpdate(void)
{
	// Move current  -> previous
	//      realtime -> current
	unsigned size = sizeof(key_states_previous[0]) * CP_NUM_KEYS;
	memcpy(key_states_previous, key_states_current, size);
	for (unsigned keyCode = 0; keyCode < CP_NUM_KEYS; ++keyCode)
	{
		key_states_current[keyCode] = CP_InputLogic_SampleButton(key_states_realtime[keyCode], key_pressed_since_update[keyCode]);
		key_pressed_since_update[keyCode] = false;
	}
	// track values for ANY key
	key_any_triggered = false;
	key_any_down = false;
	key_any_released = false;
	for (unsigned keyCode = 0; keyCode < CP_NUM_KEYS; ++keyCode)
	{
		if (!key_any_triggered && CP_InputLogic_Triggered(key_states_current[keyCode], key_states_previous[keyCode]))
		{
			key_any_triggered = true;
		}
		if (!key_any_down && key_states_current[keyCode])
		{
			key_any_down = true;
		}
		if (!key_any_released && CP_InputLogic_Released(key_states_current[keyCode], key_states_previous[keyCode]))
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
    memcpy(mouse_states_previous, mouse_states_current, size);
    for (int button = 0; button < CP_NUM_MOUSE_BUTTONS; ++button)
    {
        mouse_states_current[button] = CP_InputLogic_SampleButton(mouse_states_realtime[button], mouse_pressed_since_update[button]);
        mouse_pressed_since_update[button] = false;
    }

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
	memset(gamepad_connected, 0, sizeof(gamepad_connected));

	// copy to previous structures and zero out new structures
	memcpy(gamepad_prev_buttons, gamepad_curr_buttons, sizeof(gamepad_curr_buttons));
	memcpy(gamepad_prev_analog_states, gamepad_curr_analog_states, sizeof(gamepad_curr_analog_states));
	memset(gamepad_curr_buttons, 0, sizeof(gamepad_curr_buttons));
	memset(gamepad_curr_analog_states, 0, sizeof(gamepad_curr_analog_states));

	// assign connected GLFW gamepads to CProcessing gamepad slots in joystick order
	unsigned slot = 0;
	for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST && slot < CP_NUM_GAMEPADS; ++jid)
	{
		GLFWgamepadstate state;
		if (!glfwJoystickIsGamepad(jid) || !glfwGetGamepadState(jid, &state))
		{
			continue;
		}

		CP_GamepadRawState raw = CP_InputLogic_FromGLFWGamepad(&state);

		// mark connected and keep track of one default gamepad for basic function access
		gamepad_connected[slot] = true;
		if (_defaultGamepadId < 0)
		{
			_defaultGamepadId = (int)slot;
		}

		gamepad_curr_buttons[slot] = raw.buttons;

		// handle deadzones and store analog values (triggers 0 - 1.0f, sticks -1.0f - 1.0f)
		gamepad_curr_analog_states[slot].left_trigger = CP_InputLogic_ApplyTriggerThreshold(raw.left_trigger);
		gamepad_curr_analog_states[slot].right_trigger = CP_InputLogic_ApplyTriggerThreshold(raw.right_trigger);
		gamepad_curr_analog_states[slot].left_stick.x = CP_InputLogic_ApplyStickDeadzone(raw.left_x);
		gamepad_curr_analog_states[slot].left_stick.y = CP_InputLogic_ApplyStickDeadzone(raw.left_y);
		gamepad_curr_analog_states[slot].right_stick.x = CP_InputLogic_ApplyStickDeadzone(raw.right_x);
		gamepad_curr_analog_states[slot].right_stick.y = CP_InputLogic_ApplyStickDeadzone(raw.right_y);

		++slot;
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
	return CP_InputLogic_GamepadButtonIsValid(button);
}

CP_BOOL CP_Input_IsValidGamepadIndex(unsigned index)
{
	return index < CP_NUM_GAMEPADS;
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
        return CP_InputLogic_Triggered(key_states_current[keyCode], key_states_previous[keyCode]);
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
        return CP_InputLogic_Released(key_states_current[keyCode], key_states_previous[keyCode]);
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

    return CP_InputLogic_Triggered(mouse_states_current[button], mouse_states_previous[button]);
}

CP_API CP_BOOL CP_Input_MouseReleased(CP_MOUSE button)
{
	if (!CP_Input_IsValidMouse(button))
	{
		return FALSE;
	}

    return CP_InputLogic_Released(mouse_states_current[button], mouse_states_previous[button]);
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
		return CP_InputLogic_Triggered(
			CP_InputLogic_GamepadButtonDown(gamepad_curr_buttons[gamepadIndex], button),
			CP_InputLogic_GamepadButtonDown(gamepad_prev_buttons[gamepadIndex], button));
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
		return CP_InputLogic_Released(
			CP_InputLogic_GamepadButtonDown(gamepad_curr_buttons[gamepadIndex], button),
			CP_InputLogic_GamepadButtonDown(gamepad_prev_buttons[gamepadIndex], button));
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
		return CP_InputLogic_GamepadButtonDown(gamepad_curr_buttons[gamepadIndex], button);
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
