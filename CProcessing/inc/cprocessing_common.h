//---------------------------------------------------------
// CProcessing
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology and CProcessing contributors
// 
// https://github.com/DigiPen-Faculty/CProcessing
//---------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

//---------------------------------------------------------
// Version of this CProcessing release: major.minor.patch
// (CMakeLists.txt reads the project version from these lines)

#define CP_VERSION_MAJOR 3
#define CP_VERSION_MINOR 1
#define CP_VERSION_PATCH 0


//---------------------------------------------------------
// Exports for shared library functions
//
// This header is the same on every platform and includes no OS headers.
// (Projects that want Windows APIs include <windows.h> themselves.)
//
// CPROCESSING_EXPORTS - defined while building the library itself
// CPROCESSING_STATIC  - defined when building/using CProcessing as a static library

#if defined(CPROCESSING_STATIC)
	#define CP_API
#elif defined(_WIN32)
	#ifdef CPROCESSING_EXPORTS
		#define CP_API __declspec(dllexport)
	#else
		#define CP_API __declspec(dllimport)
	#endif
#elif defined(__GNUC__) || defined(__clang__)
	#define CP_API __attribute__((visibility("default")))
#else
	#define CP_API
#endif


//---------------------------------------------------------
// CProcessing Types

typedef unsigned int	CP_BOOL;

// Pi, for angles in radians (the Rotate and Angle functions take degrees)
#define CP_PI			3.14159265358979323846f
#define CP_TWO_PI		6.28318530717958647692f
#define CP_HALF_PI		1.57079632679489661923f

// Values for CP_BOOL
#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

// Native OS window handle returned by CP_System_GetWindowHandle; cast it to
// the platform's own type:
//   Windows - HWND
//   macOS   - NSWindow*
//   Linux   - X11 Window id (cast to a pointer) or NULL under Wayland
typedef void*			CP_WindowHandle;
typedef struct			CP_Image_Struct* CP_Image;
typedef struct			CP_Sound_Struct* CP_Sound;

// One playing sound, returned by CP_Sound_Play and CP_Sound_PlayAdvanced and
// used to stop, pause or change just that sound. 0 means no sound.
typedef unsigned int	CP_SoundInstance;
typedef struct			CP_Font_Struct* CP_Font;


//---------------------------------------------------------
// Function Pointer
typedef					void(*CP_FunctionPtr)(void);


//---------------------------------------------------------
// COLOR:
//		Structures describing the four properties of color in both RGB and HSL color space
//		Create and change colors by accessing parameters individually: color.r, color.g, color.b, color.a
typedef struct CP_Color
{
	unsigned char r, g, b, a;
} CP_Color;

typedef struct CP_ColorHSL
{
	int h, s, l, a;
} CP_ColorHSL;


//---------------------------------------------------------
// LINE CAP and JOINT MODE:
//		Cap controls how the end of the line is drawn
//		Joint controls how sharp corners are drawn
typedef enum CP_LINE_CAP_MODE
{
	CP_LINE_CAP_BUTT = 0,		// Default
	CP_LINE_CAP_ROUND = 1,
	CP_LINE_CAP_SQUARE = 2
} CP_LINE_CAP_MODE;

typedef enum CP_LINE_JOINT_MODE
{
	CP_LINE_JOINT_ROUND = 1,
	CP_LINE_JOINT_BEVEL = 3,	// Default
	CP_LINE_JOINT_MITER = 4
} CP_LINE_JOINT_MODE;


//---------------------------------------------------------
// POSITION MODE:
//		Change the local origin for draw functions
//		Shape and Image positions are calculated from their center or top left corner
typedef enum CP_POSITION_MODE
{
	CP_POSITION_CENTER,
	CP_POSITION_CORNER	// Top Left
} CP_POSITION_MODE;


