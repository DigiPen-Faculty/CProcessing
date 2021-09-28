//---------------------------------------------------------
// file:	main.c
// author:	Justin Chambers
// brief:	Main entry point for the sample usage project
//			of the CProcessing library
//
// Copyright © 2019 DigiPen, All rights reserved.
//---------------------------------------------------------

#include <windows.h>
#include "cprocessing.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define _USE_MATH_DEFINES
#include <math.h>

// #ifdef height
// #undef height
// #define height 1500
// #endif

//void debugInputChecks();

int alpha1 = 255;
const int rectangles = 100;
int red1[2500][2500] = { 0 };
int startup = 1;
CP_Font font;

const char * lorem = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
const char * lorem_short = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.";//Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";


// Pre Update Function with master settings switches

static bool theScreenIsFull = false;
static bool isAntiAliasing = true;

void PreUpdateSettingsToggle(void)
{
	if (CP_Input_KeyReleased(KEY_SPACE))
	{
		if (theScreenIsFull)
		{
			CP_System_SetWindowSize(400, 400);
		}
		else
		{
			CP_System_Fullscreen();
		}
		theScreenIsFull = !theScreenIsFull;
	}

	if (CP_Input_KeyReleased(KEY_A))
	{
		isAntiAliasing = !isAntiAliasing;
		CP_Settings_AntiAlias(isAntiAliasing);
	}

	if (CP_Input_KeyReleased(KEY_P))
	{
		printf("frameCount: %i\n", CP_System_GetFrameCount());
	}
}




CP_Image img, imgt;
CP_Image justin1;
CP_Image justin2;

int iw, ih;
void image_test_setup(void)
{
	CP_System_SetWindowSize(1000, 1000);
	//CP_System_Fullscreen();
	// set flags to store pixel data so we can sample them and do cool things.
	img = CP_Image_Load("./Assets/image.png");
	justin1 = CP_Image_Load("./Assets/justin1.png");
	justin2 = CP_Image_Load("./Assets/justin2.png");

	CP_Settings_ImageMode(CP_POSITION_CENTER);

	iw = CP_Image_GetWidth(img);
	ih = CP_Image_GetHeight(img);
}

void image_test(void)
{
	CP_Settings_ImageMode(CP_POSITION_CORNER);
    //CP_Image_Draw(img, 0, 0, (float)iw, (float)ih);

    CP_Settings_Fill(CP_Color_Create(0, 0, 0, 255));
    CP_Font_DrawText("Right Click to ZOOM", 50, 20);
    CP_Font_DrawText("Left Click to view pixel color", 50, 40);

    if (CP_Input_MouseDown(MOUSE_BUTTON_MIDDLE))
    {
		CP_Image_DrawSubImage(img, CP_Input_GetMouseX(), CP_Input_GetMouseY(), 100, 100, CP_Input_GetMouseX(), CP_Input_GetMouseY(), 200, 200, 255);
		CP_Image_DrawSubImage(img, CP_Input_GetMouseX() - 100, CP_Input_GetMouseY(), 100, 100, CP_Input_GetMouseX(), CP_Input_GetMouseY(), 200, 200, 255);
    }
    if (CP_Input_MouseDown(MOUSE_BUTTON_RIGHT))
    {
		static CP_Image shot = NULL;
		CP_Image_Free(&shot);
        shot = CP_Image_Screenshot(0, 0, CP_System_GetWindowWidth(), CP_System_GetWindowHeight());
		CP_Image_Draw(shot, CP_Input_GetMouseX(), CP_Input_GetMouseY(), 100, 100, 255);
    }

    if (CP_Input_MouseDown(MOUSE_BUTTON_LEFT))
    {
		CP_Color mycol = { 255, 255, 255, 255 }; //  GET PIXEL WAS DEPRECATED CP_Image_GetPixel(img, (int)CP_Input_GetMouseX(), (int)CP_Input_GetMouseY());
        CP_Settings_Fill(mycol);
        CP_Settings_RectMode(CP_POSITION_CORNER);
        CP_Settings_NoStroke();

		// set tint color
		CP_Settings_Tint(mycol);
		CP_Image_Draw(img, 0, 0, (float)CP_System_GetWindowWidth(), (float)CP_System_GetWindowHeight(), 255);
		// restore the tint color
		CP_Settings_NoTint();
        
		CP_Graphics_DrawRect(CP_Input_GetMouseX(), CP_Input_GetMouseY(), 50, 50);
    }
	CP_Settings_ImageMode(CP_POSITION_CENTER);
	if (CP_Input_MouseDown(MOUSE_BUTTON_LEFT))
	{
        //CP_Image_Draw(justin1, CP_Input_GetMouseX(), CP_Input_GetMouseY(), 323.f, 547.f, 255);
	}
	else
	{
        //CP_Image_Draw(justin2, CP_Input_GetMouseX(), CP_Input_GetMouseY(), -323.f, 547.f, 255);
	}
}

void random_background_rectangles(void)
{
  CP_System_SetWindowSize(1200, 800);
	if (startup)
	{
		startup = 0;
		for (int y = 0; y < CP_System_GetWindowHeight(); y += CP_System_GetWindowHeight() / rectangles)
		{
			for (int x = 0; x < CP_System_GetWindowWidth(); x += CP_System_GetWindowWidth() / rectangles)
			{
				red1[y][x] = CP_Random_RangeInt(130, 180);
			}
		}
	}
	CP_Settings_RectMode(CP_POSITION_CORNER);
  CP_Settings_NoStroke();
	for (int y = 0; y < CP_System_GetWindowHeight(); y += CP_System_GetWindowHeight() / rectangles)
	{
		for (int x = 0; x < CP_System_GetWindowWidth(); x += CP_System_GetWindowWidth() / rectangles)
		{
			CP_Settings_Fill(CP_Color_Create(red1[y][x], 30, 180, alpha1));
			CP_Graphics_DrawRect((float)x, (float)y + 1, (float)CP_System_GetWindowWidth() / (float)rectangles, 1 + (float)CP_System_GetWindowHeight() / (float)rectangles);
		}
	}


}

int gaussianData[100] = { 0 };
void gaussian_background_rectangles(void)
{
	if (startup)
	{
		startup = 0;
		for (int i = 0; i < 100000; ++i)
		{
			float gaus = CP_Random_Gaussian() * 20.0f + 50.0f;
			if (0 <= gaus && gaus < 100.0f)
			{
				++gaussianData[(int)gaus];
			}
		}
	}
	CP_Settings_RectMode(CP_POSITION_CENTER);
  CP_Settings_NoStroke();
	int index = 0;
	for (int x = 0; x < CP_System_GetWindowWidth(); x += CP_System_GetWindowWidth() / 100)
	{
		CP_Settings_Fill(CP_Color_Create(255, 180, 180, alpha1));
		CP_Graphics_DrawRect((float)x + (CP_System_GetWindowWidth() / 200), CP_System_GetWindowHeight() / 2.0f, (float)CP_System_GetWindowWidth() / 100.0f, (float)gaussianData[index++] / 5.0f);
	}
}

const int Rectangles = 100;
void noise_example(void)
{
	CP_Settings_RectMode(CP_POSITION_CORNER);
  CP_Settings_NoStroke();
	for (int y = 0; y < CP_System_GetWindowHeight(); y += CP_System_GetWindowHeight() / Rectangles)
	{
		for (int x = 0; x < CP_System_GetWindowWidth(); x += CP_System_GetWindowWidth() / Rectangles)
		{
      // get a value between 0 and 255 for our color 
      // (the smaller the x and the y, the smoother the noise will be)
      int grayscale_val = (int)(CP_Random_Noise((float)x*0.01f, (float)y*0.01f, CP_System_GetFrameCount() * 0.1f) * 255);

      // fill the color our noise generated
      CP_Settings_Fill(CP_Color_Create(grayscale_val, grayscale_val, grayscale_val, 255));

      // draw the rectangle at the position
			CP_Graphics_DrawRect((float)x, (float)y + 1, (float)CP_System_GetWindowWidth() / (float)Rectangles, 1 + (float)CP_System_GetWindowHeight() / (float)Rectangles);
		}
	}
}

float x2 = 0, y2 = 0, x3 = 0, y3 = 0;

void mouse_tracking_circle_rect(void)
{
	// x3 = 300, y3 = 300;
	// CP_Input_GetMouseX() = 400, CP_Input_GetMouseY() = 400;
	//background(255, 255, 255);

	//float size = 200;
	// b
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_EllipseMode(CP_POSITION_CORNER);
	CP_Settings_Fill(CP_Color_Create(0, 255, 0, 255));
	//strokeWeight(
	//    (float)(
	//        (float)abs((int)(CP_Input_GetMouseX() - x3)) / (float)abs((int)(CP_Input_GetMouseY() - y3))
	//        )
	//);
	//CP_Graphics_DrawRect(x3/* + size / 2*/, y3/* + size / 2*/, -size/* / 2*/, size/* / 2*/); 
  CP_Settings_NoStroke();
	CP_Graphics_DrawEllipse(x3, y3, CP_Input_GetMouseX() - x3, CP_Input_GetMouseY() - y3);


  CP_Settings_Fill(CP_Color_Create(0, 0, 255, 10));
  CP_Settings_Stroke(CP_Color_Create(255, 0, 0, 255));
	CP_Graphics_DrawRect(x3, y3, CP_Input_GetMouseX() - x3, CP_Input_GetMouseY() - y3);

  CP_Settings_Stroke(CP_Color_Create(0, 255, 255, 255));
  CP_Settings_Fill(CP_Color_Create(0, 255, 255, 255));
	CP_Graphics_DrawPoint(x3, y3);
	CP_Graphics_DrawPoint(CP_Input_GetMouseX(), CP_Input_GetMouseY());
	x3 = x2;
	y3 = y2;

	x2 = CP_Input_GetMousePreviousX();
	y2 = CP_Input_GetMousePreviousY();

	Sleep(50);
}

int h = 0;

void mouse_following_polygon(void)
{
  CP_Settings_StrokeWeight(1.0f);
	// alpha1 = 5;
	// random_background_rectangles();

  CP_Settings_Fill(CP_Color_Create(180, 0, 180, 1));
	CP_Graphics_DrawRect(0.0f, 0.0f, 1.0f * CP_System_GetWindowWidth(), 1.0f * CP_System_GetWindowHeight());
	x2 = x3;
	y2 = y3;

	x3 = (float)((CP_System_GetWindowWidth() / 3.0)  * cos((double)CP_System_GetFrameCount() / 100.0)) + (CP_System_GetWindowWidth() / 2.0f) + (float)((CP_System_GetWindowHeight() / 6.0) * cos((double)CP_System_GetFrameCount() / 240.0)) - (float)((CP_System_GetWindowHeight() / 2.0) * sin((double)CP_System_GetFrameCount() / 330.0));
	y3 = (float)((CP_System_GetWindowHeight() / 3.0) * sin((double)CP_System_GetFrameCount() / 100.0)) + (CP_System_GetWindowHeight() / 2.0f) - (float)((CP_System_GetWindowHeight() / 10.0) * cos((double)CP_System_GetFrameCount() / 120.0)) + (float)((CP_System_GetWindowHeight() / 5.0) * sin((double)CP_System_GetFrameCount() / 96.0));

	++h;
	if (h >= 255) h = 0;
	//noStroke();
	CP_Graphics_BeginShape();
	CP_Graphics_AddVertex(x3, y3);
	CP_Graphics_AddVertex(x2, y2);
	//CP_Graphics_AddShapeVertex(CP_System_GetCanvasWidth / 2.0f, canvasHeight / 2.0f);
	CP_Graphics_AddVertex(CP_Input_GetMousePreviousX(), CP_Input_GetMousePreviousY());
	CP_Graphics_AddVertex(CP_Input_GetMouseX(), CP_Input_GetMouseY());
  CP_Settings_Stroke(CP_Color_Create(255, 255, 255, 255));
  CP_Settings_Fill(CP_Color_Create(h, 0, 180, 255));
	CP_Graphics_EndShape();


	//CP_Graphics_DrawLine(x3, y3, x2, y2);

}

void printWASDStates(void)
{
	system("cls");
	printf("W STATE: %i\tPRESSED: %i\tRELEASED: %i\n", CP_Input_KeyDown(KEY_W), CP_Input_KeyTriggered(KEY_W), CP_Input_KeyReleased(KEY_W));
	printf("A STATE: %i\tPRESSED: %i\tRELEASED: %i\n", CP_Input_KeyDown(KEY_A), CP_Input_KeyTriggered(KEY_A), CP_Input_KeyReleased(KEY_A));
	printf("S STATE: %i\tPRESSED: %i\tRELEASED: %i\n", CP_Input_KeyDown(KEY_S), CP_Input_KeyTriggered(KEY_S), CP_Input_KeyReleased(KEY_S));
	printf("D STATE: %i\tPRESSED: %i\tRELEASED: %i\n\n", CP_Input_KeyDown(KEY_D), CP_Input_KeyTriggered(KEY_D), CP_Input_KeyReleased(KEY_D));
}

void printGamepadStates(void)
{
	system("cls");
	for (int i = 0; i < 4; ++i)
	{
		if (!CP_Input_GamepadConnectedAdvanced(i)) continue;
		printf("\n\nGamepad %d:\n", i);
		printf("Left Stick   X: %4.2f  Y: %4.2f\n", CP_Input_GamepadLeftStickAdvanced(i).x, CP_Input_GamepadLeftStickAdvanced(i).y);
		printf("Right Stick  X: %4.2f  Y: %4.2f\n", CP_Input_GamepadRightStickAdvanced(i).x, CP_Input_GamepadRightStickAdvanced(i).y);
		printf("Trigger      L: %4.2f  R: %4.2f\n", CP_Input_GamepadLeftTriggerAdvanced(i), CP_Input_GamepadRightTriggerAdvanced(i));
		printf("X STATE: %i\tPRESSED: %i\tRELEASED: %i\n", CP_Input_GamepadDownAdvanced(GAMEPAD_X, i), CP_Input_GamepadTriggeredAdvanced(GAMEPAD_X, i), CP_Input_GamepadReleasedAdvanced(GAMEPAD_X, i));
		printf("Y STATE: %i\tPRESSED: %i\tRELEASED: %i\n", CP_Input_GamepadDownAdvanced(GAMEPAD_Y, i), CP_Input_GamepadTriggeredAdvanced(GAMEPAD_Y, i), CP_Input_GamepadReleasedAdvanced(GAMEPAD_Y, i));
		printf("A STATE: %i\tPRESSED: %i\tRELEASED: %i\n", CP_Input_GamepadDownAdvanced(GAMEPAD_A, i), CP_Input_GamepadTriggeredAdvanced(GAMEPAD_A, i), CP_Input_GamepadReleasedAdvanced(GAMEPAD_A, i));
		printf("B STATE: %i\tPRESSED: %i\tRELEASED: %i\n", CP_Input_GamepadDownAdvanced(GAMEPAD_B, i), CP_Input_GamepadTriggeredAdvanced(GAMEPAD_B, i), CP_Input_GamepadReleasedAdvanced(GAMEPAD_B, i));
	}
}

