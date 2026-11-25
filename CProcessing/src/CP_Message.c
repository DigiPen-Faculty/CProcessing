//---------------------------------------------------------
// file:	CP_Message.c
// author:	Justin Chambers
// brief:	Messages for the programmer, when CProcessing sees a mistake
//
// The rules for messages are in DESIGN.md, "Messages".
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "Internal_Message.h"
#include "Internal_Platform.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void CP_Message_Write(const char* text)
{
	fputs(text, stderr);
	fflush(stderr);
#if defined(_WIN32)
	// A Windows program usually has no console: Visual Studio's Output window
	OutputDebugStringA(text);
#endif
}

void CP_Warn(const char* format, ...)
{
	static const char prefix[] = "CProcessing: ";
	char text[1024];
	size_t length = sizeof(prefix) - 1;
	memcpy(text, prefix, length);

	va_list args;
	va_start(args, format);
	// Leave room for the new line and the terminator; a longer message is cut
	int written = vsnprintf(text + length, sizeof(text) - length - 1, format, args);
	va_end(args);

	if (written > 0)
	{
		length += (size_t)written < sizeof(text) - length - 1 ? (size_t)written : sizeof(text) - length - 2;
	}
	text[length] = '\n';
	text[length + 1] = '\0';
	CP_Message_Write(text);
}

void CP_Message_ShowIfDebugging(const char* title, const char* text)
{
#if defined(_WIN32) && !defined(NDEBUG)
	if (IsDebuggerPresent())
	{
		MessageBoxA(NULL, text, title, MB_OK | MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND);
	}
#else
	(void)title;
	(void)text;
#endif
}
