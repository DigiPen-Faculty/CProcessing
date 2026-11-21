//---------------------------------------------------------
// CProcessing
// A functional programming library focusing on game development
//
// author:			Justin Chambers
// contributors:	Daniel Hamilton, Zachary Logsdon,
//					Andrea Ellinger, Jason Thiel, Kenny Mecham
//
// project and documentation:
// https://github.com/DigiPen-Faculty/CProcessing
//
//---------------------------------------------------------
// SPDX-License-Identifier: MIT
// MIT License
// 
// Copyright (c) 2019-2026 DigiPen Institute of Technology and CProcessing contributors
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
// Acknowledgment:
//
// NanoVG by Mikko Mononen
// SoLoud by Jari Komppa
// GLFW by Marcus Geelnard and Camilla Loewy
// miniaudio by David Reid
// Exo 2 font (built in as the default font) by Natanael Gama,
//   licensed under the SIL Open Font License 1.1
//
//---------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "cprocessing_common.h"


//---------------------------------------------------------
// ENGINE:
//		A game state is three functions: init runs once, update every frame, exit once at the end
//		Pre- and post-update functions run every frame before and after it, whatever the state
CP_API void				CP_Engine_Run						(void);
CP_API void				CP_Engine_Terminate					(void);
CP_API void				CP_Engine_SetNextGameState			(CP_FunctionPtr init, CP_FunctionPtr update, CP_FunctionPtr exit);
CP_API void				CP_Engine_SetNextGameStateForced	(CP_FunctionPtr init, CP_FunctionPtr update, CP_FunctionPtr exit);	// Also restarts the state it's already in
CP_API void				CP_Engine_PushGameState				(CP_FunctionPtr init, CP_FunctionPtr update, CP_FunctionPtr exit);
CP_API void				CP_Engine_PopGameState				(void);
CP_API void				CP_Engine_SetGameStatePassThrough	(CP_PASSTHROUGH passThrough);
CP_API int				CP_Engine_GetGameStateDepth			(void);
CP_API CP_BOOL			CP_Engine_IsTopGameState			(void);
CP_API float			CP_Engine_GetGameStateTime			(void);	// Seconds since this state's init, not counting time frozen under another
CP_API void				CP_Engine_SetPreUpdateFunction		(CP_FunctionPtr preUpdateFunction);
CP_API void				CP_Engine_SetPostUpdateFunction		(CP_FunctionPtr postUpdateFunction);


//---------------------------------------------------------
// SYSTEM:
//		Sizes are in pixels and times in seconds, except GetMillis
CP_API void				CP_System_SetWindowSize				(int width, int height);
CP_API void				CP_System_SetWindowPosition			(int x, int y);
CP_API void				CP_System_Fullscreen				(void);
CP_API void				CP_System_FullscreenAdvanced		(int targetWidth, int targetHeight);	// Switches the monitor to the closest size it has
CP_API CP_BOOL			CP_System_IsFullscreen				(void);
CP_API int				CP_System_GetWindowWidth			(void);
CP_API int				CP_System_GetWindowHeight			(void);
CP_API int				CP_System_GetDisplayWidth			(void);
CP_API int				CP_System_GetDisplayHeight			(void);
CP_API int				CP_System_GetDisplayRefreshRate		(void);
CP_API CP_WindowHandle	CP_System_GetWindowHandle			(void);
CP_API void				CP_System_SetWindowTitle			(const char* title);
CP_API CP_BOOL			CP_System_GetWindowFocus			(void);
CP_API void				CP_System_SetWindowResizable		(CP_BOOL resizable);
CP_API CP_BOOL			CP_System_GetWindowResizable		(void);
CP_API void				CP_System_ShowCursor				(CP_BOOL show);
CP_API CP_BOOL			CP_System_GetCursorVisible			(void);
CP_API void				CP_System_SetCursorMode				(CP_CURSOR_MODE mode);
CP_API CP_CURSOR_MODE	CP_System_GetCursorMode				(void);
CP_API void				CP_System_ShowConsole				(CP_BOOL show);
CP_API CP_BOOL			CP_System_GetConsoleVisible			(void);
CP_API unsigned 		CP_System_GetFrameCount				(void);
CP_API float			CP_System_GetFrameRate				(void);
CP_API void				CP_System_SetFrameRate				(float fps);	// 0 for no limit
CP_API float			CP_System_GetTargetFrameRate		(void);
CP_API void				CP_System_SetVSync					(CP_BOOL vsync);
CP_API CP_BOOL			CP_System_GetVSync					(void);
CP_API float			CP_System_GetDt						(void);	// Seconds since the last frame, at most 0.1
CP_API float			CP_System_GetMillis					(void);	// Since the program started
CP_API float			CP_System_GetSeconds				(void);	// Since the program started


