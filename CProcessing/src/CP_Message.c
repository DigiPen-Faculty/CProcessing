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
#include <math.h>
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

bool CP_Message_FirstTimeFor(const char* what, const char* name)
{
	// Each pair is kept as a 64-bit FNV-1a hash of what, a separator and
	// name. After this many pairs, further ones aren't printed: a program
	// with that many missing files has one problem, not hundreds.
	static unsigned long long seen[256];
	static int count = 0;

	unsigned long long hash = 14695981039346656037ULL;
	for (const char* c = what ? what : ""; *c; ++c)
	{
		hash = (hash ^ (unsigned char)*c) * 1099511628211ULL;
	}
	hash = (hash ^ 0xFFu) * 1099511628211ULL;
	for (const char* c = name ? name : ""; *c; ++c)
	{
		hash = (hash ^ (unsigned char)*c) * 1099511628211ULL;
	}

	for (int i = 0; i < count; ++i)
	{
		if (seen[i] == hash)
		{
			return false;
		}
	}
	if (count == (int)(sizeof(seen) / sizeof(seen[0])))
	{
		return false;
	}
	seen[count++] = hash;
	return true;
}

bool CP_Message_CheckFinite(const char* function, const char* names, const float* values, int count)
{
	for (int i = 0; i < count; ++i)
	{
		if (isfinite(values[i]))
		{
			continue;
		}
		if (CP_Message_FirstTimeFor("not finite", function))
		{
			// the i-th name in "x, y, w, h"
			const char* name = names;
			for (int skip = i; skip > 0 && name; --skip)
			{
				name = strchr(name, ',');
				name = name ? name + 1 : NULL;
			}
			name = name ? name : "?";
			while (*name == ' ')
			{
				++name;
			}
			int length = 0;
			while (name[length] && name[length] != ',')
			{
				++length;
			}
			CP_Warn("%s's %.*s %s, so the call was ignored. That usually comes from a division by zero: print the numbers it's made from to find it.",
				function, length, name, isnan(values[i]) ? "isn't a number (NaN)" : "is infinite");
		}
		return false;
	}
	return true;
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
