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
// Internal header: not part of the public API.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology
//------------------------------------------------------------------------------

#pragma once

#include <math.h>
#include <stdbool.h>
#include "cprocessing_common.h"
// Only GLFW's constants and structs are needed here: don't let glfw3.h pull
// in the system OpenGL header (deprecated, and warning-heavy, on macOS)
#ifndef GLFW_INCLUDE_NONE
	#define GLFW_INCLUDE_NONE
#endif
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

// Per-frame sampling of a key/button's down state.
//
// OS events arrive asynchronously; CProcessing samples the state once per
// frame. A press and release that both land between two samples (a quick
// tap, a touchpad click, an automated key press) would otherwise be missed
// entirely. pressedSinceLastSample latches any press seen since the previous
// sample so a tap still reads as down for exactly one frame: Triggered on
// that frame and Released on the next.
static inline bool CP_InputLogic_SampleButton(bool realtimeDown, bool pressedSinceLastSample)
{
	return realtimeDown || pressedSinceLastSample;
}

//------------------------------------------------------------------------------
// Typed text
//
// GLFW reports each typed character as a Unicode code point. CProcessing
// keeps the text typed during a frame as UTF-8 in a fixed buffer, so a game
// can read it as an ordinary C string.

// Bytes in the buffer, including the ending 0. A person can't type more than
// this in one frame, but a paste or a stuck key could, so extra is dropped.
#define CP_TYPED_TEXT_CAPACITY 64

// Whether a code point is text a person typed, as opposed to a control
// character, a surrogate (not a character by itself) or out of range
static inline bool CP_InputLogic_IsTypedCharacter(unsigned codepoint)
{
	if (codepoint < 0x20 || codepoint == 0x7F)
	{
		return false;
	}
	if (codepoint >= 0x80 && codepoint < 0xA0)
	{
		return false;
	}
	if (codepoint >= 0xD800 && codepoint <= 0xDFFF)
	{
		return false;
	}
	return codepoint <= 0x10FFFF;
}

// Writes a code point as UTF-8 into out (up to 4 bytes) and returns how many
// bytes it took, or 0 if it isn't a typed character
static inline int CP_InputLogic_EncodeUtf8(unsigned codepoint, char out[4])
{
	if (!CP_InputLogic_IsTypedCharacter(codepoint))
	{
		return 0;
	}
	if (codepoint < 0x80)
	{
		out[0] = (char)codepoint;
		return 1;
	}
	if (codepoint < 0x800)
	{
		out[0] = (char)(0xC0 | (codepoint >> 6));
		out[1] = (char)(0x80 | (codepoint & 0x3F));
		return 2;
	}
	if (codepoint < 0x10000)
	{
		out[0] = (char)(0xE0 | (codepoint >> 12));
		out[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
		out[2] = (char)(0x80 | (codepoint & 0x3F));
		return 3;
	}
	out[0] = (char)(0xF0 | (codepoint >> 18));
	out[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
	out[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
	out[3] = (char)(0x80 | (codepoint & 0x3F));
	return 4;
}

// Adds a typed code point to the end of the text in buffer, which holds
// length bytes before its ending 0, and has room for capacity bytes in all.
// Returns the new length. A character that doesn't fit is dropped whole, so
// the text never ends in half a character, and the ending 0 is always there.
static inline int CP_InputLogic_AppendTypedCharacter(char* buffer, int length, int capacity, unsigned codepoint)
{
	char encoded[4];
	int size = CP_InputLogic_EncodeUtf8(codepoint, encoded);
	if (size == 0 || length + size + 1 > capacity)
	{
		return length;
	}
	for (int i = 0; i < size; ++i)
	{
		buffer[length + i] = encoded[i];
	}
	buffer[length + size] = '\0';
	return length + size;
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
