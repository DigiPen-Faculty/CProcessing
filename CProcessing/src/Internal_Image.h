//------------------------------------------------------------------------------
// file:	Interal_Image.h
// author:	Daniel Hamilton
// brief:	API for loading and displaying images
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

#include "Internal_Platform.h"

//------------------------------------------------------------------------------
// Defines:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Consts:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Structures:
//------------------------------------------------------------------------------

typedef struct CP_Image_Struct
{
	int handle;                 // handle to the nanoVG image
	char filepath[CP_PATH_MAX]; // full path of the image
	int w;                      // width of the image
	int h;                      // height of the image
	int load_error;             // was there an error loading the image
	int ref_count;              // loads of this file not yet freed (CP_Image_Free frees at 0)
	int drawable;               // made with CP_Image_Create: it can be drawn into
	unsigned framebuffer;       // where drawing into it goes (made when it is first drawn into)
	unsigned depthStencil;
} CP_Image_Struct;

//------------------------------------------------------------------------------
// Public Enums:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Variables:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Functions:
//------------------------------------------------------------------------------

void CP_Image_Init(void);
void CP_Image_Update(void);
void CP_Image_Shutdown(void);

// Writes width * height RGBA pixels, row by row from the top left, to a PNG
// file, with stb_image_write. Returns 0 if the file couldn't be written.
// (third_party/stb/stb_image_write.c)
int CP_WritePNG(const char* filepath, int width, int height, const void* rgbaPixels);

#ifdef __cplusplus
}
#endif