//---------------------------------------------------------
// BLEND MODE:
//		Change the math applied to colors when drawn on top of each other
typedef enum CP_BLEND_MODE
{
	CP_BLEND_ALPHA,		// BLEND
	CP_BLEND_ADD,		// ADD
	CP_BLEND_SUBTRACT,	// DIFFERENCE
	CP_BLEND_MULTIPLY,	// MULTIPLY
	CP_BLEND_MIN,		// DARKEST
	CP_BLEND_MAX		// LIGHTEST
} CP_BLEND_MODE;


//---------------------------------------------------------
// IMAGE FILTER MODE:
//		Nearest - pixel perfect is good for retro games and pixel art
//		Linear - applies bilinear filtering to images and smooths things out
typedef enum CP_IMAGE_FILTER_MODE
{
	CP_IMAGE_FILTER_NEAREST,
	CP_IMAGE_FILTER_LINEAR
} CP_IMAGE_FILTER_MODE;


//---------------------------------------------------------
// IMAGE WRAP MODE:
//		Controls what happens to the edge colors of textures
typedef enum CP_IMAGE_WRAP_MODE
{
	CP_IMAGE_WRAP_CLAMP,
	CP_IMAGE_WRAP_CLAMP_EDGE,
	CP_IMAGE_WRAP_REPEAT,
	CP_IMAGE_WRAP_MIRROR
} CP_IMAGE_WRAP_MODE;


//---------------------------------------------------------
// TEXT ALIGN:
//		Horizontal and vertical text alignment settings
typedef enum CP_TEXT_ALIGN_HORIZONTAL
{
	CP_TEXT_ALIGN_H_LEFT		= 1 << 0,	// Default, align text horizontally to left.
	CP_TEXT_ALIGN_H_CENTER		= 1 << 1,	// Align text horizontally to center.
	CP_TEXT_ALIGN_H_RIGHT		= 1 << 2	// Align text horizontally to right.
} CP_TEXT_ALIGN_HORIZONTAL;

typedef enum CP_TEXT_ALIGN_VERTICAL
{
	CP_TEXT_ALIGN_V_TOP			= 1 << 3,	// Align text vertically to top.
	CP_TEXT_ALIGN_V_MIDDLE		= 1 << 4,	// Align text vertically to middle.
	CP_TEXT_ALIGN_V_BOTTOM		= 1 << 5,	// Align text vertically to bottom.
	CP_TEXT_ALIGN_V_BASELINE	= 1 << 6	// Default, align text vertically to baseline.
} CP_TEXT_ALIGN_VERTICAL;


//---------------------------------------------------------
// SOUND GROUP:
//		Organize sounds into separate groups to play/pause and control pitch and volume separately
typedef enum CP_SOUND_GROUP
{
	CP_SOUND_GROUP_0,
	CP_SOUND_GROUP_1,
	CP_SOUND_GROUP_2,
	CP_SOUND_GROUP_3,
	CP_SOUND_GROUP_4,
	CP_SOUND_GROUP_5,
	CP_SOUND_GROUP_6,
	CP_SOUND_GROUP_7,
	CP_SOUND_GROUP_8,
	CP_SOUND_GROUP_9,
	CP_SOUND_GROUP_MAX,
	CP_SOUND_GROUP_SFX = CP_SOUND_GROUP_0,
	CP_SOUND_GROUP_MUSIC = CP_SOUND_GROUP_1
} CP_SOUND_GROUP;


//---------------------------------------------------------
// MATH:
//		2D vector (x, y) and 3x3 matrix useful for basic linear algebra
typedef struct CP_Vector
{
	float x, y;
} CP_Vector;

typedef struct CP_Matrix
{
	float m00, m01, m02;
	float m10, m11, m12;
	float m20, m21, m22;
} CP_Matrix;


