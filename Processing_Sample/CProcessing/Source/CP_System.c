//---------------------------------------------------------
// file:	CProcessing.c
// author:	Justin Chambers
// brief:	Primary implementation of the CProcessing interface
//
// Copyright © 2019 DigiPen, All rights reserved.
//---------------------------------------------------------

#include "cprocessing.h"
#include "Internal_System.h"
#include "Internal_Monitor.h"
#include "tinycthread.h"
#include <stdio.h>
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

CP_BOOL _deferredSizeChange = FALSE;
int _deferredWidth = 0;
int _deferredHeight = 0;
CP_BOOL _deferredFullscreen = FALSE;

// Window settings that may be requested before the window exists; they are
// stored here and applied when CP_Initialize creates the window
static CP_BOOL _cursorVisible = TRUE;
static char _windowTitle[256] = "CProcessing Application";

typedef struct GameStateFuncs
{
	FunctionPtr init;
	FunctionPtr update;
	FunctionPtr exit;
} GameStateFuncs;

GameStateFuncs _currState = { NULL, NULL, NULL };
GameStateFuncs _nextState = { NULL, NULL, NULL };
bool _stateIsChanging = false;

FunctionPtr _preUpdateFunction = NULL;
FunctionPtr _postUpdateFunction = NULL;

//------------------------------------------------------------------------------
// Private Variables:
//------------------------------------------------------------------------------

// FrameRate Control
static double StartingTime, EndingTime, ElapsedSeconds;
static double _frametimeTarget = 1.0 / 60.0;
static double _frametime = 1.0 / 60.0;

// Frames since the start of the program
static unsigned int _frameCount;

void error_callback_glfw(int error, const char* desc)
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
// CANVAS:
//		CProcessing, like Processing, does not clear the screen between frames:
//		whatever was drawn stays until it is drawn over. Every frame is drawn
//		into a persistent offscreen framebuffer (the "canvas") that is copied
//		to a normal double-buffered window at the end of the frame.
//		Screenshots read back from the canvas.
//
//		(CProcessing used to draw straight into a single-buffered window on
//		Windows instead. Single-buffered windows are not available everywhere -
//		Wayland/EGL has none, and macOS core profile contexts are unreliable
//		with them - so the canvas is used on every platform for consistency.)
//
//		After the copy, the window's own framebuffer (0) stays bound through
//		the swap, and the canvas is bound again at the start of the next frame:
//		on macOS, a frame swapped while another framebuffer is bound is never
//		displayed, and the window stays black (SDL documents the same for
//		SDL_GL_SwapWindow).

static GLuint _canvasFramebuffer = 0;
static GLuint _canvasColorBuffer = 0;
static GLuint _canvasDepthStencilBuffer = 0;
static int _canvasBufferWidth = 0;
static int _canvasBufferHeight = 0;

static void CP_Canvas_Destroy(void)
{
	if (_canvasFramebuffer)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &_canvasFramebuffer);
		glDeleteRenderbuffers(1, &_canvasColorBuffer);
		glDeleteRenderbuffers(1, &_canvasDepthStencilBuffer);
	}
	_canvasFramebuffer = _canvasColorBuffer = _canvasDepthStencilBuffer = 0;
	_canvasBufferWidth = _canvasBufferHeight = 0;
}

// Make the canvas the target of everything CProcessing draws
static void CP_Canvas_Bind(void)
{
	glBindFramebuffer(GL_FRAMEBUFFER, _canvasFramebuffer);
	glViewport(0, 0, _canvasBufferWidth, _canvasBufferHeight);
}

