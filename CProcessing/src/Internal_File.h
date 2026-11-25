//------------------------------------------------------------------------------
// file:	Internal_File.h
// author:	Daniel Hamilton
// brief:	Helpful file IO functions
//
// Internal header: not part of the public API.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology and CProcessing contributors
//------------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Include Files:
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdbool.h>

//------------------------------------------------------------------------------
// Defines:
//------------------------------------------------------------------------------

#define CP_IS_ERROR(A)     ((A) < 0)
#define CP_OK              true
#define CP_ERROR_FAILED    false // specified action failed
#define CP_ERROR_NOT_FOUND -1    // specified file or directory wasnt found
#define CP_ERROR_NOT_DIR   -2    // checked if a directory exists and it exists, but isnt a directory
#define CP_ERROR_NOT_FILE  -3    // checked if a file exists and it exists, but isnt a file
#define CP_ERROR_INTERNAL  -5    // CProcessing internal error, its not your fault, its mine

///////////////////////////////////////////////
// Please ignore this Justin
#if defined(_WIN32)
	// Copied from linux libc sys/stat.h:
	#ifndef S_ISREG
		#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
	#endif
	#ifndef S_ISDIR
		#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
	#endif
#endif
///////////////////////////////////////////////

//------------------------------------------------------------------------------
// Public Consts:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Structures:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Enums:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Variables:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Functions:
//------------------------------------------------------------------------------

// The path to open an image, font or sound file with: the path as given if
// the file is there (an absolute path, or relative to the current folder);
// else, for a relative path, the same path next to the program, written into
// buffer, if the file is there. Unchanged if it's in neither place.
const char* file_resolveAssetPath(const char* filepath, char* buffer, size_t bufferSize);

// Says why a load gave NULL (DESIGN.md, "Messages"), once per file: no file
// name, a file that isn't there (and the folders it was looked for in), or
// one that is there but can't be read. function is the public function
// ("CP_Image_Load"), kind what it reads ("an image"), formats the kinds of
// file it reads ("PNG, JPEG, BMP and TGA files").
void file_warnLoadFailed(const char* function, const char* filepath, const char* kind, const char* formats);

// A file loaded this many times without being freed is probably loaded on
// every frame. file_warnLoadedOften says so when loads reaches it.
#define CP_LOADS_BEFORE_MESSAGE 1000
void file_warnLoadedOften(const char* function, const char* freeFunction, const char* filepath, int loads);

#ifdef __cplusplus
}
#endif
