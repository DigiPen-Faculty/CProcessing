//------------------------------------------------------------------------------
// file:	CP_File.c
// author:	Daniel Hamilton
// brief:	Helpful file IO functions
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology and CProcessing contributors
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include Files:
//------------------------------------------------------------------------------


#include <errno.h>
#include "Internal_Platform.h" // <windows.h>, CP_PATH_MAX, CP_StringCopy
#include "Internal_File.h"
#include "Internal_Message.h"

#include <sys/stat.h> // stat

#if defined(__APPLE__)
	#include <mach-o/dyld.h> // _NSGetExecutablePath
	#include <stdlib.h>      // realpath
#endif

#if defined(_WIN32)
	#include <stdlib.h> // _access
	#include <io.h>     // _access
	#include <direct.h> // _mkdir

	/* Values for the second argument to access.
	These may be OR'd together.  */
	#ifndef R_OK
		#define R_OK 4 /* Test for read permission.  */
	#endif
	#ifndef W_OK
		#define W_OK 2 /* Test for write permission.  */
	#endif
	// #define   X_OK    1       /* execute permission - unsupported in windows*/
	#ifndef F_OK
		#define F_OK 0 /* Test for existence.  */
	#endif

	#define access            _access
	#define getcwd            _getcwd
	#define CP_MAKE_DIR(path) _mkdir(path)
#else
	#include <unistd.h> // access
	#define CP_MAKE_DIR(path) mkdir(path, 0755)
#endif

//------------------------------------------------------------------------------
// Private Consts:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Private Structures:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Variables:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Private Variables:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Private Function Declarations:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Functions:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Private Functions:
//------------------------------------------------------------------------------

static int file_exists(const char* filepath)
{
	if (access(filepath, F_OK) != -1)
	{
		return CP_OK;
	}
	else
	{
		return CP_ERROR_NOT_FOUND;
	}
}

//------------------------------------------------------------------------------
// Asset paths:
//		Images, fonts and sounds given a relative path are looked for from the
//		current folder, and then next to the program. A program started from
//		a terminal or a debugger runs in whatever folder it's given, but one
//		started by double-clicking may not run in its own folder: macOS Finder
//		starts it in the home folder, and Linux file managers vary (Windows
//		Explorer does use the program's folder). The build puts the assets
//		folder next to the program, so looking there finds it either way.

static bool file_isAbsolute(const char* path)
{
#if defined(_WIN32)
	return path[0] == '\\' || path[0] == '/' || (path[0] != '\0' && path[1] == ':');
#else
	return path[0] == '/';
#endif
}

// The folder the program is in, ending with a separator ("" if unknown)
static const char* file_programFolder(void)
{
	static char folder[CP_PATH_MAX];
	static bool looked = false;
	if (looked)
	{
		return folder;
	}
	looked = true;

	char path[CP_PATH_MAX] = "";
#if defined(_WIN32)
	// narrow (ANSI) path, matching the fopen calls that open the files
	DWORD length = GetModuleFileNameA(NULL, path, CP_PATH_MAX);
	if (length == 0 || length >= CP_PATH_MAX)
	{
		path[0] = '\0';
	}
#elif defined(__APPLE__)
	char unresolved[CP_PATH_MAX];
	uint32_t size = CP_PATH_MAX;
	if (_NSGetExecutablePath(unresolved, &size) != 0 || realpath(unresolved, path) == NULL)
	{
		path[0] = '\0';
	}
#else
	ssize_t length = readlink("/proc/self/exe", path, CP_PATH_MAX - 1);
	path[length > 0 ? length : 0] = '\0';
#endif

	// keep everything up to and including the last separator
	char* end = strrchr(path, '/');
#if defined(_WIN32)
	char* backslash = strrchr(path, '\\');
	if (backslash && (!end || backslash > end))
	{
		end = backslash;
	}
#endif
	if (end)
	{
		end[1] = '\0';
	}
	else
	{
		path[0] = '\0';
	}
	CP_StringCopy(folder, sizeof(folder), path);
	return folder;
}

const char* file_resolveAssetPath(const char* filepath, char* buffer, size_t bufferSize)
{
	if (filepath == NULL || file_isAbsolute(filepath) || file_exists(filepath) == CP_OK)
	{
		return filepath;
	}
	const char* folder = file_programFolder();
	if (folder[0] == '\0')
	{
		return filepath;
	}
	snprintf(buffer, bufferSize, "%s%s", folder, filepath);
	return file_exists(buffer) == CP_OK ? buffer : filepath;
}

//------------------------------------------------------------------------------
// Messages about loading (DESIGN.md, "Messages"):
//		each names the file, once per file, so a program with three
//		misspelled names hears about all three in one run.

// The current folder, ending with a separator ("" if unknown)
static void file_currentFolder(char* buffer, int bufferSize)
{
	if (getcwd(buffer, bufferSize - 1) == NULL)
	{
		buffer[0] = '\0';
		return;
	}
	size_t length = strlen(buffer);
#if defined(_WIN32)
	const char separator = '\\';
#else
	const char separator = '/';
#endif
	if (length > 0 && buffer[length - 1] != '/' && buffer[length - 1] != separator)
	{
		buffer[length] = separator;
		buffer[length + 1] = '\0';
	}
}

void file_warnLoadFailed(const char* function, const char* filepath, const char* kind, const char* formats)
{
	if (filepath == NULL || filepath[0] == '\0')
	{
		if (CP_Message_FirstTimeFor(function, ""))
		{
			CP_Warn("%s was given no file name, so it loaded nothing. Give it a path such as \"assets/name\".", function);
		}
		return;
	}
	if (!CP_Message_FirstTimeFor(function, filepath))
	{
		return;
	}

	char resolved[CP_PATH_MAX];
	const char* path = file_resolveAssetPath(filepath, resolved, sizeof(resolved));
	if (file_exists(path) == CP_OK)
	{
		CP_Warn("%s found \"%s\" but couldn't read it as %s. It reads %s: check that the file is one of those (renaming a file doesn't change what's inside).", function, filepath, kind, formats);
		return;
	}

	const char* advice = "Check the spelling, upper and lower case included (Linux and macOS care), and that the file is there.";
	if (file_isAbsolute(filepath))
	{
		CP_Warn("%s couldn't find \"%s\". %s", function, filepath, advice);
		return;
	}
	char current[CP_PATH_MAX];
	file_currentFolder(current, (int)sizeof(current));
	const char* program = file_programFolder();
	if (program[0] == '\0' || strcmp(current, program) == 0)
	{
		CP_Warn("%s couldn't find \"%s\" in %s. %s", function, filepath, current, advice);
	}
	else
	{
		CP_Warn("%s couldn't find \"%s\" in the current folder (%s) or next to the program (%s). %s", function, filepath, current, program, advice);
	}
}

void file_warnLoadedOften(const char* function, const char* freeFunction, const char* filepath, int loads)
{
	if (loads == CP_LOADS_BEFORE_MESSAGE)
	{
		CP_Warn("%s has loaded \"%s\" %d times without a %s. Is it loaded in an update function? Load it once, in an init function, and free it in the exit function.", function, filepath, loads, freeFunction);
	}
}