//---------------------------------------------------------
// SETTINGS:
//		Each setting stays until it's changed; Save and Restore keep and bring back all of them
//		The transform (Translate, Rotate, Scale) and the clip start over every frame
CP_API void				CP_Settings_Fill					(CP_Color c);
CP_API void				CP_Settings_NoFill					(void);
CP_API void				CP_Settings_FillLinearGradient		(float x1, float y1, CP_Color color1, float x2, float y2, CP_Color color2);
CP_API void				CP_Settings_FillRadialGradient		(float x, float y, float innerDiameter, CP_Color innerColor, float outerDiameter, CP_Color outerColor);
CP_API CP_Color			CP_Settings_GetFill					(void);
CP_API void				CP_Settings_Stroke					(CP_Color c);
CP_API void				CP_Settings_NoStroke				(void);
CP_API CP_Color			CP_Settings_GetStroke				(void);
CP_API void				CP_Settings_StrokeWeight			(float weight);
CP_API float			CP_Settings_GetStrokeWeight			(void);
CP_API void				CP_Settings_Tint					(CP_Color c);	// Multiplies the colors of everything drawn; its alpha is how strong
CP_API void				CP_Settings_NoTint					(void);
CP_API void				CP_Settings_AntiAlias				(CP_BOOL antiAlias);
CP_API void				CP_Settings_LineCapMode				(CP_LINE_CAP_MODE capMode);
CP_API void				CP_Settings_LineJointMode			(CP_LINE_JOINT_MODE jointMode);
CP_API void				CP_Settings_RectMode				(CP_POSITION_MODE mode);
CP_API void				CP_Settings_EllipseMode				(CP_POSITION_MODE mode);
CP_API void				CP_Settings_ImageMode				(CP_POSITION_MODE mode);
CP_API void				CP_Settings_BlendMode				(CP_BLEND_MODE blendMode);
CP_API void				CP_Settings_ImageFilterMode			(CP_IMAGE_FILTER_MODE filterMode);
CP_API void				CP_Settings_ImageWrapMode			(CP_IMAGE_WRAP_MODE wrapMode);
CP_API void				CP_Settings_TextSize				(float size);
CP_API float			CP_Settings_GetTextSize				(void);
CP_API void				CP_Settings_TextAlignment			(CP_TEXT_ALIGN_HORIZONTAL h, CP_TEXT_ALIGN_VERTICAL v);
CP_API void				CP_Settings_Scale					(float xScale, float yScale);
CP_API void				CP_Settings_Rotate					(float degrees);	// Clockwise, around (0, 0)
CP_API void				CP_Settings_Translate				(float x, float y);
CP_API void				CP_Settings_ApplyMatrix				(CP_Matrix matrix);
CP_API void				CP_Settings_ResetMatrix				(void);
CP_API void				CP_Settings_Clip					(float x, float y, float w, float h);	// Follows RectMode; stays put under later transforms
CP_API void				CP_Settings_NoClip					(void);
CP_API void				CP_Settings_Save					(void);
CP_API void				CP_Settings_Restore					(void);


