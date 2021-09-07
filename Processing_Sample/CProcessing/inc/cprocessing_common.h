//---------------------------------------------------------
// CProcessing
// A functional programming library focusing on game development
//
// author:			Justin Chambers
// contributors:	Daniel Hamilton, Zachary Logsdon,
//					Andrea Ellinger, Jason Thiel
//
// documentation link:
// https://inside.digipen.edu/main/GSDP:GAM100/CProcessing
//
//---------------------------------------------------------
// Copyright (c) 2020 DigiPen Institute of Technology
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
// 
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1.	The origin of this software must not be misrepresented; you must not
//		claim that you wrote the original software.If you use this software
//		in a product, an acknowledgment in the product documentation would
//		be appreciated but is not required.
// 
// 2.	Altered source versions must be plainly marked as such, and must not
//		be misrepresented as being the original software.
// 
// 3.	This notice may not be removed or altered from any source
//		distribution.
//---------------------------------------------------------
// Library usage and acknowledgment:
//
// NanoVG by Mikko Mononen
// FMOD Studio by Firelight Technologies Pty Ltd.
//---------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

//---------------------------------------------------------
// Cleanup Warnings 

#ifdef _MSC_VER
#pragma warning(disable: 4201)  // nonstandard extension used : nameless struct/union
#pragma warning(disable: 4204)  // nonstandard extension used : non-constant aggregate initializer
#pragma warning(disable: 4090)  // different '__unaligned' qualifiers (this only shows up in x64)
#endif


//---------------------------------------------------------
// Windows Defines

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


//---------------------------------------------------------
// Eports for DLL functions

#ifdef CPROCESSING_EXPORTS
	#define DLL_EXPORT __declspec(dllexport)
#else
	#define DLL_EXPORT __declspec(dllimport)
#endif
#define CP_API DLL_EXPORT


//---------------------------------------------------------
// CProcessing Types

typedef unsigned int	CP_BOOL;
typedef struct			CP_Image_Struct* CP_Image;
typedef struct			CP_Sound_Struct* CP_Sound;
typedef struct			CP_Font_Struct* CP_Font;


//---------------------------------------------------------
// Function Pointer
typedef					void(*FunctionPtr)(void);


//---------------------------------------------------------
// COLOR:
//		Structures describing the four properties of color in both RGB and HSL color space
//		Create and change colors as an array or by accessing parameters individually
typedef union CP_Color
{
	unsigned char rgba[4];
	struct { unsigned char r, g, b, a; };
} CP_Color;

