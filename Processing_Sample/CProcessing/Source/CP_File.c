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
#include "Internal_File.h"

#include <sys/stat.h> // stat

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
