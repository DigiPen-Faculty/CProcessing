//------------------------------------------------------------------------------
// file:	CP_File.c
// author:	Daniel Hamilton
// brief:	Helpful file IO functions  
//
// Copyright © 2019 DigiPen, All rights reserved.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include Files:
//------------------------------------------------------------------------------


#include <errno.h>
#include "Internal_Platform.h"	// <windows.h>, CP_PATH_MAX, CP_StringCopy
#include "Internal_File.h"

#include <sys/stat.h> // stat

#if defined(__APPLE__)
	#include <mach-o/dyld.h>	// _NSGetExecutablePath
	#include <stdlib.h>		// realpath
#endif

#if defined(_WIN32)
	#include <stdlib.h>   // _access
	#include <io.h>       // _access
	#include <direct.h>   // _mkdir

	/* Values for the second argument to access.
	These may be OR'd together.  */
	#ifndef R_OK
		#define R_OK    4       /* Test for read permission.  */
	#endif
	#ifndef W_OK
		#define W_OK    2       /* Test for write permission.  */
	#endif
	//#define   X_OK    1       /* execute permission - unsupported in windows*/
	#ifndef F_OK
		#define F_OK    0       /* Test for existence.  */
	#endif

	#define access _access
	#define CP_MAKE_DIR(path) _mkdir(path)
#else
	#include <unistd.h>   // access
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

int file_exists(const char * filepath)
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

int file_dirExists(const char * dirpath)
{
    struct stat s;
    int err = stat(dirpath, &s);
    if(-1 == err) {
        if(ENOENT == errno) {
            /* does not exist */
            return CP_ERROR_NOT_FOUND;
        } else {
            // stat error
            // perror("stat");
            // exit(1);
            return CP_ERROR_INTERNAL;
        }
    } else {
        if(S_ISDIR(s.st_mode)) {
            /* it's a dir */
            return CP_OK;
        } else {
            /* exists but is no dir */
            return CP_ERROR_NOT_DIR;
        }
    }
}

int file_makedir(const char * dirpath)
{
    // _mkdir/mkdir return 0 on success
    if (CP_MAKE_DIR(dirpath) == 0)
    {
        return CP_OK;
    }
    else
    {
        return CP_ERROR_FAILED;
    }
}

//------------------------------------------------------------------------------
// Asset paths:
//		Images, fonts and sounds given a relative path are looked for from the
//		current folder, and then next to the program. A program started from
//		a terminal or a debugger runs in whatever folder it's given, but one
//		started by double-clicking may not run in its own folder: macOS Finder
//		starts it in the home folder, and Linux file managers vary (Windows
//		Explorer does use the program's folder). The build puts the Assets
//		folder next to the program, so looking there finds it either way.

static bool file_isAbsolute(const char * path)
{
#if defined(_WIN32)
    return path[0] == '\\' || path[0] == '/' || (path[0] != '\0' && path[1] == ':');
#else
    return path[0] == '/';
#endif
}

// The folder the program is in, ending with a separator ("" if unknown)
static const char * file_programFolder(void)
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
    char * end = strrchr(path, '/');
#if defined(_WIN32)
    char * backslash = strrchr(path, '\\');
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

const char * file_resolveAssetPath(const char * filepath, char * buffer, size_t bufferSize)
{
    if (filepath == NULL || file_isAbsolute(filepath) || file_exists(filepath) == CP_OK)
    {
        return filepath;
    }
    const char * folder = file_programFolder();
    if (folder[0] == '\0')
    {
        return filepath;
    }
    snprintf(buffer, bufferSize, "%s%s", folder, filepath);
    return file_exists(buffer) == CP_OK ? buffer : filepath;
}
