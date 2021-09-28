//---------------------------------------------------------
// File:	Internal_System.h
// Author:	Justin Chambers
// Brief:	Primary internal header file for the CProcessing interface
//			Include other internal only resources and headers.
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

#include "glad.h"
#include "glfw3.h"

	// NanoVG
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg.h"

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
		unsigned short window_width;
		unsigned short window_height;
		unsigned short canvas_width;
		unsigned short canvas_height;
		unsigned short native_width;
		unsigned short native_height;
		bool isFullscreen;
		float pixel_ratio;
		unsigned short window_posX;
		unsigned short window_posY;
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
