//---------------------------------------------------------
// file:	CProcessing.c
// author:	Justin Chambers
// brief:	Primary implementation of the CProcessing interface
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology
//---------------------------------------------------------

#include "cprocessing.h"
#include "Internal_System.h"
#include "Internal_Monitor.h"
#include "Internal_GameStateLogic.h"
#include "tinycthread.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#if !defined(_WIN32)
	#include <unistd.h>	// isatty
#endif

// Native window access (used only by CP_System_GetWindowHandle)
#if defined(_WIN32)
	#define GLFW_EXPOSE_NATIVE_WIN32
	#include "glfw3native.h"
#elif defined(__APPLE__)
	#define GLFW_EXPOSE_NATIVE_COCOA
	#include "glfw3native.h"
#elif defined(CP_HAS_X11)
	#define GLFW_EXPOSE_NATIVE_X11
	#include "glfw3native.h"
#endif

#define isRunning !glfwWindowShouldClose(_CORE.window)

// Internal information (everything not listed starts as zero/NULL)
static CP_Core _CORE = {
	.pixel_ratio = 1.0f,
	.window_posX = -1,
	.window_posY = -1
};
static bool _isInitialized = false;

static CP_BOOL _deferredSizeChange = FALSE;
static int _deferredWidth = 0;
static int _deferredHeight = 0;
static CP_BOOL _deferredFullscreen = FALSE;

// Window settings that may be requested before the window exists; they are
// stored here and applied when CP_Initialize creates the window
static CP_CURSOR_MODE _cursorMode = CP_CURSOR_NORMAL;
static char _windowTitle[256] = "CProcessing Application";

static FunctionPtr _preUpdateFunction = NULL;
static FunctionPtr _postUpdateFunction = NULL;

//------------------------------------------------------------------------------
// Private Variables:
//------------------------------------------------------------------------------

// FrameRate Control
static double StartingTime, EndingTime, ElapsedSeconds;
static double _frametimeTarget = 1.0 / 60.0;
static double _frametime = 1.0 / 60.0;

// Window and display options that can be chosen before CP_Engine_Run, and
// are used when the window opens
static CP_BOOL _vsync = FALSE;
static CP_BOOL _windowResizable = FALSE;

// Frames since the start of the program
static unsigned int _frameCount;

static void error_callback_glfw(int error, const char* desc)
{
	printf("GLFW error %d: %s\n", error, desc);
}

// Wayland does not let applications position their own windows (the
// compositor decides), and GLFW reports an error if asked to
static void CP_SetWindowPosIfSupported(GLFWwindow* window, int x, int y)
{
	if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND)
	{
		return;
	}
	glfwSetWindowPos(window, x, y);
}

// A monitor's rectangle in screen coordinates: its position and the size of
// its current video mode
static CP_ScreenRect CP_MonitorRect(GLFWmonitor* monitor)
{
	CP_ScreenRect rect = { 0, 0, 0, 0 };
	const GLFWvidmode* mode = monitor ? glfwGetVideoMode(monitor) : NULL;
	if (mode)
	{
		glfwGetMonitorPos(monitor, &rect.x, &rect.y);
		rect.width = mode->width;
		rect.height = mode->height;
	}
	return rect;
}

// The monitor the window is on: the one it's fullscreen on, or else the one
// with the largest part of the window on it. Fullscreen, the display size and
// centering all use it. The primary monitor before the window exists, on
// Wayland (where a window can't find out its position), and when the window
// isn't on any monitor.
static GLFWmonitor* CP_WindowMonitor(void)
{
	GLFWmonitor* primary = glfwGetPrimaryMonitor();
	if (!_CORE.window)
	{
		return primary;
	}
	GLFWmonitor* fullscreenMonitor = glfwGetWindowMonitor(_CORE.window);
	if (fullscreenMonitor)
	{
		return fullscreenMonitor;
	}
	if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND)
	{
		return primary;
	}

	CP_ScreenRect window = { 0, 0, 0, 0 };
	glfwGetWindowPos(_CORE.window, &window.x, &window.y);
	glfwGetWindowSize(_CORE.window, &window.width, &window.height);

	int count = 0;
	GLFWmonitor** monitors = glfwGetMonitors(&count);
	CP_ScreenRect rects[16];
	count = count < 16 ? count : 16;
	for (int i = 0; i < count; ++i)
	{
		rects[i] = CP_MonitorRect(monitors[i]);
	}
	const int most = CP_Monitor_MostOfWindow(window, rects, count);
	return most >= 0 ? monitors[most] : primary;
}

// Store the size of a monitor's current video mode as the display size
static void CP_UpdateDisplaySize(GLFWmonitor* monitor)
{
	const GLFWvidmode* mode = monitor ? glfwGetVideoMode(monitor) : NULL;
	if (mode)
	{
		_CORE.native_width = mode->width;
		_CORE.native_height = mode->height;
	}
}

// The position that centers the window, title bar and borders included, in
// the part of a monitor windows can use: on macOS below the menu bar and
// above the Dock, on Windows above the taskbar
static void CP_CenterWindowOn(GLFWmonitor* monitor, int width, int height, int* x, int* y)
{
	const CP_ScreenRect monitorRect = CP_MonitorRect(monitor);
	CP_ScreenRect workarea = { 0, 0, 0, 0 };
	if (monitor)
	{
		glfwGetMonitorWorkarea(monitor, &workarea.x, &workarea.y, &workarea.width, &workarea.height);
	}
	// Right after a video mode change, macOS can still report the work area
	// for the old mode, so only the part on the monitor counts
	CP_ScreenRect area = CP_Monitor_Overlap(workarea, monitorRect);
	if (area.width <= 0 || area.height <= 0)
	{
		area = monitorRect;
	}

	CP_ScreenFrame frame = { 0, 0, 0, 0 };
	if (_CORE.window)
	{
		glfwGetWindowFrameSize(_CORE.window, &frame.left, &frame.top, &frame.right, &frame.bottom);
	}
	CP_Monitor_Center(area, width, height, frame, x, y);
}

static void CP_UpdatePixelRatio(void)
{
	if (_CORE.window_width > 0 && _CORE.canvas_width > 0)
	{
		// Calculate pixel ratio for hi-dpi devices.
		_CORE.pixel_ratio = (float)_CORE.canvas_width / (float)_CORE.window_width;
	}
}

// Window and framebuffer sizes are tracked through callbacks because on X11,
// Wayland and macOS a resize/fullscreen request is applied asynchronously by
// the window system: reading the size straight after requesting it can still
// return the old values. Zero sizes (minimized windows) are ignored so the
// last real size and pixel ratio are kept.
static void CP_WindowSizeCallback(GLFWwindow* window, int width, int height)
{
	UNREFERENCED_PARAMETER(window);
	if (width <= 0 || height <= 0)
	{
		return;
	}
	_CORE.window_width = width;
	_CORE.window_height = height;
	CP_UpdatePixelRatio();
}

static void CP_FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	UNREFERENCED_PARAMETER(window);
	if (width <= 0 || height <= 0)
	{
		return;
	}
	_CORE.canvas_width = width;
	_CORE.canvas_height = height;
	CP_UpdatePixelRatio();
	glViewport(0, 0, width, height);
}


//---------------------------------------------------------
// CANVAS AND LAYERS:
//		CProcessing, like Processing, does not clear the screen between frames:
//		whatever was drawn stays until it is drawn over. Drawing goes into a
//		persistent offscreen framebuffer (a "layer") that is copied to a
//		normal double-buffered window at the end of the frame. Screenshots
//		read back from it.
//
//		Each game state on the stack draws into a layer of its own. The
//		bottom state's layer is the canvas a single-state program has always
//		had: it starts with whatever the state before it drew, and is copied
//		straight to the window. A state pushed on top gets a new layer that
//		starts transparent, and is blended over the ones below it. A state
//		that is covered isn't updated, so its layer keeps its last picture:
//		that's how a frozen game shows behind a pause.
//
//		Layers drawn on top hold premultiplied alpha (what NanoVG draws), so
//		CP_Graphics_ClearBackground premultiplies its color there. The bottom
//		layer is copied, not blended, exactly as the single canvas was.
//
//		(CProcessing used to draw straight into a single-buffered window on
//		Windows instead. Single-buffered windows are not available everywhere -
//		Wayland/EGL has none, and macOS core profile contexts are unreliable
//		with them - so a canvas is used on every platform for consistency.)
//
//		After the copy, the window's own framebuffer (0) stays bound through
//		the swap, and the drawing layer is bound again at the start of the
//		next frame: on macOS, a frame swapped while another framebuffer is
//		bound is never displayed, and the window stays black (SDL documents
//		the same for SDL_GL_SwapWindow).

#define CP_MAX_LAYERS (CP_MAX_GAME_STATES * 2 + 6)	// states, states leaving in a transition, and spares

typedef struct CP_Layer
{
	GLuint framebuffer;		// 0 when the layer has no GL objects
	GLuint color;			// texture
	int width;
	int height;
	int image;				// NanoVG image of the texture, made when first blended (0: none yet)
	bool inUse;
} CP_Layer;

static CP_Layer _layers[CP_MAX_LAYERS];
static GLuint _layerDepthStencil = 0;	// shared: NanoVG only needs stencil while it draws
static int _layerWidth = 0;
static int _layerHeight = 0;

static int _drawLayer = -1;				// where drawing goes now
static bool _drawLayerPremultiplied = false;
static bool _nvgFrameOpen = false;		// between the frame's nvgBeginFrame and nvgEndFrame
static int _readLayer = -1;				// a spare layer to put the picture together in, for reading it

static void CP_Layer_DestroyGL(CP_Layer* layer)
{
	if (layer->image && _CORE.nvg)
	{
		nvgDeleteImage(_CORE.nvg, layer->image);	// NVG_IMAGE_NODELETE: the texture stays
	}
	if (layer->framebuffer)
	{
		glDeleteFramebuffers(1, &layer->framebuffer);
		glDeleteTextures(1, &layer->color);
	}
	layer->framebuffer = layer->color = 0;
	layer->image = 0;
	layer->width = layer->height = 0;
}

