//------------------------------------------------------------------------------
// File:	CP_Image.c
// Author:	Daniel Hamilton
// Brief:	API for loading and displaying images
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

//------------------------------------------------------------------------------
// Include Files:
//------------------------------------------------------------------------------

#include <stdlib.h>
#include "cprocessing.h"
#include "Internal_Image.h"
#include "Internal_System.h"

//------------------------------------------------------------------------------
// Defines and Internal Variables:
//------------------------------------------------------------------------------

#define CP_INITIAL_IMAGE_COUNT 255

static CP_Image* images = NULL;
static unsigned char	image_num = 0;
static unsigned char	image_max = CP_INITIAL_IMAGE_COUNT;

//------------------------------------------------------------------------------
// Internal Functions:
//------------------------------------------------------------------------------

static CP_Image CP_CheckIfImageIsLoaded(const char* filepath)
{
	for (unsigned char i = 0; i < image_num; ++i)
		if (images[i] && !strcmp(filepath, images[i]->filepath))
			return images[i];

	return NULL;
}

static void CP_AddImageHandle(CP_Image img)
{
	// allocate the array if it doesnt exist
	if (images == NULL)
		images = (CP_Image*)calloc(CP_INITIAL_IMAGE_COUNT, sizeof(CP_Image));

	// track the image handle for unloading
	images[image_num++] = img;

	if (image_num == image_max)
	{
		// store the current array
		CP_Image* temp = images;

		// allocate an array twice the size
		images = (CP_Image*)calloc(image_max * 2, sizeof(CP_Image));

		// copy over the old data
		memcpy_s(images, image_max * 2 * sizeof(CP_Image), temp, image_max * sizeof(CP_Image));

		// double the size
		image_max *= 2;

		// free the old array
		free(temp);
	}
}

void CP_ImageShutdown(void)
{
	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg) return;
	for (unsigned char i = 0; i < image_num; ++i)
		if (images[i]) // check if its null
		{
			nvgDeleteImage(CORE->nvg, images[i]->handle); // free nanoVG's data
			free(images[i]); // free the image struct
		}
}

static void CP_Image_DrawInternal(CP_Image img, float x, float y, float w, float h, float s0, float t0, float s1, float t1, unsigned char alpha, float degrees)
{
	if (!img)
		return;

	CP_CorePtr CORE = GetCPCore();
	CP_DrawInfoPtr DI = GetDrawInfo();

	if (!CORE || !CORE->nvg || !DI)
		return;

	switch (DI->image_mode)
	{
	case CP_POSITION_CENTER:
		x -= w * 0.5f;
		y -= h * 0.5f;
		break;
	case CP_POSITION_CORNER:

		// default for NanoVG
	default:
		break;
	}

	float a = CP_Math_ClampInt(alpha, 0, 255) / 255.0f;

	// translate and scale image pattern for subimages
	NVGpaint image = { 0 };
	if (s0 != s1 && t0 != t1)
	{
		float posRatioX = (w / (s1 - s0));
		float posRatioY = (h / (t1 - t0));
		float scaleRatioX = (img->w / w) * posRatioX;
		float scaleRatioY = (img->h / h) * posRatioY;

		image = nvgImagePattern(CORE->nvg, x - s0 * posRatioX, y - t0 * posRatioY, w * scaleRatioX, h * scaleRatioY, 0, img->handle, a);
	}
	else
		image = nvgImagePattern(CORE->nvg, x, y, w, h, 0, img->handle, a);

	// rotation
	nvgSave(CORE->nvg);
	nvgTranslate(CORE->nvg, x + (w / 2.0f), y + (h / 2.0f));
	nvgRotate(CORE->nvg, CP_Math_Radians(degrees));
	nvgTranslate(CORE->nvg, -(x + (w / 2.0f)), -(y + (h / 2.0f)));

	nvgBeginPath(CORE->nvg);
	nvgRect(CORE->nvg, x, y, w, h);
	nvgFillPaint(CORE->nvg, image);
	nvgFill(CORE->nvg);

	nvgRestore(CORE->nvg);
}

//------------------------------------------------------------------------------
// Library Functions:
//------------------------------------------------------------------------------

