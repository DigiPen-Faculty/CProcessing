//------------------------------------------------------------------------------
// file:	Internal_Sound.h
// author:	Daniel Hamilton, Andrea Ellinger, Justin Chambers
// brief:	Internal structs and functions for Sound
//
// Internal header: not part of the public API.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology
//------------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Include Files:
//------------------------------------------------------------------------------

#include "Internal_Platform.h"
#include "soloud_c.h"

//------------------------------------------------------------------------------
// Defines:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Consts:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Enums:
//------------------------------------------------------------------------------

//---------------------------------------------------------
// AUDIO SOURCE TYPE:
//		Audio source type manages streaming content
typedef enum SL_AUDIOSOURCE_TYPE
{
	SL_AUDIOSOURCE_WAV = 0,		// Default
	SL_AUDIOSOURCE_STREAM = 1
} SL_AUDIOSOURCE_TYPE;

//------------------------------------------------------------------------------
// Public Structures:
//------------------------------------------------------------------------------

typedef struct CP_Sound_Struct
{
	char filepath[CP_PATH_MAX];
    AudioSource* sound;
	SL_AUDIOSOURCE_TYPE type;
	int ref_count;		// loads of this file not yet freed (CP_Sound_Free frees at 0)
} CP_Sound_Struct;

typedef struct CP_VoiceGroup_Struct
{
	unsigned int handle;
	float volume;
	float pitch;
} CP_VoiceGroup_Struct;

//------------------------------------------------------------------------------
// Public Variables:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Functions:
//------------------------------------------------------------------------------

void CP_Sound_Init(void);
void CP_Sound_Update(void);
void CP_Sound_Shutdown(void);

#ifdef __cplusplus
}
#endif
