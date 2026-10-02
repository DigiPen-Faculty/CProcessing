//------------------------------------------------------------------------------
// file:	Internal_InputLogic.h
// brief:	Pure, state-free input logic used by CP_Input.c.
//
//			Everything in here is a deterministic function of its arguments:
//			no GLFW calls, no globals, no window. That keeps the parts of
//			CP_Input that are easy to get subtly wrong (edge detection,
//			deadzones, trigger thresholds, GLFW -> CProcessing gamepad
//			mapping) directly unit-testable without a real window, OS input
//			events or a physical controller. The Tier 1 test project includes
//			this header directly.
//
//			Only GLFW's *constants* and the GLFWgamepadstate struct are used
//			from glfw3.h; no GLFW functions are called.
//
// INTERNAL USE ONLY, DO NOT DISTRIBUTE
//
// Copyright (c) 2026 DigiPen, All rights reserved.
//------------------------------------------------------------------------------

#pragma once

#include <math.h>
#include <stdbool.h>
#include "cprocessing_common.h"
#include "glfw3.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Gamepad tuning
//
// These reproduce the values the original XInput implementation used so
// gameplay "feel" is unchanged by the move to GLFW's gamepad API:
//   XINPUT_GAMEPAD_TRIGGER_THRESHOLD    = 30    (of 255)
//   XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE = 8689  (of 32767)
#define CP_GAMEPAD_TRIGGER_THRESHOLD	30.0f
#define CP_GAMEPAD_TRIGGER_RANGE		255.0f
#define CP_GAMEPAD_THUMB_DEADZONE		8689.0f
#define CP_GAMEPAD_THUMB_RANGE			32767.0f

#define CP_GAMEPAD_BUTTON_COUNT			(GAMEPAD_Y + 1)

// Raw, per-frame snapshot of one gamepad, independent of the input backend.
// Sticks use the XInput/CProcessing convention: +x is right, +y is UP.
typedef struct CP_GamepadRawState
{
	bool connected;
	unsigned buttons;		// bit (1u << CP_GAMEPAD) set while that button is held
	float left_trigger;		// 0 (released) .. 1 (fully pressed), no threshold applied
	float right_trigger;
	float left_x;			// -1 .. 1, no deadzone applied
	float left_y;
	float right_x;
	float right_y;
} CP_GamepadRawState;

//------------------------------------------------------------------------------
// Edge detection, shared by keyboard, mouse and gamepad buttons

static inline bool CP_InputLogic_Triggered(bool current, bool previous)
{
	// wasn't down last frame and is down this frame
	return current && !previous;
}

static inline bool CP_InputLogic_Released(bool current, bool previous)
{
	// was down last frame and isn't down this frame
	return !current && previous;
}

//------------------------------------------------------------------------------
// Mouse double click: a left-button release within this many seconds of the
// previous left-button release counts as a double click
#define CP_DOUBLE_CLICK_TIME 0.5

static inline bool CP_InputLogic_IsDoubleClick(double previousReleaseTime, double currentReleaseTime)
{
	return (currentReleaseTime - previousReleaseTime) <= CP_DOUBLE_CLICK_TIME;
}

//------------------------------------------------------------------------------
// Gamepad buttons

static inline bool CP_InputLogic_GamepadButtonIsValid(CP_GAMEPAD button)
{
	return (int)button >= 0 && (int)button < CP_GAMEPAD_BUTTON_COUNT;
}

static inline unsigned CP_InputLogic_GamepadButtonMask(CP_GAMEPAD button)
{
	return CP_InputLogic_GamepadButtonIsValid(button) ? (1u << (unsigned)button) : 0u;
}

static inline bool CP_InputLogic_GamepadButtonDown(unsigned buttons, CP_GAMEPAD button)
{
	return (buttons & CP_InputLogic_GamepadButtonMask(button)) != 0;
}