// (Re)create the canvas at the given size, keeping whatever was already drawn
// (anchored to the top-left corner, matching CProcessing's coordinate origin).
static bool CP_Canvas_Resize(int width, int height)
{
	if (width <= 0 || height <= 0)
	{
		return false;
	}
	if (_canvasFramebuffer && width == _canvasBufferWidth && height == _canvasBufferHeight)
	{
		return true;
	}

	GLuint framebuffer = 0, color = 0, depthStencil = 0;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glGenRenderbuffers(1, &color);
	glBindRenderbuffer(GL_RENDERBUFFER, color);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color);

	// NanoVG needs a stencil buffer for fills and stencil strokes
	glGenRenderbuffers(1, &depthStencil);
	glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencil);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("CProcessing: could not create the %d x %d drawing canvas.\n", width, height);
		glBindFramebuffer(GL_FRAMEBUFFER, _canvasFramebuffer);
		glDeleteFramebuffers(1, &framebuffer);
		glDeleteRenderbuffers(1, &color);
		glDeleteRenderbuffers(1, &depthStencil);
		return false;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	if (_canvasFramebuffer)
	{
		// carry the old drawing over to the new canvas
		int copyWidth = width < _canvasBufferWidth ? width : _canvasBufferWidth;
		int copyHeight = height < _canvasBufferHeight ? height : _canvasBufferHeight;
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _canvasFramebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
		glDisable(GL_SCISSOR_TEST);
		glBlitFramebuffer(0, _canvasBufferHeight - copyHeight, copyWidth, _canvasBufferHeight,
			0, height - copyHeight, copyWidth, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glDeleteFramebuffers(1, &_canvasFramebuffer);
		glDeleteRenderbuffers(1, &_canvasColorBuffer);
		glDeleteRenderbuffers(1, &_canvasDepthStencilBuffer);
	}

	_canvasFramebuffer = framebuffer;
	_canvasColorBuffer = color;
	_canvasDepthStencilBuffer = depthStencil;
	_canvasBufferWidth = width;
	_canvasBufferHeight = height;

	CP_Canvas_Bind();
	return true;
}

