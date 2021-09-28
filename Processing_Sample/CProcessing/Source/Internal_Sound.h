//------------------------------------------------------------------------------
// File:	Internal_Sound.h
// Author:	Daniel Hamilton, Andrea Ellinger
// Brief:	Internal structs and functions for Sound
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

#include "fmod.h"

//------------------------------------------------------------------------------
// Public Structures:
//------------------------------------------------------------------------------

	typedef struct CP_Sound_Struct
	{
		char filepath[MAX_PATH];
		FMOD_SOUND* sound;
	} CP_Sound_Struct;

	//------------------------------------------------------------------------------
	// Public Functions:
	//------------------------------------------------------------------------------

	void CP_Sound_Init(void);
	void CP_Sound_Update(void);
	void CP_Sound_Shutdown(void);

#ifdef __cplusplus
}
#endif
