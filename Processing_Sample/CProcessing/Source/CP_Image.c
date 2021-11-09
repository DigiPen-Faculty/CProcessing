//------------------------------------------------------------------------------
// file:	CP_Image.c
// author:	Daniel Hamilton
// brief:	API for loading and displaying images
//
// Copyright © 2019 DigiPen, All rights reserved.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include Files:
//------------------------------------------------------------------------------

#include <stdlib.h>
#include "cprocessing.h"
#include "Internal_Image.h"
#include "Internal_System.h"
#include "vect.h"

//------------------------------------------------------------------------------
// Defines and Internal Variables:
//------------------------------------------------------------------------------

#define CP_INITIAL_IMAGE_COUNT 100

VECT_GENERATE_TYPE(CP_Image)

static vect_CP_Image* active_images = NULL;
// This part is for later.
static vect_CP_Image* free_image_queue = NULL;

//------------------------------------------------------------------------------
// Internal Functions:
//------------------------------------------------------------------------------

static CP_Image CP_CheckIfImageIsLoaded(const char* filepath)
{
	for (unsigned i = 0; i < active_images->size; ++i)
	{
		CP_Image const image = vect_at_CP_Image(active_images, i);
		if (image && !strcmp(filepath, image->filepath))
		{
			return image;
		}
	}

	return NULL;
}

static void CP_AddImageHandle(CP_Image img)
{
	vect_push_CP_Image(active_images, img);
	return;
}

void CP_Image_Init(void)
{
	if (active_images == NULL)
	{
		active_images = vect_init_CP_Image(CP_INITIAL_IMAGE_COUNT);
	}
	if (free_image_queue == NULL)
	{
		free_image_queue = vect_init_CP_Image(CP_INITIAL_IMAGE_COUNT);
	}
}

void CP_ImageShutdown(void)
{
	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg) return;

	// Free all images.
	for (unsigned i = 0; i < active_images->size; ++i)
	{
		CP_Image const image = vect_at_CP_Image(active_images, i);
		if (image) // check if its null
		{
			nvgDeleteImage(CORE->nvg, image->handle); // free nanoVG's data
			free(image); // free the image struct
		}
	}
	// Also gotta clear the image queue just to make sure.
	CP_Image_ClearQueue_Free();

	vect_free_CP_Image(active_images);
	vect_free_CP_Image(free_image_queue);
	return;
}

static void CP_Image_DrawInternal(CP_Image img, float x, float y, float w, float h, float s0, float t0, float s1, float t1, int alpha, float degrees)
{
	if (!img)
	{
		return;
	}

	CP_CorePtr CORE = GetCPCore();
	CP_DrawInfoPtr DI = GetDrawInfo();

	if (!CORE || !CORE->nvg || !DI)
	{
		return;
	}

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

	const float a = CP_Math_ClampInt(alpha, 0, 255) / 255.0f;

	// translate and scale image pattern for subimages
	NVGpaint image = { 0 };
	if (s0 != s1 && t0 != t1)
	{
		const float posRatioX = (w / (s1 - s0));
		const float posRatioY = (h / (t1 - t0));
		const float scaleRatioX = (img->w / w) * posRatioX;
		const float scaleRatioY = (img->h / h) * posRatioY;

		image = nvgImagePattern(CORE->nvg, x - s0 * posRatioX, y - t0 * posRatioY, w * scaleRatioX, h * scaleRatioY, 0, img->handle, a);
	}
	else
	{
		image = nvgImagePattern(CORE->nvg, x, y, w, h, 0, img->handle, a);
	}

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
	// In case people load before running c processing (it already handles stuff being null)
	CP_Image_Init();

	if (active_images == NULL)
	{
		active_images = vect_init_CP_Image(CP_INITIAL_IMAGE_COUNT);
	}

	if (!filepath)
	{
		return NULL;
	}

	CP_Image img = NULL;
	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg)
	{
		return NULL;
	}

	// Check if the image is already loaded
	img = CP_CheckIfImageIsLoaded(filepath);
	if (img)
	{
		return img;
	}

	// allocate the struct
	img = (CP_Image)malloc(sizeof(CP_Image_Struct));
	if (!img)
	{
		return NULL;
	}

	strcpy_s(img->filepath, MAX_PATH, filepath);

	// load the image
	img->handle = nvgCreateImage(CORE->nvg, filepath, 0);

	if (img->handle == 0)
	{
		if (img)
		{
			free(img);
		}
		return NULL;
	}

	// populate width/height
	nvgImageSize(CORE->nvg, img->handle, &img->w, &img->h);
	img->load_error = FALSE;

	CP_AddImageHandle(img);

	return img;
}

