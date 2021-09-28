//------------------------------------------------------------------------------
// File:	Internal_File.h
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

#define CP_IS_ERROR(A)		( ( A < 0 ) ? true : false )
#define CP_OK				true
#define CP_ERROR_FAILED		false	// specified action failed
#define CP_ERROR_NOT_FOUND	-1		// specified file or directory wasnt found
#define CP_ERROR_NOT_DIR	-2		// checked if a directory exists and it exists, but isnt a directory
#define CP_ERROR_NOT_FILE	-3		// checked if a file exists and it exists, but isnt a file
#define CP_ERROR_INTERNAL	-5		// CProcessing internal error, its not your fault, its mine

///////////////////////////////////////////////
// Justin, please ignore this.
#if defined(WIN32) || defined(WIN64)

// Copied from linux libc sys/stat.h:
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif
///////////////////////////////////////////////

//------------------------------------------------------------------------------
// Public Functions:
//------------------------------------------------------------------------------

// Check if a file exist
	int file_exists(const char* filepath);

	// Check if a folder exists
	int file_dirExists(const char* dirpath);

	// Make a directory
	int file_makedir(const char* dirpath);

#ifdef __cplusplus
}
#endif