static bool CP_Layer_CreateGL(CP_Layer* layer, int width, int height)
{
	GLuint color = 0, framebuffer = 0;
	glGenTextures(1, &color);
	glBindTexture(GL_TEXTURE_2D, color);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _layerDepthStencil);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("CProcessing: could not create a %d x %d drawing canvas.\n", width, height);
		glDeleteFramebuffers(1, &framebuffer);
		glDeleteTextures(1, &color);
		return false;
	}

	glDisable(GL_SCISSOR_TEST);
	glViewport(0, 0, width, height);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	layer->framebuffer = framebuffer;
	layer->color = color;
	layer->width = width;
	layer->height = height;
	layer->image = 0;
	return true;
}

// Make drawing go to the layer that should get it now
static void CP_Canvas_Bind(void)
{
	if (_drawLayer >= 0 && _layers[_drawLayer].framebuffer)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, _layers[_drawLayer].framebuffer);
		glViewport(0, 0, _layers[_drawLayer].width, _layers[_drawLayer].height);
	}
}

// Draw into another layer from now on, finishing what was drawn so far
static void CP_SetDrawLayer(int layer, bool premultiplied)
{
	_drawLayerPremultiplied = premultiplied;
	if (layer == _drawLayer)
	{
		return;
	}
	if (_nvgFrameOpen)
	{
		nvgEndFrame(_CORE.nvg);
	}
	_drawLayer = layer;
	CP_Canvas_Bind();
	if (_nvgFrameOpen)
	{
		nvgBeginFrame(_CORE.nvg, (float)_CORE.window_width, (float)_CORE.window_height, _CORE.pixel_ratio);
	}
}

// A layer that isn't in use, at the canvas size, cleared to a color
static int CP_Layer_Acquire(float r, float g, float b, float a)
{
	int found = -1;
	for (int i = 0; i < CP_MAX_LAYERS && found < 0; ++i)
	{
		if (!_layers[i].inUse && _layers[i].framebuffer)
		{
			found = i;		// one with GL objects already: reuse it
		}
	}
	for (int i = 0; i < CP_MAX_LAYERS && found < 0; ++i)
	{
		if (!_layers[i].inUse)
		{
			found = i;
		}
	}
	if (found < 0)
	{
		printf("CProcessing: too many drawing layers in use.\n");
		return -1;
	}

	CP_Layer* layer = &_layers[found];
	if (!layer->framebuffer && !CP_Layer_CreateGL(layer, _layerWidth, _layerHeight))
	{
		CP_Canvas_Bind();
		return -1;
	}
	layer->inUse = true;

	glBindFramebuffer(GL_FRAMEBUFFER, layer->framebuffer);
	glViewport(0, 0, layer->width, layer->height);
	glDisable(GL_SCISSOR_TEST);
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	CP_Canvas_Bind();
	return found;
}

static void CP_Layer_Release(int layer)
{
	if (layer >= 0 && layer < CP_MAX_LAYERS)
	{
		_layers[layer].inUse = false;
	}
}

static void CP_Layer_Copy(int from, int to)
{
	if (from < 0 || to < 0 || !_layers[from].framebuffer || !_layers[to].framebuffer)
	{
		return;
	}
	glBindFramebuffer(GL_READ_FRAMEBUFFER, _layers[from].framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _layers[to].framebuffer);
	glDisable(GL_SCISSOR_TEST);
	glBlitFramebuffer(0, 0, _layers[from].width, _layers[from].height,
		0, 0, _layers[to].width, _layers[to].height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	CP_Canvas_Bind();
}

// The NanoVG image of a layer's texture, for blending it
static int CP_Layer_Image(int layer)
{
	CP_Layer* l = &_layers[layer];
	if (!l->image)
	{
		// GL textures are bottom-up, and NanoVG draws premultiplied alpha
		l->image = nvglCreateImageFromHandleGL3(_CORE.nvg, l->color, l->width, l->height,
			(int)NVG_IMAGE_FLIPY | (int)NVG_IMAGE_PREMULTIPLIED | (int)NVG_IMAGE_NODELETE);
	}
	return l->image;
}

static void CP_Canvas_Destroy(void)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	for (int i = 0; i < CP_MAX_LAYERS; ++i)
	{
		CP_Layer_DestroyGL(&_layers[i]);
		_layers[i].inUse = false;
	}
	if (_layerDepthStencil)
	{
		glDeleteRenderbuffers(1, &_layerDepthStencil);
	}
	_layerDepthStencil = 0;
	_layerWidth = _layerHeight = 0;
	_drawLayer = -1;
	_readLayer = -1;
}