CP_API void CP_Image_Free(CP_Image* img)
{
	if (img == NULL || *img == NULL)
	{
		return;
	}

	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg) return;

	for (unsigned i = 0; i < active_images->size; ++i)
	{
		CP_Image* image = vect_ptr_CP_Image(active_images, i);
		if (*image && *image == *img)
		{
			vect_push_CP_Image(free_image_queue, *image);
			*image = NULL;
		}
	}
}

CP_API int CP_Image_GetWidth(CP_Image img)
{
	if (!img)
	{
		return 0;
	}
	return img->w;
}

CP_API int CP_Image_GetHeight(CP_Image img)
{
	if (!img)
	{
		return 0;
	}
	return img->h;
}

CP_API void CP_Image_Draw(CP_Image img, float x, float y, float w, float h, int alpha)
{
	CP_Image_DrawInternal(img, x, y, w, h, 0, 0, 0, 0, alpha, 0);
}

CP_API void CP_Image_DrawAdvanced(CP_Image img, float x, float y, float w, float h, int alpha, float degrees)
{
	CP_Image_DrawInternal(img, x, y, w, h, 0, 0, 0, 0, alpha, degrees);
}

CP_API void CP_Image_DrawSubImage(CP_Image img, float x, float y, float w, float h, float u0, float v0, float u1, float v1, int alpha)
{
	CP_Image_DrawInternal(img, x, y, w, h, u0, v0, u1, v1, alpha, 0);
}

CP_API CP_Image CP_Image_CreateFromData(int w, int h, unsigned char* pixelDataInput)
{
	if (!pixelDataInput)
	{
		return NULL;
	}

	CP_Image img = NULL;
	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg)
	{
		return NULL;
	}

	// allocate the struct
	img = (CP_Image)malloc(sizeof(CP_Image_Struct));
	if (!img)
	{
		// error handling
		return NULL;
	}

	char buffer[MAX_PATH] = { 0 };
	strcpy_s(img->filepath, MAX_PATH, buffer);

	// load the image
	img->handle = nvgCreateImageRGBA(CORE->nvg, w, h, 0, pixelDataInput);

	if (img->handle == 0)
	{
		if (img) free(img);
		return NULL;
	}

	// populate width/height
	img->w = w;
	img->h = h;

	img->load_error = FALSE;

	CP_AddImageHandle(img);

	return img;
}

CP_API CP_Image CP_Image_Screenshot(int x, int y, int w, int h)
{
	unsigned char* buffer = (unsigned char*)malloc(4 * w * h);
	unsigned char* rowTemp = (unsigned char*)malloc(4 * w);
	if (!buffer || !rowTemp)
	{
		return NULL;
	}

	CP_CorePtr CORE = GetCPCore();

	// glReadPixles uses x,y as the lower left, so lets convert that
	// to the top right for the sake of consistency
	y = (CORE->window_height - h) - y;

	// flush nanovg so image can be captured
	nvgEndFrame(CORE->nvg);

	glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	nvgBeginFrame(CORE->nvg, CORE->window_width, CORE->window_height, CORE->pixel_ratio);

	int rowWidth = w * 4;
	int size = h;
	
	for (int rowIndex = 0; rowIndex < (size / 2); ++rowIndex) // loop through each row
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
    if (!img) return;

    CP_CorePtr CORE = GetCPCore();
    if (!CORE || !CORE->nvg)
    {
        return;
    }

    nvgGetImagePixelsRGBA(CORE->nvg, img->handle, (unsigned char*)pixelDataOutput);
}

CP_API void CP_Image_UpdatePixelData(CP_Image img, CP_Color* pixelDataInput)
{
	if (!img) return;

	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg)
	{
		return;
	}

	nvgUpdateImage(CORE->nvg, img->handle, (unsigned char*)pixelDataInput);
}

void CP_Image_ClearQueue_Free()
{
	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg) return;

	// We want to free every image
	while(free_image_queue->size)
	{
		// Grab the image at the end
		CP_Image* image = vect_ptr_CP_Image(free_image_queue, free_image_queue->size - 1);

		// Deletion of image
		nvgDeleteImage(CORE->nvg, (*image)->handle); // free nanoVG's data
		free(*image);

		vect_pop_CP_Image(free_image_queue);
	}
}
