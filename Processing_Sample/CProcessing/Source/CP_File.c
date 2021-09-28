//------------------------------------------------------------------------------
// File:	CP_File.c
// Author:	Daniel Hamilton
// Brief:	Helpful file IO functions
//
// GitHub Repository:
// https://github.com/DigiPen-Faculty/CProcessing
//
//---------------------------------------------------------
// MIT License
//
// Copyright (C) 2021 DigiPen Institute of Technology
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//---------------------------------------------------------

//------------------------------------------------------------------------------
// Include Files:
//------------------------------------------------------------------------------

#include <errno.h>
#include "Internal_File.h"

// for access
#ifndef _UNISTD_H
#define _UNISTD_H    1
#endif
#include <stdlib.h>   // _access
#include <io.h>       // _access
#include <sys/stat.h> // stat
#include <direct.h>   // _mkdir

//------------------------------------------------------------------------------
// Defines:
//------------------------------------------------------------------------------

/* Values for the second argument to access.
These may be OR'd together.  */
#define R_OK    4       /* Test for read permission.  */
#define W_OK    2       /* Test for write permission.  */

//#define   X_OK    1       /* execute permission - unsupported in windows*/
#define F_OK    0       /* Test for existence.  */

#define access _access

//------------------------------------------------------------------------------
// Private Functions:
//------------------------------------------------------------------------------

int file_exists(const char* filepath)
{
	if (access(filepath, F_OK) != -1)
		return CP_OK;
	return CP_ERROR_NOT_FOUND;
}

int file_dirExists(const char* dirpath)
{
	struct stat s;
	int err = stat(dirpath, &s);
	if (-1 == err)
		if (ENOENT == errno)

			// does not exist
			return CP_ERROR_NOT_FOUND;
		else
		{
			// stat error
			// perror("stat");
			// exit(1);
			return CP_ERROR_INTERNAL;
		}
	else if (S_ISDIR(s.st_mode))
		/* it's a dir */
		return CP_OK;
	/* exists but is no dir */
	return CP_ERROR_NOT_DIR;
}

int file_makedir(const char* dirpath)
{
	if (_mkdir(dirpath))
		return CP_OK;
	return CP_ERROR_FAILED;
}