// (Re)create every layer in use at the given size, keeping whatever was
// already drawn on it (anchored to the top-left corner, matching
// CProcessing's coordinate origin). The first call makes the first canvas.
static bool CP_Canvas_Resize(int width, int height)
{
	if (width <= 0 || height <= 0)
	{
		return false;
	}
	if (_layerDepthStencil && width == _layerWidth && height == _layerHeight)
	{
		return true;
	}

	GLuint oldDepthStencil = _layerDepthStencil;
	glGenRenderbuffers(1, &_layerDepthStencil);
	glBindRenderbuffer(GL_RENDERBUFFER, _layerDepthStencil);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	bool ok = true;
	for (int i = 0; i < CP_MAX_LAYERS; ++i)
	{
		CP_Layer* layer = &_layers[i];
		if (!layer->inUse)
		{
			CP_Layer_DestroyGL(layer);	// made again at the new size when needed
			continue;
		}
		CP_Layer old = *layer;
		if (!CP_Layer_CreateGL(layer, width, height))
		{
			*layer = old;
			ok = false;
			continue;
		}
		if (old.framebuffer)
		{
			// carry the old drawing over to the new layer
			const int copyWidth = width < old.width ? width : old.width;
			const int copyHeight = height < old.height ? height : old.height;
			glBindFramebuffer(GL_READ_FRAMEBUFFER, old.framebuffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, layer->framebuffer);
			glDisable(GL_SCISSOR_TEST);
			glBlitFramebuffer(0, old.height - copyHeight, copyWidth, old.height,
				0, height - copyHeight, copyWidth, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			CP_Layer_DestroyGL(&old);
		}
	}
	if (oldDepthStencil)
	{
		glDeleteRenderbuffers(1, &oldDepthStencil);
	}
	_layerWidth = width;
	_layerHeight = height;

	if (_drawLayer < 0)
	{
		// The very first canvas: black, until something is drawn
		_drawLayer = CP_Layer_Acquire(0.0f, 0.0f, 0.0f, 1.0f);
		ok = _drawLayer >= 0;
	}
	CP_Canvas_Bind();
	return ok;
}

// One layer of the picture on screen. A layer of -1 is a black cover.
typedef struct CP_ComposeItem
{
	int layer;
	bool bottom;		// a bottom layer: copied, not blended, when it can be
	float alpha;
	float dx;			// offset, in window coordinates
	float dy;
} CP_ComposeItem;

#define CP_MAX_COMPOSE_ITEMS (CP_MAX_LAYERS + 2)

static bool CP_ComposeItem_IsPlainCopy(const CP_ComposeItem* item)
{
	return item->layer >= 0 && item->bottom && item->alpha >= 1.0f && item->dx == 0.0f && item->dy == 0.0f;
}

// Draw the picture made of the items, in order, into a framebuffer
static void CP_Compose(const CP_ComposeItem* items, int count, GLuint target, int targetWidth, int targetHeight)
{
	glBindFramebuffer(GL_FRAMEBUFFER, target);
	glViewport(0, 0, targetWidth, targetHeight);
	glDisable(GL_SCISSOR_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	int first = 0;
	if (count > 0 && CP_ComposeItem_IsPlainCopy(&items[0]))
	{
		// the usual case: a straight copy, anchored to the top left
		const CP_Layer* layer = &_layers[items[0].layer];
		glBindFramebuffer(GL_READ_FRAMEBUFFER, layer->framebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target);
		glBlitFramebuffer(0, 0, layer->width, layer->height,
			0, targetHeight - layer->height, layer->width, targetHeight,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, target);
		first = 1;
	}
	if (first >= count)
	{
		return;
	}

	const float ratio = _CORE.pixel_ratio > 0.0f ? _CORE.pixel_ratio : 1.0f;
	NVGcontext* vg = _CORE.nvg;
	nvgBeginFrame(vg, targetWidth / ratio, targetHeight / ratio, ratio);
	nvgSave(vg);
	nvgReset(vg);
	for (int i = first; i < count; ++i)
	{
		const CP_ComposeItem* item = &items[i];
		const float alpha = item->alpha < 0.0f ? 0.0f : (item->alpha > 1.0f ? 1.0f : item->alpha);
		nvgBeginPath(vg);
		if (item->layer < 0)
		{
			nvgRect(vg, 0.0f, 0.0f, targetWidth / ratio, targetHeight / ratio);
			nvgFillColor(vg, nvgRGBAf(0.0f, 0.0f, 0.0f, alpha));
		}
		else
		{
			const CP_Layer* layer = &_layers[item->layer];
			const float w = layer->width / ratio;
			const float h = layer->height / ratio;
			nvgRect(vg, item->dx, item->dy, w, h);
			nvgFillPaint(vg, nvgImagePattern(vg, item->dx, item->dy, w, h, 0.0f, CP_Layer_Image(item->layer), alpha));
		}
		nvgFill(vg);
	}
	nvgRestore(vg);
	nvgEndFrame(vg);
}

static int CP_GameStates_BuildPicture(CP_ComposeItem* items);

// Copy the picture (every layer that can be seen) to the window's back
// buffer (call before swapping). This leaves the window's framebuffer bound
// for the swap; CP_Canvas_Bind goes back to the drawing layer.
static void CP_Canvas_Present(void)
{
	int windowFbWidth = 0, windowFbHeight = 0;
	glfwGetFramebufferSize(_CORE.window, &windowFbWidth, &windowFbHeight);
	CP_ComposeItem items[CP_MAX_COMPOSE_ITEMS];
	const int count = CP_GameStates_BuildPicture(items);
	CP_Compose(items, count, 0, windowFbWidth, windowFbHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// For reading pixels (CP_Image_Screenshot): bind the picture as it will be
// shown. With one layer that's the layer itself; otherwise the layers are
// put together in a spare layer first. (NanoVG's frame must be finished.)
void CP_Canvas_BindForReading(void)
{
	CP_ComposeItem items[CP_MAX_COMPOSE_ITEMS];
	const int count = CP_GameStates_BuildPicture(items);
	if (count == 1 && CP_ComposeItem_IsPlainCopy(&items[0]))
	{
		glBindFramebuffer(GL_FRAMEBUFFER, _layers[items[0].layer].framebuffer);
		return;
	}
	if (_readLayer < 0)
	{
		_readLayer = CP_Layer_Acquire(0.0f, 0.0f, 0.0f, 1.0f);
		if (_readLayer < 0)
		{
			return;
		}
	}
	CP_Compose(items, count, _layers[_readLayer].framebuffer, _layers[_readLayer].width, _layers[_readLayer].height);
	glBindFramebuffer(GL_FRAMEBUFFER, _layers[_readLayer].framebuffer);
}

static bool _drawingIntoImage = false;
static GLuint _imageFramebuffer = 0;
static int _imageWidth = 0;
static int _imageHeight = 0;
static bool _premultipliedBeforeImage = false;

void CP_Canvas_EndReading(void)
{
	if (_drawingIntoImage)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, _imageFramebuffer);
		glViewport(0, 0, _imageWidth, _imageHeight);
		return;
	}
	CP_Canvas_Bind();
}

void CP_Canvas_ResumeFrame(void)
{
	if (_drawingIntoImage)
	{
		nvgBeginFrame(_CORE.nvg, (float)_imageWidth, (float)_imageHeight, 1.0f);
	}
	else
	{
		nvgBeginFrame(_CORE.nvg, (float)_CORE.window_width, (float)_CORE.window_height, _CORE.pixel_ratio);
	}
}

// Drawing into an image: finish what was drawn so far, and draw into the
// image's framebuffer, one image pixel to one unit, until CP_Canvas_EndImage
void CP_Canvas_BeginImage(unsigned framebuffer, int width, int height)
{
	if (_nvgFrameOpen)
	{
		nvgEndFrame(_CORE.nvg);
	}
	_drawingIntoImage = true;
	_imageFramebuffer = framebuffer;
	_imageWidth = width;
	_imageHeight = height;
	_premultipliedBeforeImage = _drawLayerPremultiplied;
	_drawLayerPremultiplied = true;		// NanoVG draws premultiplied alpha
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glViewport(0, 0, width, height);
	nvgBeginFrame(_CORE.nvg, (float)width, (float)height, 1.0f);
}

void CP_Canvas_EndImage(void)
{
	if (!_drawingIntoImage)
	{
		return;
	}
	nvgEndFrame(_CORE.nvg);
	_drawingIntoImage = false;
	_drawLayerPremultiplied = _premultipliedBeforeImage;
	CP_Canvas_Bind();
	if (_nvgFrameOpen)
	{
		nvgBeginFrame(_CORE.nvg, (float)_CORE.window_width, (float)_CORE.window_height, _CORE.pixel_ratio);
	}
}

CP_BOOL CP_Canvas_IsDrawingIntoImage(void)
{
	return _drawingIntoImage ? TRUE : FALSE;
}

CP_BOOL CP_Canvas_IsPremultiplied(void)
{
	return _drawLayerPremultiplied ? TRUE : FALSE;
}

CP_CorePtr GetCPCore(void)
{
	return &_CORE;
}

CP_DrawInfoPtr GetDrawInfo(void)
{
	// The first draw state gets its defaults the first time it's needed,
	// which can be a CP_Settings_ call before CP_Engine_Run
	if (_CORE.nstates == 0)
	{
		_CORE.nstates = 1;
		CP_Settings_InitDefaults(&_CORE.states[0]);
	}
	return &_CORE.states[_CORE.nstates - 1];
}


//---------------------------------------------------------
// GAME STATES:
//		A stack of states (Internal_GameStateLogic.h has the rules). Each
//		state in it has a slot here with what the engine keeps for it: its
//		layer, its drawing settings while another state draws, and its time.

typedef struct CP_StateSlot
{
	int layer;					// where it draws
	bool bottom;				// the bottom state: its layer is the canvas
	float time;					// seconds it has been updated
	CP_DrawInfo settings;		// its settings, while another state's are in use
	CP_Vector mouseWhenCovered;	// what it sees of the mouse while it runs under another state
	CP_Vector worldMouseWhenCovered;
} CP_StateSlot;

static CP_GSL_Stack _stack;
static CP_StateSlot _slots[CP_MAX_GAME_STATES];
static int _inheritLayer = -1;				// the picture the next bottom state starts with
static CP_StateSlot* _settingsOwner = NULL;	// the state whose settings are in use
static int _runningSlot = -1;				// the slot whose function is running

//---------------------------------------------------------
// Transitions. While one plays, the states that left are "ghosts": each
// keeps its layer, the one that was on top keeps running (without input),
// and their exits run when the transition ends.

typedef struct CP_Ghost
{
	CP_GSL_State state;
	CP_StateSlot slot;
	bool running;
} CP_Ghost;

typedef struct CP_TransitionPlay
{
	bool active;
	CP_TRANSITION type;
	CP_TransitionFunction function;	// for a transition drawn by the program
	float duration;
	float elapsed;
	CP_GSL_RequestKind kind;		// the change it shows
	CP_Ghost ghosts[CP_MAX_GAME_STATES];	// bottom first
	int ghostCount;
	int fromLayer;					// the pictures before and after (put together when needed)
	int toLayer;
	int outputLayer;				// what a transition function draws
} CP_TransitionPlay;

static CP_TRANSITION _transitionType = CP_TRANSITION_NONE;
static CP_TransitionFunction _transitionFunction = NULL;
static float _transitionSeconds = 0.0f;

static CP_TransitionPlay _transition = { .fromLayer = -1, .toLayer = -1, .outputLayer = -1 };
static bool _transitionPending = false;	// the request being applied will play a transition
static int _pendingOldTop = -1;
static CP_Ghost* _runningGhost = NULL;	// a ghost whose function is running

static int CP_GameStates_Top(void)
{
	return _stack.depth - 1;
}

// Make a state the one that draws and whose settings are in use, before
// running one of its functions
static void CP_GameStates_Activate(CP_StateSlot* slot, bool top)
{
	if (_settingsOwner != slot)
	{
		CP_DrawInfoPtr DI = GetDrawInfo();
		if (_settingsOwner)
		{
			_settingsOwner->settings = *DI;
		}
		*DI = slot->settings;
		CP_Settings_ApplyAll();
		_settingsOwner = slot;
		if (_CORE.nvg)
		{
			// each state starts drawing untransformed, as a frame does
			nvgResetTransform(_CORE.nvg);
		}
	}
	CP_SetDrawLayer(slot->layer, !slot->bottom);
	CP_Input_SetMuted(top ? FALSE : TRUE, slot->mouseWhenCovered, slot->worldMouseWhenCovered);
}

static void CP_GameStates_MakeCurrent(int index)
{
	CP_GameStates_Activate(&_slots[index], index == CP_GameStates_Top());
	_runningSlot = index;
	_runningGhost = NULL;
}

static void CP_GameStates_MakeGhostCurrent(CP_Ghost* ghost)
{
	CP_GameStates_Activate(&ghost->slot, false);
	_runningSlot = -1;
	_runningGhost = ghost;
}

// The bottom ghost's layer: the picture a new bottom state starts with
static int CP_Transition_BottomGhostLayer(void)
{
	for (int i = 0; i < _transition.ghostCount; ++i)
	{
		if (_transition.ghosts[i].slot.bottom)
		{
			return _transition.ghosts[i].slot.layer;
		}
	}
	return -1;
}

// End the transition: the ghosts exit (the top one first) and give back
// their layers
static void CP_Transition_Finish(void)
{
	if (!_transition.active)
	{
		return;
	}
	_transition.active = false;
	for (int i = _transition.ghostCount - 1; i >= 0; --i)
	{
		CP_Ghost* ghost = &_transition.ghosts[i];
		CP_GameStates_MakeGhostCurrent(ghost);
		if (ghost->state.exit)
		{
			ghost->state.exit();
		}
		CP_Layer_Release(ghost->slot.layer);
		if (_settingsOwner == &ghost->slot)
		{
			_settingsOwner = NULL;
		}
	}
	_transition.ghostCount = 0;
	_runningGhost = NULL;
	CP_Layer_Release(_transition.fromLayer);
	CP_Layer_Release(_transition.toLayer);
	CP_Layer_Release(_transition.outputLayer);
	_transition.fromLayer = _transition.toLayer = _transition.outputLayer = -1;
	if (_stack.depth > 0)
	{
		CP_GameStates_MakeCurrent(CP_GameStates_Top());
	}
}

static void CP_GameStates_Enter(void* user, int index, CP_GSL_RequestKind why)
{
	UNREFERENCED_PARAMETER(user);
	UNREFERENCED_PARAMETER(why);
	CP_StateSlot* slot = &_slots[index];
	slot->time = 0.0f;
	if (index == 0)
	{
		// A new bottom state draws on the canvas, starting with what the last
		// one drew, as it always has. (In a transition the last one still
		// draws on its own, so this one starts with a copy.)
		slot->bottom = true;
		if (_inheritLayer >= 0)
		{
			slot->layer = _inheritLayer;
			_inheritLayer = -1;
		}
		else
		{
			slot->layer = CP_Layer_Acquire(0.0f, 0.0f, 0.0f, 1.0f);
			CP_Layer_Copy(CP_Transition_BottomGhostLayer(), slot->layer);
		}
	}
	else
	{
		// A state on top starts with a clear sheet over the ones below
		slot->bottom = false;
		slot->layer = CP_Layer_Acquire(0.0f, 0.0f, 0.0f, 0.0f);
		_slots[index - 1].mouseWhenCovered = CP_Vector_Set(CP_Input_GetMouseX(), CP_Input_GetMouseY());
		_slots[index - 1].worldMouseWhenCovered = CP_Vector_Set(CP_Input_GetMouseWorldX(), CP_Input_GetMouseWorldY());
	}

	// It starts with the settings in use now, as a new state always has
	if (_settingsOwner && _settingsOwner != slot)
	{
		_settingsOwner->settings = *GetDrawInfo();
	}
	slot->settings = *GetDrawInfo();
	_settingsOwner = slot;

	CP_GameStates_MakeCurrent(index);
	const FunctionPtr init = _stack.states[index].init;
	if (init)
	{
		init();
	}
	CP_ApplyWindowSizeChosenInInit();
}

static void CP_GameStates_Leave(void* user, int index, CP_GSL_RequestKind why)
{
	UNREFERENCED_PARAMETER(user);
	CP_StateSlot* slot = &_slots[index];
	CP_GameStates_MakeCurrent(index);

	if (_transitionPending && _transition.ghostCount < CP_MAX_GAME_STATES)
	{
		// It stays on screen, as a ghost, until the transition ends. (Ghosts
		// are added top first; CP_Transition_Start puts them bottom first.)
		slot->settings = *GetDrawInfo();
		CP_Ghost* ghost = &_transition.ghosts[_transition.ghostCount++];
		ghost->state = _stack.states[index];
		ghost->slot = *slot;
		ghost->running = index == _pendingOldTop;
	}
	else
	{
		const FunctionPtr exit = _stack.states[index].exit;
		if (exit)
		{
			exit();
		}
		if (index == 0 && why != CP_GSL_POP)
		{
			// the next bottom state starts with this picture
			_inheritLayer = slot->layer;
		}
		else
		{
			CP_Layer_Release(slot->layer);
		}
	}
	slot->layer = -1;
	// its settings stay in use, so the next state starts with them
	_settingsOwner = NULL;
	_runningSlot = -1;
}

static void CP_GameStates_Uncover(void* user, int index)
{
	UNREFERENCED_PARAMETER(user);
	CP_GameStates_MakeCurrent(index);
}

static const CP_GSL_Hooks _hooks = {
	CP_GameStates_Enter,
	CP_GameStates_Leave,
	CP_GameStates_Uncover,
	NULL
};

static void CP_GameStates_Warn(CP_GSL_Result result)
{
	switch (result)
	{
	case CP_GSL_NO_UPDATE:
		printf("CProcessing: a game state needs an update function; this one was ignored.\n");
		break;
	case CP_GSL_POP_LAST:
		printf("CProcessing: CP_Engine_PopGameState was called with only one game state, so it was ignored.\n"
			"Use CP_Engine_SetNextGameState to go to another state, or CP_Engine_Terminate to quit.\n");
		break;
	case CP_GSL_STACK_FULL:
		printf("CProcessing: CP_Engine_PushGameState was ignored: there are already %d game states.\n", CP_MAX_GAME_STATES);
		break;
	case CP_GSL_QUEUE_FULL:
		printf("CProcessing: more than %d game state changes in one frame; the extra ones were ignored.\n", CP_MAX_GAME_STATE_REQUESTS);
		break;
	default:
		break;
	}
}

static void CP_GameStates_Request(CP_GSL_RequestKind kind, FunctionPtr init, FunctionPtr update, FunctionPtr exit, CP_BELOW_MODE below)
{
	if (_runningGhost)
	{
		// a state that is leaving can't change the stack
		return;
	}
	CP_GSL_State state = { init, update, exit, below };
	CP_GameStates_Warn(CP_GSL_Ask(&_stack, kind, state));
}

static bool CP_Transition_IsSet(void)
{
	return _transitionSeconds > 0.0f && (_transitionType != CP_TRANSITION_NONE || _transitionFunction != NULL);
}

// Apply the changes asked for since the last frame
static void CP_GameStates_ApplyRequests(void)
{
	CP_GSL_Request requests[CP_MAX_GAME_STATE_REQUESTS];
	const int count = CP_GSL_TakeRequests(&_stack, requests);
	for (int i = 0; i < count; ++i)
	{
		// a change during a transition ends that transition first
		CP_Transition_Finish();

		// the very first state appears at once
		_transitionPending = CP_Transition_IsSet() && _stack.depth > 0;
		_pendingOldTop = CP_GameStates_Top();
		_transition.ghostCount = 0;

		const CP_GSL_Result result = CP_GSL_Apply(&_stack, &requests[i], &_hooks);
		CP_GameStates_Warn(result);

		if (_transitionPending && result == CP_GSL_DONE)
		{
			_transition.active = true;
			_transition.type = _transitionType;
			_transition.function = _transitionFunction;
			_transition.duration = _transitionSeconds;
			_transition.elapsed = 0.0f;
			_transition.kind = requests[i].kind;
			// bottom first
			for (int a = 0, b = _transition.ghostCount - 1; a < b; ++a, --b)
			{
				CP_Ghost swap = _transition.ghosts[a];
				_transition.ghosts[a] = _transition.ghosts[b];
				_transition.ghosts[b] = swap;
			}
		}
		_transitionPending = false;
	}
	if (count > 0 && _stack.depth > 0)
	{
		CP_GameStates_MakeCurrent(CP_GameStates_Top());
	}
}

// Run this frame's updates: a transition's leaving state, the states running
// under the top one (from the bottom up, each on its own layer and without
// input), then the top one
static void CP_GameStates_Update(void)
{
	const float dt = CP_System_GetDt();
	if (_transition.active)
	{
		_transition.elapsed += dt;
		if (_transition.elapsed >= _transition.duration)
		{
			CP_Transition_Finish();
		}
		else
		{
			for (int i = 0; i < _transition.ghostCount; ++i)
			{
				CP_Ghost* ghost = &_transition.ghosts[i];
				if (ghost->running && ghost->state.update)
				{
					CP_GameStates_MakeGhostCurrent(ghost);
					ghost->slot.time += dt;
					ghost->state.update();
				}
			}
		}
	}

	const int top = CP_GameStates_Top();
	if (top < 0)
	{
		return;
	}
	for (int i = 0; i < top; ++i)
	{
		if (CP_GSL_IsRunning(&_stack, i))
		{
			CP_GameStates_MakeCurrent(i);
			_slots[i].time += dt;
			_stack.states[i].update();
		}
	}
	// (an update above can't change the stack: changes wait for the next frame)
	CP_GameStates_MakeCurrent(top);
	_slots[top].time += dt;
	_stack.states[top].update();
	// the pre- and post-update functions draw as the top state
	CP_GameStates_MakeCurrent(top);
}

//---------------------------------------------------------
// The picture on screen

// The layers of a list of states that can be seen, from the bottom up
static int CP_GameStates_AddVisible(CP_ComposeItem* items, int count, const CP_GSL_State* states, const CP_StateSlot* const* slots, int depth)
{
	for (int i = CP_GSL_LowestVisible(states, depth); i < depth; ++i)
	{
		if (slots[i]->layer >= 0)
		{
			CP_ComposeItem item = { slots[i]->layer, slots[i]->bottom, 1.0f, 0.0f, 0.0f };
			items[count++] = item;
		}
	}
	return count;
}

// The states on the stack (the first `depth` of them) and, on top, the
// ghosts if `withGhosts`
static int CP_GameStates_AddStack(CP_ComposeItem* items, int count, int depth, bool withGhosts)
{
	CP_GSL_State states[CP_MAX_GAME_STATES * 2];
	const CP_StateSlot* slots[CP_MAX_GAME_STATES * 2];
	int n = 0;
	for (int i = 0; i < depth; ++i, ++n)
	{
		states[n] = _stack.states[i];
		slots[n] = &_slots[i];
	}
	for (int i = 0; withGhosts && i < _transition.ghostCount; ++i, ++n)
	{
		states[n] = _transition.ghosts[i].state;
		slots[n] = &_transition.ghosts[i].slot;
	}
	return CP_GameStates_AddVisible(items, count, states, slots, n);
}

// The picture before the change the transition shows, and after it
static int CP_Transition_Before(CP_ComposeItem* items)
{
	switch (_transition.kind)
	{
	case CP_GSL_PUSH:
		return CP_GameStates_AddStack(items, 0, _stack.depth - 1, false);
	case CP_GSL_POP:
		return CP_GameStates_AddStack(items, 0, _stack.depth, true);
	default:	// a set: the ghosts were the whole stack
		return CP_GameStates_AddStack(items, 0, 0, true);
	}
}

static int CP_Transition_After(CP_ComposeItem* items)
{
	return CP_GameStates_AddStack(items, 0, _stack.depth, false);
}

// Put a picture together in a spare layer (made when first needed)
static int CP_Transition_ComposeInto(int* layer, const CP_ComposeItem* items, int count)
{
	if (*layer < 0)
	{
		*layer = CP_Layer_Acquire(0.0f, 0.0f, 0.0f, 1.0f);
		if (*layer < 0)
		{
			return -1;
		}
	}
	CP_Compose(items, count, _layers[*layer].framebuffer, _layers[*layer].width, _layers[*layer].height);
	return *layer;
}

static void CP_Transition_DrawFunction(float progress, int from, int to);
static bool CP_Transition_InFunction(void);

static int CP_Transition_BuildPicture(CP_ComposeItem* items)
{
	const float progress = CP_Math_ClampFloat(_transition.elapsed / _transition.duration, 0.0f, 1.0f);
	const float eased = CP_Math_EaseInOutCubic(progress);
	CP_ComposeItem before[CP_MAX_COMPOSE_ITEMS];
	CP_ComposeItem after[CP_MAX_COMPOSE_ITEMS];
	const int beforeCount = CP_Transition_Before(before);
	const int afterCount = CP_Transition_After(after);
	int count = 0;

	if (_transition.function || _transition.type == CP_TRANSITION_FADE || _transition.type == CP_TRANSITION_CROSSFADE)
	{
		// these work on the whole pictures
		const int from = CP_Transition_ComposeInto(&_transition.fromLayer, before, beforeCount);
		const int to = CP_Transition_ComposeInto(&_transition.toLayer, after, afterCount);
		if (from < 0 || to < 0)
		{
			memcpy(items, after, sizeof(CP_ComposeItem) * (size_t)afterCount);
			return afterCount;
		}
		if (_transition.function)
		{
			CP_Transition_DrawFunction(progress, from, to);
			CP_ComposeItem item = { _transition.outputLayer, true, 1.0f, 0.0f, 0.0f };
			items[count++] = item;
		}
		else if (_transition.type == CP_TRANSITION_CROSSFADE)
		{
			CP_ComposeItem item = { from, true, 1.0f, 0.0f, 0.0f };
			items[count++] = item;
			item.layer = to;
			item.bottom = false;
			item.alpha = eased;
			items[count++] = item;
		}
		else
		{
			// fade: out to black, then in from black
			const bool second = eased >= 0.5f;
			CP_ComposeItem item = { second ? to : from, true, 1.0f, 0.0f, 0.0f };
			items[count++] = item;
			CP_ComposeItem cover = { -1, false, second ? 2.0f * (1.0f - eased) : 2.0f * eased, 0.0f, 0.0f };
			items[count++] = cover;
		}
		return count;
	}

	// Slides move only the states that come or go
	float dirX = 0.0f, dirY = 0.0f;
	switch (_transition.type)
	{
	case CP_TRANSITION_SLIDE_LEFT:  dirX = -1.0f; break;
	case CP_TRANSITION_SLIDE_RIGHT: dirX = 1.0f; break;
	case CP_TRANSITION_SLIDE_UP:    dirY = -1.0f; break;
	default:                        dirY = 1.0f; break;	// down
	}
	const float width = (float)_CORE.window_width;
	const float height = (float)_CORE.window_height;
	const float inX = -dirX * width * (1.0f - eased), inY = -dirY * height * (1.0f - eased);
	const float outX = dirX * width * eased, outY = dirY * height * eased;

	switch (_transition.kind)
	{
	case CP_GSL_PUSH:
		memcpy(items, after, sizeof(CP_ComposeItem) * (size_t)afterCount);
		count = afterCount;
		if (count > 0 && items[count - 1].layer == _slots[CP_GameStates_Top()].layer)
		{
			items[count - 1].dx = inX;
			items[count - 1].dy = inY;
		}
		return count;
	case CP_GSL_POP:
		memcpy(items, after, sizeof(CP_ComposeItem) * (size_t)afterCount);
		count = afterCount;
		for (int i = 0; i < _transition.ghostCount; ++i)
		{
			CP_ComposeItem item = { _transition.ghosts[i].slot.layer, false, 1.0f, outX, outY };
			items[count++] = item;
		}
		return count;
	default:
		for (int i = 0; i < beforeCount; ++i)
		{
			before[i].dx = outX;
			before[i].dy = outY;
			items[count++] = before[i];
		}
		for (int i = 0; i < afterCount; ++i)
		{
			after[i].dx = inX;
			after[i].dy = inY;
			items[count++] = after[i];
		}
		return count;
	}
}

// The layers that make up the picture on screen, from the bottom up
static int CP_GameStates_BuildPicture(CP_ComposeItem* items)
{
	if (_stack.depth == 0 && !_transition.active)
	{
		int count = 0;
		if (_drawLayer >= 0)
		{
			CP_ComposeItem item = { _drawLayer, true, 1.0f, 0.0f, 0.0f };
			items[count++] = item;
		}
		return count;
	}
	if (_transition.active && !CP_Transition_InFunction())
	{
		return CP_Transition_BuildPicture(items);
	}
	return CP_GameStates_AddStack(items, 0, _stack.depth, false);
}

// A freed font must not stay in a state's saved settings (CP_Text.c)
void CP_GameStates_ForgetFont(CP_Font font)
{
	for (int i = 0; i < CP_MAX_GAME_STATES; ++i)
	{
		if (_slots[i].settings.font == font)
		{
			_slots[i].settings.font = NULL;
		}
		if (_transition.ghosts[i].slot.settings.font == font)
		{
			_transition.ghosts[i].slot.settings.font = NULL;
		}
	}
}

// A transition the program draws: it gets the pictures before and after as
// images, and draws with the usual functions, with the default settings,
// into a layer that is then shown
static CP_Image_Struct _transitionFromImage;
static CP_Image_Struct _transitionToImage;
static bool _inTransitionFunction = false;

static bool CP_Transition_InFunction(void)
{
	return _inTransitionFunction;
}

static void CP_Transition_DrawFunction(float progress, int from, int to)
{
	if (_transition.outputLayer < 0)
	{
		_transition.outputLayer = CP_Layer_Acquire(0.0f, 0.0f, 0.0f, 1.0f);
		if (_transition.outputLayer < 0)
		{
			return;
		}
	}
	const CP_Layer* output = &_layers[_transition.outputLayer];
	glBindFramebuffer(GL_FRAMEBUFFER, output->framebuffer);
	glViewport(0, 0, output->width, output->height);
	glDisable(GL_SCISSOR_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	_transitionFromImage.handle = CP_Layer_Image(from);
	_transitionFromImage.w = _CORE.window_width;
	_transitionFromImage.h = _CORE.window_height;
	_transitionToImage.handle = CP_Layer_Image(to);
	_transitionToImage.w = _CORE.window_width;
	_transitionToImage.h = _CORE.window_height;

	const int drawLayer = _drawLayer;
	const bool premultiplied = _drawLayerPremultiplied;
	_drawLayer = _transition.outputLayer;
	_drawLayerPremultiplied = false;

	nvgBeginFrame(_CORE.nvg, (float)_CORE.window_width, (float)_CORE.window_height, _CORE.pixel_ratio);
	CP_Settings_Save();
	CP_DrawInfoPtr DI = GetDrawInfo();
	CP_Settings_InitDefaults(DI);
	CP_Settings_ApplyAll();
	nvgResetTransform(_CORE.nvg);
	_inTransitionFunction = true;
	_transition.function(progress, &_transitionFromImage, &_transitionToImage);
	_inTransitionFunction = false;
	CP_Settings_Restore();
	nvgEndFrame(_CORE.nvg);

	_drawLayer = drawLayer;
	_drawLayerPremultiplied = premultiplied;
}


//---------------------------------------------------------
// ENGINE:
//		Functions managing code flow

// Run begins the CProcessing engine and starts calling init, update and exit functions.
// This is the start and core of any C Processing program.
CP_API void CP_Engine_Run(void)
{
	if (_isInitialized)
	{
		// don't allow multiple Run loops
		return;
	}

	// initialize the CProcessing Engine
	CP_Initialize();
	if (!_isInitialized)
	{
		// window/context creation failed - the reason has already been printed
		return;
	}
	// the first state starts with the first canvas
	_inheritLayer = _drawLayer;

	// main loop
	while (isRunning)
	{
		CP_FrameStart();
		CP_Update();

		if (_preUpdateFunction) _preUpdateFunction();

		// change states, and call their init and exit functions
		CP_GameStates_ApplyRequests();

		CP_GameStates_Update();

		if (_postUpdateFunction) _postUpdateFunction();

		CP_FrameEnd();
	}

	// End every state when the program is terminating, from the top down
	// (anything they draw or screenshot uses their layers, as during a frame)
	CP_Canvas_Bind();
	nvgBeginFrame(_CORE.nvg, (float)_CORE.window_width, (float)_CORE.window_height, _CORE.pixel_ratio);
	_nvgFrameOpen = true;
	CP_Transition_Finish();
	CP_GSL_LeaveAll(&_stack, &_hooks, CP_GSL_SET_FORCED);
	nvgEndFrame(_CORE.nvg);
	_nvgFrameOpen = false;

	CP_Shutdown();
}

CP_API void CP_Engine_Terminate(void)
{
	// mark the program for termination
	if (_CORE.window)
	{
		glfwSetWindowShouldClose(_CORE.window, GL_TRUE);
	}
}

// Go to a state: at the start of the next frame, every state ends (the top
// one first) and this one starts. Nothing happens if it is already the only
// state. update must have a valid input function, init and exit may be NULL.
CP_API void CP_Engine_SetNextGameState(FunctionPtr init, FunctionPtr update, FunctionPtr exit)
{
	CP_GameStates_Request(CP_GSL_SET, init, update, exit, CP_BELOW_FROZEN);
}

// The same, but it also restarts the state it's already in: its exit runs,
// then its init.
CP_API void CP_Engine_SetNextGameStateForced(FunctionPtr init, FunctionPtr update, FunctionPtr exit)
{
	CP_GameStates_Request(CP_GSL_SET_FORCED, init, update, exit, CP_BELOW_FROZEN);
}

// Put a state on top of the current one, like a pause menu: the one below
// stops, and its last picture stays visible behind the new one
CP_API void CP_Engine_PushGameState(FunctionPtr init, FunctionPtr update, FunctionPtr exit)
{
	CP_GameStates_Request(CP_GSL_PUSH, init, update, exit, CP_BELOW_FROZEN);
}

CP_API void CP_Engine_PushGameStateAdvanced(FunctionPtr init, FunctionPtr update, FunctionPtr exit, CP_BELOW_MODE below)
{
	if (below != CP_BELOW_RUNNING && below != CP_BELOW_HIDDEN)
	{
		below = CP_BELOW_FROZEN;
	}
	CP_GameStates_Request(CP_GSL_PUSH, init, update, exit, below);
}

// Take the top state off: its exit runs, and the one below carries on where
// it was (its init doesn't run again)
CP_API void CP_Engine_PopGameState(void)
{
	CP_GameStates_Request(CP_GSL_POP, NULL, NULL, NULL, CP_BELOW_FROZEN);
}

CP_API int CP_Engine_GetGameStateDepth(void)
{
	return _stack.depth;
}

CP_API CP_BOOL CP_Engine_IsTopGameState(void)
{
	return (!_runningGhost && _runningSlot >= 0 && _runningSlot == CP_GameStates_Top()) ? TRUE : FALSE;
}

CP_API float CP_Engine_GetGameStateTime(void)
{
	if (_runningGhost)
	{
		return _runningGhost->slot.time;
	}
	if (_runningSlot >= 0 && _runningSlot < _stack.depth)
	{
		return _slots[_runningSlot].time;
	}
	return 0.0f;
}

// From now on, a change of game state shows this transition, taking this
// many seconds. CP_TRANSITION_NONE (or 0 seconds) changes at once again.
CP_API void CP_Engine_SetTransition(CP_TRANSITION transition, float seconds)
{
	if (transition < CP_TRANSITION_NONE || transition > CP_TRANSITION_SLIDE_DOWN)
	{
		transition = CP_TRANSITION_NONE;
	}
	_transitionType = transition;
	_transitionFunction = NULL;
	_transitionSeconds = transition == CP_TRANSITION_NONE ? 0.0f : seconds;
}

// A transition the program draws: the function is called every frame of it
// with the progress (0 to 1) and the pictures before and after. NULL changes
// at once again.
CP_API void CP_Engine_SetTransitionFunction(CP_TransitionFunction transition, float seconds)
{
	_transitionType = CP_TRANSITION_NONE;
	_transitionFunction = transition;
	_transitionSeconds = transition ? seconds : 0.0f;
}

CP_API void CP_Engine_SetPreUpdateFunction(FunctionPtr preUpdateFunction)
{
	_preUpdateFunction = preUpdateFunction;
}

CP_API void CP_Engine_SetPostUpdateFunction(FunctionPtr postUpdateFunction)
{
	_postUpdateFunction = postUpdateFunction;
}


//---------------------------------------------------------
// SYSTEM:
//		OS functions supporting window management and timing

CP_API void CP_System_SetWindowSize(int new_width, int new_height)
{
	CP_SetWindowSizeInternal(new_width, new_height, false);
}

CP_API void CP_System_SetWindowPosition(int x, int y)
{
	_CORE.window_posX = x;
	_CORE.window_posY = y;
	if (_CORE.nvg)
	{
		CP_SetWindowPosIfSupported(_CORE.window, x, y);
	}
}

CP_API void CP_System_Fullscreen(void)
{
	CP_SetWindowSizeInternal(0, 0, true);
}

CP_API void CP_System_FullscreenAdvanced(int targetWidth, int targetHeight)
{
	CP_SetWindowSizeInternal(targetWidth, targetHeight, true);
}

CP_API CP_BOOL CP_System_IsFullscreen(void)
{
	if (_CORE.window && _CORE.nvg)
	{
		// a window on a monitor is fullscreen
		return glfwGetWindowMonitor(_CORE.window) != NULL ? TRUE : FALSE;
	}
	// no window yet: what was asked for before CP_Engine_Run
	return (_deferredSizeChange ? _deferredFullscreen : (CP_BOOL)_CORE.isFullscreen) ? TRUE : FALSE;
}

CP_API void CP_System_SetWindowResizable(CP_BOOL resizable)
{
	_windowResizable = resizable ? TRUE : FALSE;
	if (_CORE.window && _CORE.nvg)
	{
		glfwSetWindowAttrib(_CORE.window, GLFW_RESIZABLE, _windowResizable);
	}
}

CP_API CP_BOOL CP_System_GetWindowResizable(void)
{
	return _windowResizable;
}

CP_API void CP_System_SetVSync(CP_BOOL vsync)
{
	_vsync = vsync ? TRUE : FALSE;
	if (_CORE.window && _CORE.nvg)
	{
		glfwSwapInterval(_vsync ? 1 : 0);
	}
}

CP_API CP_BOOL CP_System_GetVSync(void)
{
	return _vsync;
}

// The window size in the coordinates everything is drawn in, which is also
// what the mouse position uses. On hi-dpi displays (e.g. 2x on a Retina Mac)
// the window has more pixels than that; the canvas is that many pixels.
CP_API int CP_System_GetWindowWidth(void)
{
	return _CORE.window_width;
}

CP_API int CP_System_GetWindowHeight(void)
{
	return _CORE.window_height;
}

// The size of the monitor the window is on (see CP_WindowMonitor)
CP_API int CP_System_GetDisplayWidth(void)
{
	if (_CORE.window)
	{
		CP_UpdateDisplaySize(CP_WindowMonitor());
	}
	return _CORE.native_width;
}

CP_API int CP_System_GetDisplayHeight(void)
{
	if (_CORE.window)
	{
		CP_UpdateDisplaySize(CP_WindowMonitor());
	}
	return _CORE.native_height;
}

CP_API int CP_System_GetDisplayRefreshRate(void)
{
	// 0 means unknown, as before CP_Engine_Run there is no display information yet
	if (!_CORE.window)
	{
		return 0;
	}
	GLFWmonitor* monitor = CP_WindowMonitor();
	const GLFWvidmode* mode = monitor ? glfwGetVideoMode(monitor) : NULL;
	return mode ? mode->refreshRate : 0;
}

CP_API CP_WindowHandle CP_System_GetWindowHandle(void)
{
	return _CORE.hwnd;
}

// Query the OS-level handle for the GLFW window (see CP_WindowHandle)
static CP_WindowHandle CP_GetNativeWindowHandle(GLFWwindow* window)
{
	if (!window)
	{
		return NULL;
	}
#if defined(_WIN32)
	return (CP_WindowHandle)glfwGetWin32Window(window);
#elif defined(__APPLE__)
	return (CP_WindowHandle)glfwGetCocoaWindow(window);
#elif defined(CP_HAS_X11)
	if (glfwGetPlatform() == GLFW_PLATFORM_X11)
	{
		return (CP_WindowHandle)(uintptr_t)glfwGetX11Window(window);
	}
	return NULL;
#else
	return NULL;
#endif
}

CP_API void CP_System_SetWindowTitle(const char* title)
{
	CP_StringCopy(_windowTitle, sizeof(_windowTitle), title);
	if (_CORE.window)
	{
		glfwSetWindowTitle(_CORE.window, _windowTitle);
	}
}

CP_API CP_BOOL CP_System_GetWindowFocus(void)
{
	return _CORE.window && glfwGetWindowAttrib(_CORE.window, GLFW_FOCUSED) ? TRUE : FALSE;
}

static void CP_ApplyCursorMode(void)
{
	int mode = GLFW_CURSOR_NORMAL;
	switch (_cursorMode)
	{
	case CP_CURSOR_HIDDEN:
		mode = GLFW_CURSOR_HIDDEN;
		break;
	case CP_CURSOR_CAPTURED:
		// GLFW can't keep the cursor in the window on macOS yet, and reports
		// an error when asked to, so the cursor is just left free there
		mode = glfwGetPlatform() == GLFW_PLATFORM_COCOA ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_CAPTURED;
		break;
	case CP_CURSOR_DISABLED:
		mode = GLFW_CURSOR_DISABLED;
		break;
	default:
		break;
	}
	glfwSetInputMode(_CORE.window, GLFW_CURSOR, mode);

	// Mouse movement without the operating system's acceleration, for mouse
	// look. GLFW only uses it while the cursor is disabled.
	if (glfwRawMouseMotionSupported())
	{
		glfwSetInputMode(_CORE.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}
}

CP_API void CP_System_ShowCursor(CP_BOOL show)
{
	CP_System_SetCursorMode(show ? CP_CURSOR_NORMAL : CP_CURSOR_HIDDEN);
}

CP_API CP_BOOL CP_System_GetCursorVisible(void)
{
	return _cursorMode == CP_CURSOR_NORMAL || _cursorMode == CP_CURSOR_CAPTURED ? TRUE : FALSE;
}

CP_API void CP_System_SetCursorMode(CP_CURSOR_MODE mode)
{
	if ((unsigned)mode > (unsigned)CP_CURSOR_DISABLED)
	{
		return;
	}
	_cursorMode = mode;
	if (_CORE.window)
	{
		CP_ApplyCursorMode();
	}
}

CP_API CP_CURSOR_MODE CP_System_GetCursorMode(void)
{
	return _cursorMode;
}


//---------------------------------------------------------
// CONSOLE:
//		A text console, so printf debugging works.
//
//		Windows: programs built as GUI apps (the CProcessing template and
//		demos) start without a console. The first CP_System_ShowConsole(TRUE)
//		attaches to the terminal the program was started from, or else opens a
//		new console window, and points stdout, stderr and stdin at it.
//		A console window the program has to itself is then only ever hidden
//		and shown, never freed, so the streams stay valid. Its close button is
//		removed, because closing a console window ends the whole program.
//		A console shared with other programs (such as the terminal the program
//		was started from) is never hidden.
//
//		Linux/macOS: output goes to the terminal the program was started from,
//		if there is one. There is no portable way to open a console window, so
//		showing and hiding change nothing visible; run the program from a
//		terminal to see its output.
//
//		On every platform, showing the console turns off stdout/stderr
//		buffering, so output appears immediately, even when the program
//		crashes right after printing.

static bool _consoleStreamsReady = false;
#if defined(_WIN32)
static HWND _consoleWindow = NULL;	// a console window the program has to itself, which may be hidden
static bool _consoleHidden = false;

// Point a standard stream at the console, unless it already goes somewhere
// (for example redirected to a file or a pipe)
static void CP_Console_ConnectStream(FILE* stream, const char* device, const char* mode)
{
	if (_fileno(stream) < 0)
	{
		FILE* reopened = NULL;
		freopen_s(&reopened, device, mode, stream);
	}
}
#endif

static void CP_Console_PrepareStreams(void)
{
	if (_consoleStreamsReady)
	{
		return;
	}
	_consoleStreamsReady = true;
#if defined(_WIN32)
	CP_Console_ConnectStream(stdout, "CONOUT$", "w");
	CP_Console_ConnectStream(stderr, "CONOUT$", "w");
	CP_Console_ConnectStream(stdin, "CONIN$", "r");
#endif
	fflush(stdout);
	fflush(stderr);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
}

CP_API void CP_System_ShowConsole(CP_BOOL show)
{
#if defined(_WIN32)
	if (show && GetConsoleWindow() == NULL)
	{
		// no console yet: use the terminal the program was started from,
		// or else open a new console window
		if (!AttachConsole(ATTACH_PARENT_PROCESS) && !AllocConsole())
		{
			return;
		}
		if (_CORE.window)
		{
			// a new console window takes the focus; give it back to the game
			glfwFocusWindow(_CORE.window);
		}
	}

	HWND console = GetConsoleWindow();
	if (console == NULL)
	{
		return;
	}
	if (!_consoleStreamsReady)
	{
		CP_Console_PrepareStreams();

		// Only a console the program has to itself may be hidden
		DWORD processes[2];
		if (GetConsoleProcessList(processes, 2) == 1)
		{
			_consoleWindow = console;
			HMENU menu = GetSystemMenu(console, FALSE);
			if (menu)
			{
				DeleteMenu(menu, SC_CLOSE, MF_BYCOMMAND);
			}
		}
	}
	if (_consoleWindow)
	{
		// showing must not take the keyboard focus away from the game
		ShowWindow(_consoleWindow, show ? SW_SHOWNOACTIVATE : SW_HIDE);
		_consoleHidden = !show;
	}
#else
	if (show)
	{
		CP_Console_PrepareStreams();
	}
#endif
}

CP_API CP_BOOL CP_System_GetConsoleVisible(void)
{
#if defined(_WIN32)
	return GetConsoleWindow() != NULL && !_consoleHidden ? TRUE : FALSE;
#else
	return isatty(STDOUT_FILENO) ? TRUE : FALSE;
#endif
}

CP_API unsigned CP_System_GetFrameCount(void)
{
	return _frameCount;
}

CP_API float CP_System_GetFrameRate(void)
{
	return (float)(1.0 / _frametime);
}

CP_API void CP_System_SetFrameRate(float fps)
{
	// 0 or less means no limit: run as fast as possible. (1 / 0 used to make
	// the frame limiter wait forever.)
	_frametimeTarget = fps > 0.0f ? 1.0 / fps : 0.0; // seconds per frame
}

CP_API float CP_System_GetTargetFrameRate(void)
{
	// 0 for no limit, as for SetFrameRate
	return _frametimeTarget > 0.0 ? (float)(1.0 / _frametimeTarget) : 0.0f;
}

CP_API float CP_System_GetDt(void)
{
	return (float)_frametime;
}

CP_API float CP_System_GetMillis(void)
{
	return (float)(glfwGetTime() * 1000.0);
}

CP_API float CP_System_GetSeconds(void)
{
	return (float)glfwGetTime();
}


//---------------------------------------------------------
// INTERNAL Engine and System:
//		Support functions not exposed to the user

// Undo a partially completed CP_Initialize so nothing is left pointing at a
// destroyed window or GL context (CP_Engine_Run then returns immediately)
static void CP_InitializeFailed(void)
{
	if (_CORE.nvg)
	{
		nvgDeleteGL3(_CORE.nvg);
		_CORE.nvg = NULL;
	}
	glfwTerminate();
	_CORE.window = NULL;
	_CORE.hwnd = NULL;
}

void CP_Initialize(void)
{
	if (_isInitialized == true)
		return;

	// Initialize CP_Core to default
	_CORE.nvg = NULL;
	_CORE.window = NULL;

	// Set the error callback first so initialization problems are reported too
	glfwSetErrorCallback(error_callback_glfw);

	// Initialize GLFW
	if (!glfwInit()) {
		printf("Failed to init GLFW (is a display available?).\n");
		return;
	}

	// we need GLFW to query the monitor, then set the correct resolution for the window
	const GLFWvidmode* structure = glfwGetVideoMode(glfwGetPrimaryMonitor());
	if (!structure)
	{
		printf("Failed to query the primary monitor.\n");
		CP_InitializeFailed();
		return;
	}
	_CORE.native_width = structure->width;
	_CORE.native_height = structure->height;
	if (_CORE.isFullscreen && _CORE.canvas_width == 0 && _CORE.canvas_height == 0)
	{
		// force full screen values
		_CORE.window_width = _CORE.native_width;
		_CORE.window_height = _CORE.native_height;
	}
	else
	{
		// constrain input to valid values
		_CORE.window_width = CP_Math_ClampInt(_CORE.canvas_width > 0 ? _CORE.canvas_width : 400, 0, _CORE.native_width);
		_CORE.window_height = CP_Math_ClampInt(_CORE.canvas_height > 0 ? _CORE.canvas_height : 400, 0, _CORE.native_height);
	}

	// Create the window
	glfwDefaultWindowHints();
#ifndef NDEBUG
	// A debug context reports OpenGL errors in detail but can be slower,
	// so only Debug builds ask for one
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
#endif
	glfwWindowHint(GLFW_DOUBLEBUFFER, 1);	// persistence comes from the canvas framebuffer
	glfwWindowHint(GLFW_RESIZABLE, _windowResizable);
	glfwWindowHint(GLFW_VISIBLE, 0);
#if defined(__APPLE__)
	// macOS only provides OpenGL 3.2+ as a forward-compatible core profile,
	// and NanoVG's GL3 backend needs 3.2
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
	_CORE.window = glfwCreateWindow(_CORE.window_width, _CORE.window_height, _windowTitle, _CORE.isFullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);

	if (!_CORE.window) {
		printf("Failed to create the CProcessing window.\n");
		CP_InitializeFailed();
		return;
	}

	_CORE.hwnd = CP_GetNativeWindowHandle(_CORE.window);

	glfwMakeContextCurrent(_CORE.window);
	// Load GL through GLFW so the right loader is used for whichever context
	// API GLFW picked (WGL, GLX, EGL/Wayland, NSGL)
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("Failed to load OpenGL functions.\n");
		CP_InitializeFailed();
		return;
	}
	// NVG_DEBUG checks for an OpenGL error after each NanoVG draw step, which
	// can stall the GPU, so only Debug builds turn it on
#ifndef NDEBUG
	_CORE.nvg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
#else
	_CORE.nvg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
#endif
	if (_CORE.nvg == NULL)
	{
		printf("Could not init nanovg (OpenGL 3.2 or newer is required).\n");
		CP_InitializeFailed();
		return;
	}

	// Give NanoVG the draw settings: the defaults (CP_Settings_InitDefaults),
	// with any CP_Settings_ calls made before CP_Engine_Run
	CP_Settings_ApplyAll();

	glfwSwapInterval(_vsync ? 1 : 0);
	glfwSetTime(0);

	// Initialize random number generators
	// (Random first so Noise can use the better RNG)
	CP_Random_Init();
	CP_NoiseInit();

	// Initialize mouse position and keyboard state
	CP_Input_Init();

	// "Swap" buffers once so it properlly sets the background color
	glfwSwapBuffers(_CORE.window);

	glfwGetFramebufferSize(_CORE.window, &_CORE.canvas_width, &_CORE.canvas_height);
	glfwGetWindowSize(_CORE.window, &_CORE.window_width, &_CORE.window_height);
	CP_UpdatePixelRatio();

	int windowPosX = 0;
	int windowPosY = 0;
	if (_CORE.window_posX >= 0 || _CORE.window_posY >= 0)
	{
		// custom position has been set prior to initialization
		windowPosX = _CORE.window_posX;
		windowPosY = _CORE.window_posY;
	}
	else
	{
		// set the window to the middle of the primary monitor
		CP_CenterWindowOn(glfwGetPrimaryMonitor(), _CORE.window_width, _CORE.window_height, &windowPosX, &windowPosY);
	}
	CP_SetWindowPosIfSupported(_CORE.window, windowPosX, windowPosY);

	// Update and render
	glViewport(0, 0, _CORE.canvas_width, _CORE.canvas_height);

	// Create the persistent drawing canvas (also binds it and sets the viewport)
	if (!CP_Canvas_Resize(_CORE.canvas_width, _CORE.canvas_height))
	{
		CP_InitializeFailed();
		return;
	}

	// Set the background color
	CP_Graphics_ClearBackground(CP_Color_Create(150, 150, 150, 255));

	// Set input callback
	glfwSetInputMode(_CORE.window, GLFW_STICKY_KEYS, 1);
	glfwSetKeyCallback(_CORE.window, CP_Input_KeyboardCallback);
	glfwSetMouseButtonCallback(_CORE.window, CP_Input_MouseCallback);
	glfwSetScrollCallback(_CORE.window, CP_Input_MouseWheelCallback);
	glfwSetCharCallback(_CORE.window, CP_Input_CharCallback);

	// Apply a cursor setting requested before the window existed
	CP_ApplyCursorMode();

	// Track size changes the window system applies after the fact
	glfwSetWindowSizeCallback(_CORE.window, CP_WindowSizeCallback);
	glfwSetFramebufferSizeCallback(_CORE.window, CP_FramebufferSizeCallback);

	// Init frame rate control
	CP_FrameRate_Init();

	// Text Init
	CP_Text_Init();

	// Camera Init
	GetDrawInfo()->camera = CP_Matrix_Identity();

	// Sound Init
	CP_Sound_Init();

	// Image Init
	CP_Image_Init();

	// once everything is setup, show the window
	glfwShowWindow(_CORE.window);

	_isInitialized = true;
}

void CP_Update(void)
{
	// Update Input
	CP_Input_Update();
	glfwPollEvents();

	// Reset camera transforms
	nvgResetTransform(_CORE.nvg);

	// Audio Update
	CP_Sound_Update();

	// Image Update
	CP_Image_Update();
}

void CP_Report_AddLine(CP_Report* report, const char* format, ...)
{
	size_t room = sizeof(report->text) - report->length;
	va_list args;
	va_start(args, format);
	int written = room > 1 ? vsnprintf(report->text + report->length, room, format, args) : -1;
	va_end(args);

	if (written < 0 || (size_t)written + 1 >= room)
	{
		// keep only whole lines
		report->text[report->length] = '\0';
		report->truncated = TRUE;
		return;
	}
	report->length += (size_t)written;
	report->text[report->length++] = '\n';
	report->text[report->length] = '\0';
	++report->lines;
}

void CP_Report_AddAsset(CP_Report* report, const char* kind, const char* filepath, int loads)
{
	if (loads == 1)
	{
		CP_Report_AddLine(report, "  %-5s  %s: loaded, never freed", kind, filepath);
	}
	else
	{
		// Thousands of loads usually mean a Load call in an update function
		CP_Report_AddLine(report, "  %-5s  %s: %d loads not freed%s", kind, filepath, loads,
			loads >= 60 ? " (is it loaded every frame?)" : "");
	}
}

// Tell the programmer about assets they never freed, before CP_Shutdown frees
// them. Students learn to pair every Load with a Free, and a Load in an update
// function (which works, since loads of a file share it) shows up here.
static void CP_ReportLoadedAssets(void)
{
	CP_Report report = { 0 };
	CP_Image_ReportLoaded(&report);
	CP_Sound_ReportLoaded(&report);
	CP_Text_ReportLoaded(&report);
	if (report.lines == 0 && !report.truncated)
	{
		return;
	}

	char message[sizeof(report.text) + 1024];
	snprintf(message, sizeof(message),
		"CProcessing: these were still loaded when the program ended:\n"
		"%s%s"
		"CProcessing freed them, but every CP_Image_Load, CP_Sound_Load,\n"
		"CP_Sound_LoadStream and CP_Font_Load needs its own Free, and so does\n"
		"every image from CP_Image_Create, CP_Image_CreateFromData or\n"
		"CP_Image_Screenshot.\n"
		"Load assets once, in an init function, and free them in the exit\n"
		"function: https://github.com/DigiPen-Faculty/CProcessing/wiki/Assets-and-paths\n",
		report.text, report.truncated ? "  ...and more\n" : "");

	fputs(message, stderr);
	fflush(stderr);
#if defined(_WIN32)
	// Windows programs usually have no console: Visual Studio's Output window
	OutputDebugStringA(message);
#if !defined(NDEBUG)
	// Debug library, started from Visual Studio: make sure it's seen. Never
	// otherwise, so a game someone double-clicks, or a test run, can't stop here.
	if (IsDebuggerPresent())
	{
		MessageBoxA(NULL, message, "CProcessing: files still loaded", MB_OK | MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND);
	}
#endif
#endif
}

void CP_Shutdown(void)
{
	CP_ReportLoadedAssets();

	CP_Text_Shutdown();
	CP_Sound_Shutdown();
	CP_Image_Shutdown();

	// Clean up GL resources while the GL context still exists, then glfw
	// (which destroys the window and context)
	CP_Canvas_Destroy();
	nvgDeleteGL3(_CORE.nvg);
	_CORE.nvg = NULL;
	glfwTerminate();
	_CORE.window = NULL;
	_CORE.hwnd = NULL;
}

// A window size chosen in a state's init (the usual place) is applied before
// that state's first update, not a frame later, so the first update sees the
// size it asked for, and what it draws (even if it only draws once) fits.
// Anything init drew is finished into the old canvas, which keeps it in its
// top left corner.
void CP_ApplyWindowSizeChosenInInit(void)
{
	if (!_deferredSizeChange || !_CORE.nvg)
	{
		return;
	}
	nvgEndFrame(_CORE.nvg);
	_deferredSizeChange = FALSE;
	CP_DeferredSetWindowSizeInternal(_deferredWidth, _deferredHeight, _deferredFullscreen);
	CP_Canvas_Resize(_CORE.canvas_width, _CORE.canvas_height);
	CP_Canvas_Bind();
	nvgBeginFrame(_CORE.nvg, (float)_CORE.window_width, (float)_CORE.window_height, _CORE.pixel_ratio);
}

void CP_FrameStart(void)
{
	CP_FrameRate_FrameStart();

	if (_deferredSizeChange)
	{
		_deferredSizeChange = FALSE;
		CP_DeferredSetWindowSizeInternal(_deferredWidth, _deferredHeight, _deferredFullscreen);
	}

	// follow any window size change the window system has applied, and draw
	// into the drawing layer again (the last frame ended with the window bound)
	CP_Canvas_Resize(_CORE.canvas_width, _CORE.canvas_height);
	CP_Canvas_Bind();

	nvgBeginFrame(_CORE.nvg, (float)_CORE.window_width, (float)_CORE.window_height, _CORE.pixel_ratio);
	_nvgFrameOpen = true;
}

void CP_FrameEnd(void)
{
	if (_drawingIntoImage)
	{
		printf("CProcessing: CP_Image_BeginDrawing had no CP_Image_EndDrawing before the end of the frame.\n");
		CP_Image_EndDrawing();
	}
	nvgEndFrame(_CORE.nvg);
	_nvgFrameOpen = false;
	CP_Canvas_Present();	// leaves the window's framebuffer bound, as macOS needs for the swap
	glfwSwapBuffers(_CORE.window);
	glFlush();
	glfwPollEvents();

	// Limit framerate
	CP_FrameRate_FrameEnd();
}

void CP_IncFrameCount(void)
{
	++_frameCount;
}

void CP_FrameRate_Init(void)
{
	// start the app at frame zero
	_frameCount = 0;
}

void CP_FrameRate_FrameStart(void)
{
	StartingTime = glfwGetTime();

	// Update frame count
	CP_IncFrameCount();
}

static double averageSleepCycles = 0.002;
static double additionalBuffer = 0.001;	// Sleep() isn't very accurate and we want to make sure we don't over sleep

void CP_FrameRate_FrameEnd(void)
{
	double prevSeconds = 0;
	double currSeconds = 0;

	do
	{
		CP_UpdateFrameTime();

		// compute remaining microseconds in the frame
		currSeconds = _frametimeTarget - ElapsedSeconds;

		// if our remaining microseconds this frame are greater than the Sleep function's
		// average margin of error (plus an additional buffer value) then go ahead and Sleep
		if (currSeconds > averageSleepCycles + additionalBuffer)
		{
			prevSeconds = currSeconds;

			// give back cycles to other processes if we don't need them
			thrd_sleep(&(struct timespec) { .tv_nsec = 1000000 }, NULL);	// sleep 1 millisecond

			// update the time after sleeping
			CP_UpdateFrameTime();

			// recompute remaining micros
			currSeconds = _frametimeTarget - ElapsedSeconds;

			// update the average sleep over 16 samples so we maintain awareness of the system's margin of error.
			averageSleepCycles = ((averageSleepCycles * 15.0) + (prevSeconds - currSeconds)) / 16.0;	// multiply by 15, add new sample, divide by 16
		}

	} while (ElapsedSeconds < _frametimeTarget);

	// Update time of the last frame
	_frametime = ElapsedSeconds;
}

void CP_UpdateFrameTime(void)
{
	EndingTime = glfwGetTime();
	ElapsedSeconds = EndingTime - StartingTime;
}

void CP_SetWindowSizeInternal(int new_width, int new_height, bool isFullscreen)
{
	_deferredSizeChange = TRUE;
	_deferredWidth = new_width;
	_deferredHeight = new_height;
	_deferredFullscreen = isFullscreen;
}

void CP_DeferredSetWindowSizeInternal(int new_width, int new_height, bool isFullscreen)
{
	if (!_CORE.nvg)
	{
		// this will only happen if setting size before OpenGL init
		// use these sizes to override defaults during init
		_CORE.canvas_width = new_width;
		_CORE.canvas_height = new_height;
		_CORE.isFullscreen = isFullscreen;
		return;
	}

	// Fullscreen goes to the monitor the window is on now, and a window is
	// centered on it. (Leaving fullscreen, that's the monitor it was
	// fullscreen on.)
	GLFWmonitor* monitor = CP_WindowMonitor();
	if (!isFullscreen && glfwGetWindowMonitor(_CORE.window))
	{
		// Leave fullscreen before measuring the monitor. Fullscreen at a size
		// other than the monitor's own switches the monitor to the closest
		// video mode until the window leaves fullscreen, and a window centered
		// on that mode's size ends up off center on the desktop.
		const CP_ScreenRect fullscreenRect = CP_MonitorRect(monitor);
		glfwSetWindowMonitor(_CORE.window, NULL, fullscreenRect.x, fullscreenRect.y, new_width, new_height, GLFW_DONT_CARE);
	}
	CP_UpdateDisplaySize(monitor);

	if (isFullscreen && new_width == 0 && new_height == 0)
	{
		// force full screen values
		new_width = _CORE.native_width;
		new_height = _CORE.native_height;
	}
	else
	{
		// constrain input to valid values
		new_width = CP_Math_ClampInt(new_width, 0, _CORE.native_width);
		new_height = CP_Math_ClampInt(new_height, 0, _CORE.native_height);
	}

	// set window
	int windowPosX = 0;
	int windowPosY = 0;
	if (_CORE.window_posX >= 0 || _CORE.window_posY >= 0)
	{
		// custom position has been set prior to initialization
		windowPosX = _CORE.window_posX;
		windowPosY = _CORE.window_posY;
	}
	else if (!isFullscreen)
	{
		// set the window to the middle of the monitor (fullscreen ignores the
		// position)
		CP_CenterWindowOn(monitor, new_width, new_height, &windowPosX, &windowPosY);
	}
	// GLFW_DONT_CARE: in fullscreen, GLFW picks the monitor's highest refresh
	// rate for the size, rather than always 60 Hz
	glfwSetWindowMonitor(_CORE.window, isFullscreen ? monitor : NULL, windowPosX, windowPosY, new_width, new_height, GLFW_DONT_CARE);
	if (!isFullscreen)
	{
		// returning from fullscreen needs a size refresh to properly include window decoration dimensions
		glfwSetWindowSize(_CORE.window, new_width, new_height);
	}

	// get and store size values based on the actual window
	glfwGetFramebufferSize(_CORE.window, &_CORE.canvas_width, &_CORE.canvas_height);
	glfwGetWindowSize(_CORE.window, &_CORE.window_width, &_CORE.window_height);
	CP_UpdatePixelRatio();
	// update openGL frame size
	glViewport(0, 0, _CORE.canvas_width, _CORE.canvas_height);
}
