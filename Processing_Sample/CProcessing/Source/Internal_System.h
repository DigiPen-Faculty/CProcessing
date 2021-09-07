//---------------------------------------------------------
// file:	Internal_System.h
// author:	Justin Chambers
// brief:	Primary internal header file for the CProcessing interface
//			Include other internal only resources and headers.
//
// INTERNAL USE ONLY, DO NOT DISTRIBUTE
//
// Copyright © 2019 DigiPen, All rights reserved.
//---------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "glad.h"
#include "glfw3.h"

// NanoVG
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg.h"

#include "Internal_Color.h"
#include "Internal_File.h"
#include "Internal_Image.h"
#include "Internal_Input.h"
#include "Internal_Math.h"
#include "Internal_Random.h"
#include "Internal_Noise.h"
#include "Internal_Sound.h"
#include "Internal_Text.h"

typedef struct GLFWwindow GLFWwindow;
typedef struct NVGcontext NVGcontext;
typedef struct NVGcolor NVGcolor;

#define CP_MAX_STATES 32	// this should match NVG_MAX_STATES


//////////////////
typedef struct CP_DrawInfo
{
    CP_POSITION_MODE rect_mode;
    CP_POSITION_MODE ellipse_mode;
    CP_POSITION_MODE image_mode;
    int stroke;
    int fill;
    CP_Matrix camera;
} CP_DrawInfo;
typedef CP_DrawInfo* CP_DrawInfoPtr;
//////////////////

//////////////////
typedef struct CP_Core
{
    NVGcontext* nvg;
    GLFWwindow* window;
	HWND hwnd;
	int window_width;
	int window_height;
    int canvas_width;
    int canvas_height;
	int native_width;
	int native_height;
	bool isFullscreen;
    float pixel_ratio;
    int window_posX;
    int window_posY;
	CP_DrawInfo states[CP_MAX_STATES];
	int nstates;
} CP_Core;
typedef CP_Core* CP_CorePtr;
//////////////////

CP_CorePtr GetCPCore(void);
CP_DrawInfoPtr GetDrawInfo(void);

void CP_SetWindowSizeInternal(int new_width, int new_height, bool isFullscreen);

void CP_Initialize(void);
void CP_Update(void);
void CP_Shutdown(void);
void CP_FrameStart(void);
void CP_FrameEnd(void);

void CP_FrameRate_Init(void);
void CP_FrameRate_FrameStart(void);
void CP_FrameRate_FrameEnd(void);
void CP_UpdateFrameTime(void);
void CP_IncFrameCount(void);

#ifdef __cplusplus
}
#endif