//---------------------------------------------------------
// INPUT:
//		Keyboard and mouse values match GLFW
//		Gamepad buttons follow the Xbox controller layout (via GLFW's gamepad mappings)
typedef enum CP_KEY
{
	CP_KEY_ANY = 0,			// Special value for checking any key
	CP_KEY_SPACE = 32,
	CP_KEY_APOSTROPHE = 39,	// '''
	CP_KEY_COMMA = 44,			// ','
	CP_KEY_MINUS = 45,			// '-'
	CP_KEY_PERIOD = 46,		// '.'
	CP_KEY_SLASH = 47,			// '/'
	CP_KEY_0 = 48,
	CP_KEY_1 = 49,
	CP_KEY_2 = 50,
	CP_KEY_3 = 51,
	CP_KEY_4 = 52,
	CP_KEY_5 = 53,
	CP_KEY_6 = 54,
	CP_KEY_7 = 55,
	CP_KEY_8 = 56,
	CP_KEY_9 = 57,
	CP_KEY_SEMICOLON = 59,		// ';'
	CP_KEY_EQUAL = 61,			// '='
	CP_KEY_A = 65,
	CP_KEY_B = 66,
	CP_KEY_C = 67,
	CP_KEY_D = 68,
	CP_KEY_E = 69,
	CP_KEY_F = 70,
	CP_KEY_G = 71,
	CP_KEY_H = 72,
	CP_KEY_I = 73,
	CP_KEY_J = 74,
	CP_KEY_K = 75,
	CP_KEY_L = 76,
	CP_KEY_M = 77,
	CP_KEY_N = 78,
	CP_KEY_O = 79,
	CP_KEY_P = 80,
	CP_KEY_Q = 81,
	CP_KEY_R = 82,
	CP_KEY_S = 83,
	CP_KEY_T = 84,
	CP_KEY_U = 85,
	CP_KEY_V = 86,
	CP_KEY_W = 87,
	CP_KEY_X = 88,
	CP_KEY_Y = 89,
	CP_KEY_Z = 90,
	CP_KEY_LEFT_BRACKET = 91,	// '['
	CP_KEY_BACKSLASH = 92,		// '\'
	CP_KEY_RIGHT_BRACKET = 93,	// ']'
	CP_KEY_GRAVE_ACCENT = 96,	// '`'
	CP_KEY_WORLD_1 = 161,		// non-US #1
	CP_KEY_WORLD_2 = 162,		// non-US #2
	CP_KEY_ESCAPE = 256,
	CP_KEY_ENTER = 257,
	CP_KEY_TAB = 258,
	CP_KEY_BACKSPACE = 259,
	CP_KEY_INSERT = 260,
	CP_KEY_DELETE = 261,
	CP_KEY_RIGHT = 262,
	CP_KEY_LEFT = 263,
	CP_KEY_DOWN = 264,
	CP_KEY_UP = 265,
	CP_KEY_PAGE_UP = 266,
	CP_KEY_PAGE_DOWN = 267,
	CP_KEY_HOME = 268,
	CP_KEY_END = 269,
	CP_KEY_CAPS_LOCK = 280,
	CP_KEY_SCROLL_LOCK = 281,
	CP_KEY_NUM_LOCK = 282,
	CP_KEY_PRINT_SCREEN = 283,
	CP_KEY_PAUSE = 284,
	CP_KEY_F1 = 290,
	CP_KEY_F2 = 291,
	CP_KEY_F3 = 292,
	CP_KEY_F4 = 293,
	CP_KEY_F5 = 294,
	CP_KEY_F6 = 295,
	CP_KEY_F7 = 296,
	CP_KEY_F8 = 297,
	CP_KEY_F9 = 298,
	CP_KEY_F10 = 299,
	CP_KEY_F11 = 300,
	CP_KEY_F12 = 301,
	CP_KEY_F13 = 302,
	CP_KEY_F14 = 303,
	CP_KEY_F15 = 304,
	CP_KEY_F16 = 305,
	CP_KEY_F17 = 306,
	CP_KEY_F18 = 307,
	CP_KEY_F19 = 308,
	CP_KEY_F20 = 309,
	CP_KEY_F21 = 310,
	CP_KEY_F22 = 311,
	CP_KEY_F23 = 312,
	CP_KEY_F24 = 313,
	CP_KEY_F25 = 314,
	CP_KEY_KP_0 = 320,
	CP_KEY_KP_1 = 321,
	CP_KEY_KP_2 = 322,
	CP_KEY_KP_3 = 323,
	CP_KEY_KP_4 = 324,
	CP_KEY_KP_5 = 325,
	CP_KEY_KP_6 = 326,
	CP_KEY_KP_7 = 327,
	CP_KEY_KP_8 = 328,
	CP_KEY_KP_9 = 329,
	CP_KEY_KP_DECIMAL = 330,
	CP_KEY_KP_DIVIDE = 331,
	CP_KEY_KP_MULTIPLY = 332,
	CP_KEY_KP_SUBTRACT = 333,
	CP_KEY_KP_ADD = 334,
	CP_KEY_KP_ENTER = 335,
	CP_KEY_KP_EQUAL = 336,
	CP_KEY_LEFT_SHIFT = 340,
	CP_KEY_LEFT_CONTROL = 341,
	CP_KEY_LEFT_ALT = 342,
	CP_KEY_LEFT_SUPER = 343,
	CP_KEY_RIGHT_SHIFT = 344,
	CP_KEY_RIGHT_CONTROL = 345,
	CP_KEY_RIGHT_ALT = 346,
	CP_KEY_RIGHT_SUPER = 347,
	CP_KEY_MENU = 348
} CP_KEY;