// Copy the canvas to the window's back buffer (call before swapping). This
// leaves the window's framebuffer bound for the swap; CP_Canvas_Bind goes
// back to the canvas.
static void CP_Canvas_Present(void)
{
	if (!_canvasFramebuffer)
	{
		return;
	}
	int windowFbWidth = 0, windowFbHeight = 0;
	glfwGetFramebufferSize(_CORE.window, &windowFbWidth, &windowFbHeight);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, _canvasFramebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDisable(GL_SCISSOR_TEST);
	glBlitFramebuffer(0, 0, _canvasBufferWidth, _canvasBufferHeight,
		0, windowFbHeight - _canvasBufferHeight, _canvasBufferWidth, windowFbHeight,
		GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

CP_CorePtr GetCPCore(void)
{
	return &_CORE;
}

CP_DrawInfoPtr GetDrawInfo(void)
{
	return &_CORE.states[_CORE.nstates - 1];
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

	// main loop
	while (isRunning)
	{
		CP_FrameStart();
		CP_Update();

		if (_preUpdateFunction) _preUpdateFunction();

		// change states and call associated functions
		if (_stateIsChanging)
		{
			// exit current state
			if (_currState.exit) _currState.exit();

			// switch state tracking variables
			_currState.init = _nextState.init;
			_currState.update = _nextState.update;
			_currState.exit = _nextState.exit;

			// init
			if (_currState.init) _currState.init();

			_stateIsChanging = false;
		}

		if (_currState.update) _currState.update();

		if (_postUpdateFunction) _postUpdateFunction();

		CP_FrameEnd();
	}

	// Exit the current state when the program is terminating (anything it
	// draws or screenshots uses the canvas, as during a frame)
	CP_Canvas_Bind();
	if (_currState.exit) _currState.exit();

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

// Set the init, update and exit functions which CProcessing will call.
// This is aware of the current state and won't re-initialize if called with the same functions.
// update must have a valid input function, init and exit may be NULL if desired.
CP_API void CP_Engine_SetNextGameState(FunctionPtr init, FunctionPtr update, FunctionPtr exit)
{
	if (update == NULL || (_currState.init == init && _currState.update == update && _currState.exit == exit))
	{
		return;
	}

	CP_Engine_SetNextGameStateForced(init, update, exit);
}

// This forcefully overrides the current state so you can call this function
// with the same inputs and it will cause the state to exit and re-initialize.
// update must have a valid input function, init and exit may be NULL if desired.
CP_API void CP_Engine_SetNextGameStateForced(FunctionPtr init, FunctionPtr update, FunctionPtr exit)
{
	_stateIsChanging = true;
	_nextState.init = init;
	_nextState.update = update;
	_nextState.exit = exit;
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

static void CP_ApplyCursorVisible(void)
{
	glfwSetInputMode(_CORE.window, GLFW_CURSOR, _cursorVisible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
}

CP_API void CP_System_ShowCursor(CP_BOOL show)
{
	_cursorVisible = show ? TRUE : FALSE;
	if (_CORE.window)
	{
		CP_ApplyCursorVisible();
	}
}

CP_API CP_BOOL CP_System_GetCursorVisible(void)
{
	return _cursorVisible;
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
	_frametimeTarget = 1.0 / fps; // seconds per frame
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

	// Initialize first CP_DrawInfo
	_CORE.nstates = 1;
	GetDrawInfo()->rect_mode = CP_POSITION_CENTER;
	GetDrawInfo()->ellipse_mode = CP_POSITION_CENTER;
	GetDrawInfo()->image_mode = CP_POSITION_CENTER;
	GetDrawInfo()->fill = TRUE;
	GetDrawInfo()->stroke = TRUE;
	// NanoVG's own defaults (nvgReset)
	GetDrawInfo()->fill_color = CP_Color_Create(255, 255, 255, 255);
	GetDrawInfo()->stroke_color = CP_Color_Create(0, 0, 0, 255);
	GetDrawInfo()->stroke_weight = 1.0f;
	GetDrawInfo()->line_cap = CP_LINE_CAP_BUTT;

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
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
	glfwWindowHint(GLFW_DOUBLEBUFFER, 1);	// persistence comes from the canvas framebuffer
	glfwWindowHint(GLFW_RESIZABLE, 0);
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
	_CORE.nvg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
	if (_CORE.nvg == NULL)
	{
		printf("Could not init nanovg (OpenGL 3.2 or newer is required).\n");
		CP_InitializeFailed();
		return;
	}

	// Init default draw settings state items here:
	CP_Settings_Fill(CP_Color_Create(200, 200, 200, 255));
	CP_Settings_Stroke(CP_Color_Create(0, 0, 0, 255));
	CP_Settings_StrokeWeight(3.0f);
	CP_Settings_LineCapMode(CP_LINE_CAP_BUTT);
	CP_Settings_LineJointMode(CP_LINE_JOINT_BEVEL);
	CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_LINEAR);

	glfwSwapInterval(0);
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
		CP_Monitor_Center(CP_MonitorRect(glfwGetPrimaryMonitor()), _CORE.window_width, _CORE.window_height, &windowPosX, &windowPosY);
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

	// Apply a cursor setting requested before the window existed
	CP_ApplyCursorVisible();

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

void CP_Shutdown(void)
{
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

void CP_FrameStart(void)
{
	CP_FrameRate_FrameStart();

	if (_deferredSizeChange)
	{
		_deferredSizeChange = FALSE;
		CP_DeferredSetWindowSizeInternal(_deferredWidth, _deferredHeight, _deferredFullscreen);
	}

	// follow any window size change the window system has applied, and draw
	// into the canvas again (the last frame ended with the window bound)
	CP_Canvas_Resize(_CORE.canvas_width, _CORE.canvas_height);
	CP_Canvas_Bind();

	nvgBeginFrame(_CORE.nvg, (float)_CORE.window_width, (float)_CORE.window_height, _CORE.pixel_ratio);
}

void CP_FrameEnd(void)
{
	nvgEndFrame(_CORE.nvg);
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

double averageSleepCycles = 0.002;
double additionalBuffer = 0.001;	// Sleep() isn't very accurate and we want to make sure we don't over sleep

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
	else
	{
		// set the window to the middle of the monitor
		CP_Monitor_Center(CP_MonitorRect(monitor), new_width, new_height, &windowPosX, &windowPosY);
	}
	glfwSetWindowMonitor(_CORE.window, isFullscreen ? monitor : NULL, windowPosX, windowPosY, new_width, new_height, 60);
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