//---------------------------------------------------------
// COLOR:
//		RGBA values are in the range of 0-255
//		HSLA values are h (0-360), s (0-100), l (0-100), a (0-255)
CP_API CP_Color			CP_Color_Create						(int r, int g, int b, int a);
CP_API CP_Color			CP_Color_CreateHex					(unsigned hexCode);	// 0xRRGGBBAA: 6 digits are interpreted as 0x00GGBBAA
CP_API CP_Color			CP_Color_Lerp						(CP_Color a, CP_Color b, float lerpFactor);
CP_API CP_Color			CP_Color_FromColorHSL				(CP_ColorHSL hsl);
CP_API CP_ColorHSL		CP_ColorHSL_Create					(int h, int s, int l, int a);
CP_API CP_ColorHSL		CP_ColorHSL_Lerp					(CP_ColorHSL a, CP_ColorHSL b, float lerpFactor);
CP_API CP_ColorHSL		CP_ColorHSL_FromColor				(CP_Color rgb);


//---------------------------------------------------------
// GRAPHICS:
//		Shapes use the fill and stroke; points, lines, arcs and Bezier curves use only the stroke
//		Positions follow RectMode and EllipseMode (CENTER by default)
//		Angles are degrees: 0 points right and positive turns clockwise (y points down)
//		A shape: BeginShape, its whole outline, then any holes, then EndShape, which closes it
CP_API void				CP_Graphics_ClearBackground			(CP_Color c);
CP_API void				CP_Graphics_DrawPoint				(float x, float y);
CP_API void				CP_Graphics_DrawLine				(float x1, float y1, float x2, float y2);
CP_API void				CP_Graphics_DrawRect				(float x, float y, float w, float h);
CP_API void				CP_Graphics_DrawRoundedRect			(float x, float y, float w, float h, float cornerRadius);
CP_API void				CP_Graphics_DrawCircle				(float x, float y, float diameter);
CP_API void				CP_Graphics_DrawEllipse				(float x, float y, float w, float h);
CP_API void				CP_Graphics_DrawArc					(float x, float y, float diameter, float startDegrees, float sweepDegrees);
CP_API void				CP_Graphics_DrawPie					(float x, float y, float diameter, float startDegrees, float sweepDegrees);
CP_API void				CP_Graphics_DrawBezier				(float x1, float y1, float controlX1, float controlY1, float controlX2, float controlY2, float x2, float y2);
CP_API void				CP_Graphics_DrawTriangle			(float x1, float y1, float x2, float y2, float x3, float y3);
CP_API void				CP_Graphics_DrawQuad				(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
CP_API void				CP_Graphics_BeginShape				(void);
CP_API void				CP_Graphics_AddVertex				(float x, float y);
CP_API void				CP_Graphics_AddQuadraticVertex		(float controlX, float controlY, float x, float y);	// Curves to (x, y), bending toward the control point
CP_API void				CP_Graphics_AddBezierVertex			(float controlX1, float controlY1, float controlX2, float controlY2, float x, float y);
CP_API void				CP_Graphics_BeginHole				(void);
CP_API void				CP_Graphics_EndHole					(void);
CP_API void				CP_Graphics_EndShape				(void);
CP_API void				CP_Graphics_EndShapeOpen			(void);


//---------------------------------------------------------
// IMAGE:
//		Positions follow ImageMode (CENTER by default), and alpha is 0-255
//		An image stays until CP_Image_Free, or until the program ends
CP_API CP_Image			CP_Image_Load						(const char* filepath);
CP_API void				CP_Image_Free						(CP_Image* img);
CP_API int				CP_Image_GetWidth					(CP_Image img);
CP_API int				CP_Image_GetHeight					(CP_Image img);
CP_API void				CP_Image_Draw						(CP_Image img, float x, float y, float w, float h, int alpha);
CP_API void				CP_Image_DrawSubImage				(CP_Image img, float x, float y, float w, float h, float left, float top, float right, float bottom, int alpha);	// Left to bottom: the part of the image to draw, in its pixels
CP_API CP_Image			CP_Image_CreateFromData				(int width, int height, const CP_Color* pixelDataInput);	// width * height colors, row by row from the top left, copied
CP_API CP_Image			CP_Image_Create						(int width, int height);
CP_API void				CP_Image_BeginDrawing				(CP_Image img);	// Draw into the image until EndDrawing (an image from CP_Image_Create)
CP_API void				CP_Image_EndDrawing					(void);
CP_API CP_Image			CP_Image_Screenshot					(int x, int y, int w, int h);
CP_API void				CP_Image_GetPixelData				(CP_Image img, CP_Color* pixelDataOutput);		// Fills an array of width * height colors that you made
CP_API void				CP_Image_UpdatePixelData			(CP_Image img, const CP_Color* pixelDataInput);	// width * height colors


//---------------------------------------------------------
// SOUND:
//		Volume: 0 is silent and 1 is normal; pitch: 1 is normal and 2 an octave up
//		A playing sound's volume and pitch multiply its group's
CP_API CP_Sound			CP_Sound_Load						(const char* filepath);
CP_API CP_Sound			CP_Sound_LoadStream					(const char* filepath);	// For long music: plays from the file instead of memory
CP_API void				CP_Sound_Free						(CP_Sound* sound);
CP_API CP_SoundInstance	CP_Sound_Play						(CP_Sound sound);	// Loaded: once, in SFX; streamed: looping, in MUSIC
CP_API CP_SoundInstance	CP_Sound_PlayAdvanced				(CP_Sound sound, float volume, float pitch, CP_BOOL looping, CP_SOUND_GROUP group);
CP_API CP_BOOL			CP_Sound_IsPlaying					(CP_SoundInstance instance);
CP_API void				CP_Sound_Stop						(CP_SoundInstance instance);
CP_API void				CP_Sound_Pause						(CP_SoundInstance instance);
CP_API void				CP_Sound_Resume						(CP_SoundInstance instance);
CP_API void				CP_Sound_SetVolume					(CP_SoundInstance instance, float volume);
CP_API void				CP_Sound_SetPitch					(CP_SoundInstance instance, float pitch);
CP_API void				CP_Sound_PauseAll					(void);
CP_API void				CP_Sound_PauseGroup					(CP_SOUND_GROUP group);
CP_API void				CP_Sound_ResumeAll					(void);
CP_API void				CP_Sound_ResumeGroup				(CP_SOUND_GROUP group);
CP_API void				CP_Sound_StopAll					(void);
CP_API void				CP_Sound_StopGroup					(CP_SOUND_GROUP group);
CP_API void				CP_Sound_SetGroupVolume				(CP_SOUND_GROUP group, float volume);
CP_API float			CP_Sound_GetGroupVolume				(CP_SOUND_GROUP group);
CP_API void				CP_Sound_SetGroupPitch				(CP_SOUND_GROUP group, float pitch);
CP_API float			CP_Sound_GetGroupPitch				(CP_SOUND_GROUP group);


//---------------------------------------------------------
// FONT:
//		Text uses the fill color, TextSize and TextAlignment
//		By default x is the left end of the text and y the line the letters sit on
CP_API CP_Font			CP_Font_GetDefault					(void);
CP_API CP_Font			CP_Font_Load						(const char* filepath);
CP_API void				CP_Font_Free						(CP_Font* font);
CP_API void				CP_Font_Set							(CP_Font font);
CP_API CP_Font			CP_Font_GetCurrent					(void);
CP_API void				CP_Font_DrawText					(const char* text, float x, float y);
CP_API void				CP_Font_DrawTextBox					(const char* text, float x, float y, float rowWidth);
CP_API float			CP_Font_GetTextWidth				(const char* text);
CP_API CP_Vector		CP_Font_GetTextSize					(const char* text);
CP_API CP_Vector		CP_Font_GetTextBoxSize				(const char* text, float rowWidth);


//---------------------------------------------------------
// INPUT:
//		Triggered: pressed this frame; Down: held down; Released: let go this frame
//		Gamepad sticks go from -1 to 1, with y positive up; triggers go from 0 to 1
//		The Advanced gamepad functions take a gamepad index from 0 to 3
CP_API CP_BOOL			CP_Input_KeyTriggered				(CP_KEY keyCode);
CP_API CP_KEY			CP_Input_GetKeyTriggered			(void);
CP_API CP_BOOL			CP_Input_KeyReleased				(CP_KEY keyCode);
CP_API CP_BOOL			CP_Input_KeyDown					(CP_KEY keyCode);
CP_API CP_BOOL			CP_Input_MouseTriggered				(CP_MOUSE button);
CP_API CP_BOOL			CP_Input_MouseReleased				(CP_MOUSE button);
CP_API CP_BOOL			CP_Input_MouseDown					(CP_MOUSE button);
CP_API CP_BOOL			CP_Input_MouseMoved					(void);
CP_API CP_BOOL			CP_Input_MouseClicked				(void);	// The left button was let go this frame
CP_API CP_BOOL			CP_Input_MouseDoubleClicked			(void);
CP_API CP_BOOL			CP_Input_MouseDragged				(CP_MOUSE button);
CP_API float			CP_Input_MouseWheel					(void);	// How far it turned this frame: positive away from you
CP_API const char*		CP_Input_GetTypedText				(void);	// Typed in the last frame, as UTF-8; copy it to keep it
CP_API float			CP_Input_GetMouseX					(void);
CP_API float			CP_Input_GetMouseY					(void);
CP_API float			CP_Input_GetMousePreviousX			(void);
CP_API float			CP_Input_GetMousePreviousY			(void);
CP_API float			CP_Input_GetMouseDeltaX				(void);
CP_API float			CP_Input_GetMouseDeltaY				(void);
CP_API float			CP_Input_GetMouseWorldX				(void);
CP_API float			CP_Input_GetMouseWorldY				(void);
CP_API CP_BOOL			CP_Input_GamepadTriggered			(CP_GAMEPAD button);
CP_API CP_BOOL			CP_Input_GamepadTriggeredAdvanced	(CP_GAMEPAD button, unsigned gamepadIndex);
CP_API CP_BOOL			CP_Input_GamepadReleased			(CP_GAMEPAD button);
CP_API CP_BOOL			CP_Input_GamepadReleasedAdvanced	(CP_GAMEPAD button, unsigned gamepadIndex);
CP_API CP_BOOL			CP_Input_GamepadDown				(CP_GAMEPAD button);
CP_API CP_BOOL			CP_Input_GamepadDownAdvanced		(CP_GAMEPAD button, unsigned gamepadIndex);
CP_API float			CP_Input_GamepadRightTrigger		(void);
CP_API float			CP_Input_GamepadRightTriggerAdvanced(unsigned gamepadIndex);
CP_API float			CP_Input_GamepadLeftTrigger			(void);
CP_API float			CP_Input_GamepadLeftTriggerAdvanced	(unsigned gamepadIndex);
CP_API CP_Vector		CP_Input_GamepadRightStick			(void);
CP_API CP_Vector		CP_Input_GamepadRightStickAdvanced	(unsigned gamepadIndex);
CP_API CP_Vector		CP_Input_GamepadLeftStick			(void);
CP_API CP_Vector		CP_Input_GamepadLeftStickAdvanced	(unsigned gamepadIndex);
CP_API CP_BOOL			CP_Input_GamepadConnected			(void);
CP_API CP_BOOL			CP_Input_GamepadConnectedAdvanced	(unsigned gamepadIndex);


//---------------------------------------------------------
// MATH:
//		Lerp factors go from 0 (gives a) to 1 (gives b) and are kept in that range; Map's aren't
CP_API int				CP_Math_ClampInt					(int value, int min, int max);
CP_API float			CP_Math_ClampFloat					(float value, float min, float max);
CP_API int				CP_Math_LerpInt						(int a, int b, float lerpFactor);
CP_API float			CP_Math_LerpFloat					(float a, float b, float lerpFactor);
CP_API float			CP_Math_Square						(float value);
CP_API float			CP_Math_Distance					(float x1, float y1, float x2, float y2);
CP_API float			CP_Math_Degrees						(float radians);
CP_API float			CP_Math_Radians						(float degrees);
CP_API float			CP_Math_Map							(float value, float inMin, float inMax, float outMin, float outMax);
CP_API void				CP_Math_ScreenToWorld				(float xIn, float yIn, float* xOut, float* yOut);	// Through the current transform
CP_API void				CP_Math_WorldToScreen				(float xIn, float yIn, float* xOut, float* yOut);	// Through the current transform


//---------------------------------------------------------
// VECTOR:
//		Angles are degrees: 0 points right and positive turns clockwise (y points down)
CP_API CP_Vector		CP_Vector_Set						(float x, float y);
CP_API CP_Vector		CP_Vector_Zero						(void);
CP_API CP_Vector		CP_Vector_Negate					(CP_Vector vec);
CP_API CP_Vector		CP_Vector_Add						(CP_Vector a, CP_Vector b);
CP_API CP_Vector		CP_Vector_Subtract					(CP_Vector a, CP_Vector b);
CP_API CP_Vector		CP_Vector_Scale						(CP_Vector vec, float scale);
CP_API CP_Vector		CP_Vector_Normalize					(CP_Vector vec);	// A zero vector stays zero
CP_API CP_Vector		CP_Vector_Lerp						(CP_Vector a, CP_Vector b, float lerpFactor);
CP_API CP_Vector		CP_Vector_Rotate					(CP_Vector vec, float degrees);
CP_API CP_Vector		CP_Vector_FromAngle					(float degrees);
CP_API CP_Vector		CP_Vector_Limit						(CP_Vector vec, float maxLength);
CP_API CP_Vector		CP_Vector_MatrixMultiply			(CP_Matrix mat, CP_Vector vec);
CP_API float			CP_Vector_Length					(CP_Vector vec);
CP_API float			CP_Vector_Distance					(CP_Vector a, CP_Vector b);
CP_API float			CP_Vector_DotProduct				(CP_Vector a, CP_Vector b);
CP_API float			CP_Vector_CrossProduct				(CP_Vector a, CP_Vector b);		// Positive when b is clockwise from a
CP_API float			CP_Vector_Angle						(CP_Vector a, CP_Vector b);		// 0 to 180, the shorter way
CP_API float			CP_Vector_AngleCW					(CP_Vector from, CP_Vector to);	// 0 to 360
CP_API float			CP_Vector_AngleCCW					(CP_Vector from, CP_Vector to);	// 0 to -360


//---------------------------------------------------------
// MATRIX:
//		2D transforms, for CP_Settings_ApplyMatrix and CP_Vector_MatrixMultiply
CP_API CP_Matrix		CP_Matrix_Set						(float m00, float m01, float m02,
															 float m10, float m11, float m12,
															 float m20, float m21, float m22);
CP_API CP_Matrix		CP_Matrix_Identity					(void);
CP_API CP_Matrix		CP_Matrix_FromVector				(CP_Vector col1, CP_Vector col2, CP_Vector col3);
CP_API CP_Matrix		CP_Matrix_Scale						(CP_Vector scale);
CP_API CP_Matrix		CP_Matrix_Translate					(CP_Vector offset);
CP_API CP_Matrix		CP_Matrix_Rotate					(float degrees);
CP_API CP_Matrix		CP_Matrix_RotateRadians				(float radians);
CP_API CP_Matrix		CP_Matrix_Transpose					(CP_Matrix original);
CP_API CP_Matrix		CP_Matrix_Inverse					(CP_Matrix original);	// The identity if there is none (a scale of 0)
CP_API CP_Matrix		CP_Matrix_Multiply					(CP_Matrix a, CP_Matrix b);


//---------------------------------------------------------
// RANDOM:
//		RangeInt includes both bounds; GetFloat and RangeFloat never reach the upper one
//		Noise has its own seed, so a new NoiseSeed doesn't change the other random numbers
CP_API CP_BOOL			CP_Random_GetBool					(void);
CP_API unsigned int		CP_Random_GetInt					(void);
CP_API int				CP_Random_RangeInt					(int lowerBound, int upperBound);
CP_API float			CP_Random_GetFloat					(void);
CP_API float			CP_Random_RangeFloat				(float lowerBound, float upperBound);
CP_API void				CP_Random_Seed						(int seed);
CP_API float			CP_Random_Gaussian					(void);							// Average 0, standard deviation 1
CP_API float			CP_Random_Noise						(float x, float y, float z);	// 0 to 1: points close together give close values
CP_API float			CP_Random_NoiseOctaves				(float x, float y, float z, int octaves, float persistence);
CP_API void				CP_Random_NoiseSeed					(int seed);


#ifdef __cplusplus
}
#endif
