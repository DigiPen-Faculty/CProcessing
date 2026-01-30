//---------------------------------------------------------
// file:	NoiseDemo.c
// 
// CProcessing features in this demo:
// CP_Image_CreateFromData	- create an image at runtime from raw pixel data
// CP_Image_UpdatePixelData - modify raw pixel data and update the image for drawing
// CP_Random_Noise			- use perlin noise to compute a random color
//---------------------------------------------------------

#include "cprocessing.h"
#include "NoiseDemo.h"
#include <stdlib.h>

const int imageWidth = 128;
const int imageHeight = 72;

CP_Color* pixelData = NULL;
CP_Image noiseImage;

void NoiseDemoInit(void)
{
	CP_Settings_Save();
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	pixelData = malloc(imageWidth * imageHeight * sizeof(CP_Color));
	noiseImage = CP_Image_CreateFromData(imageWidth, imageHeight, (unsigned char*)pixelData);
}

void NoiseDemoUpdate(void)
{
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_NoStroke();
	for (int y = 0; y < imageHeight; ++y)
	{
		for (int x = 0; x < imageWidth; ++x)
		{
			// get a value between 0 and 360 for our color hue
			// (the smaller the x and the y, the smoother the noise will be, hence the multiplier)
			float zSpeed = CP_System_GetSeconds();
			int hue_val = (int)(CP_Random_Noise(x * 0.05f, y * 0.05f, zSpeed) * 500 + CP_System_GetFrameCount());

			// fill the color our noise generated
			pixelData[y * imageWidth + x] = CP_Color_FromColorHSL(CP_ColorHSL_Create(hue_val, 50, 50, 255));
		}
	}
	CP_Image_UpdatePixelData(noiseImage, pixelData);
	CP_Image_Draw(noiseImage, 0, 0, (float)CP_System_GetWindowWidth(), (float)CP_System_GetWindowHeight(), 255);
}

void NoiseDemoExit(void)
{
	CP_Image_Free(&noiseImage);
	free(pixelData);
	CP_Settings_Restore();
}