void mouse_demo(void)
{
	//background(255, 255, 255);
	if (CP_Input_MouseDoubleClicked())
	{
		//int color = irand_min_max(180, 220);
    CP_Settings_Fill(CP_Color_Create(CP_Random_RangeInt(180, 220), CP_Random_RangeInt(0, 220), CP_Random_RangeInt(100, 220), 255));
		CP_Graphics_DrawRect(0, 0, (float)CP_System_GetWindowWidth(), (float)CP_System_GetWindowHeight());
	}
	//if (CP_Input_MouseDragged(MOUSE_BUTTON_1))
	//{
	//    int color = 0;
	//    fill(color, color, color, 5);
	//    CP_Graphics_DrawEllipse(CP_Input_GetMouseX(), CP_Input_GetMouseY(), 60, 60);
	//}
	if (CP_Input_MouseTriggered(MOUSE_BUTTON_1))
	{
		int c = 0;
    CP_Settings_Fill(CP_Color_Create(255, c, c, 255));
		CP_Graphics_DrawEllipse(CP_Input_GetMouseX(), CP_Input_GetMouseY(), 30, 30);
	}
	if (CP_Input_MouseReleased(MOUSE_BUTTON_1))
	{
		int c = 0;
    CP_Settings_Fill(CP_Color_Create(c, c, 255, 255));
		CP_Graphics_DrawEllipse(CP_Input_GetMouseX(), CP_Input_GetMouseY(), 30, 30);
	}
	if (CP_Input_MouseDragged(MOUSE_BUTTON_1))
	{
		//int color = irand_min_max(180, 220);
    CP_Settings_Fill(CP_Color_Create(CP_Random_RangeInt(0, 250), CP_Random_RangeInt(0, 250), CP_Random_RangeInt(0, 250), 255));
		CP_Graphics_DrawEllipse(CP_Random_RangeFloat(0, (float)CP_System_GetWindowWidth()), CP_Random_RangeFloat(0, (float)CP_System_GetWindowHeight()), 50, 50);
	}
	// else if (CP_Input_MouseMoved())
	// {
	//     //int color = irand_min_max(180, 220);
	//     fill(irand_min_max(0, 250), irand_min_max(0, 250), irand_min_max(0, 250), 255);
	//     CP_Graphics_DrawRect(frand_min_max(0.0f, (float)canvasWidth), frand_min_max(0.0f, (float)canvasHeight), 100.0f, 100.0f);
	// }

}

void text_demo(void)
{
	// if (CP_Input_MouseClicked())
	// 	fullscreen();
	// else if (CP_Input_MouseTriggered(MOUSE_BUTTON_RIGHT))
	// 	size(500, 500);
	// 
	int button_pressed = FALSE;
	// CP_Graphics_SetEllipseMode(CP_POSITION_CORNER);
	// CP_Graphics_DrawCircle(CP_Input_GetMouseX(), CP_Input_GetMouseY(), 50);
	// 
	 //translate(CP_Input_GetMouseX(), CP_Input_GetMouseY());
  CP_Settings_Translate(50.0f + 50.0f*(float)sin(CP_System_GetFrameCount() / 300.0), 0);
	//translate(canvasWidth / 2.0f, canvasHeight / 2.0f - 400.0f);
	//rotate(frameCount / 10.0f);
  float scale = 1.0f * (float)sin(CP_System_GetFrameCount() / 300.0) + 0.5f;
  CP_Settings_Scale(scale, scale);
	//scaleY((float)sin(frameCount / 150.0f) * 0.5f);
	//scaleX((float)cos(frameCount / 300.0) * 0.5f);

  CP_Settings_Fill(CP_Color_Create(100, 40, 40, 20));
	CP_Graphics_DrawRect(0, 0, (float)CP_System_GetWindowWidth(), (float)CP_System_GetWindowHeight());
	if (!font)
	{
		// test font load efficiency
		for (int i = 0; i < 1000; ++i)
			font = CP_Font_Load("./Assets/Exo2-LightItalic.ttf");
	}

  CP_Font_Set(CP_Font_GetDefault());
  CP_Settings_TextSize(140.0f);
  CP_Settings_Fill(CP_Color_Create(160, 0, 0, 255));
  CP_Font_DrawText("HELLO WORLD", 100, 120);

	// side lines
	float line_y_offset = 20.0f;
	float line_X = 40;
	float line_spacing = 4;
  CP_Settings_Stroke(CP_Color_Create(160, 0, 0, 255));
	// vertical lines on the sides
	for (int i = 0; i < 3; ++i)
	{
		CP_Graphics_DrawLine(line_X, line_y_offset, line_X, (float)(CP_System_GetWindowHeight() - line_y_offset));
		CP_Graphics_DrawLine((float)(CP_System_GetWindowWidth() - line_X), line_y_offset, (float)(CP_System_GetWindowWidth() - line_X), (float)(CP_System_GetWindowHeight() - line_y_offset));
		line_X += line_spacing;
	}
  CP_Settings_StrokeWeight(1.0f);

	// main text block
  CP_Settings_TextSize(50.0f);
  CP_Settings_Fill(CP_Color_Create(200, 0, 0, 255));
  CP_Font_DrawTextBox(lorem, 100, 200, (float)(CP_System_GetWindowWidth() - 200));

	// put a box around the text
  CP_Settings_Stroke(CP_Color_Create(255, 0, 0, 255));
  CP_Settings_NoFill();
	//              height                       arbirary
	CP_Graphics_DrawRect(90, 200 - 50.0f, (float)(CP_System_GetWindowWidth() - 190), 52.0f * 8);

	// subtitle below box
  CP_Font_Set(font);
  CP_Settings_TextSize(32.0f);
  CP_Settings_Fill(CP_Color_Create(160, 80, 80, 255));
  CP_Font_DrawTextBox(lorem_short, 120.0f, 620.0f, (float)(CP_System_GetWindowWidth() - 220));

	// button 1
	float x_offset = 100.0f;
	float button_spacing_x = 50.0f;
#if 1 // rectangle buttons
	float button_y = (float)(CP_System_GetWindowHeight() - 250);
	float button_w = 200.0f;
	float button_h = 80.0f;
	float button_r = 10.0f;
#else // oval buttons
	float button_y = (float)(CP_System_GetCanvasHeight() - 285);
	float button_w = 200.0f;
	float button_h = button_w - 40.0f;
	float button_r = button_w / 2.0f;//150.0f;
#endif
	float button_text_offset_x = (button_w / 2.0f) - 50.0f;
	float button_text_offset_y = (button_h / 2.0f) + 10.0f;

	// buttons
	char button_text[] = "Button 0\0";
	for (int num = 1; num < 6; ++num)
	{
		float x = (x_offset)+(button_w * (num - 1)) + (button_spacing_x * (num - 1));

		if ((CP_Input_GetMouseWorldX() > x && CP_Input_GetMouseWorldX() < (x + button_w)) && (CP_Input_GetMouseWorldY() > button_y && CP_Input_GetMouseWorldY() < (button_y + button_h)))
      CP_Settings_NoFill();
		else
      CP_Settings_Fill(CP_Color_Create(255, 120, 120, 120));

		CP_Graphics_DrawRectAdvanced(x, button_y, button_w, button_h, 0, button_r);
		CP_Graphics_DrawRectAdvanced(x, button_y, button_w, button_h, 0, button_r);

		if ((CP_Input_GetMouseWorldX() > x && CP_Input_GetMouseWorldX() < (x + button_w)) && (CP_Input_GetMouseWorldY() > button_y && CP_Input_GetMouseWorldY() < (button_y + button_h)))
		{
      CP_Settings_TextSize(32.0f + 20.0f);
      CP_Settings_Fill(CP_Color_Create(255, 120, 120, 120));
			button_text_offset_x = (button_w / 2.0f) - 84.0f;
			button_text_offset_y = (button_h / 2.0f) + 16.0f;
			button_pressed = TRUE;
		}
		else
		{
			button_text_offset_x = (button_w / 2.0f) - 50.0f;
			button_text_offset_y = (button_h / 2.0f) + 10.0f;
      CP_Settings_TextSize(32.0f + 0.0f);
      CP_Settings_Fill(CP_Color_Create(60, 0, 0, 255));
		}
    CP_Font_DrawText(button_text, x + button_text_offset_x, button_y + button_text_offset_y);
		++button_text[7];
	}

	// offset
	int offset = 80;
	int spacing = 8;
	int tall = 20;
	int y = CP_System_GetWindowHeight() - 100;
	// bottom diagonal line row
	for (int i = offset; i < CP_System_GetWindowWidth() - offset - spacing; i += spacing)
	{
		CP_Graphics_DrawLine((float)i + spacing, (float)y, (float)i, (float)y + tall);
	}

	// mouse status text
	if (button_pressed)
	{
		float button_x = 150.0f;
		//float button_offset = 200.0f;
		// hover
    CP_Settings_TextSize(30.0f);
    CP_Settings_Fill(CP_Color_Create(255, 0, 0, 255));
    CP_Font_DrawText("HOVER", 300.0f, CP_System_GetWindowHeight() - 30.0f);
		if (CP_Input_MouseDown(MOUSE_BUTTON_1))
		{
      CP_Settings_TextSize(30.0f);
      CP_Settings_Fill(CP_Color_Create(255, 0, 0, 255));
      CP_Font_DrawText("HELD", button_x, CP_System_GetWindowHeight() - 30.0f);
		}
		if (CP_Input_MouseTriggered(MOUSE_BUTTON_1))
		{
      CP_Settings_TextSize(30.0f);
      CP_Settings_Fill(CP_Color_Create(255, 0, 0, 255));
      CP_Font_DrawText("PRESSED", 450.0f, CP_System_GetWindowHeight() - 30.0f);
		}
		if (CP_Input_MouseReleased(MOUSE_BUTTON_1))
		{
      CP_Settings_TextSize(30.0f);
      CP_Settings_Fill(CP_Color_Create(255, 0, 0, 255));
      CP_Font_DrawText("RELEASED", 600.0f, CP_System_GetWindowHeight() - 30.0f);
		}
		if (CP_Input_MouseReleased(MOUSE_BUTTON_1))
		{
      CP_Settings_TextSize(30.0f);
      CP_Settings_Fill(CP_Color_Create(255, 0, 0, 255));
      CP_Font_DrawText("CLICK", 750.0f, CP_System_GetWindowHeight() - 30.0f);
		}
		if (CP_Input_MouseDragged(MOUSE_BUTTON_1))
		{
      CP_Settings_TextSize(30.0f);
      CP_Settings_Fill(CP_Color_Create(255, 0, 0, 255));
      CP_Font_DrawText("DRAGGED", 900.0f, CP_System_GetWindowHeight() - 30.0f);
		}
		if (CP_Input_MouseDoubleClicked())
		{
      CP_Settings_TextSize(30.0f);
      CP_Settings_Fill(CP_Color_Create(255, 0, 0, 255));
      CP_Font_DrawText("DOUBLE CLICK", 1050.0f, CP_System_GetWindowHeight() - 30.0f);
		}
	}
}

void transform_test(void)
{
    // Variables to pass into the conversion functions
	float x = (float)M_PI;
	float y = (float)M_PI;

	// Run back and forth 100 times
	for (int i = 0; i < 100; ++i)
	{
    CP_Math_ScreenToWorld(x, y, &x, &y);
    CP_Math_WorldToScreen(x, y, &x, &y);
	}

	// Should still be PI
	printf(" %9f\n(%9f,%9f)\n", (float)M_PI, x, y);
}

void time_test(void)
{
	CP_System_SetFrameRate(1.f);
  CP_System_SetWindowSize(100, 100);
	system("cls");
	printf("    DT:\t%f\n", CP_System_GetDt());
	printf("Millis:\t%f\n", CP_System_GetMillis());
	printf("Second:\t%f\n", CP_System_GetSeconds());
}


static int boolean_ = 0;
static CP_Sound sound1 = NULL;
static CP_Sound notes[2][7] = { 0 };
static CP_Sound drums[4] = { 0 };
static int index50 = 0;
static int playmusicforme = FALSE;

void audio_test(void)
{
	++index50;
	//setFrameRate(1.f);
	if (!boolean_) {
		sound1 = CP_Sound_Load("./Assets/808cowbell.wav");
		boolean_ = 1;

		// Piano
		notes[0][0] = CP_Sound_Load("./Assets/Piano/c.wav");
		notes[0][1] = CP_Sound_Load("./Assets/Piano/d.wav");
		notes[0][2] = CP_Sound_Load("./Assets/Piano/e.wav");
		notes[0][3] = CP_Sound_Load("./Assets/Piano/f.wav");
		notes[0][4] = CP_Sound_Load("./Assets/Piano/g.wav");
		notes[0][5] = CP_Sound_Load("./Assets/Piano/a.wav");
		notes[0][6] = CP_Sound_Load("./Assets/Piano/b.wav");

		// Fancy Sounds
		notes[1][0] = CP_Sound_Load("./Assets/Notes/c.wav");
		notes[1][1] = CP_Sound_Load("./Assets/Notes/d.wav");
		notes[1][2] = CP_Sound_Load("./Assets/Notes/e.wav");
		notes[1][3] = CP_Sound_Load("./Assets/Notes/f.wav");
		notes[1][4] = CP_Sound_Load("./Assets/Notes/g.wav");
		notes[1][5] = CP_Sound_Load("./Assets/Notes/a.wav");
		notes[1][6] = CP_Sound_Load("./Assets/Notes/b.wav");

		//drums
		drums[0] = CP_Sound_Load("./Assets/BassDrum.wav");
		drums[1] = CP_Sound_Load("./Assets/Snare.wav");
		drums[2] = CP_Sound_Load("./Assets/Clap.wav");
		drums[3] = CP_Sound_Load("./Assets/HiHat.wav");

		CP_Settings_RectMode(CP_POSITION_CENTER);
    CP_System_SetWindowSize(800, 400);
    CP_Graphics_ClearBackground(CP_Color_Create(10, 50, 140, 255));
	}

#if 0
	background(0, 0, 0);
	setPitch(channel, 1.0f - (index50 / 20.0f));
	soundChannel(notes[0][6], channel);
	setReverb(channel, 10.0f);
	Sleep(400);
#else
	if (index50 == 3)
    CP_Graphics_ClearBackground(CP_Color_Create(10, 50, 140, 255));
	//background(10, 50, 140);
	//setReverb(channel, 10.0f);

  CP_Settings_TextSize(32);
  CP_Font_DrawText("FANCY NOTES", 20, 60);
  CP_Font_DrawText("PIANO NOTES", 20, 160);
  CP_Font_DrawText("DRUMS", 20, 260);

  CP_Settings_NoFill();
	CP_Graphics_DrawRect(CP_System_GetWindowWidth() / 2.0f, 100, CP_System_GetWindowWidth() - 40.0f, 60.0f);
	CP_Graphics_DrawRect(CP_System_GetWindowWidth() / 2.0f, 200, CP_System_GetWindowWidth() - 40.0f, 60.0f);
	CP_Graphics_DrawRect(CP_System_GetWindowWidth() / 2.0f, 300, CP_System_GetWindowWidth() - 40.0f, 60.0f);
  CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));

	if (playmusicforme)
	{
		if (CP_Input_MouseDown(MOUSE_BUTTON_1))
		{
			playmusicforme = FALSE;
		}
	}
	if (CP_Input_GetMouseX() > (CP_System_GetWindowWidth() / 2.0f) - 20.0f && CP_Input_GetMouseX() < (CP_System_GetWindowWidth() / 2.0f) + 20.0f) {
		if (CP_Input_GetMouseY() > 40.0f && CP_Input_GetMouseY() < 60.0f)
		{
			if (CP_Input_MouseTriggered(MOUSE_BUTTON_1))
			{
				playmusicforme = TRUE;
			}
		}
	}
	if (playmusicforme)
    CP_Settings_Fill(CP_Color_Create(255, 0, 0, 20));
	CP_Graphics_DrawRect(CP_System_GetWindowWidth() / 2.0f, 50.0f, 40.0f, 20.0f);
	if (playmusicforme)
    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 10));

	for (int i = 0; i < 7; ++i)
	{
		if (CP_Input_GetMouseX() > (float)i * (CP_System_GetWindowWidth() / 7) + (CP_System_GetWindowWidth() / 14) - (float)(CP_System_GetWindowWidth() / 28) && CP_Input_GetMouseX() < (float)i * (CP_System_GetWindowWidth() / 7) + (CP_System_GetWindowWidth() / 14) + (float)(CP_System_GetWindowWidth() / 28))
		{
			if (CP_Input_GetMouseY() > 100 - (CP_System_GetWindowHeight() / 28) && CP_Input_GetMouseY() < 100 + (CP_System_GetWindowHeight() / 28))
			{
				if (CP_Input_MouseTriggered(MOUSE_BUTTON_1))
				{
					CP_Sound_Play(notes[1][i]);
          CP_Settings_Fill(CP_Color_Create(255, 0, 0, 10));
				}
			}
		}
		CP_Graphics_DrawRectAdvanced((float)i * (CP_System_GetWindowWidth() / 7) + (CP_System_GetWindowWidth() / 14), (float)100, (float)(CP_System_GetWindowWidth() / 14), (float)(CP_System_GetWindowHeight() / 14), 0, 5.0f);
    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 10));
	}

	for (int i = 0; i < 7; ++i)
	{
		if (CP_Input_GetMouseX() > (float)i * (CP_System_GetWindowWidth() / 7) + (CP_System_GetWindowWidth() / 14) - (float)(CP_System_GetWindowWidth() / 28) && CP_Input_GetMouseX() < (float)i * (CP_System_GetWindowWidth() / 7) + (CP_System_GetWindowWidth() / 14) + (float)(CP_System_GetWindowWidth() / 28))
		{
			if (CP_Input_GetMouseY() > 200 - (CP_System_GetWindowHeight() / 28) && CP_Input_GetMouseY() < 200 + (CP_System_GetWindowHeight() / 28))
			{
				if (CP_Input_MouseTriggered(MOUSE_BUTTON_1))
				{
					CP_Sound_Play(notes[0][i]);
          CP_Settings_Fill(CP_Color_Create(255, 0, 0, 10));
				}
			}
		}
		CP_Graphics_DrawRectAdvanced((float)i * (CP_System_GetWindowWidth() / 7) + (CP_System_GetWindowWidth() / 14), (float)200, (float)(CP_System_GetWindowWidth() / 14), (float)(CP_System_GetWindowHeight() / 14), 0, 5.0f);
    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 10));
	}


	for (int i = 0; i < 4; ++i)
	{
		if (CP_Input_GetMouseX() > (float)i * (CP_System_GetWindowWidth() / 4) + (CP_System_GetWindowWidth() / 8) - (float)(CP_System_GetWindowWidth() / 16) && CP_Input_GetMouseX() < (float)i * (CP_System_GetWindowWidth() / 4) + (CP_System_GetWindowWidth() / 8) + (float)(CP_System_GetWindowWidth() / 16))
		{
			if (CP_Input_GetMouseY() > 300 - (CP_System_GetWindowHeight() / 16) && CP_Input_GetMouseY() < 300 + (CP_System_GetWindowHeight() / 16))
			{
				if (CP_Input_MouseTriggered(MOUSE_BUTTON_1))
				{
					CP_Sound_Play(drums[i]);
          CP_Settings_Fill(CP_Color_Create(255, 0, 0, 10));
				}
			}
		}
		CP_Graphics_DrawRectAdvanced((float)i * (CP_System_GetWindowWidth() / 4) + (CP_System_GetWindowWidth() / 8), (float)300, (float)(CP_System_GetWindowWidth() / 8), (float)(CP_System_GetWindowHeight() / 8), 0, 5.0f);
    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 10));
	}

	//noCursor();

