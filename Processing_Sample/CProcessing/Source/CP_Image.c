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

#define CP_INITIAL_IMAGE_COUNT 12

VECT_GENERATE_TYPE(CP_Image)

static vect_CP_Image* image_vector = NULL;
static vect_CP_Image* free_image_queue = NULL; // free image in the same frame can result in the image not being drawn

//------------------------------------------------------------------------------
// Internal Functions:
//------------------------------------------------------------------------------

static CP_Image CP_CheckIfImageIsLoaded(const char* filepath)
{
	for (unsigned i = 0; i < image_vector->size; ++i)
	{
		CP_Image const image = vect_at_CP_Image(image_vector, i);
		if (image && !strcmp(filepath, image->filepath))
		{
			return image;
		}
	}

	return NULL;
}

static void CP_AddImageHandle(CP_Image img)
{
	// push a new image
	vect_push_CP_Image(image_vector, img);
}

void CP_Image_Clear_Vect(vect_CP_Image* vector)
{
	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg) return;

	// free every queued image
	while (vector->size)
	{
		// start at the end
		CP_Image const image = vect_at_CP_Image(vector, vector->size - 1);

		nvgDeleteImage(CORE->nvg, image->handle); // free nanoVG's data
		free(image);

		vect_pop_CP_Image(vector);
	}
}

void CP_Image_Init(void)
{
	if (image_vector == NULL)
	{
		image_vector = vect_init_CP_Image(CP_INITIAL_IMAGE_COUNT);
	}
	if (free_image_queue == NULL)
	{
		free_image_queue = vect_init_CP_Image(CP_INITIAL_IMAGE_COUNT);
	}
}

void CP_Image_Update(void)
{
	CP_Image_Clear_Vect(free_image_queue);
}

void CP_Image_Shutdown(void)
{
	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg) return;

	// free all images
	CP_Image_Clear_Vect(image_vector);
	CP_Image_Clear_Vect(free_image_queue);

	vect_free_CP_Image(image_vector);
	vect_free_CP_Image(free_image_queue);
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

	CP_StringCopy(img->filepath, CP_PATH_MAX, filepath);

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

	// find the image in the list
	for (unsigned i = 0; i < image_vector->size; ++i)
	{
		if (vect_at_CP_Image(image_vector, i) == *img)
		{
			// remove the image from the list and place on the free queue
			vect_rem_CP_Image(image_vector, i);
			vect_push_CP_Image(free_image_queue, *img);
			*img = NULL;
			return;
		}
	}

	// TODO: handle error - we reached the end of the list without finding the image
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

CP_API void CP_Image_DrawSubImageAdvanced(CP_Image img, float x, float y, float w, float h, float u0, float v0, float u1, float v1, int alpha, float degrees)
{
	CP_Image_DrawInternal(img, x, y, w, h, u0, v0, u1, v1, alpha, degrees);
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

	// images created from data have no file path
	img->filepath[0] = '\0';

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
	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg || w <= 0 || h <= 0)
	{
		return NULL;
	}

	// x, y, w, h are in window coordinates; the framebuffer can have more
	// pixels than that on hi-dpi displays (e.g. 2x on a Retina Mac), so read
	// the matching framebuffer region and scale it back to w x h.
	// (pixel_ratio is 1 on standard displays, where this is a plain copy)
	const float ratio = CORE->pixel_ratio > 0.0f ? CORE->pixel_ratio : 1.0f;
	const int readX = (int)(x * ratio + 0.5f);
	const int readW = (int)(w * ratio + 0.5f);
	const int readH = (int)(h * ratio + 0.5f);
	// glReadPixels uses x,y as the lower left, so convert from the top left
	const int readY = CORE->canvas_height - (int)((y + h) * ratio + 0.5f);

	unsigned char* readBuffer = (unsigned char*)malloc((size_t)4 * readW * readH);
	unsigned char* buffer = (unsigned char*)malloc((size_t)4 * w * h);
	if (!readBuffer || !buffer)
	{
		free(readBuffer);
		free(buffer);
		return NULL;
	}

	// flush nanovg so image can be captured
	nvgEndFrame(CORE->nvg);

	glReadPixels(readX, readY, readW, readH, GL_RGBA, GL_UNSIGNED_BYTE, readBuffer);

	nvgBeginFrame(CORE->nvg, (float)CORE->window_width, (float)CORE->window_height, CORE->pixel_ratio);

	// Flip rows (GL is bottom-up) and, for hi-dpi, scale down to w x h: each
	// pixel is the average of the framebuffer pixels it covers (a 2x2 block on
	// a Retina Mac), so the screenshot looks like the window, and a mirrored
	// drawing gives a mirrored screenshot. At ratio 1 this is a plain copy.
	for (int row = 0; row < h; ++row)
	{
		// framebuffer rows covered, counted from the top
		const int top = (int)((long long)row * readH / h);
		int bottom = (int)((long long)(row + 1) * readH / h);
		if (bottom <= top) bottom = top + 1;
		for (int col = 0; col < w; ++col)
		{
			const int left = (int)((long long)col * readW / w);
			int right = (int)((long long)(col + 1) * readW / w);
			if (right <= left) right = left + 1;

			unsigned sum[4] = { 0, 0, 0, 0 };
			for (int srcRow = top; srcRow < bottom; ++srcRow)
			{
				const unsigned char* src = &readBuffer[((size_t)(readH - 1 - srcRow) * readW + left) * 4];
				for (int srcCol = left; srcCol < right; ++srcCol, src += 4)
				{
					sum[0] += src[0];
					sum[1] += src[1];
					sum[2] += src[2];
					sum[3] += src[3];
				}
			}
			const unsigned count = (unsigned)((bottom - top) * (right - left));
			unsigned char* dst = &buffer[((size_t)row * w + col) * 4];
			for (int c = 0; c < 4; ++c)
			{
				dst[c] = (unsigned char)((sum[c] + count / 2) / count);
			}
		}
	}

	// createImage returns NULL if it fails, we want to pass that along as well
	CP_Image newImg = CP_Image_CreateFromData(w, h, buffer);

	free(readBuffer);
	free(buffer);

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

    // nvglImageHandleGL3 doesn't check the handle, so make sure NanoVG
    // knows the image first: nvgImageSize leaves the size alone if not.
    int w = 0, h = 0;
    nvgImageSize(CORE->nvg, img->handle, &w, &h);
    if (w <= 0 || h <= 0)
    {
        return;
    }

    // Read the pixels back from the image's OpenGL texture, leaving the
    // texture binding as NanoVG had it
    GLint previousTexture = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);
    glBindTexture(GL_TEXTURE_2D, nvglImageHandleGL3(CORE->nvg, img->handle));
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelDataOutput);
    glBindTexture(GL_TEXTURE_2D, (GLuint)previousTexture);
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
