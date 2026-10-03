//------------------------------------------------------------------------------
// file:	Internal_Platform.h
// brief:	Small portability layer for the library internals.
//
//			Everything the library used to pick up implicitly from <windows.h>
//			(MAX_PATH, UNREFERENCED_PARAMETER, the *_s string functions) is
//			provided here instead, so the internals compile the same way on
//			Windows, Linux and macOS. The public headers include no OS headers.
//
// INTERNAL USE ONLY, DO NOT DISTRIBUTE
//
// Copyright (c) 2026 DigiPen, All rights reserved.
//------------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// On Windows the internals include <windows.h> privately (GLFW's native
// window header needs it), always before GLAD/GLFW so the macros they share
// (APIENTRY, ...) are defined once, by <windows.h>.
#if defined(_WIN32)
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#endif

//------------------------------------------------------------------------------
// Path length used for the internal filepath caches (images, sounds, fonts).
// Long enough for Linux/macOS PATH_MAX and Windows long paths alike, so real
// paths are never truncated.
#define CP_PATH_MAX 4096

#ifndef UNREFERENCED_PARAMETER
	#define UNREFERENCED_PARAMETER(P) ((void)(P))
#endif

//------------------------------------------------------------------------------
// Bounded string copy that always NUL-terminates.
// Replaces strcpy_s, which is MSVC-only (C11 Annex K is not provided by glibc
// or Apple's libc). On overflow the destination is truncated rather than the
// process being aborted the way MSVC's invalid-parameter handler would.
static inline void CP_StringCopy(char* dest, size_t destSize, const char* src)
{
	if (dest == NULL || destSize == 0)
	{
		return;
	}
	if (src == NULL)
	{
		dest[0] = '\0';
		return;
	}
	size_t length = strlen(src);
	if (length >= destSize)
	{
		length = destSize - 1;
	}
	memcpy(dest, src, length);
	dest[length] = '\0';
}

#ifdef __cplusplus
}
#endif