// music generator
	if (playmusicforme) {
		Sleep(250);
		if (index50 %= 2) {
			if (CP_Random_RangeFloat(0, 10.0) < 5.0)
				CP_Sound_Play(notes[1][CP_Random_RangeInt(0, 6)]);
			if (CP_Random_RangeFloat(0, 10.0) < 5.0)
				CP_Sound_Play(notes[0][CP_Random_RangeInt(0, 5)]);
		}
		if (CP_Random_RangeFloat(0, 10.0) < 5.0)
			CP_Sound_Play(drums[CP_Random_RangeInt(0, 2)]);
		CP_Sound_Play(drums[3]);
	}
#endif
}


static float x5, y5;
CP_Vector dist;
CP_Vector vel2;
void mouse_setup(void)
{
	x5 = CP_Input_GetMouseX();
	y5 = CP_Input_GetMouseY();
	dist.x = 0;
	dist.y = 0;
	vel2.x = 0;
	vel2.y = 0;
}
void mouse_follower(void)
{
  CP_System_SetFrameRate(24);
  CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 255));
	dist.x = CP_Input_GetMouseX() - x5;
	dist.y = CP_Input_GetMouseY() - y5;

	float mass = 100;// kg;
	float constant = 0.0000000000667f;
	//float r = CP_Vector_length(vel);

	// calculate force
	CP_Vector force;
	force.x = constant * 100000000000000 /* 1 bil kg */ / (dist.x * dist.x);
	force.y = constant * 100000000000000 /* 1 bil kg */ / (dist.y * dist.y);

	// calculate acceleration based on force
	CP_Vector accel;
	accel.x = force.x / mass;
	accel.y = force.y / mass;

	// fix signs on accel
	if (dist.x < 0 && accel.x > 0)
		accel.x *= -1.0f;
	if (dist.y < 0 && accel.y > 0)
		accel.y *= -1.0f;

	float factor = 1;

	// apply acceleration to velocity
	vel2.x += accel.x * CP_System_GetDt() * factor;
	vel2.y += accel.y * CP_System_GetDt() * factor;

	// "drag"
	vel2 = CP_Vector_Scale(vel2, 0.6f);

	// if (x5 > canvasWidth)
	//     vel2.x *= -1.0f;
	// if (y5 > canvasHeight)
	//     vel2.y *= -1.0f;
	// if (x5 < 0)
	//     vel2.x *= -1.0f;
	// if (y5 < 0)
	//     vel2.y *= -1.0f;

	while (x5 > CP_System_GetWindowWidth())
	{
		x5 = (float)((int)x5 % CP_System_GetWindowWidth());
	}
	while (y5 > CP_System_GetWindowHeight())
	{
		y5 = (float)((int)y5 % CP_System_GetWindowHeight());
	}
	while (x5 < 0)
	{
		x5 += CP_System_GetWindowWidth();
	}
	while (y5 < 0)
	{
		y5 += CP_System_GetWindowHeight();
	}


	// apply velocity to position
	x5 += vel2.x;
	y5 += vel2.x;

	CP_Graphics_DrawEllipse(x5, y5, 10, 10);
	printf("(%f, %f)\n", CP_Input_GetMouseX(), CP_Input_GetMouseY());
	printf("(%f, %f)\n", x5, y5);
	printf("\n");
}

void selector(void)
{
	static int setup = 0;
	static int i = 0;
	if (CP_Input_KeyTriggered(KEY_SPACE))
	{
		i++;
		setup = 0;
    CP_System_SetWindowSize(1820, 900);
	}

	switch (i)
	{
	case 0:
		mouse_following_polygon();
		break;
	case 1:
		text_demo();
		break;
	case 2:
		audio_test();
		break;
	case 3:
		if (!setup)
		{
			setup = TRUE;
			mouse_setup();
		}
		mouse_follower();
		break;
	case 4:
		if (!setup)
		{
			setup = TRUE;
			image_test_setup();
		}
		image_test();
		break;
	case 5:
		//noise_background_rectangles();
		break;
	case 6:
		i = 0;
		break;
	}
}


CP_Vector a, b, c, d;
void racecar_setup(void)
{
  CP_System_SetWindowSize(800, 600);
	float pos = CP_System_GetWindowHeight() / 8.0f;
	a.y = pos * 1;
	b.y = pos * 2;
	c.y = pos * 3;
	d.y = pos * 4;

	a.x = b.x = c.x = d.x = 100;

	// black lines
  CP_Settings_Stroke(CP_Color_Create(0, 0, 0, 255));

	CP_Settings_RectMode(CP_POSITION_CENTER);
}

void racecar_draw(void)
{
	// white background
  CP_Graphics_ClearBackground(CP_Color_Create(255, 255, 255, 255));

	// start/finish line, probably too advanced
	CP_Graphics_DrawLine(100, 0, 100, (float)CP_System_GetWindowHeight());
	CP_Graphics_DrawLine(120, 0, 120, (float)CP_System_GetWindowHeight());
  CP_Settings_Fill(CP_Color_Create(0, 0, 0, 255));
	int i = 0;
	for (i = 0; i < CP_System_GetWindowHeight(); i += (CP_System_GetWindowHeight() / 60))
	{
		if (i % 20 == 0)
			CP_Graphics_DrawRect(105, (float)i, 10, 10);
		else
			CP_Graphics_DrawRect(115, (float)i, 10, 10);
	}

	// speed
	a.x += 2;
	b.x += 4;
	c.x += 6;
	d.x += 8;

	// keep them on-screen
	if (a.x > CP_System_GetWindowWidth())
		a.x = 0;
	if (b.x > CP_System_GetWindowWidth())
		b.x = 0;
	if (c.x > CP_System_GetWindowWidth())
		c.x = 0;
	if (d.x > CP_System_GetWindowWidth())
		d.x = 0;

	//a
  CP_Settings_Fill(CP_Color_Create(255, 0, 0, 255)); // red
	CP_Graphics_DrawRect(a.x, a.y, 50, 20);
	//b
  CP_Settings_Fill(CP_Color_Create(0, 255, 0, 255)); // green
	CP_Graphics_DrawRect(b.x, b.y, 50, 20);
	//c
  CP_Settings_Fill(CP_Color_Create(0, 0, 255, 255)); // blue
	CP_Graphics_DrawRect(c.x, c.y, 50, 20);
	//d
  CP_Settings_Fill(CP_Color_Create(255, 0, 255, 255)); // purple
	CP_Graphics_DrawRect(d.x, d.y, 50, 20);
}

typedef struct square
{
	CP_Vector pos;
	CP_Vector vel;
	CP_Vector size;
	CP_Color color;
} square;

const int count = 100;
square squares[100];
void bounce_setup(void)
{
  CP_Settings_BlendMode(CP_BLEND_ADD);
	//size(800, 600);
  CP_System_Fullscreen();
	int i = 0;
	for (; i < count; ++i)
	{
		// pos
		squares[i].pos.x = CP_Random_RangeFloat(30, (float)CP_System_GetWindowWidth() - 30);
		squares[i].pos.y = CP_Random_RangeFloat(30, (float)CP_System_GetWindowHeight() - 30);
		// vel
		squares[i].vel.x = CP_Random_RangeFloat(-5, 5);
		squares[i].vel.y = CP_Random_RangeFloat(-5, 5);
		// size
		squares[i].size.x = CP_Random_RangeFloat(5, 30);
		squares[i].size.y = CP_Random_RangeFloat(5, 30);

		// color
		squares[i].color = CP_Color_Create(CP_Random_RangeInt(0, 10), CP_Random_RangeInt(0, 10), CP_Random_RangeInt(0, 10), 255);
	}

	CP_Settings_RectMode(CP_POSITION_CENTER);
}

void bounce_draw(void)
{
	// update
	int i = 0;
	for (; i < count; ++i)
	{
		// add velocity
		squares[i].pos = CP_Vector_Add(squares[i].pos, squares[i].vel);

		// constrain, flip velocities
		if (squares[i].pos.x < 0) {
			squares[i].pos.x = 0;
			squares[i].vel.x *= -1;
		}
		if (squares[i].pos.y < 0) {
			squares[i].pos.y = 0;
			squares[i].vel.y *= -1;
		}
		if (squares[i].pos.x > CP_System_GetWindowWidth()) {
			squares[i].pos.x = (float)CP_System_GetWindowWidth() - 1;
			squares[i].vel.x *= -1;
		}
		if (squares[i].pos.y > CP_System_GetWindowHeight()) {
			squares[i].pos.y = (float)CP_System_GetWindowHeight() - 1;
			squares[i].vel.y *= -1;
		}

		// draw
    CP_Settings_Fill(squares[i].color);
		CP_Graphics_DrawRect(squares[i].pos.x, squares[i].pos.y, squares[i].size.x, squares[i].size.y);
	}
}

//-------------------------------------------
// DANCING LINES
// Including Particle "class"
//
typedef struct _Particle
{
	CP_Vector pos;
	CP_Vector vel;
	CP_Color* color;
} Particle;

const float EPSILON = 0.0000001f;

float particleSize = 3.0f;

CP_Color randomColors[] = {
	{ 127, 0,   0,   255 },
	{ 127, 127, 0,   255 },
	{ 0,   127, 0,   255 },
	{ 0,   127, 127, 255 },
	{ 0,   0,   127, 255 },
	{ 127, 0,   127, 255 } };

void ParticleCreate(Particle* part) {
	part->pos.x = CP_Random_RangeFloat(0, (float)CP_System_GetWindowWidth());
	part->pos.y = CP_Random_RangeFloat(0, (float)CP_System_GetWindowHeight());
	part->vel.x = CP_Random_RangeFloat(-150, 150);
	part->vel.y = CP_Random_RangeFloat(-150, 150);
	part->color = &randomColors[CP_Random_RangeInt(0, 5)];
}

void ParticleDisplay(Particle* part)
{
	CP_Graphics_DrawEllipse(part->pos.x, part->pos.y, particleSize, particleSize);
}

void ParticleUpdate(Particle* part)
{
	// move particle based on velocity and correct for wall collisions
	float time = CP_System_GetDt();
	float timeX = time;
	float timeY = time;

	while (time > EPSILON)
	{
		bool collisionX = false;
		bool collisionY = false;

		float newPosX = part->pos.x + part->vel.x * time;
		float newPosY = part->pos.y + part->vel.y * time;
		float newTime = time;

		// check wall collisions X and Y
		if (newPosX <= 0)
		{
			timeX = part->pos.x / (part->pos.x - newPosX) * time;
			collisionX = true;
		}
		else if (newPosX >= CP_System_GetWindowWidth())
		{
			timeX = (CP_System_GetWindowWidth() - part->pos.x) / (newPosX - part->pos.x) * time;
			collisionX = true;
		}

		if (newPosY <= 0)
		{
			timeY = part->pos.y / (part->pos.y - newPosY) * time;
			collisionY = true;
		}
		else if (newPosY >= CP_System_GetWindowHeight())
		{
			timeY = (CP_System_GetWindowHeight() - part->pos.y) / (newPosY - part->pos.y) * time;
			collisionY = true;
		}

		// resolve collisions
		if ((collisionX == true) || (collisionY == true))
		{

			// take the nearest time
			if (timeX < timeY)
			{
				newTime = timeX;
			}
			else
			{
				newTime = timeY;
			}

			// move the particle
			part->pos.x += part->vel.x * newTime;
			part->pos.y += part->vel.y * newTime;

			// flip velocity vectors to reflect off walls
			if ((collisionX == true) && (collisionY == false))
			{
				part->vel.x *= -1;
			}
			else if ((collisionX == false) && (collisionY == true))
			{
				part->vel.y *= -1;
			}
			else
			{	// they must both be colliding for this condition to occur
				if (timeX < timeY)
				{
					part->vel.x *= -1;
				}
				else if (timeX > timeY)
				{
					part->vel.y *= -1;
				}
				else
				{	// they must be colliding at the same time (ie. a corner)
					part->vel.x *= -1;
					part->vel.y *= -1;
				}
			}

			// decrease time and iterate
			time -= newTime;
		}
		else
		{
			// no collision
			part->pos.x = newPosX;
			part->pos.y = newPosY;
			time = 0;
		}
	}
}

Particle particles[240];
int numParticles = 240;

float lineProximityDistance = 100.0f;
float mouseProximityDistance = 200.0f;

int recommendedWidth = 1600;
int recommendedHeight = 900;

bool drawColors = true;
bool drawFPS = true;

void DancingLinesInit(void)
{
	//setFrameRate(60.0f);

	for (int i = 0; i < numParticles; ++i) {
		ParticleCreate(&particles[i]);
	}
}

