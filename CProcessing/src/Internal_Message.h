//---------------------------------------------------------
// file:	Internal_Message.h
// author:	Justin Chambers
// brief:	Messages for the programmer, when CProcessing sees a mistake
//
// Internal header: not part of the public API.
//
// The rules for messages (when to print one, what it says, where it goes)
// are in DESIGN.md, "Messages".
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// GCC and Clang check each message's format against its arguments, as they
// do for printf
#if defined(__GNUC__) || defined(__clang__)
	#define CP_PRINTF_FORMAT(f, a) __attribute__((format(printf, f, a)))
#else
	#define CP_PRINTF_FORMAT(f, a)
#endif

// Prints one message, printf style: "CProcessing: " in front, a new line
// after. It goes to stderr (a terminal, or the console that
// CP_System_ShowConsole opens) and, on Windows, to Visual Studio's Output
// window. Call it through CP_WARN_ONCE, unless it can only happen once.
void CP_Warn(const char* format, ...) CP_PRINTF_FORMAT(1, 2);

// CP_Warn, once per run from this line, so a mistake made every frame
// prints one message, not sixty a second
#define CP_WARN_ONCE(...)               \
	do                                  \
	{                                   \
		static bool cp_warned_ = false; \
		if (!cp_warned_)                \
		{                               \
			cp_warned_ = true;          \
			CP_Warn(__VA_ARGS__);       \
		}                               \
	} while (0)

// True the first time it's asked about this pair in this run, false after:
// for a message printed once per file instead of once per place (a load that
// failed names each file that couldn't be loaded, once)
bool CP_Message_FirstTimeFor(const char* what, const char* name);

// True if every number is finite. If one is NaN or infinite (usually from a
// division by zero), says which, once per function, and gives false: the
// call is then ignored, since NanoVG would draw nothing, or after a
// transform, nothing more that frame. names is the numbers' names, with
// commas between them, as CP_CHECK_FINITE writes them.
bool CP_Message_CheckFinite(const char* function, const char* names, const float* values, int count);
#define CP_CHECK_FINITE(...) \
	CP_Message_CheckFinite(__func__, #__VA_ARGS__, (const float[]){ __VA_ARGS__ }, (int)(sizeof((const float[]){ __VA_ARGS__ }) / sizeof(float)))

// Writes text that is already a whole message (with its "CProcessing: " and
// its line breaks) where CP_Warn writes
void CP_Message_Write(const char* text);

// Also shows the message in a message box, but only from the Debug library
// on Windows, started from Visual Studio's debugger: for a problem nobody
// would see otherwise (the program ends without a window, or the list of
// files never freed). Never otherwise, so a game someone double-clicks, or
// a test run, can't stop here.
void CP_Message_ShowIfDebugging(const char* title, const char* text);

#ifdef __cplusplus
}
#endif