CP_API CP_Image CP_Image_Load(const char* filepath)
{
	if (!filepath)
		return NULL;

	CP_Image img = NULL;
	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg)
		return NULL;

	// Check if the image is already loaded
	img = CP_CheckIfImageIsLoaded(filepath);
	if (img)
		return img;

	// allocate the struct
	img = (CP_Image)malloc(sizeof(CP_Image_Struct));
	if (!img)
		return NULL;

	strcpy_s(img->filepath, MAX_PATH, filepath);

	// load the image
	img->handle = nvgCreateImage(CORE->nvg, filepath, 0);

	if (img->handle == 0)
	{
		if (img)
			free(img);
		return NULL;
	}

	// populate width/height
	nvgImageSize(CORE->nvg, img->handle, (int*)(&img->w), (int*)(&img->h));

	img->load_error = FALSE;

	CP_AddImageHandle(img);

	return img;
}

CP_API void CP_Image_Free(CP_Image* img)
{
	if (img == NULL || *img == NULL)
		return;

	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg) return;

	for (unsigned char i = 0; i < image_num; ++i)
		if (images[i] && images[i] == *img)
		{
			nvgDeleteImage(CORE->nvg, images[i]->handle); // free nanoVG's data
			free(images[i]);
			images[i] = NULL;
			*img = NULL;
			return;
		}
}

CP_API unsigned short CP_Image_GetWidth(CP_Image img)
{
	if (!img)
		return 0;
	return img->w;
}

CP_API unsigned short CP_Image_GetHeight(CP_Image img)
{
	if (!img)
		return 0;
	return img->h;
}

CP_API void CP_Image_Draw(CP_Image img, float x, float y, float w, float h, unsigned char alpha)
{
	CP_Image_DrawInternal(img, x, y, w, h, 0, 0, 0, 0, alpha, 0);
}

CP_API void CP_Image_DrawAdvanced(CP_Image img, float x, float y, float w, float h, unsigned char alpha, float degrees)
{
	CP_Image_DrawInternal(img, x, y, w, h, 0, 0, 0, 0, alpha, degrees);
}

CP_API void CP_Image_DrawSubImage(CP_Image img, float x, float y, float w, float h, float u0, float v0, float u1, float v1, unsigned char alpha)
{
	CP_Image_DrawInternal(img, x, y, w, h, u0, v0, u1, v1, alpha, 0);
}

CP_API CP_Image CP_Image_CreateFromData(unsigned short w, unsigned short h, unsigned char* pixelDataInput)
{
	if (!pixelDataInput)
		return NULL;

	CP_Image img = NULL;
	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg)
		return NULL;

	// allocate the struct
	img = (CP_Image)malloc(sizeof(CP_Image_Struct));
	if (!img)

		// error handling
		return NULL;

	char buffer[MAX_PATH] = { 0 };
	strcpy_s(img->filepath, MAX_PATH, buffer);

	// load the image
	img->handle = nvgCreateImageRGBA(CORE->nvg, w, h, 0, pixelDataInput);

	if (img->handle == 0)
	{
		if (img)
			free(img);
		return NULL;
	}

	// populate width/height
	img->w = w;
	img->h = h;

	img->load_error = FALSE;

	CP_AddImageHandle(img);

	return img;
}

CP_API CP_Image CP_Image_Screenshot(unsigned short x, unsigned short y, unsigned short w, unsigned short h)
{
	unsigned char* buffer = (unsigned char*)malloc(4 * w * h);
	unsigned char* rowTemp = (unsigned char*)malloc(4 * w);
	if (!buffer || !rowTemp)
		return NULL;

	glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	int rowWidth = w * 4;
	unsigned short size = h;
	for (unsigned short rowIndex = 0; rowIndex < size / 2; ++rowIndex) // loop through each row
	{
		unsigned char* rowFront = &buffer[rowIndex * rowWidth];
		unsigned char* rowBack = &buffer[(h - rowIndex - 1) * rowWidth];

		memcpy(rowTemp, rowFront, rowWidth);
		memcpy(rowFront, rowBack, rowWidth);
		memcpy(rowBack, rowTemp, rowWidth);
	}

	// createImage returns NULL if it fails, we want to pass that along as well
	CP_Image newImg = CP_Image_CreateFromData(w, h, buffer);

	free(buffer);
	free(rowTemp);

	return newImg;
}

CP_API void CP_Image_GetPixelData(CP_Image img, CP_Color* pixelDataOutput)
{
	if (!img)
		return;

	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg)
		return;

	nvgGetImagePixelsRGBA(CORE->nvg, img->handle, (unsigned char*)pixelDataOutput);
}

CP_API void CP_Image_UpdatePixelData(CP_Image img, CP_Color* pixelDataInput)
{
	if (!img)
		return;

	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg)
		return;

	nvgUpdateImage(CORE->nvg, img->handle, (unsigned char*)pixelDataInput);
}