void DancingLinesUpdate(void)
{
  CP_Settings_BlendMode(CP_BLEND_ALPHA);
  CP_Graphics_ClearBackground(CP_Color_Create(51, 51, 51, 255));

	//background(CP_Color_Create((int)(CP_Input_GetMouseX() / canvasWidth * 255.0f), 0, 0, 255));
	//CP_Graphics_Background(CP_CreateColor((int)(CP_Input_GetMouseX() / CP_Graphics_GetCanvasWidth() * 255.0f), 0, 0, 255));

  CP_Settings_NoStroke();
  CP_Settings_Fill(CP_Color_Create(0, 0, 0, 255));

	for (int i = 0; i < numParticles; ++i)
	{
		ParticleUpdate(&particles[i]);
		ParticleDisplay(&particles[i]);
	}

  CP_Settings_BlendMode(CP_BLEND_ADD);
  CP_Settings_StrokeWeight(3);
	CP_Color lineColor;

	for (int i = 0; i < numParticles; ++i)
	{
		// draw white lines from the particles to the mouse position
		float distXMouse = (float)fabsf(particles[i].pos.x - (float)CP_Input_GetMouseX());
		float distYMouse = (float)fabsf(particles[i].pos.y - (float)CP_Input_GetMouseY());
		if (distXMouse < mouseProximityDistance && distYMouse < mouseProximityDistance)
		{
			lineColor.r = 127;
			lineColor.g = 127;
			lineColor.b = 127;
			lineColor.a = (unsigned char)(255.0f * min(1.0f, (mouseProximityDistance - max(distXMouse, distYMouse)) / (mouseProximityDistance * 0.3f)));
      CP_Settings_Stroke(lineColor);
			CP_Graphics_DrawLine(particles[i].pos.x, particles[i].pos.y, CP_Input_GetMouseX(), CP_Input_GetMouseY());
		}

		// draw lines between particles based on the additive color of both particles
		if (!drawColors)
		{
			continue;
		}
		for (int j = i + 1; j < numParticles; ++j)
		{
			float distX = (float)fabsf(particles[i].pos.x - particles[j].pos.x);
			float distY = (float)fabsf(particles[i].pos.y - particles[j].pos.y);
			if (distX < lineProximityDistance && distY < lineProximityDistance)
			{
				lineColor.r = particles[i].color->r + particles[j].color->r;
				lineColor.g = particles[i].color->g + particles[j].color->g;
				lineColor.b = particles[i].color->b + particles[j].color->b;
				lineColor.a = (unsigned char)(255.0f * min(1.0f, (lineProximityDistance - max(distX, distY)) / (lineProximityDistance * 0.3f)));
        CP_Settings_Stroke(lineColor);
				CP_Graphics_DrawLine(particles[i].pos.x, particles[i].pos.y, particles[j].pos.x, particles[j].pos.y);
			}
		}
	}

	if (CP_Input_KeyTriggered(KEY_SPACE))
	{
		drawColors = !drawColors;
	}

	// Profiling info and frameRate testing
	if (drawFPS)
	{
    CP_Settings_TextSize(20);
    CP_Settings_BlendMode(CP_BLEND_ALPHA);
    CP_Settings_Fill(CP_Color_Create(0, 0, 0, 128));
    CP_Settings_NoStroke();
		CP_Graphics_DrawRect(0, 0, 150, 30);
    CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
		char buffer[100];
		sprintf_s(buffer, 100, "FPS: %f", CP_System_GetFrameRate());
    CP_Font_DrawText(buffer, 20, 20);
	}
}
//
// end DANCING LINES
//----------------------------------------------

void demo14init(void)
{
  CP_System_SetWindowSize(1280, 720);
	CP_System_SetFrameRate(2);
	float fontSize = 60.0f;
  CP_Settings_TextSize(fontSize);
}

void demo14(void)
{

	// clear the screen every frame (black)
  CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 255));

  CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_TOP);

	if (CP_System_GetFrameCount() % 2 == 0)
    CP_Settings_Fill(CP_Color_Create(255, 40, 120, 255));
	else
    CP_Settings_Fill(CP_Color_Create(0, 215, 135, 255));

	// set the font to be the default font
  CP_Font_Set(CP_Font_GetDefault());

	// write "Hello, World!" in the center of the screen
  CP_Font_DrawText("Hello, World!", 100, 100);

	// Load the font Exo2 Light Italic and store it in my PFont variable
	CP_Font font1 = CP_Font_Load("./Assets/Exo2-LightItalic.ttf");

	// set the new font
  CP_Font_Set(font1);

	// write "Hello, World!" in the center of the screen
  CP_Font_DrawText("Hello, World!", 100, 150);

	// set the font to be the default font
  CP_Font_Set(CP_Font_GetDefault());
	// write "Hello, World!" in the center of the screen
  CP_Font_DrawText("Hello, World!", 100, 200);

	// set the new font
  CP_Font_Set(font1);
	// write "Hello, World!" in the center of the screen
  CP_Font_DrawText("Hello, World!", 100, 250);



  CP_Settings_Fill(CP_Color_Create(200, 200, 200, 255));

	// set the font to be the default font
  CP_Font_Set(CP_Font_GetDefault());
  CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_RIGHT, CP_TEXT_ALIGN_V_TOP);
	// write "Hello, World!" in the center of the screen
  CP_Font_DrawTextBox("Hello, World! THIS is a test of the text alignment functions. \
This is only a test and should not be reproduced, copied or distributed \
for personal, educational or commercial gain.  :)", 100, 300, 800);
}


static float rotation = 0.0f;
static int value = 0;
static float stroke_w = 0;
void rotated_shapes_update(void)
{
	CP_Settings_TextSize(36.0f);

	if (!value) { value = 1; CP_System_SetWindowSize(1000, 400); }
	CP_Color myColor = CP_Color_Create(0, 255, 0, 255);

	CP_Graphics_ClearBackground(myColor);
	rotation += 2.0f;
	while (rotation > 360.0f) {
		rotation -= 360.0f;
	}

	stroke_w = CP_Math_LerpFloat(stroke_w, (rotation / 8.0f) + 1.0f, 0.1f);
	CP_Settings_StrokeWeight(stroke_w);

	//////////
	// y, x //
	//////////

	//1, 1
	CP_Settings_RectMode(CP_POSITION_CENTER);
	CP_Graphics_DrawRectAdvanced(100, 100, 50, 20, rotation, 0);
	CP_Graphics_DrawPoint(100, 100);

	//1, 2
	CP_Settings_EllipseMode(CP_POSITION_CENTER);
	CP_Graphics_DrawEllipseAdvanced(350, 110, 50, 20, rotation);
	CP_Graphics_DrawPoint(350, 110);

	//1, 3
	CP_Graphics_DrawLineAdvanced(500, 125, 550, 125, rotation);
	CP_Graphics_DrawPoint(525, 125);

	//1, 4
	CP_Graphics_DrawTriangleAdvanced(700, 100, 750, 100, 725, 150, rotation);
	CP_Graphics_DrawPoint(725, 117);

	//1, 5
	CP_Graphics_DrawQuadAdvanced(910, 100, 960, 100, 940, 150, 890, 150, rotation);
	CP_Graphics_DrawPoint(925, 125);


	//2, 1
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Graphics_DrawRectAdvanced(100, 300, 50, 20, rotation, 0);
	CP_Graphics_DrawPoint(100, 300);

	//2, 2
	CP_Settings_EllipseMode(CP_POSITION_CORNER);
	CP_Graphics_DrawEllipseAdvanced(350, 310, 50, 20, rotation);
	CP_Graphics_DrawPoint(350, 310);

	//2, 3
	CP_Graphics_DrawLine(500, 325, 550, 325);
	CP_Graphics_DrawPoint(525, 325);

	//2, 4
	CP_Graphics_DrawTriangle(700, 300, 750, 300, 725, 350);
	CP_Graphics_DrawPoint(725, 317);

	//1, 5
	CP_Graphics_DrawQuad(910, 300, 960, 300, 940, 350, 890, 350);
	CP_Graphics_DrawPoint(925, 325);

	char buffer[1024] = { 0 };
	sprintf_s(buffer, 1024, "%f", rotation);
	CP_Font_DrawText(buffer, CP_Input_GetMouseX() + 10, CP_Input_GetMouseY() + 10);

	// Set the background color to blue
	//background(60, 120, 255);

	// Load a font from the assets folder
	CP_Font font1 = CP_Font_Load("./Assets/Exo2-Regular.ttf");

	// Select it as the current font, setting the size to 36
	CP_Font_Set(font1);

	// Write "Hello, World!" at the mouse position
	CP_Font_DrawText("Hello, World!", CP_Input_GetMouseX(), CP_Input_GetMouseY());

	// Select the default font, size 36
	CP_Font_Set(CP_Font_GetDefault());

	// Write "Hello, World!" at the point (25, 25)
	CP_Font_DrawText("Hello, World!", 25.0f, 25.0f);

	// Set the background color to blue
	//background(60, 120, 255);

	// Load an image from the assets folder and store it in a CP_Image
	CP_Image img1 = CP_Image_Load("./Assets/image.png");

	// Display the image at the mouse position
	CP_Image_Draw(img1, CP_Input_GetMouseX(), CP_Input_GetMouseY(), 100, 100, 255);

	// Load a sound from the assets folder
	// Pass 0 as FALSE so the sound doesn't loop
	CP_Sound mySound = CP_Sound_Load("./Assets/Clap.wav");

	// If the left mouse is clicked, play the sound
	if (CP_Input_MouseClicked())
	{
		CP_Sound_Play(mySound);
	}

	// create a transform and set it to initialized values
	CP_Matrix transform = CP_Matrix_Identity();

	// make the matrix a translation transform by (100, 100)
	transform = CP_Matrix_Translate((CP_Vector) { 100, 100 });

	// create a CP_Vector position
	CP_Vector position;
	position.x = 100;
	position.y = 100;

	// apply that matrix to the point and store it in the position
	position = CP_Vector_MatrixMultiply(transform, position);


	//CP_Color myColor = CP_Color_Create(20, 39, 67, 255);

	//backgroundColor(myColor);

	// Clear the background to a blue color
	//background(20, 200, 255);

	//// Draw a rectangle at the point (100, 100)
	//CP_Graphics_DrawRect(100.0f, 100.0f, 50.0f, 50.0f);

	//// Draw a rectangle at the mouse position
	//CP_Graphics_DrawRect(CP_Input_GetMouseX(), CP_Input_GetMouseY(), 25.0f, 25.0f);

	//// Draw an ellipse at the point (100, 100)
	//CP_Graphics_DrawEllipse(100.0f, 100.0f, 50.0f, 80.0f);

	//// Draw an ellipse at the mouse position
	//CP_Graphics_DrawEllipse(CP_Input_GetMouseX(), CP_Input_GetMouseY(), 200.0f, 200.0f);

	//// set the background color
	//CP_Graphics_ClearBackground(CP_Color_Create(255, 40, 200, 255));

	////draw a circle at (100, 100)
	//CP_Graphics_DrawCircle(100, 100, 50.0f);

	//// draw a circle at the mouse position
	//CP_Graphics_DrawCircle(CP_Input_GetMouseX(), CP_Input_GetMouseY(), 20.0f);

	//// draw a line from (100, 100) to (100, 200)
	////   x1      y1      x2      y2
	//CP_Graphics_DrawLine(100.0f, 100.0f, 200.0f, 100.0f);

	//// draw a line from the origin to the mouse position
	//CP_Graphics_DrawLine(0.0f, 0.0f, CP_Input_GetMouseX(), CP_Input_GetMouseY());
}

static int val4 = 1;
void line_func(void)
{
	//if (val4)
	//{
	//    fill(255, 0, 0, 255);
	//    noStroke();
	//}
	//else
	//{
	//    stroke(0, 255, 0, 255);
	//    noFill();
	//
	//}
	//
	//if (CP_Input_MouseClicked())
	//{
	//    if (val4)
	//        val4 = 0;
	//    else val4 = 1;
	//}
	//
	//CP_Graphics_DrawRect(CP_Input_GetMouseX(), CP_Input_GetMouseY(), 50, 50);

	// set the fill color to red
	// fill(255, 0, 0, 255);
	// 
	// // draw a triangle with three points
	// CP_Graphics_DrawTriangle(100.0f, 100.0f,  // point 1
	//          200.0f, 100.0f,  // point 2
	//          200.0f, 200.0f); // point 3
	// 
	// // set the fill color to dark blue
	// fill(0, 0, 160, 255);
	// 
	// // draw a triangle with three points
	// CP_Graphics_DrawTriangle(0.0f, 0.0f,        // point 1
	//          CP_Input_GetMouseX(), CP_Input_GetMouseY(),    // point 2
	//          CP_Input_GetMousePreviousX(), CP_Input_GetMousePreviousY()); // point 3
	// 
	// fill(255, 0, 0, 255);
	// CP_Graphics_DrawCircle(CP_Input_GetMouseX(), CP_Input_GetMouseY(), 50.0f);
	// fill(0, 255, 0, 255);
	// CP_Graphics_DrawEllipse(CP_Input_GetMouseX(), CP_Input_GetMouseY(), 50.0f, 50.0f);

	// draw a point at the mouse position
	// noStroke();
	// noCursor();
	// fill(0, 0, 0, 255);
	// CP_Graphics_DrawPoint(CP_Input_GetMouseX(), CP_Input_GetMouseY());


}

CP_Vector a, b, c, d;

void init(void)
{
	//background(0, 0, 0);

	////strokeWeight(4.0f);
	//fill(255, 255, 255, 10);
	//stroke(255, 255, 255, 40);

	//a.x = (float) CP_Input_GetMouseX();
	//a.y = (float) CP_Input_GetMouseY();

	//b = CP_Vector_Set((float)CP_Input_GetMouseX() + 100.0f, (float)CP_Input_GetMouseY() );
	//c = CP_Vector_Set((float)CP_Input_GetMouseX() + 100.0f, (float)CP_Input_GetMouseY() + 50 );
	//d = CP_Vector_Set((float)CP_Input_GetMouseX(), CP_Input_GetMouseY() + 50.0f );
	CP_Graphics_DrawRect(CP_Input_GetMouseX(), CP_Input_GetMouseY(), 100, 100);
}


void update(void)
{
  CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 255));
	a.x = CP_Input_GetMouseX();
	a.y = CP_Input_GetMouseY();

	float factor = 0.2f;
	b.x = CP_Math_LerpFloat(b.x, CP_Input_GetMouseX() + 100.0f, CP_Math_ClampFloat(factor*2.0f, 0.0f, 1.0f));
	b.y = CP_Math_LerpFloat(b.y, CP_Input_GetMouseY(), CP_Math_ClampFloat(factor*2.0f, 0.0f, 1.0f));

	c.x = CP_Math_LerpFloat(c.x, CP_Input_GetMouseX() + 100.0f, CP_Math_ClampFloat(factor, 0.0f, 1.0f));
	c.y = CP_Math_LerpFloat(c.y, CP_Input_GetMouseY() + 50.0f, CP_Math_ClampFloat(factor, 0.0f, 1.0f));

	d.x = CP_Math_LerpFloat(d.x, CP_Input_GetMouseX(), CP_Math_ClampFloat(factor*2.0f, 0.0f, 1.0f));
	d.y = CP_Math_LerpFloat(d.y, CP_Input_GetMouseY() + 50.0f, CP_Math_ClampFloat(factor*2.0f, 0.0f, 1.0f));

	//CP_Graphics_DrawQuad(a.x, a.y, b.x, b.y, c.x, c.y, d.x, d.y);

	//CP_Image i = CP_Image_Load("./Assets/image.png");
  CP_Settings_Fill(CP_Color_Create(0, 255, 255, 255));
	//CP_Image_Draw(i, CP_Input_GetMouseX(), CP_Input_GetMouseY(), 100, 100);

	//CP_Graphics_DrawCircle(100, 100, 50);

	CP_Vector forward = CP_Vector_Set(-1, 0);
	CP_Vector to_mouse = CP_Vector_Set(CP_Input_GetMouseX() - 125, CP_Input_GetMouseY() - 200);
	float d2 = CP_Vector_Angle(forward, to_mouse);
	if (CP_Input_GetMouseY() > 200)
		d2 *= -1;
	CP_Graphics_DrawTriangleAdvanced(100, 200, 150, 180, 150, 220, d2);

	//CP_Graphics_DrawRect(CP_Input_GetMouseX(), CP_Input_GetMouseY(), 100, 100);
	//CP_Graphics_DrawRectAdvanced(CP_Input_GetMouseX(), CP_Input_GetMouseY(), 100.0f, 50.0f, 45.0f, 0);
   // CP_Graphics_DrawTriangleAdvanced(100.0f, 100.0f, 125.0f, 150.0f, 75.0f, 150.0f, 15.0f);
   //
   // CP_Image im1 = CP_Image_Load("./Assets/image.png");
   //
   // CP_Image_DrawAdvanced(im1, CP_Input_GetMouseX(), CP_Input_GetMouseY(), 100, 100, degrees(frameCount / (10.0f)));
}

// variable to track fullscreen or not, set it to initialize in either windowed or fullscreen mode
bool full = false;
unsigned frameTimer = (unsigned int)-1;

void EngineInit(void)
{
	if (full)
	{
    CP_System_Fullscreen();
	}
}

