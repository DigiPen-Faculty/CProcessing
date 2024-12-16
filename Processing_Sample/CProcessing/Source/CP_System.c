//---------------------------------------------------------
// file:	CProcessing.c
// author:	Justin Chambers
// brief:	Primary implementation of the CProcessing interface
//
// Copyright © 2019 DigiPen, All rights reserved.
//---------------------------------------------------------

#include "cprocessing.h"
#include "Internal_System.h"
#include "nanovg_gl.h"
#include "tinycthread.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw3native.h"

#define isRunning !glfwWindowShouldClose(_CORE.window)

// Internal information
static CP_Core _CORE = { 0 };
static bool _isInitialized = false;

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

CP_CorePtr GetCPCore(void)
{
	return &_CORE;
}

CP_DrawInfoPtr GetDrawInfo(void)
{
	return &_CORE.states[_CORE.nstates - 1];
}

void SetCPCoreValues(void)
{
	_CORE.nvg			= NULL;
	_CORE.window		= NULL;
	_CORE.hwnd			= NULL;
	_CORE.window_width	= 0;
	_CORE.window_height	= 0;
	_CORE.canvas_width	= 0;
	_CORE.canvas_height	= 0;
	_CORE.native_width	= 0;
	_CORE.native_height	= 0;
	_CORE.isFullscreen	= FALSE;
	_CORE.pixel_ratio	= 1.0f;
	_CORE.window_posX	= -1;
	_CORE.window_posY	= -1;
	_CORE.nstates		= 0;
	memset(_CORE.states, 0, sizeof(CP_DrawInfo) * CP_MAX_STATES);
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

	// Exit the current state when the program is terminating
	if (_currState.exit) _currState.exit();

	CP_Shutdown();
}

CP_API void CP_Engine_Terminate(void)
{
	// mark the program for termination
	glfwSetWindowShouldClose(GetCPCore()->window, GL_TRUE);
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
		glfwSetWindowPos(_CORE.window, x, y);
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

CP_API int CP_System_GetWindowWidth(void)
{
	return _CORE.canvas_width;
}

CP_API int CP_System_GetWindowHeight(void)
{
	return _CORE.canvas_height;
}

CP_API int CP_System_GetDisplayWidth(void)
{
	return _CORE.native_width;
}

CP_API int CP_System_GetDisplayHeight(void)
{
	return _CORE.native_height;
}

CP_API int CP_System_GetDisplayRefreshRate(void)
{
	return glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate;
}

CP_API HWND CP_System_GetWindowHandle(void)
{
	return _CORE.hwnd;
}

CP_API void CP_System_SetWindowTitle(const char* title)
{
	glfwSetWindowTitle(_CORE.window, title);
}

CP_API CP_BOOL CP_System_GetWindowFocus(void)
{
	return glfwGetWindowAttrib(_CORE.window, GLFW_FOCUSED);
}

CP_API void CP_System_ShowCursor(CP_BOOL show)
{
	glfwSetInputMode(_CORE.window, GLFW_CURSOR, show ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
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

	// Initialize GLFW
	if (!glfwInit()) {
		printf("Failed to init GLFW.");
	}

	// we need GLFW to query the monitor, then set the correct resolution for the window
	const GLFWvidmode* structure = glfwGetVideoMode(glfwGetPrimaryMonitor());
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

	// Set error call back and create the window
	glfwSetErrorCallback(error_callback_glfw);
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
	glfwWindowHint(GLFW_DOUBLEBUFFER, 0);
	glfwWindowHint(GLFW_RESIZABLE, 0);
	glfwWindowHint(GLFW_VISIBLE, 0);
	_CORE.window = glfwCreateWindow(_CORE.window_width, _CORE.window_height, "CProcessing Application", _CORE.isFullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);

	if (!_CORE.window) {
		glfwTerminate();
	}

	_CORE.hwnd = glfwGetWin32Window(_CORE.window);

	glfwMakeContextCurrent(_CORE.window);
	gladLoadGL();
	_CORE.nvg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
	if (_CORE.nvg == NULL)
	{
		printf("Could not init nanovg.\n");
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
	// Calculate pixel ratio for hi-dpi devices.
	_CORE.pixel_ratio = (float)_CORE.canvas_width / (float)_CORE.window_width;

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
		// set the window to the middle of the screen
		structure = glfwGetVideoMode(glfwGetPrimaryMonitor());	// already requested above
		windowPosX = (structure->width / 2) - (_CORE.window_width / 2);
		windowPosY = (structure->height / 2) - (_CORE.window_height / 2);
	}
	glfwSetWindowPos(_CORE.window, windowPosX, windowPosY);

	// Update and render
	glViewport(0, 0, _CORE.canvas_width, _CORE.canvas_height);

	// Set readable front buffer for screen shot functionality
	glReadBuffer(GL_FRONT);

	// Set the background color
	CP_Graphics_ClearBackground(CP_Color_Create(150, 150, 150, 255));

	// Set input callback
	glfwSetInputMode(_CORE.window, GLFW_STICKY_KEYS, 1);
	glfwSetKeyCallback(_CORE.window, CP_Input_KeyboardCallback);
	glfwSetMouseButtonCallback(_CORE.window, CP_Input_MouseCallback);
	glfwSetScrollCallback(_CORE.window, CP_Input_MouseWheelCallback);

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

	// Clean up glfw and nvg
	glfwTerminate();
	nvgDeleteGL3(_CORE.nvg);
}

void CP_FrameStart(void)
{
	CP_FrameRate_FrameStart();

	nvgBeginFrame(_CORE.nvg, _CORE.window_width, _CORE.window_height, _CORE.pixel_ratio);
}

void CP_FrameEnd(void)
{
	nvgEndFrame(_CORE.nvg);
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
	if (!_CORE.nvg)
	{
		// this will only happen if setting size before OpenGL init
		// use these sizes to override defaults during init
		_CORE.canvas_width = new_width;
		_CORE.canvas_height = new_height;
		_CORE.isFullscreen = isFullscreen;
		return;
	}

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
		// set the window to the middle of the screen
		windowPosX = (int)(_CORE.native_width / 2.0f) - (int)(new_width / 2.0f);
		windowPosY = (int)(_CORE.native_height / 2.0f) - (int)(new_height / 2.0f);
	}
	glfwSetWindowMonitor(_CORE.window, isFullscreen ? glfwGetPrimaryMonitor() : NULL, windowPosX, windowPosY, new_width, new_height, 60);

	// get and store size values based on the actual window
	glfwGetFramebufferSize(_CORE.window, &_CORE.canvas_width, &_CORE.canvas_height);
	glfwGetWindowSize(_CORE.window, &_CORE.window_width, &_CORE.window_height);
	// Calculate pixel ratio for hi-dpi devices.
	_CORE.pixel_ratio = (float)_CORE.canvas_width / (float)_CORE.window_width;
	// update openGL frame size
	glViewport(0, 0, _CORE.canvas_width, _CORE.canvas_height);
}
