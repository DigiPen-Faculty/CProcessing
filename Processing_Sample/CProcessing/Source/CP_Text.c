//------------------------------------------------------------------------------
// File:	CP_Text.c
// Author:	Daniel Hamilton, Justin Chambers
// Brief:	Text functions for loading fonts and drawing text
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

#include "cprocessing.h"
#include "Internal_Text.h"
#include "Internal_System.h"
#include "vect.h"
#include "Internal_Resources.h"

//------------------------------------------------------------------------------
// Defines and Internal Variables:
//------------------------------------------------------------------------------

#define FONT_LOAD_ERROR -1
#define CP_INITIAL_FONT_COUNT 16

VECT_GENERATE_TYPE(CP_Font)

static CP_Font  _default_font = NULL;

static vect_CP_Font* font_vector;

//------------------------------------------------------------------------------
// Internal Functions:
//------------------------------------------------------------------------------

static CP_Font CP_Font_IsLoaded(const char* filepath)
{
	for (int i = 0; i < font_vector->size; ++i)
	{
		CP_Font font = vect_at_CP_Font(font_vector, i);
		if (font && !strcmp(filepath, font->filepath))
			return font;
	}

	return NULL;
}

static void CP_Font_AddHandle(CP_Font font)
{
	vect_push_CP_Font(font_vector, font);
}

static CP_Font CP_Font_LoadInternal(const char* filepath, bool fromMemory, unsigned char* data, int ndata, int freeData)
{
	CP_Font new_font = NULL;
	CP_CorePtr CORE = GetCPCore();

	// Check if the font is already loaded
	new_font = CP_Font_IsLoaded(filepath);
	if (new_font)
		return new_font;

	// Allocate the struct
	new_font = (CP_Font)malloc(sizeof(CP_Font_Struct));
	if (!new_font)
		return NULL;

	new_font->load_error = FALSE;
	new_font->handle = -1;
	strcpy_s(new_font->filepath, MAX_PATH, filepath);

	if (!CORE || !CORE->nvg)
	{
		new_font->load_error = TRUE;
		free(new_font);
		return NULL;
	}

	if (fromMemory)
		new_font->handle = nvgCreateFontMem(CORE->nvg, filepath, data, ndata, freeData);

	new_font->handle = nvgCreateFont(CORE->nvg, filepath, filepath);

	if (new_font->handle == FONT_LOAD_ERROR)
	{
		new_font->load_error = TRUE;
		free(new_font);
		return NULL;
	}

	new_font->load_error = FALSE;

	// Store a pointer to the font
	CP_Font_AddHandle(new_font);

	return new_font;
}

void CP_Text_Init(void)
{
	// initialize our vector
	font_vector = vect_init_CP_Font(CP_INITIAL_FONT_COUNT);

	// load the default font from internal binary resource data
	_default_font = CP_Font_LoadInternal("./Assets/Exo2-Regular.ttf", true, Exo2_Regular_ttf, Exo2_Regular_ttf_size, 0);
}

void CP_Text_Shutdown(void)
{
	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg)
		return;

	for (int i = 0; i < font_vector->size; ++i)
	{
		CP_Font font = vect_at_CP_Font(font_vector, i);
		if (font)
			free(font);
	}

	vect_free_CP_Font(font_vector);
}

//------------------------------------------------------------------------------
// Library Functions:
//------------------------------------------------------------------------------

CP_API CP_Font CP_Font_GetDefault(void)
{
	return _default_font;
}

CP_API CP_Font CP_Font_Load(const char* filepath)
{
	return CP_Font_LoadInternal(filepath, false, NULL, 0, 0);
}

CP_API void CP_Font_Set(CP_Font font)
{
	CP_CorePtr CORE = GetCPCore();

	if (font == NULL || !CORE || !CORE->nvg)
		return;

	nvgFontFaceId(CORE->nvg, font->handle);
}

CP_API void CP_Font_DrawText(const char* text, float x, float y)
{
	CP_CorePtr CORE = GetCPCore();

	if (!CORE || !CORE->nvg)
		return;

	nvgText(CORE->nvg, x, y, text, NULL);
}

CP_API void CP_Font_DrawTextBox(const char* text, float x, float y, float rowWidth)
{
	CP_CorePtr CORE = GetCPCore();

	if (!CORE || !CORE->nvg)
		return;

	nvgTextBox(CORE->nvg, x, y, rowWidth, text, NULL);
}