void EngineUpdate(void)
{
  CP_Graphics_ClearBackground(CP_Color_Create(100, 100, 100, 255));

	// delayed break for soft lock testing
	if (CP_System_GetFrameCount() == frameTimer)
	{
		// place breakpoint on this line and press 'B' while the app is running
		frameTimer = CP_System_GetFrameCount();
	}
	if (CP_Input_KeyTriggered(KEY_B))
	{
    CP_Engine_Terminate();
//		frameTimer = frameCount + 60;
	}

	// toggle fullscreen
	if (CP_Input_KeyTriggered(KEY_R))
	{
		if (full)
		{
      CP_System_SetWindowSize(400, 400);
		}
		else
		{
      CP_System_Fullscreen();
		}
		full = !full;
	}

	CP_Graphics_DrawRect(10, 10, 10, 10);
	char buffer[128] = { 0 };
	sprintf_s(buffer, 128, "( %d , %d )", CP_System_GetWindowWidth(), CP_System_GetWindowHeight());
  CP_Font_DrawText(buffer, 30, 30);
}






int mycounter = 0;
void image_stress(void)
{
    unsigned char* data = calloc(256 * 256 * 4, sizeof(char));
    for (int i = 0; i < 256 * 256 * 4; i += 4)
    {
        data[i]     = 255;
        data[i + 1] = 200;
        data[i + 2] = 100;
        data[i + 3] = 255;
    }
    CP_Image myimg = CP_Image_CreateFromData(256, 256, data);
    if (++mycounter >= 100 && myimg)
    {
        mycounter = 0;
    }
	free(data);
}


CP_Sound s;
void test_sound_rev(void)
{
    s = CP_Sound_Load("./Assets/beep.wav");
    Sleep(1000);

    CP_Sound_Play(s);
    Sleep(1000);
}


void lerp_test_init(void)
{

}

void lerp_test_update(void)
{
	// Create colors for the four corners of the screen
	CP_Color Red = CP_Color_Create(255, 0, 0, 255);
	CP_Color Green = CP_Color_Create(0, 255, 0, 255);
	CP_Color Blue = CP_Color_Create(0, 0, 255, 255);
	CP_Color White = CP_Color_Create(255, 255, 255, 255);

	// Get the mouse position relative to the canvas
	float mx = (float)CP_Input_GetMouseWorldX() / (float)CP_System_GetWindowWidth();
	float my = (float)CP_Input_GetMouseWorldY() / (float)CP_System_GetWindowHeight();

	// Clamp the values
	mx = CP_Math_ClampFloat(mx, 0.0f, 1.0f);
	my = CP_Math_ClampFloat(my, 0.0f, 1.0f);

	// Lerp the colors based on position along the x-axis
	CP_Color lerpx1 = CP_Color_Lerp(Red, Blue, mx);
	CP_Color lerpx2 = CP_Color_Lerp(Green, White, mx);

	// Lerp the two previous colors based on y-axis position
	CP_Color lerp = CP_Color_Lerp(lerpx1, lerpx2, my);

	// Set the background based on the lerp
	CP_Graphics_ClearBackground(lerp);
}

int Up = 1;
float Count = 0.0f;

void dt_init(void)
{
}
void dt_example(void)
{
	CP_Color Red = CP_Color_Create(255, 0, 0, 255);
	CP_Color Blue = CP_Color_Create(0, 0, 255, 255);
	
	if (Up)
	{
		Count += CP_System_GetDt();
		if (Count > 1.0f)
		{
			Up = 0;
			Count = 1.0f;
		}
	}
	else
	{
		Count -= CP_System_GetDt();
		if (Count < 0.0f)
		{
			Up = 1;
			Count = 0.0f;
		}
	}

	CP_Color lerp = CP_Color_Lerp(Red, Blue, Count);
  CP_Graphics_ClearBackground(lerp);
}

void save_restore_test_init(void)
{
	CP_Settings_RectMode(CP_POSITION_CENTER);
}

void save_restore_test_update(void)
{
	// Draw a white background
  CP_Graphics_ClearBackground(CP_Color_Create(255, 255, 255, 255));

	// Draw a red square in the top left corner
  CP_Settings_Fill(CP_Color_Create(255, 0, 0, 255));
  CP_Settings_Stroke(CP_Color_Create(0, 0, 255, 255));
	CP_Graphics_DrawRect(CP_System_GetWindowWidth() * 0.25f, CP_System_GetWindowHeight() * 0.20f, 100.0f, 100.0f);

	// Save the settings of the first square
  CP_Settings_Save();

	// Draw a blue square in the top middle
  CP_Settings_Fill(CP_Color_Create(0, 0, 255, 255));
  CP_Settings_Stroke(CP_Color_Create(255, 0, 0, 255));
  CP_Settings_StrokeWeight(5);
	CP_Graphics_DrawRect(CP_System_GetWindowWidth() * 0.5f, CP_System_GetWindowHeight() * 0.35f, 100.0f, 100.0f);

	// Save the settings of the second square
  CP_Settings_Save();

  CP_Settings_Translate(CP_System_GetWindowWidth() * 0.75f, CP_System_GetWindowHeight() * 0.5f);
  CP_Settings_Rotate(CP_System_GetFrameCount() * 2.5f);

	// Now draw a pink and green square on the right side of the screen
  CP_Settings_Fill(CP_Color_Create(200, 0, 200, 255));
  CP_Settings_Stroke(CP_Color_Create(0, 255, 0, 255));
  CP_Settings_StrokeWeight(10);
	CP_Graphics_DrawRect(-50.0f, 50.0f, 100.0f, 100.0f);

	// Restore the blue settings and draw a blue square in the bottom middle
  CP_Settings_Restore();

	CP_Graphics_DrawRect(CP_System_GetWindowWidth() * 0.5f, CP_System_GetWindowHeight() * 0.65f, 100.0f, 100.0f);

	// Restore the red settings and draw a red square in the bottom left
  CP_Settings_Restore();
	
	CP_Graphics_DrawRect(CP_System_GetWindowWidth() * 0.25f, CP_System_GetWindowHeight() * 0.80f, 100.0f, 100.0f);
}

void matrix_test_init(void)
{
	CP_Settings_RectMode(CP_POSITION_CENTER);
  CP_Settings_NoStroke();
}
void matrix_test_update(void)
{
	// White background
  CP_Graphics_ClearBackground(CP_Color_Create(255, 255, 255, 255));

	CP_Vector position = CP_Vector_Set(CP_System_GetWindowWidth() * 0.5f, CP_System_GetWindowHeight() * 0.5f);

	// Create a transform matrix
	CP_Matrix scale = CP_Matrix_Scale(CP_Vector_Set(150, 100));
	CP_Matrix translate = CP_Matrix_Translate(position);
	CP_Matrix rotate = CP_Matrix_RotateRadians((float)M_PI * 0.25f);

	// Combine transfrom
	// Translate * rotation * scale
	CP_Matrix transform = CP_Matrix_Multiply(translate, CP_Matrix_Multiply(rotate, scale));

	// Set the camera transfrom to the created matrix
  CP_Settings_ApplyMatrix(transform);

	// Draw a blue cube at the "center" of the screen
  CP_Settings_Fill(CP_Color_Create(0, 0, 255, 255));
	CP_Graphics_DrawRect(0, 0, 1, 1);

	// Reset the matrix to the identity matrix
  CP_Settings_ResetMatrix();

	// Draw a red cube in the center of the screen
  CP_Settings_Fill(CP_Color_Create(255, 0, 0, 255));
	CP_Graphics_DrawRect(position.x, position.y, 50, 50);
}

CP_Vector end_position;
void matrix_test_init2(void)
{
	end_position = CP_Vector_Set(CP_System_GetWindowWidth() / 2.0f + 80.0f, CP_System_GetWindowHeight() / 2.0f + 80.0f);
}

void matrix_test_update2(void)
{
  CP_Graphics_ClearBackground(CP_Color_Create(255, 255, 255, 255));

	// Create transformation matrices
	CP_Matrix trans = CP_Matrix_FromVector(CP_Vector_Set(1.0f, 0.0f), CP_Vector_Set(0.0f, 1.0f), CP_Vector_Set(CP_System_GetWindowWidth() / 2.0f, CP_System_GetWindowHeight() / 2.0f));

	//CP_Matrix trans = CP_Matrix_translate(CP_Vector_Set(canvasWidth / 2.0f, canvasHeight / 2.0f));
	CP_Matrix rotate = CP_Matrix_Rotate(5.0f);

	// Combine
	CP_Matrix transform = CP_Matrix_Multiply(trans, CP_Matrix_Multiply(rotate, CP_Matrix_Inverse(trans)));

	// Update the end point of the line
	end_position = CP_Vector_MatrixMultiply(transform, end_position);

	CP_Graphics_DrawLine(CP_System_GetWindowWidth() / 2.0f, CP_System_GetWindowHeight() / 2.0f, end_position.x, end_position.y);
}

#include <stdio.h>

CP_Vector random_v;
void transpose_init(void)
{
  CP_Font_Set(CP_Font_GetDefault());
  CP_Settings_TextSize(30.0f);
  CP_Settings_Fill(CP_Color_Create(0, 0, 0, 255));

	random_v = CP_Vector_Set(CP_Random_RangeFloat(0, 50), CP_Random_RangeFloat(0, 50));
}

void transpose_test(void)
{
  CP_Graphics_ClearBackground(CP_Color_Create(255, 255, 255, 255));

	// Print out the default vector
  CP_Font_DrawText("Random: ", 0, 30);
	char matrix[128] = { 0 };
	sprintf_s(matrix, 128, "[%.0f]\n[%.0f]\n",
		random_v.x, random_v.y);
  CP_Font_DrawTextBox(matrix, 175, 30, 200);
}

CP_Vector position;
CP_Vector position2;
CP_Vector speed;
CP_Vector norm;

void vector_init(void)
{
	position = CP_Vector_Set(CP_System_GetWindowWidth() * 0.5f, CP_System_GetWindowHeight() * 0.25f);
	position2 = CP_Vector_Set(CP_System_GetWindowWidth() * 0.5f, 3 * CP_System_GetWindowHeight() * 0.25f);
	speed = CP_Vector_Set(CP_Random_RangeFloat(5.0f, 10.0f), 0.0f);
	norm = CP_Vector_Normalize(speed);

	CP_Settings_EllipseMode(CP_POSITION_CENTER);
  CP_Font_Set(CP_Font_GetDefault());
  CP_Settings_TextSize(20.0f);
}

void vector_update(void)
{
  CP_Graphics_ClearBackground(CP_Color_Create(255, 255, 255, 255));

	// Check if direction should be inverted
	if (position.x > CP_System_GetWindowWidth())
	{
		speed = CP_Vector_Negate(speed);
		norm = CP_Vector_Normalize(speed);
		position.x = (float)CP_System_GetWindowWidth();
	}
	else if (position.x < 0)
	{
		speed = CP_Vector_Negate(speed);
		norm = CP_Vector_Normalize(speed);
		position.x = 0;
	}

	// Update position
	position = CP_Vector_Add(position, speed);
	position2 = CP_Vector_Add(position2, norm);

	// Draw the circles
  CP_Settings_Fill(CP_Color_Create(255, 0, 0, 255));
	CP_Graphics_DrawCircle(position.x, position.y, 40);
  CP_Settings_Fill(CP_Color_Create(0, 0, 255, 255));
	CP_Graphics_DrawCircle(position2.x, position2.y, 40);

	// Draw the speed and normzlied speed
	char speed_text[128] = {0};
	sprintf_s(speed_text, 128, "Speed: [%.2f,%.2f] Normalized: [%.2f,%.2f]", speed.x, speed.y, norm.x, norm.y);
  CP_Settings_Fill(CP_Color_Create(0, 0, 0, 255));
  CP_Font_DrawText(speed_text, CP_System_GetWindowWidth() * 0.5f - 160, CP_System_GetWindowHeight() * 0.5f);
}

CP_Vector vector1;
CP_Vector vector2;
float cross1;
float cross2;

void vector2_init(void)
{
	vector1 = CP_Vector_Set(CP_Random_RangeFloat(0.0f, 50.0f), CP_Random_RangeFloat(0.0f, 50.0f));
	vector2 = CP_Vector_Set(CP_Random_RangeFloat(0.0f, 50.0f), CP_Random_RangeFloat(0.0f, 50.0f));

	cross1 = CP_Vector_CrossProduct(vector1, vector2);
	cross2 = CP_Vector_CrossProduct(vector2, vector1);

  CP_Font_Set(CP_Font_GetDefault());
  CP_Settings_TextSize(30.0f);
  CP_Settings_Fill(CP_Color_Create(0, 0, 0, 255));
}

void vector2_update(void)
{
	// Print out the randomly generated vectors
	char vec_text[128] = { 0 };
	sprintf_s(vec_text, 128, "1st Vector: [%.2f, %.2f]", vector1.x, vector1.y);
  CP_Font_DrawText(vec_text, 0, 50);
	sprintf_s(vec_text, 128, "2st Vector: [%.2f, %.2f]", vector2.x, vector2.y);
  CP_Font_DrawText(vec_text, 0, 100);

	// Print out the first cross product
	sprintf_s(vec_text, 128, "Cross AxB: %.2f", cross1);
  CP_Font_DrawText(vec_text, 0, 150);

	// Print out the second cross product
	sprintf_s(vec_text, 128, "Cross BxA: %.2f", cross2);
  CP_Font_DrawText(vec_text, 0, 200);
}

CP_Image justin_face;
void i_init(void)
{
	justin_face = CP_Image_Load("./Assets/justin1.png");
}
float rot_counter = 0;
void i_update(void)
{
	// Increment rotation lerp factor or reset
	if (rot_counter >= 1.0f)
		rot_counter -= 1.0f;
	else
		rot_counter += CP_System_GetDt();

	// Lerp rotation and draw image
	float rot = CP_Math_LerpFloat(0.0f, 360.0f, rot_counter);
  CP_Graphics_ClearBackground(CP_Color_Create(255, 255, 255, 255));

	CP_Settings_Tint(CP_Color_Create(CP_System_GetFrameCount(), 0, 0, 255));
	CP_Image_DrawAdvanced(justin_face, CP_Input_GetMouseX(), CP_Input_GetMouseY(), 100, 150, 255, rot);
	// restore the tint color
	CP_Settings_NoTint();
}

void deg_rad_init(void)
{
	CP_Settings_EllipseMode(CP_POSITION_CENTER);
  CP_Font_Set(CP_Font_GetDefault());
  CP_Settings_TextSize(30.0f);
  CP_Settings_Fill(CP_Color_Create(0, 0, 0, 255));
}

float r_degrees;
float counter = 2.0f;
void deg_rad_update(void)
{
  CP_Graphics_ClearBackground(CP_Color_Create(255, 255, 255, 255));

	// Change degrees every few seconds
	if (counter >= 2.0f)
	{
		r_degrees = CP_Random_RangeFloat(0.0f, 360.0f);
		counter = 0.0f;
	}
	else
		counter += CP_System_GetDt();

	// Print out number of degrees and convert to radians
	char buffer[128] = { 0 };
	sprintf_s(buffer, 128, "Degrees: %.3f\nis equal to\nRadians: %.3f", r_degrees, CP_Math_Radians(r_degrees));
  CP_Font_DrawTextBox(buffer, 100, 150, 200);
}

void square_init(void)
{
  CP_Font_Set(CP_Font_GetDefault());
  CP_Settings_TextSize(50.0f);
  CP_Settings_Fill(CP_Color_Create(0, 0, 0, 255));
  CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_BOTTOM);
}

float random_value = 0.0f;
float time_elapsed = 1.5;
void square_update(void)
{
  CP_Graphics_ClearBackground(CP_Color_Create(255, 255, 255, 255));

	if (time_elapsed >= 1.5f)
	{
		random_value = CP_Random_RangeFloat(0.0f, 100.0f);
		time_elapsed = 0.0f;
	}
	else
		time_elapsed += CP_System_GetDt();

	char num_string[128] = { 0 };
	sprintf_s(num_string, 128, "The square of %.2f is %.2f", random_value, CP_Math_Square(random_value));
  CP_Font_DrawTextBox(num_string, 0, CP_System_GetWindowHeight() * 0.5f, (float)CP_System_GetWindowWidth());
}

void align_test_init(void)
{
	CP_System_SetWindowSize(900, 400);
  CP_Settings_TextSize(35);
}

