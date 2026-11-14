//---------------------------------------------------------
// file:	Internal_System.h
// author:	Justin Chambers
// brief:	Primary internal header file for the CProcessing interface
//			Include other internal only resources and headers.
//
// Internal header: not part of the public API.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology
//---------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "Internal_Platform.h"
#include "glad.h"
#include "glfw3.h"

// NanoVG and its OpenGL 3 back end: declarations only. The back end itself
// is compiled once, in nanovg_gl3.c.
#define NANOVG_GL3 1
#include "nanovg.h"
#include "nanovg_gl.h"

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
    // Copies of the settings NanoVG keeps privately. They let CP_Settings_
    // functions work before CP_Engine_Run, when there's no NanoVG context
    // yet (CP_Settings_ApplyAll gives them to NanoVG when the window opens),
    // and CP_Graphics_DrawPoint uses the colors, weight and cap.
    CP_Color fill_color;
    CP_Color stroke_color;
    float stroke_weight;
    CP_LINE_CAP_MODE line_cap;
    CP_LINE_JOINT_MODE line_joint;
    CP_Color tint;
    CP_BOOL anti_alias;
    CP_BLEND_MODE blend_mode;
    CP_IMAGE_FILTER_MODE filter_mode;
    CP_IMAGE_WRAP_MODE wrap_mode;
    float text_size;
    int text_align;
    CP_Font font;           // NULL until CP_Font_Set: the default font
} CP_DrawInfo;
typedef CP_DrawInfo* CP_DrawInfoPtr;

// The settings a program starts with (CP_Setting.c)
void CP_Settings_InitDefaults(CP_DrawInfoPtr DI);
// Gives NanoVG the current settings, once its context exists (CP_Setting.c)
void CP_Settings_ApplyAll(void);
//////////////////

//////////////////
typedef struct CP_Core
{
    NVGcontext* nvg;
    GLFWwindow* window;
	CP_WindowHandle hwnd;
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
void CP_DeferredSetWindowSizeInternal(int new_width, int new_height, bool isFullscreen);

void CP_Initialize(void);
void CP_Update(void);
void CP_Shutdown(void);

//////////////////
// A message for the programmer, built a line at a time: at exit, the assets
// that were never freed (CP_System.c)
typedef struct CP_Report
{
	char text[4096];
	size_t length;
	int lines;
	CP_BOOL truncated;	// more lines than fit
} CP_Report;

void CP_Report_AddLine(CP_Report* report, const char* format, ...);
// "  image  Assets/logo.png: 3 loads not freed", with a hint when there are many
void CP_Report_AddAsset(CP_Report* report, const char* kind, const char* filepath, int loads);

// Each adds a line to the report for every asset still loaded
// (CP_Image.c, CP_Sound.c, CP_Text.c)
void CP_Image_ReportLoaded(CP_Report* report);
void CP_Sound_ReportLoaded(CP_Report* report);
void CP_Text_ReportLoaded(CP_Report* report);
//////////////////
void CP_FrameStart(void);

// Reading the picture on screen (CP_Image_Screenshot): bind it for
// glReadPixels, then go back to drawing. NanoVG's frame must be finished
// first. (CP_System.c)
void CP_Canvas_BindForReading(void);
void CP_Canvas_EndReading(void);
// Does the layer being drawn on hold premultiplied alpha? (A state pushed on
// top of another draws on one; CP_Graphics_ClearBackground needs to know.)
CP_BOOL CP_Canvas_IsPremultiplied(void);
// Forget a freed font in every game state's saved settings (CP_Text.c)
void CP_GameStates_ForgetFont(CP_Font font);
void CP_ApplyWindowSizeChosenInInit(void);
void CP_FrameEnd(void);

void CP_FrameRate_Init(void);
void CP_FrameRate_FrameStart(void);
void CP_FrameRate_FrameEnd(void);
void CP_UpdateFrameTime(void);
void CP_IncFrameCount(void);

#ifdef __cplusplus
}
#endif
