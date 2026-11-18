//---------------------------------------------------------
// file:	ColorPickerDemo.h
//
// An interactive color picker, the companion to Lesson 3: Color.
//
// CProcessing features in this demo:
// CP_Color_Create, CP_Color_Lerp	- RGB colors, and the colors between two colors
// CP_ColorHSL_Create				- HSL colors
// CP_Color_FromColorHSL			- convert HSL to RGB
// CP_ColorHSL_FromColor			- convert RGB to HSL
// CP_Input_MouseDown				- drag the sliders
//---------------------------------------------------------

#pragma once

void ColorPickerDemoInit(void);
void ColorPickerDemoUpdate(void);
void ColorPickerDemoExit(void);