int h_choice = 0;
int v_choice = 2;

const char* h_text[] = { "LEFT ", "CENTER ", "RIGHT " };
const char* v_text[] = { "TOP ", "MIDDLE ", "BASELINE ", "BOTTOM "};
const char* sample_text = "Sample";
void align_text_update(void)
{
  CP_Graphics_ClearBackground(CP_Color_Create(255, 255, 255, 255));

	// Draw the crosshairs
  CP_Settings_Fill(CP_Color_Create(0, 0, 0, 255));
	CP_Graphics_DrawLine(CP_System_GetWindowWidth() * 0.5f - 75, CP_System_GetWindowHeight() * 0.5f, CP_System_GetWindowWidth() * 0.5f + 75, CP_System_GetWindowHeight() * 0.5f);
	CP_Graphics_DrawLine(CP_System_GetWindowWidth() * 0.5f, CP_System_GetWindowHeight() * 0.5f - 75, CP_System_GetWindowWidth() * 0.5f, CP_System_GetWindowHeight() * 0.5f + 75);

	// Lets user cycle through alignment choices
	if (CP_Input_KeyTriggered(KEY_LEFT))
		h_choice++;
	else if (CP_Input_KeyTriggered(KEY_RIGHT))
		h_choice--;

	if (CP_Input_KeyTriggered(KEY_DOWN))
		v_choice--;
	else if (CP_Input_KeyTriggered(KEY_UP))
		v_choice++;

	// Clamp the user choice to the available options
	h_choice = CP_Math_ClampInt(h_choice, 0, 2);
	v_choice = CP_Math_ClampInt(v_choice, 0, 3);

	// Set the alinment based on the choices
	CP_TEXT_ALIGN_HORIZONTAL h_alignment = CP_TEXT_ALIGN_H_LEFT;
	switch (h_choice)
	{
	case 0 :
		h_alignment = CP_TEXT_ALIGN_H_LEFT;
		break;
	case 1 :
		h_alignment = CP_TEXT_ALIGN_H_CENTER;
		break;
	case 2:
		h_alignment = CP_TEXT_ALIGN_H_RIGHT;
		break;
	}

	CP_TEXT_ALIGN_VERTICAL v_alignment = CP_TEXT_ALIGN_V_TOP;
	switch (v_choice)
	{
	case 0:
		v_alignment = CP_TEXT_ALIGN_V_TOP;
		break;
	case 1:
		v_alignment = CP_TEXT_ALIGN_V_MIDDLE;
		break;
	case 2:
		v_alignment = CP_TEXT_ALIGN_V_BASELINE;
		break;
	case 3:
		v_alignment = CP_TEXT_ALIGN_V_BOTTOM;
		break;
	}

  CP_Settings_TextAlignment(h_alignment, v_alignment);

	// Create text to draw based on choices
	char align_text[32] = { 0 };
	strcat_s(align_text, 32, h_text[h_choice]);
	strcat_s(align_text, 32, v_text[v_choice]);
	strcat_s(align_text, 32, sample_text);

	// Draw the text to the screen
  CP_Settings_Fill(CP_Color_Create(255, 0, 0, 255));
  CP_Font_DrawText(align_text, CP_System_GetWindowWidth() * 0.5f, CP_System_GetWindowHeight() * 0.5f);

	// Draw text telling user what to do
  CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_TOP);
  CP_Font_DrawTextBox("Use arrow keys to cycle through text alignments.", 0, 0, (float)CP_System_GetWindowWidth());
}

void initRedRects(void)
{
	// test stuff
  CP_System_SetWindowSize(800, 800);

	printf("this is a TEST (%i, %i)", CP_System_GetWindowWidth(), CP_System_GetWindowHeight());
}

void updateRedRects(void)
{
  CP_Settings_Fill(CP_Color_Create(CP_Random_RangeInt(0, 255), 0, 0, 255));
	float randW = CP_Random_RangeFloat(20, 200);
	float randH = CP_Random_RangeFloat(20, 200);
	float randX = CP_Random_RangeFloat(20, CP_System_GetWindowWidth() - 20 - randW);
	float randY = CP_Random_RangeFloat(20, CP_System_GetWindowHeight() - 20 - randH);
	CP_Graphics_DrawRect(randX, randY, randW, randH);
}

void initCircleTests(void)
{
	// test stuff
  CP_System_SetWindowSize(400, 400);
  CP_Settings_RectMode(CP_POSITION_CORNER);
}

void updateCircleTests(void)
{
	CP_Graphics_DrawRectAdvanced(10, 10, 380, 380, 0, (float)CP_System_GetFrameCount());
}

//*
void initHSVTests(void)
{
	// test stuff
  CP_System_SetWindowSize(400, 400);
	CP_Settings_RectMode(CP_POSITION_CORNER);
}

void updateHSVTests(void)
{
  CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 255));
	CP_ColorHSL hsl = CP_ColorHSL_Create((CP_System_GetFrameCount() * 3) % 360, 100, 50, 255);
  CP_Settings_Fill(CP_Color_FromColorHSL(hsl));
  CP_Graphics_DrawRectAdvanced(10, 10, 380, 190, 0, 20);

	CP_Color rgb = CP_Color_Create((int)((sinf(CP_System_GetFrameCount() * 0.05f) + 1.0f) * 128.0f) % 256,
					   (int)((sinf(CP_System_GetFrameCount() * 0.05f + 2.0f * (float)M_PI / 3.0f) + 1.0f) * 128.0f) % 256,
					   (int)((sinf(CP_System_GetFrameCount() * 0.05f + 4.0f * (float)M_PI / 3.0f) + 1.0f) * 128.0f) % 256, 255);
	
	//printf("rgb: %d, %d, %d\n", rgb.r, rgb.g, rgb.b);

  CP_Settings_Fill(CP_Color_FromColorHSL(CP_ColorHSL_FromColor(rgb)));
  CP_Graphics_DrawRectAdvanced(10, 210, 380, 190, 0, 20);

	CP_ColorHSL testColor1 = CP_ColorHSL_FromColor(CP_Color_Create(180, 0, 179, 255));
	CP_ColorHSL testColor2 = CP_ColorHSL_FromColor(CP_Color_Create(179, 0, 180, 255));

  CP_Settings_Fill(CP_Color_FromColorHSL(testColor1));
	CP_Graphics_DrawCircle(50, 50, 100);

  CP_Settings_Fill(CP_Color_FromColorHSL(testColor2));
	CP_Graphics_DrawCircle(150, 150, 100);
}

//rgb: 193, 0, 190
//rgb: 190, 0, 193

// Horizontal position of the square
float x_pos;
void ForcedInit(void)
{
  // Start the square at the left of the screen
  x_pos = 0;

  // Set the square to draw yellow
  CP_Settings_Fill(CP_Color_Create(255, 255, 0, 255));
}

void ForcedUpdate(void)
{
  // Set background to black
  CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 255));

  // Draw the square
  CP_Graphics_DrawRect(x_pos, (float)CP_System_GetWindowHeight() / 2.0f, 100, 100);
  x_pos += 2;

  // If space pressed, reset the state 
  if (CP_Input_KeyTriggered(KEY_SPACE))
    CP_Engine_SetNextGameStateForced(ForcedInit, ForcedUpdate, NULL);
  else if (CP_Input_KeyTriggered(KEY_Q))
    CP_Engine_Terminate();
}

void titleupdate(void)
{
  // Set the window title to the number of frames that have passed.
  char buffer[256] = { 0 };
  sprintf_s(buffer, 256, "%d frames have passed", CP_System_GetFrameCount());
  CP_System_SetWindowTitle(buffer);
}

//*

CP_Image whole;
float x_size, y_size;

void initsub(void)
{
	CP_System_SetWindowSize(600, 600);
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_Settings_ImageWrapMode(CP_IMAGE_WRAP_REPEAT);
  whole = CP_Image_Load("./Assets/justin1.png");

  x_size = CP_Image_GetWidth(whole) * 0.5f;
  y_size = CP_Image_GetHeight(whole) * 0.5f;
}

void updatesub(void)
{
  CP_Graphics_ClearBackground(CP_Color_Create(255, 255, 255, 255));

  float mx = CP_Input_GetMouseX();
  float my = CP_Input_GetMouseY();

  float offset = (float)CP_System_GetFrameCount();

  // Draws the image with each quarter tinted a different color
  CP_Settings_Tint(CP_Color_Create(255, 0, 0, 255));
  CP_Image_DrawSubImage(whole, mx - x_size, my - y_size, x_size, y_size, 0 + offset, 0, x_size + offset, y_size, 255);
  CP_Settings_Tint(CP_Color_Create(255, 255, 0, 255));
  CP_Image_DrawSubImage(whole, mx, my - y_size, x_size, y_size, x_size + offset, 0, x_size * 2.0f + offset, y_size, 255);
  CP_Settings_Tint(CP_Color_Create(0, 255, 0, 255));
  CP_Image_DrawSubImage(whole, mx - x_size, my, x_size, y_size, 0 + offset, y_size, x_size + offset, y_size * 2.0f, 255);
  CP_Settings_Tint(CP_Color_Create(0, 0, 255, 255));
  CP_Image_DrawSubImage(whole, mx, my, x_size, y_size, x_size + offset, y_size, x_size * 2.0f + offset, y_size * 2.0f, 255);
}


CP_Image jImage;
float x_size, y_size;

void initbuff(void)
{
  jImage = CP_Image_Load("./Assets/image.png");

  x_size = (float)CP_Image_GetWidth(jImage);
  y_size = (float)CP_Image_GetHeight(jImage);

  CP_Settings_Fill(CP_Color_Create(230, 220, 220, 255));
  CP_Settings_Stroke(CP_Color_Create(170, 180, 180, 255));
  CP_Settings_StrokeWeight(20);
  CP_Settings_TextSize(100);
  CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_NEAREST);
}

CP_Image frameBuffer;

void updatebuff(void)
{
  if(frameBuffer)
    CP_Image_Free(&frameBuffer);

  // Read in the frame buffer
  frameBuffer = CP_Image_Screenshot(0, 0, CP_System_GetWindowWidth(), CP_System_GetWindowHeight());
  
  // Choose a random tint color
  // WARNING: Causes flashing colors
  CP_Color rTint = CP_Color_Create(100, 100, 100, 255);// CP_Color_Create(CP_Random_RangeInt(0, 255), CP_Random_RangeInt(0, 255), CP_Random_RangeInt(0, 255), CP_System_GetFrameCount() % 255);

  // Draw to the screen
  CP_Graphics_ClearBackground(CP_Color_Create(255, 255, 255, 255));
  CP_Settings_Tint(rTint);
  CP_Graphics_DrawCircle(0, 0, 200);
  CP_Image_Draw(jImage, CP_Input_GetMouseX(), CP_Input_GetMouseY(), (float)CP_System_GetWindowWidth() / 2.0f, (float)CP_System_GetWindowHeight() / 2.0f, 255);
  CP_Font_DrawText("DEMO TINT", 50, 350);
  CP_Settings_NoTint();
  CP_Image_DrawSubImage(jImage, (float)CP_System_GetWindowWidth() / 2.0f, (float)CP_System_GetWindowHeight() / 2.0f, x_size * 0.5f, y_size * 0.5f, 100, 100, 300, 300, 255);
}

void initfr(void)
{
  // Set frame rate to two frame a second
  CP_System_SetFrameRate(2.0f);

  // Set parameters of the text
  CP_Settings_Fill(CP_Color_Create(0, 0, 0, 255));
  CP_Settings_TextSize(64);
  CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
}

void updatefr(void)
{
  CP_Graphics_ClearBackground(CP_Color_Create(255, 255, 255, 255));

  // Create a string with the frame rate 
  char buffer[128] = { 0 };
  sprintf_s(buffer, 128, "%d frames have elapsed", CP_System_GetFrameCount());

  // Draw to the screen
  CP_Font_DrawText(buffer, (float)CP_System_GetWindowWidth() / 2.0f, (float)CP_System_GetWindowHeight() / 2.0f);
}

CP_Sound snare;
CP_Sound cowbell;
CP_Sound hiHat;

void initS(void)
{
  snare = CP_Sound_Load("./Assets/Snare.wav");
  cowbell = CP_Sound_Load("./Assets/Cowbell.wav");
  hiHat = CP_Sound_Load("./Assets/HiHat.wav");

  CP_Sound_PlayAdvanced(snare, 1.0, 1.0, TRUE, CP_SOUND_GROUP_SFX);
  CP_Sound_PlayAdvanced(hiHat, 1.0, 1.0, TRUE, CP_SOUND_GROUP_SFX);
}

bool isPaused = false;

void updateS(void)
{
  // If the left mouse is clicked, play the sound
  if (CP_Input_MouseClicked())
    CP_Sound_PlayAdvanced(cowbell, 2.0, 0.5, FALSE, CP_SOUND_GROUP_SFX);

  // If space is pressed pause/unpause sounds
  if (CP_Input_KeyTriggered(KEY_SPACE))
  {
    if (isPaused)
    {
      CP_Sound_PlayAdvanced(snare, 1.0, 1.0, TRUE, CP_SOUND_GROUP_SFX);
      CP_Sound_PlayAdvanced(hiHat, 1.0, 1.0, TRUE, CP_SOUND_GROUP_SFX);
    }
    else
      CP_Sound_StopAll();

    // Invert the paused bool
    isPaused = !isPaused;
  }
}

CP_Image created;

void inittint(void)
{
  unsigned char colors[] = { 255,0,0,255,   // Red
                            255,255,0,255, // Yellow
                            0,0,255,255,   // Blue
                            0,255,0,255 };  // Green

  created = CP_Image_CreateFromData(2, 2, colors);
  CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_LINEAR);
}

void updatetint(void)
{
  CP_Graphics_ClearBackground(CP_Color_Create(255, 255, 255, 255));

  CP_Settings_ImageFilterMode(CP_System_GetFrameCount() % 2);
  CP_Image_Draw(created, (float)CP_System_GetWindowWidth() / 2.0f - 100, (float)CP_System_GetWindowHeight() / 2.0f, 100, 100, 255);
  CP_Settings_ImageFilterMode((1 + CP_System_GetFrameCount()) % 2);
  CP_Settings_Tint(CP_Color_Create(CP_Random_RangeInt(0, 255), 127, CP_Random_RangeInt(0, 255), 255));
  CP_Image_Draw(created, (float)CP_System_GetWindowWidth() / 2.0f + 100, (float)CP_System_GetWindowHeight() / 2.0f, 100, 100, 255);
  CP_Settings_NoTint();
}