// GLFW gamepad button index for each CP_GAMEPAD value (Xbox layout)
static inline int CP_InputLogic_GamepadToGLFWButton(CP_GAMEPAD button)
{
	switch (button)
	{
	case GAMEPAD_DPAD_UP:			return GLFW_GAMEPAD_BUTTON_DPAD_UP;
	case GAMEPAD_DPAD_DOWN:			return GLFW_GAMEPAD_BUTTON_DPAD_DOWN;
	case GAMEPAD_DPAD_LEFT:			return GLFW_GAMEPAD_BUTTON_DPAD_LEFT;
	case GAMEPAD_DPAD_RIGHT:		return GLFW_GAMEPAD_BUTTON_DPAD_RIGHT;
	case GAMEPAD_START:				return GLFW_GAMEPAD_BUTTON_START;
	case GAMEPAD_BACK:				return GLFW_GAMEPAD_BUTTON_BACK;
	case GAMEPAD_LEFT_THUMB:		return GLFW_GAMEPAD_BUTTON_LEFT_THUMB;
	case GAMEPAD_RIGHT_THUMB:		return GLFW_GAMEPAD_BUTTON_RIGHT_THUMB;
	case GAMEPAD_LEFT_SHOULDER:		return GLFW_GAMEPAD_BUTTON_LEFT_BUMPER;
	case GAMEPAD_RIGHT_SHOULDER:	return GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER;
	case GAMEPAD_A:					return GLFW_GAMEPAD_BUTTON_A;
	case GAMEPAD_B:					return GLFW_GAMEPAD_BUTTON_B;
	case GAMEPAD_X:					return GLFW_GAMEPAD_BUTTON_X;
	case GAMEPAD_Y:					return GLFW_GAMEPAD_BUTTON_Y;
	default:						return -1;
	}
}

// Convert GLFW's gamepad state into the backend-independent raw snapshot.
// GLFW reports triggers in [-1, 1] with -1 at rest, and stick Y with +y DOWN;
// both are converted to the XInput conventions CProcessing has always exposed.
static inline CP_GamepadRawState CP_InputLogic_FromGLFWGamepad(const GLFWgamepadstate* state)
{
	CP_GamepadRawState raw = { 0 };
	if (!state)
	{
		return raw;
	}

	raw.connected = true;
	for (int b = 0; b < CP_GAMEPAD_BUTTON_COUNT; ++b)
	{
		int glfwButton = CP_InputLogic_GamepadToGLFWButton((CP_GAMEPAD)b);
		if (glfwButton >= 0 && state->buttons[glfwButton] == GLFW_PRESS)
		{
			raw.buttons |= 1u << (unsigned)b;
		}
	}

	raw.left_trigger = (state->axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] + 1.0f) * 0.5f;
	raw.right_trigger = (state->axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] + 1.0f) * 0.5f;
	raw.left_x = state->axes[GLFW_GAMEPAD_AXIS_LEFT_X];
	raw.left_y = -state->axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
	raw.right_x = state->axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
	raw.right_y = -state->axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
	return raw;
}

//------------------------------------------------------------------------------
// Gamepad analog processing

static inline float CP_InputLogic_Clamp01(float value)
{
	return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
}

// Trigger: raw 0..1 -> 0..1 with the bottom of the travel ignored as noise
static inline float CP_InputLogic_ApplyTriggerThreshold(float raw)
{
	float scaled = CP_InputLogic_Clamp01(raw) * CP_GAMEPAD_TRIGGER_RANGE;
	return CP_InputLogic_Clamp01((scaled - CP_GAMEPAD_TRIGGER_THRESHOLD) / (CP_GAMEPAD_TRIGGER_RANGE - CP_GAMEPAD_TRIGGER_THRESHOLD));
}

// Stick axis: raw -1..1 -> -1..1 with a centered deadzone, rescaled so output
// still reaches +-1 at full deflection and grows smoothly from 0 at the edge
// of the deadzone
static inline float CP_InputLogic_ApplyStickDeadzone(float raw)
{
	const float deadzone = CP_GAMEPAD_THUMB_DEADZONE / CP_GAMEPAD_THUMB_RANGE;
	float value = raw < -1.0f ? -1.0f : (raw > 1.0f ? 1.0f : raw);
	float magnitude = fabsf(value);
	if (magnitude < deadzone)
	{
		return 0.0f;
	}
	float sign = value < 0.0f ? -1.0f : 1.0f;
	return (magnitude - deadzone) * sign / (1.0f - deadzone);
}

#ifdef __cplusplus
}
#endif
