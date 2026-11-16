//------------------------------------------------------------------------------
// file:	CP_Text.c
// author:	Daniel Hamilton, Justin Chambers
// brief:	Text functions for loading fonts and drawing text
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include Files:
//------------------------------------------------------------------------------

#include <limits.h>
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
	for (unsigned i = 0; i < font_vector->size; ++i)
	{
		CP_Font font = vect_at_CP_Font(font_vector, i);
		if (font && !strcmp(filepath, font->filepath))
		{
			return font;
		}
	}

	return NULL;
}

static void CP_Font_AddHandle(CP_Font font)
{
	// a new font, used once so far
	font->ref_count = 1;
	vect_push_CP_Font(font_vector, font);
}

static CP_Font CP_Font_LoadInternal(const char* filepath, bool fromMemory, unsigned char* data, int ndata, int freeData)
{
	CP_Font new_font = NULL;
	CP_CorePtr CORE = GetCPCore();

	if (!filepath || !font_vector)
	{
		return NULL;
	}

	// Check if the font is already loaded: share it, and count the load so
	// one CP_Font_Free doesn't free it for every other load of the file
	new_font = CP_Font_IsLoaded(filepath);
	if (new_font)
	{
		if (new_font->ref_count < INT_MAX)
		{
			++new_font->ref_count;
		}
		return new_font;
	}

	// Allocate the struct
	new_font = (CP_Font)malloc(sizeof(CP_Font_Struct));
	if (!new_font)
	{
		return NULL;
	}

	new_font->load_error = FALSE;
	new_font->handle = -1;
	CP_StringCopy(new_font->filepath, CP_PATH_MAX, filepath);

	if (!CORE || !CORE->nvg)
	{
		new_font->load_error = TRUE;
		free(new_font);
		return NULL;
	}

	if (fromMemory)
	{
		new_font->handle = nvgCreateFontMem(CORE->nvg, filepath, data, ndata, freeData);
	}
	else
	{
		// from the current folder, or else next to the program
		char resolved[CP_PATH_MAX];
		new_font->handle = nvgCreateFont(CORE->nvg, filepath, file_resolveAssetPath(filepath, resolved, sizeof(resolved)));
	}

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
	// freeData = 0: NanoVG only reads the embedded font data, so casting away const is safe
	_default_font = CP_Font_LoadInternal("./Assets/Exo2-Regular.ttf", true, (unsigned char*)Exo2_Regular_ttf, Exo2_Regular_ttf_size, 0);
}

void CP_Text_ReportLoaded(CP_Report* report)
{
	if (!font_vector) return;

	for (unsigned i = 0; i < font_vector->size; ++i)
	{
		CP_Font font = vect_at_CP_Font(font_vector, i);
		// the built-in font isn't the program's to free
		if (font && font != _default_font)
		{
			CP_Report_AddAsset(report, "font", font->filepath, font->ref_count);
		}
	}
}

void CP_Text_Shutdown(void)
{
	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg)
	{
		return;
	}

	for (unsigned i = 0; i < font_vector->size; ++i)
	{
		CP_Font font = vect_at_CP_Font(font_vector, i);
		if (font)
		{
			nvgFreeFont(CORE->nvg, font->filepath);
			free(font);
		}
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

CP_API void CP_Font_Free(CP_Font* font)
{
	if (font == NULL || *font == NULL)
	{
		return;
	}

	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg)
	{
		return;
	}

	// find the font in the list
	for (unsigned i = 0; i < font_vector->size; ++i)
	{
		if (vect_at_CP_Font(font_vector, i) == *font)
		{
			if (i == 0)
			{
				// don't allow the unloading of the built-in default font Exo2-Regular.ttf
				return;
			}

			// another load of the same file still uses it
			if (--(*font)->ref_count > 0)
			{
				*font = NULL;
				return;
			}

			// settings (and saved settings) that use this font go back to the
			// default font, so none keeps a font that is gone
			CP_CorePtr core = GetCPCore();
			for (int s = 0; s < core->nstates; ++s)
			{
				if (core->states[s].font == *font)
				{
					core->states[s].font = NULL;
				}
			}
			CP_GameStates_ForgetFont(*font);

			// remove the font from the list and also ask NVG to remove and free
			vect_rem_CP_Font(font_vector, i);
			nvgFreeFont(CORE->nvg, (*font)->filepath);
			free(*font);
			*font = NULL;
			return;
		}
	}

	// TODO: handle error - we reached the end of the list without finding the image
}

CP_API void CP_Font_Set(CP_Font font)
{
	CP_CorePtr CORE = GetCPCore();

	if (font == NULL || !CORE || !CORE->nvg)
	{
		return;
	}

	nvgFontFaceId(CORE->nvg, font->handle);
	GetDrawInfo()->font = font;
}

CP_API CP_Font CP_Font_GetCurrent(void)
{
	CP_Font font = GetDrawInfo()->font;
	return font ? font : _default_font;
}

CP_API void CP_Font_DrawText(const char* text, float x, float y)
{
	CP_CorePtr CORE = GetCPCore();

	if (!CORE || !CORE->nvg)
	{
		return;
	}

	nvgText(CORE->nvg, x, y, text, NULL);
}

CP_API void CP_Font_DrawTextBox(const char* text, float x, float y, float rowWidth)
{
	CP_CorePtr CORE = GetCPCore();

	if (!CORE || !CORE->nvg)
	{
		return;
	}

	nvgTextBox(CORE->nvg, x, y, rowWidth, text, NULL);
}

CP_API float CP_Font_GetTextWidth(const char* text)
{
	CP_CorePtr CORE = GetCPCore();

	if (!CORE || !CORE->nvg || !text)
	{
		return 0.0f;
	}

	// The advance is where the next character would start, so it counts
	// spaces at the end of the text, which the glyphs' bounds don't
	return nvgTextBounds(CORE->nvg, 0.0f, 0.0f, text, NULL, NULL);
}

CP_API CP_Vector CP_Font_GetTextSize(const char* text)
{
	CP_CorePtr CORE = GetCPCore();

	if (!CORE || !CORE->nvg || !text)
	{
		return CP_Vector_Zero();
	}

	// Height is the font's line height at the current size, so it's the
	// same for every string and text lines up when drawn one under another
	float lineHeight = 0.0f;
	nvgTextMetrics(CORE->nvg, NULL, NULL, &lineHeight);
	return CP_Vector_Set(CP_Font_GetTextWidth(text), lineHeight);
}

CP_API CP_Vector CP_Font_GetTextBoxSize(const char* text, float rowWidth)
{
	CP_CorePtr CORE = GetCPCore();

	if (!CORE || !CORE->nvg || !text)
	{
		return CP_Vector_Zero();
	}

	float bounds[4];
	nvgTextBoxBounds(CORE->nvg, 0.0f, 0.0f, rowWidth, text, NULL, bounds);
	return CP_Vector_Set(bounds[2] - bounds[0], bounds[3] - bounds[1]);
}