void DrawGamepadData(float posX, float posY, int i)
{
	if (!CP_Input_GamepadConnectedAdvanced(i))
	{
		// if the gamepad isn't connected then don't render the input state
		return;
	}

  CP_Settings_Save();
  CP_Settings_Translate(posX, posY);

	// find default gamepad
	int defaultId = -1;
	for (int id = 0; id < CP_MAX_GAMEPADS; ++id)
	{
		if (CP_Input_GamepadConnectedAdvanced(id))
		{
			defaultId = id;
			break;
		}
	}

	// INDEX and DEFAULT
	char buffer[16] = { 0 };
	sprintf_s(buffer, _countof(buffer), "Gamepad: %d", i + 1);
  CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
  CP_Font_DrawText(buffer, 10, 25);
	if (defaultId == i)
	{
    CP_Font_DrawText("Default", 10, 45);
	}

  CP_Settings_Translate(0, -5.0f);

	// STICKS
	float circleDiameter = 60.0f;
	float movementOffset = 30.0f;
	CP_Vector rightVec = CP_Vector_Zero(); rightVec.x = 1.0f;
	CP_Vector stick = CP_Input_GamepadLeftStickAdvanced(i);
	int stickClick = CP_Input_GamepadDownAdvanced(GAMEPAD_LEFT_THUMB, i) ? 80 : 50;
  CP_Settings_Fill(CP_Color_FromColorHSL(CP_ColorHSL_Create(stick.y < 0 ? (int)CP_Vector_Angle(stick, rightVec) : 360 - (int)CP_Vector_Angle(stick, rightVec), (int)(CP_Vector_Length(stick) * 100), stickClick, (int)(CP_Vector_Length(stick) * 255) + (CP_Input_GamepadDownAdvanced(GAMEPAD_LEFT_THUMB, i) ? 255 : 0))));
	CP_Graphics_DrawCircle(100.0f + (stick.x * movementOffset), 180.0f + (-stick.y * movementOffset), circleDiameter);
	stick = CP_Input_GamepadRightStickAdvanced(i);
	stickClick = CP_Input_GamepadDownAdvanced(GAMEPAD_RIGHT_THUMB, i) ? 80 : 50;
  CP_Settings_Fill(CP_Color_FromColorHSL(CP_ColorHSL_Create(stick.y < 0 ? (int)CP_Vector_Angle(stick, rightVec) : 360 - (int)CP_Vector_Angle(stick, rightVec), (int)(CP_Vector_Length(stick) * 100), stickClick, (int)(CP_Vector_Length(stick) * 255) + (CP_Input_GamepadDownAdvanced(GAMEPAD_RIGHT_THUMB, i) ? 255 : 0))));
	CP_Graphics_DrawCircle(300.0f + (stick.x * movementOffset), 180.0f + (-stick.y * movementOffset), circleDiameter);

	// TRIGGERS
	float rectWidth = 80.0f;
  CP_Input_GamepadDownAdvanced(GAMEPAD_LEFT_SHOULDER, i) ? CP_Settings_Fill(CP_Color_Create(200, 200, 200, 255)) : CP_Settings_NoFill();
	CP_Graphics_DrawRect(60.0f, 80.0f, rectWidth, 20.0f);
  CP_Input_GamepadDownAdvanced(GAMEPAD_RIGHT_SHOULDER, i) ? CP_Settings_Fill(CP_Color_Create(200, 200, 200, 255)) : CP_Settings_NoFill();
	CP_Graphics_DrawRect(260.0f, 80.0f, rectWidth, 20.0f);
	float trigger = CP_Input_GamepadLeftTriggerAdvanced(i);
  CP_Settings_Fill(CP_Color_Create(255 - (int)(trigger * 255), 0, (int)(trigger * 255), 255));
	CP_Graphics_DrawRect(60.0f, 80.0f, trigger * rectWidth, 20.0f);
	trigger = CP_Input_GamepadRightTriggerAdvanced(i);
  CP_Settings_Fill(CP_Color_Create((int)(trigger * 255), 255, 0, 255));
	CP_Graphics_DrawRect(260.0f, 80.0f, trigger * rectWidth, 20.0f);

	// DPAD
	CP_Settings_RectMode(CP_POSITION_CENTER);
	float buttonSize = 30.0f;
	CP_Color fillcolor = CP_Color_FromColorHSL(CP_ColorHSL_Create((int)(CP_System_GetSeconds() * 60.0f) % 360, 100, 50, 255));
  CP_Input_GamepadDownAdvanced(GAMEPAD_DPAD_UP, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawRect(100.0f, 280.0f, buttonSize, buttonSize);
  CP_Input_GamepadDownAdvanced(GAMEPAD_DPAD_LEFT, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawRect(60.0f, 320.0f, buttonSize, buttonSize);
  CP_Input_GamepadDownAdvanced(GAMEPAD_DPAD_RIGHT, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawRect(140.0f, 320.0f, buttonSize, buttonSize);
  CP_Input_GamepadDownAdvanced(GAMEPAD_DPAD_DOWN, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawRect(100.0f, 360.0f, buttonSize, buttonSize);

	// ABXY
  CP_Input_GamepadDownAdvanced(GAMEPAD_Y, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawCircle(300.0f, 280.0f, buttonSize);
  CP_Input_GamepadDownAdvanced(GAMEPAD_X, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawCircle(260.0f, 320.0f, buttonSize);
  CP_Input_GamepadDownAdvanced(GAMEPAD_B, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawCircle(340.0f, 320.0f, buttonSize);
  CP_Input_GamepadDownAdvanced(GAMEPAD_A, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawCircle(300.0f, 360.0f, buttonSize);

  CP_Settings_Restore();
}

void GamepadDemoInit(void)
{
  CP_System_SetWindowSize(800, 800);
	CP_System_SetFrameRate(60);
  CP_Settings_TextSize(25);
  CP_Settings_Stroke(CP_Color_Create(255, 255, 255, 255));
  CP_Settings_StrokeWeight(3);
}

void GamepadDemoUpdate(void)
{
  CP_Graphics_ClearBackground(CP_Color_Create(40, 40, 40, 255));
	CP_Graphics_DrawLine(CP_System_GetWindowWidth() * 0.5f, 0, CP_System_GetWindowWidth() * 0.5f, (float)CP_System_GetWindowHeight());
	CP_Graphics_DrawLine(0, CP_System_GetWindowHeight() * 0.5f, (float)CP_System_GetWindowWidth(), CP_System_GetWindowHeight() * 0.5f);
	DrawGamepadData(0, 0, 0);
	DrawGamepadData(400, 0, 1);
	DrawGamepadData(0, 400, 2);
	DrawGamepadData(400, 400, 3);
}


void JUSTIN_DEMO_INIT(void)
{
	CP_System_SetFrameRate(10);
}

void JUSTIN_DEMO_UPDATE/*_GetFrameCount*/(void)
{
	// print the current frame rate to the center of the window
	CP_Settings_TextSize(200);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Color bgrColor = CP_Color_Create(200, 200, 200, 255);
	if (CP_Input_KeyDown(KEY_ANY))
	{
		bgrColor = CP_Color_Create(0, 100, 0, 255);
	}
	if (CP_Input_KeyTriggered(KEY_ANY))
	{
		bgrColor = CP_Color_Create(100, 0, 0, 255);
	}
	if (CP_Input_KeyReleased(KEY_ANY))
	{
		bgrColor = CP_Color_Create(0, 0, 100, 255);
	}
	CP_Graphics_ClearBackground(bgrColor);
	CP_Settings_Fill(CP_Color_Create(100, 20, 100, 255));

	float currentFrameRate = CP_System_GetFrameRate();

	char buffer[16] = { 0 };
	sprintf_s(buffer, _countof(buffer), "%.1f", currentFrameRate);
	CP_Font_DrawText(buffer, 200, 200);
}

void JUSTIN_DEMO_UPDATE_GetFrameCount(void)
{
    // print the current frame count to the center of the window
    CP_Settings_TextSize(200);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
    CP_Graphics_ClearBackground(CP_Color_Create(200, 200, 200, 255));
    CP_Settings_Fill(CP_Color_Create(100, 20, 100, 255));
    
    unsigned currentFrameCount = CP_System_GetFrameCount();
    
    char buffer[16] = { 0 };
    sprintf_s(buffer, _countof(buffer), "%u", currentFrameCount);
    CP_Font_DrawText(buffer, 200, 200);
}

CP_Font fontSizeDemoFont = NULL;
void JUSTIN_DEMO_UPDATE_FontSizeTest(void)
{

	//CP_Settings_TextSize(20);
	//fontSizeDemoFont = CP_Font_Load("./Assets/Exo2-LightItalic.ttf");

	CP_Graphics_ClearBackground(CP_Color_Create(150, 150, 150, 255));
	CP_Font_Set(CP_Font_GetDefault());
	CP_Font_DrawText("default - 20", 20, 50);
	CP_Settings_TextSize(50);
	CP_Font_DrawText("default - 50", 20, 100);
	CP_Font_Set(fontSizeDemoFont);
	CP_Font_DrawText("demoFont - 50", 20, 150);
	CP_Font_Set(CP_Font_GetDefault());
	CP_Font_DrawText("default - 50", 20, 200);
	CP_Font_Set(fontSizeDemoFont);
	CP_Font_DrawText("demoFont - 50", 20, 250);
	CP_Settings_TextSize(20);
	CP_Font_DrawText("demoFont - 20", 20, 300);
}

void JUSTIN_DEMO_UPDATE_CAPandJOINT(void)
{
	//CP_System_SetWindowSize(400, 800);
	CP_Graphics_ClearBackground(CP_Color_Create(150, 150, 150, 255));
	CP_LINE_CAP_MODE capModes[] = { CP_LINE_CAP_BUTT, CP_LINE_CAP_ROUND, CP_LINE_CAP_SQUARE };
	CP_LINE_JOINT_MODE jointModes[] = { CP_LINE_JOINT_ROUND, CP_LINE_JOINT_BEVEL, CP_LINE_JOINT_MITER };

	CP_Settings_NoFill();

	for (int capIndex = 0; capIndex < _countof(capModes); ++capIndex)
	{
		// set cap mode
		CP_Settings_LineCapMode(capModes[capIndex]);
		for (int jointIndex = 0; jointIndex < _countof(jointModes); ++jointIndex)
		{
			// set joint mode
			CP_Settings_LineJointMode(jointModes[jointIndex]);

			CP_Settings_Save();

			// translate across the row and column
			CP_Settings_ResetMatrix();
			CP_Settings_Translate(145.0f * jointIndex, 145.0f * capIndex);

			// wide stroke weight to exagerate the cap and joint settings
			CP_Settings_StrokeWeight(15.0f);
			CP_Settings_Stroke(CP_Color_Create(50, 50, 50, 255));

			for (int lineCount = 0; lineCount < 2; ++lineCount)
			{
				// draw a set of lines
				CP_Graphics_BeginShape();
				CP_Graphics_AddVertex(30.0f, 80.0f);
				CP_Graphics_AddVertex(50.0f, 50.0f);
				CP_Graphics_AddVertex(80.0f, 50.0f);
				CP_Graphics_AddVertex(60.0f, 30.0f);
				CP_Graphics_EndShape();

				// narrow stroke weight to show the defined line path
				CP_Settings_StrokeWeight(2.0f);
				CP_Settings_Stroke(CP_Color_Create(50, 200, 200, 255));
			}
		}
	}

	CP_Settings_ResetMatrix();
	CP_Settings_Fill(CP_Color_Create(100, 30, 100, 50));
	CP_Graphics_DrawRect(0, 400.0f, 400.0f, 400.0f);

	for (int capIndex = 0; capIndex < _countof(capModes); ++capIndex)
	{
		for (int jointIndex = 0; jointIndex < _countof(jointModes); ++jointIndex)
		{
			CP_Settings_Restore();

			// translate across the row and column
			CP_Settings_ResetMatrix();
			CP_Settings_Translate(145.0f * jointIndex, 145.0f * capIndex + 400.0f);

			// wide stroke weight to exagerate the cap and joint settings
			CP_Settings_StrokeWeight(15.0f);
			CP_Settings_Stroke(CP_Color_Create(50, 50, 50, 255));

			for (int lineCount = 0; lineCount < 2; ++lineCount)
			{
				// draw a set of lines
				CP_Graphics_BeginShape();
				CP_Graphics_AddVertex(30.0f, 80.0f);
				CP_Graphics_AddVertex(50.0f, 50.0f);
				CP_Graphics_AddVertex(80.0f, 50.0f);
				CP_Graphics_AddVertex(60.0f, 30.0f);
				CP_Graphics_EndShape();

				// narrow stroke weight to show the defined line path
				CP_Settings_StrokeWeight(2.0f);
				CP_Settings_Stroke(CP_Color_Create(50, 200, 200, 255));
			}
		}
	}

}

void JUSTIN_DEMO_UPDATE_DRAWLINES(void)	//NOT IMPLEMENTED - just uses AddVertex
{
	CP_Settings_NoFill();
	CP_Graphics_ClearBackground(CP_Color_Create(150, 150, 150, 255));
	CP_Settings_StrokeWeight(6);
	CP_Settings_Stroke(CP_Color_Create(100, 0, 100, 255));

	static const int vertCount = 128;
	static CP_Vector vertArray[128] = { 0 };
	for (int index = vertCount - 1; index > 0; --index)
	{
		vertArray[index] = vertArray[index - 1];
	}
	vertArray[0].x = CP_Input_GetMouseX();
	vertArray[0].y = CP_Input_GetMouseY();

	CP_Graphics_BeginShape();
	for (int index = 0; index < vertCount; ++index)
	{
		CP_Graphics_AddVertex(vertArray[index].x, vertArray[index].y);
	}
	CP_Graphics_EndShape();
}

void JUSTIN_DEMO_UPDATE_GETBOOL(void)
{
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_BOTTOM);
	CP_Settings_TextSize(40);
	CP_Settings_Fill(CP_Color_Create(100, 20, 100, 255));
	static int headsCounter = 0;
	static int tailsCounter = 0;

	CP_BOOL result = CP_Random_GetBool();
	if (result == TRUE)
	{
		++headsCounter;
	}
	else
	{
		++tailsCounter;
	}

	char buffer[16] = { 0 };

	CP_Graphics_ClearBackground(CP_Color_Create(200, 200, 200, 255));
	CP_Font_DrawText("Heads", 100, 150);
	sprintf_s(buffer, _countof(buffer), "%i", headsCounter);
	CP_Font_DrawText(buffer, 100, 200);
	CP_Font_DrawText("Tails", 300, 250);
	sprintf_s(buffer, _countof(buffer), "%i", tailsCounter);
	CP_Font_DrawText(buffer, 300, 200);
}

void JUSTIN_DEMO_UPDATE_COLORHSV_FROMCOLOR(void)
{
    // Get the frameCount and create a starting color
    int frameCount = CP_System_GetFrameCount();
    CP_Color rgb = CP_Color_Create(255, 0, 0, 255);
    
    // Convert the starting color to HSV
    CP_ColorHSL hsl = CP_ColorHSL_FromColor(rgb);
    
    // Shift the hue of the starting color based on the frameCount
    hsl.h += frameCount * 3;
    
    // Convert back to RBG and set the background color
    CP_Graphics_ClearBackground(CP_Color_FromColorHSL(hsl));
}

void JUSTIN_DEMO_UPDATE_COLOR_FROMCOLORHSV(void)
{
	// Get the current framecount and set the background to black
	int frameCount = CP_System_GetFrameCount();
	CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 255));

	// Use framecount to slowly change the HSV color through all colors of the rainbow
	CP_ColorHSL hsl = CP_ColorHSL_Create(360 - (frameCount * 3), 100, 50, 255);

	// Convert from HSV to RGB to be used as the fill color and draw a rectangle
	CP_Settings_Fill(CP_Color_FromColorHSL(hsl));
	CP_Graphics_DrawRectAdvanced(10, 10, 380, 380, 0, 20);
}

void JUSTIN_DEMO_UPDATE_COLORHSV_LERP(void)
{
    // Create colors for the four corners of the screen
    CP_ColorHSL start = CP_ColorHSL_Create(0, 100, 50, 255);
    CP_ColorHSL end = CP_ColorHSL_Create(359, 100, 50, 255);
    
    // Get the mouse position relative to the canvas
    float mx = (float)CP_Input_GetMouseWorldX() / (float)CP_System_GetWindowWidth();
    
    // Clamp the values
    mx = CP_Math_ClampFloat(mx, 0.0f, 1.0f);
    
    // Lerp the colors based on mouse position along the x-axis
    CP_ColorHSL lerp = CP_ColorHSL_Lerp(start, end, mx);
    
    // Set the background based on the lerp
    CP_Graphics_ClearBackground(CP_Color_FromColorHSL(lerp));
}

void JUSTIN_DEMO_UPDATE_COLOR_LERP(void)
{
    // Create colors for the four corners of the screen
    CP_Color Red = CP_Color_Create(255, 0, 0, 255);
    CP_Color Green = CP_Color_Create(0, 255, 0, 255);
    CP_Color Blue = CP_Color_Create(0, 0, 255, 255);
    CP_Color White = CP_Color_Create(255, 255, 255, 255);
    
    // Get the mouse position relative to the canvas
    float mx = (float)CP_Input_GetMouseWorldX() / (float)CP_System_GetWindowWidth();
    float my = (float)CP_Input_GetMouseWorldY() / (float)CP_System_GetWindowHeight();
    
    // Clamp the values
    mx = CP_Math_ClampFloat(mx, 0.0f, 1.0f);
    my = CP_Math_ClampFloat(my, 0.0f, 1.0f);
    
    // Lerp the colors based on position along the x-axis
    CP_Color lerpx1 = CP_Color_Lerp(Red, Blue, mx);
    CP_Color lerpx2 = CP_Color_Lerp(Green, White, mx);
    
    // Lerp the two previous colors based on y-axis position
    CP_Color lerp = CP_Color_Lerp(lerpx1, lerpx2, my);
    
    // Set the background based on the lerp
    CP_Graphics_ClearBackground(lerp);
}

void JUSTIN_DEMO_UPDATE_CP_COLORHSV(void)
{
    // Get the current framecount and set the background to black
    int frameCount = CP_System_GetFrameCount();
    CP_Graphics_ClearBackground(CP_Color_CreateHex(0x000000FF));
    
    // Use framecount to slowly change the HSV color through all colors of the rainbow
    CP_ColorHSL hsl = CP_ColorHSL_Create(360 - (frameCount * 3), 100, 50, 255);
    CP_Settings_Fill(CP_Color_FromColorHSL(hsl));
    CP_Graphics_DrawRectAdvanced(10, 10, 380, 185, 0, 20);
    
    // Do some similar color math on RBG (less convenient)
    CP_Color rgb = CP_Color_Create((int)((sinf(frameCount * 0.05f) + 1.0f) * 128.0f) % 256,
        (int)((sinf(frameCount * 0.05f + 2.0f * (float)M_PI / 3.0f) + 1.0f) * 128.0f) % 256,
        (int)((sinf(frameCount * 0.05f + 4.0f * (float)M_PI / 3.0f) + 1.0f) * 128.0f) % 256, 255);
    CP_Settings_Fill(rgb);
    CP_Graphics_DrawRectAdvanced(10, 205, 190, 185, 0, 20);

	int hex = 0;
	hex |= rgb.r << 24;
	hex |= rgb.g << 16;
	hex |= rgb.b << 8;
	hex |= rgb.a;

	CP_Settings_Fill(CP_Color_CreateHex(hex));
	CP_Graphics_DrawRectAdvanced(200, 205, 190, 185, 0, 20);
}

void JUSTIN_DEMO_UPDATE_CP_COLOR(void)
{
    // Use the CP_Color_Create function to create a variable
    CP_Color color1 = CP_Color_Create(255, 40, 100, 255);
    
    // Set the background with color1 (berry red)
    CP_Graphics_ClearBackground(color1);
    
    // Create a color and pass it directly as a function parameter
    CP_Settings_Fill(CP_Color_Create(0, 200, 255, 255));
    
    // Draw a rectangle at the top left of the screen (blue)
    float rectWidth = CP_System_GetWindowWidth() * 0.5f;
    float rectHeight = CP_System_GetWindowHeight() * 0.5f;
    CP_Graphics_DrawRect(0, 0, rectWidth, rectHeight);
    
    // Create a third CP_Color and set the values by accessing the array
    CP_Color color3;
    color3.rgba[0] = 150;	// Red
    color3.rgba[1] = 255;	// Green
    color3.rgba[2] = 40;	// Blue
    color3.rgba[3] = 255;	// Alpha (transparency)
    
    // Set the fill color for the final shape
    CP_Settings_Fill(color3);
    
    // Draw another smaller rectange (green)
    rectWidth = CP_System_GetWindowWidth() * 0.4f;
    rectHeight = CP_System_GetWindowHeight() * 0.4f;
    CP_Graphics_DrawRect(0, 0, rectWidth, rectHeight);
}


// main() the starting point for the program
// Run() is used to tell the program which init and update functions to use.
int main(void)
{
	//CP_Engine_SetPreUpdateFunction(PreUpdateSettingsToggle);

	//CP_System_SetWindowSize(1600, 900);
	//test_random();

	// setKeyboardInputCallback(callback_key);
	// setMouseInputCallback(callback_mouse);
	// setMouseWheelCallback(callback_wheel);


	// Currently allows any void(*)(void) function
	//  Setup (called once)           Draw (looped)
	//CP_Engine_SetNextGameState(random_background_rectangles, mouse_following_polygon, NULL);

	//CP_Engine_SetNextGameState(NULL, selector);
	//CP_Engine_SetNextGameState(racecar_setup, racecar_draw, NULL);
	//CP_Engine_SetNextGameState(bounce_setup, bounce_draw, NULL);

	//CP_Engine_SetNextGameState(NULL, text_demo, NULL);
	//CP_Engine_SetNextGameState(NULL, time_test, NULL);
	//CP_Engine_SetNextGameState(NULL, audio_test, NULL);
	//CP_Engine_SetNextGameState(mouse_setup, mouse_follower, NULL);

	//CP_Engine_SetNextGameState(image_test_setup, image_test, NULL);

	//CP_Engine_SetNextGameState(gaussian_background_rectangles, NULL, NULL);
	//CP_Engine_SetNextGameState(NULL, noise_example, NULL);
	//CP_Engine_SetNextGameState(NULL, transform_test, NULL);
	//CP_Engine_SetNextGameState(NULL, image_stress, NULL);
	//CP_Engine_SetNextGameState(NULL, test_sound_rev, NULL);
	//CP_Engine_SetNextGameState(DancingLinesInit, DancingLinesUpdate, NULL);
	//CP_Engine_SetNextGameState(demo14init, demo14, NULL);
	//CP_Engine_SetNextGameState(NULL, rotated_shapes_update, NULL);
	//CP_Engine_SetNextGameState(0, line_func, NULL);
	//CP_Engine_SetNextGameState(EngineInit, EngineUpdate, NULL);
	//CP_Engine_SetNextGameState(SceneTestInit, SceneTestUpdate, NULL);

	//CP_Engine_SetNextGameState(lerp_test_init, lerp_test_update, NULL);
	//CP_Engine_SetNextGameState(dt_init, dt_example, NULL);
	//CP_Engine_SetNextGameState(save_restore_test_init, save_restore_test_update, NULL);
	//CP_Engine_SetNextGameState(matrix_test_init2, matrix_test_update2, NULL);
	//CP_Engine_SetNextGameState(i_init, i_update, NULL);
	//CP_Engine_SetNextGameState(transpose_init, transpose_test, NULL);
	//CP_Engine_SetNextGameState(vector_init, vector_update, NULL);
	//CP_Engine_SetNextGameState(vector_init, vector_update, NULL);
	//CP_Engine_SetNextGameState(deg_rad_init, deg_rad_update, NULL);
	//CP_Engine_SetNextGameState(init, update, NULL);
	//CP_Engine_SetNextGameState(square_init, square_update, NULL);
	//CP_Engine_SetNextGameState(align_test_init, align_text_update, NULL);
	//CP_Engine_SetNextGameState(matrix_test_init, matrix_test_update, NULL);
	//CP_Engine_SetNextGameState(DancingLinesInit, DancingLinesUpdate, NULL);
	//CP_Engine_SetNextGameState(GamepadDemoInit, GamepadDemoUpdate, NULL);

	//CP_Engine_SetNextGameState(initRedRects, updateRedRects, NULL);

	//CP_Engine_SetNextGameState(initCircleTests, updateCircleTests, NULL);
	//CP_Engine_SetNextGameState(initHSVTests, updateHSVTests, NULL);

	//CP_Engine_SetNextGameState(ForcedInit, ForcedUpdate, NULL);
//	CP_Engine_SetNextGameState(initsub, updatesub, NULL);
//	CP_Engine_SetNextGameState(initbuff, updatebuff, NULL);
	//CP_Engine_SetNextGameState(initS, updateS, NULL);
	//CP_Engine_SetNextGameState(initfr, updatefr, NULL);
	//CP_Engine_SetNextGameState(inittint, updatetint, NULL);

	CP_Engine_SetNextGameState(JUSTIN_DEMO_INIT, JUSTIN_DEMO_UPDATE_CP_COLORHSV, NULL);

	CP_Engine_Run();
	return 0;
}

/*/

void initlogo();
void updatelogo();
void exitlogo();
void initmenu();
void updatemenu();
void exitmenu();
void initgame();
void updategame();
void exitgame();

void drawStateName(char* str)
{
	textSize(50);
	fill(CP_Color_Create(0, 0, 0, 255));
	text(str, 20, 50);
	fill(CP_Color_Create(255, 255, 255, 255));
	text(str, 20, 100);
	fill(CP_Color_Create(255, 0, 0, 255));
	text(str, 20, 150);
	fill(CP_Color_Create(0, 255, 0, 255));
	text(str, 20, 200);
	fill(CP_Color_Create(0, 0, 255, 255));
	text(str, 20, 250);
}

// when used with Run() this function will be called once at the beginning of the program
void init0()
{
	fullscreenAdvanced(1920, 1080);
	setFrameRate(4);
	background(CP_Color_Create(0, 0, 0, 255));
	drawStateName(__FUNCDNAME__);
}

// when used with Run() this function will be called repeatedly every frame
void update0()
{
	SetNextGameState(initlogo, updatelogo, exitlogo);
	drawStateName(__FUNCDNAME__);
}

// TESTING
void initlogo()
{
	fullscreenAdvanced(1280, 720);
	background(CP_Color_Create(0, 0, 0, 255));
	drawStateName(__FUNCDNAME__);
}

void updatelogo()
{
	background(CP_Color_Create(150, 150, 150, 255));
	drawStateName(__FUNCDNAME__);

	if (CP_Input_KeyTriggered(KEY_SPACE))
	{
		SetNextGameState(initmenu, updatemenu, exitmenu);
	}
}

void exitlogo()
{
	background(CP_Color_Create(255, 0, 0, 255));
	drawStateName(__FUNCDNAME__);
}

void initmenu()
{
	fullscreen();
	background(CP_Color_Create(0, 0, 0, 255));
	CP_Graphics_DrawCircle(100, 100, 100);
	drawStateName(__FUNCDNAME__);
}

void updatemenu()
{
	background(CP_Color_Create(150, 150, 150, 255));
	drawStateName(__FUNCDNAME__);

	if (CP_Input_KeyTriggered(KEY_SPACE))
	{
		SetNextGameState(initgame, updategame, exitgame);
	}
}

void exitmenu()
{
	background(CP_Color_Create(255, 0, 0, 255));
	drawStateName(__FUNCDNAME__);
}

void initgame()
{
	fullscreenAdvanced(1280, 720);
	background(CP_Color_Create(0, 0, 0, 255));
	drawStateName(__FUNCDNAME__);
}

void updategame()
{
	background(CP_Color_Create(150, 150, 150, 255));
	drawStateName(__FUNCDNAME__);

	if (CP_Input_KeyTriggered(KEY_SPACE))
	{
		SetNextGameState(initmenu, updatemenu, exitmenu);
	}
}

void exitgame()
{
	background(CP_Color_Create(255, 0, 0, 255));
	drawStateName(__FUNCDNAME__);
}

// main() the starting point for the program
// Run() is used to tell the program which init and update functions to use.
int main(void)
{
	setFrameRate(60);
	fullscreenAdvanced(recommendedMaxWidth, recommendedMaxHeight);
	SetNextGameState(DancingLinesInit, DancingLinesUpdate, NULL);

	//fullscreen();
	//SetNextGameState(init0, update0, NULL);
	
	Run();
	return 0;
}

//*/





/*

static int currBlendMode = CP_BLEND_ALPHA;
static bool IsCMY = false;
static bool IsAlpha = false;

void DrawBackground()
{
	// draw background with normal alpha blending, half dark, half light
	blendMode(CP_BLEND_ALPHA);
	background(CP_Color_Create(50, 50, 50, 255));
	fill(CP_Color_Create(200, 200, 200, 255));
	CP_Graphics_DrawRect(0, 0, canvasWidth * 0.5f, (float)canvasHeight);
}

void DrawCircles()
{
	// setup position, size and color values
	float centerX = canvasWidth * 0.5f;
	float centerY = canvasHeight * 0.5f;
	float circleRadius = 400.0f;
	float offset = 120.0f;
	int bonusColor = 0;
	if (IsCMY == true)
	{
		bonusColor = 255;
	}
	int A = 255;
	if (IsAlpha == true)
	{
		A = 128;
	}

	// set current blend mode for circle drawing
	blendMode(currBlendMode);

	// red circle
	fill(CP_Color_Create(255, bonusColor, 0, A));
	CP_Graphics_DrawCircle(centerX - offset, centerY - offset, circleRadius);
	// green circle
	fill(CP_Color_Create(0, 255, bonusColor, A));
	CP_Graphics_DrawCircle(centerX + offset, centerY - offset, circleRadius);
	// blue circle
	fill(CP_Color_Create(bonusColor, 0, 255, A));
	CP_Graphics_DrawCircle(centerX, centerY + offset, circleRadius);
}

void PrintBlendMode()
{
	char* names[] = {
		"CP_BLEND_ALPHA",
		"CP_BLEND_ADD",
		"CP_BLEND_SUBTRACT",
		"CP_BLEND_MULTIPLY",
		"CP_BLEND_MIN",
		"CP_BLEND_MAX"
	};

	blendMode(CP_BLEND_ALPHA);
	fill(CP_Color_Create(150, 0, 150, 255));
	text(names[currBlendMode], 20, 50);
}

void HandleInput()
{
	// Press "C" to toggle RGB to CMY circle colors
	if (CP_Input_KeyReleased(KEY_C))
	{
		IsCMY = !IsCMY;
	}
	// Press "A" to toggle alpha value to draw semi-transparent circles
	if (CP_Input_KeyReleased(KEY_A))
	{
		IsAlpha = !IsAlpha;
	}
	// Press "]" to switch blend mode
	if (CP_Input_KeyReleased(KEY_RIGHT_BRACKET))
	{
		++currBlendMode;
	}
	// Press "[" to switch blend mode
	else if (CP_Input_KeyReleased(KEY_LEFT_BRACKET))
	{
		--currBlendMode;
	}

	// keep blend mode within valid range
	currBlendMode = CP_Math_ClampInt(currBlendMode, 0, CP_BLEND_MAX);
}

void blendModeExampleInit()
{
	noStroke();
	textSize(40);
}

void blendModeExampleUpdate()
{
	// draw background, half dark, half light
	DrawBackground();

	// draw red, green and blue circles
	DrawCircles();

	// print blend modes to screen
	PrintBlendMode();

	// handle input for changing blend modes
	HandleInput();
	debugInputChecks();
}





int noiseExampleFrameCount = 0;

void noiseExampleInit()
{
	// test stuff
	CP_Settings_RectMode(CP_POSITION_CORNER);
	noiseExampleFrameCount = 0;
}

void noiseExampleUpdate()
{
	++noiseExampleFrameCount;
	background(CP_Color_Create(200, 200, 200, 255));
	fill(CP_Color_Create(100, 100, 100, 255));
	CP_Graphics_DrawRectAdvanced(10, 10, 380, 380, 0, (float)noiseExampleFrameCount);

	debugInputChecks();
}



void ___init()
{
	save();
}

void ___restoreSave()
{
	restore();
	save();
}

void ___update()
{
	fullscreenAdvanced(1280, 720);
	SetNextGameState(blendModeExampleInit, blendModeExampleUpdate, ___restoreSave);
}



static int gameStateIndex = 1;
static const int gsIndexMax = 2;

void debugInputChecks()
{
	bool stateIndexChange = false;
	if (CP_Input_KeyTriggered(KEY_F3))
	{
		stateIndexChange = true;
		--gameStateIndex;
	}
	if (CP_Input_KeyTriggered(KEY_F4))
	{
		stateIndexChange = true;
		++gameStateIndex;
	}

	if (stateIndexChange)
	{
		if (gameStateIndex < 0)
		{
			gameStateIndex = gsIndexMax;
		}
		if (gameStateIndex > gsIndexMax)
		{
			gameStateIndex = 0;
		}

		switch (gameStateIndex)
		{
		case 0:
			fullscreenAdvanced(recommendedMaxWidth, recommendedMaxHeight);
			SetNextGameState(DancingLinesInit, DancingLinesUpdate, ___restoreSave);
			break;
		case 1:
			fullscreenAdvanced(1280, 720);
			SetNextGameState(blendModeExampleInit, noiseExampleUpdate, ___restoreSave);
			break;
		case 2:
			fullscreenAdvanced(1280, 720);
			SetNextGameState(blendModeExampleInit, blendModeExampleUpdate, ___restoreSave);
			break;
		default:
			break;
		}
	}
}




// main() the starting point for the program
// Run() is used to tell the program which init and update functions to use.
int main(void)
{
	fullscreen();

	SetNextGameState(___init, ___update, NULL);

	Run();
	return 0;
}


*/