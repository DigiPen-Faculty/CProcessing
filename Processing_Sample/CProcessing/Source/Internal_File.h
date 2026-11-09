//------------------------------------------------------------------------------
// file:	Internal_File.h
// author:	Daniel Hamilton
// brief:	Helpful file IO functions
//
// INTERNAL USE ONLY, DO NOT DISTRIBUTE
//
// Copyright © 2019 DigiPen, All rights reserved.
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

#define CP_IS_ERROR(A)      ( ( A < 0 ) ? true : false )
#define CP_OK               true
#define CP_ERROR_FAILED     false // specified action failed
#define CP_ERROR_NOT_FOUND -1 // specified file or directory wasnt found
#define CP_ERROR_NOT_DIR   -2 // checked if a directory exists and it exists, but isnt a directory
#define CP_ERROR_NOT_FILE  -3 // checked if a file exists and it exists, but isnt a file
#define CP_ERROR_INTERNAL  -5 // CProcessing internal error, its not your fault, its mine

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

// Check if a file exist
int file_exists(const char * filepath);
// Check if a folder exists
int file_dirExists(const char * dirpath);
// Make a directory
int file_makedir(const char * dirpath);
// The path to open an image, font or sound file with: the path as given if
// the file is there (an absolute path, or relative to the current folder);
// else, for a relative path, the same path next to the program, written into
// buffer, if the file is there. Unchanged if it's in neither place.
const char * file_resolveAssetPath(const char * filepath, char * buffer, size_t bufferSize);

#ifdef __cplusplus
}
#endif