typedef enum CP_MOUSE
{
	CP_MOUSE_1 = 0,
	CP_MOUSE_2 = 1,
	CP_MOUSE_3 = 2,
	CP_MOUSE_4 = 3,
	CP_MOUSE_5 = 4,
	CP_MOUSE_6 = 5,
	CP_MOUSE_7 = 6,
	CP_MOUSE_8 = 7,
	CP_MOUSE_LAST = CP_MOUSE_8,
	CP_MOUSE_LEFT = CP_MOUSE_1,
	CP_MOUSE_MIDDLE = CP_MOUSE_3,
	CP_MOUSE_RIGHT = CP_MOUSE_2
} CP_MOUSE;

#define CP_MAX_GAMEPADS 4

typedef enum CP_GAMEPAD
{
	CP_GAMEPAD_DPAD_UP,
	CP_GAMEPAD_DPAD_DOWN,
	CP_GAMEPAD_DPAD_LEFT,
	CP_GAMEPAD_DPAD_RIGHT,
	CP_GAMEPAD_START,
	CP_GAMEPAD_BACK,
	CP_GAMEPAD_LEFT_THUMB,
	CP_GAMEPAD_RIGHT_THUMB,
	CP_GAMEPAD_LEFT_SHOULDER,
	CP_GAMEPAD_RIGHT_SHOULDER,
	CP_GAMEPAD_A,
	CP_GAMEPAD_B,
	CP_GAMEPAD_X,
	CP_GAMEPAD_Y
} CP_GAMEPAD;

typedef enum CP_CURSOR_MODE
{
	CP_CURSOR_NORMAL,	// Default, visible and free to leave the window
	CP_CURSOR_HIDDEN,	// Hidden over the window
	CP_CURSOR_CAPTURED,	// Visible and kept in the window (not on macOS)
	CP_CURSOR_DISABLED	// Hidden and unlimited, for mouse look: use the mouse delta
} CP_CURSOR_MODE;

//---------------------------------------------------------
// Game states

// What a game state lets the states under it do: its pass-through
// (CP_Engine_SetGameStatePassThrough). Only the top state gets input.
// A set of bits, so the names below can also be combined with |; it isn't
// an enum type, so that works in C++ too.
typedef unsigned int CP_STATE;
enum
{
	CP_STATE_UPDATING	= 1 << 0,	// their update functions run
	CP_STATE_VISIBLE	= 1 << 1,	// they can be seen (their last picture, if they aren't updating)

	CP_STATE_NONE		= 0,		// they are kept, but don't run and can't be seen
	CP_STATE_FROZEN		= CP_STATE_VISIBLE,							// Default: they stop, and stay visible
	CP_STATE_PASSIVE	= CP_STATE_UPDATING | CP_STATE_VISIBLE		// they keep running and drawing, without input
};


#ifdef __cplusplus
}
#endif