typedef union CP_ColorHSL
{
	int hsla[4];
	struct { int h, s, l, a; };
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
typedef union CP_Vector
{
	float v[2];
	struct { float x, y; };
} CP_Vector;

typedef union CP_Matrix
{
	float m[3][3];
	struct {
		float m00, m01, m02;
		float m10, m11, m12;
		float m20, m21, m22;
	};
} CP_Matrix;


//---------------------------------------------------------
// INPUT:
//		Keyboard and mouse values match GLFW
//		Gamepad uses a mapping to XINPUT
typedef enum CP_KEY
{
	KEY_ANY = 0,			// Special value for checking any key
	KEY_SPACE = 32,
	KEY_APOSTROPHE = 39,	// '''
	KEY_COMMA = 44,			// ','
	KEY_MINUS = 45,			// '-'
	KEY_PERIOD = 46,		// '.'
	KEY_SLASH = 47,			// '/'
	KEY_0 = 48,
	KEY_1 = 49,
	KEY_2 = 50,
	KEY_3 = 51,
	KEY_4 = 52,
	KEY_5 = 53,
	KEY_6 = 54,
	KEY_7 = 55,
	KEY_8 = 56,
	KEY_9 = 57,
	KEY_SEMICOLON = 59,		// ';'
	KEY_EQUAL = 61,			// '='
	KEY_A = 65,
	KEY_B = 66,
	KEY_C = 67,
	KEY_D = 68,
	KEY_E = 69,
	KEY_F = 70,
	KEY_G = 71,
	KEY_H = 72,
	KEY_I = 73,
	KEY_J = 74,
	KEY_K = 75,
	KEY_L = 76,
	KEY_M = 77,
	KEY_N = 78,
	KEY_O = 79,
	KEY_P = 80,
	KEY_Q = 81,
	KEY_R = 82,
	KEY_S = 83,
	KEY_T = 84,
	KEY_U = 85,
	KEY_V = 86,
	KEY_W = 87,
	KEY_X = 88,
	KEY_Y = 89,
	KEY_Z = 90,
	KEY_LEFT_BRACKET = 91,	// '['
	KEY_BACKSLASH = 92,		// '\'
	KEY_RIGHT_BRACKET = 93,	// ']'
	KEY_GRAVE_ACCENT = 96,	// '`'
	KEY_WORLD_1 = 161,		// non-US #1
	KEY_WORLD_2 = 162,		// non-US #2
	KEY_ESCAPE = 256,
	KEY_ENTER = 257,
	KEY_TAB = 258,
	KEY_BACKSPACE = 259,
	KEY_INSERT = 260,
	KEY_DELETE = 261,
	KEY_RIGHT = 262,
	KEY_LEFT = 263,
	KEY_DOWN = 264,
	KEY_UP = 265,
	KEY_PAGE_UP = 266,
	KEY_PAGE_DOWN = 267,
	KEY_HOME = 268,
	KEY_END = 269,
	KEY_CAPS_LOCK = 280,
	KEY_SCROLL_LOCK = 281,
	KEY_NUM_LOCK = 282,
	KEY_PRINT_SCREEN = 283,
	KEY_PAUSE = 284,
	KEY_F1 = 290,
	KEY_F2 = 291,
	KEY_F3 = 292,
	KEY_F4 = 293,
	KEY_F5 = 294,
	KEY_F6 = 295,
	KEY_F7 = 296,
	KEY_F8 = 297,
	KEY_F9 = 298,
	KEY_F10 = 299,
	KEY_F11 = 300,
	KEY_F12 = 301,
	KEY_F13 = 302,
	KEY_F14 = 303,
	KEY_F15 = 304,
	KEY_F16 = 305,
	KEY_F17 = 306,
	KEY_F18 = 307,
	KEY_F19 = 308,
	KEY_F20 = 309,
	KEY_F21 = 310,
	KEY_F22 = 311,
	KEY_F23 = 312,
	KEY_F24 = 313,
	KEY_F25 = 314,
	KEY_KP_0 = 320,
	KEY_KP_1 = 321,
	KEY_KP_2 = 322,
	KEY_KP_3 = 323,
	KEY_KP_4 = 324,
	KEY_KP_5 = 325,
	KEY_KP_6 = 326,
	KEY_KP_7 = 327,
	KEY_KP_8 = 328,
	KEY_KP_9 = 329,
	KEY_KP_DECIMAL = 330,
	KEY_KP_DIVIDE = 331,
	KEY_KP_MULTIPLY = 332,
	KEY_KP_SUBTRACT = 333,
	KEY_KP_ADD = 334,
	KEY_KP_ENTER = 335,
	KEY_KP_EQUAL = 336,
	KEY_LEFT_SHIFT = 340,
	KEY_LEFT_CONTROL = 341,
	KEY_LEFT_ALT = 342,
	KEY_LEFT_SUPER = 343,
	KEY_RIGHT_SHIFT = 344,
	KEY_RIGHT_CONTROL = 345,
	KEY_RIGHT_ALT = 346,
	KEY_RIGHT_SUPER = 347,
	KEY_MENU = 348
} CP_KEY;

typedef enum CP_MOUSE
{
	MOUSE_BUTTON_1 = 0,
	MOUSE_BUTTON_2 = 1,
	MOUSE_BUTTON_3 = 2,
	MOUSE_BUTTON_4 = 3,
	MOUSE_BUTTON_5 = 4,
	MOUSE_BUTTON_6 = 5,
	MOUSE_BUTTON_7 = 6,
	MOUSE_BUTTON_8 = 7,
	MOUSE_BUTTON_LAST = MOUSE_BUTTON_8,
	MOUSE_BUTTON_LEFT = MOUSE_BUTTON_1,
	MOUSE_BUTTON_MIDDLE = MOUSE_BUTTON_3,
	MOUSE_BUTTON_RIGHT = MOUSE_BUTTON_2
} CP_MOUSE;

static const int CP_MAX_GAMEPADS = 4;

typedef enum CP_GAMEPAD
{
	GAMEPAD_DPAD_UP,
	GAMEPAD_DPAD_DOWN,
	GAMEPAD_DPAD_LEFT,
	GAMEPAD_DPAD_RIGHT,
	GAMEPAD_START,
	GAMEPAD_BACK,
	GAMEPAD_LEFT_THUMB,
	GAMEPAD_RIGHT_THUMB,
	GAMEPAD_LEFT_SHOULDER,
	GAMEPAD_RIGHT_SHOULDER,
	GAMEPAD_A,
	GAMEPAD_B,
	GAMEPAD_X,
	GAMEPAD_Y
} CP_GAMEPAD;


#ifdef __cplusplus
}
#endif
