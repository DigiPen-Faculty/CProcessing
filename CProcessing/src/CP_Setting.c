//------------------------------------------------------------------------------
// file:	CP_Setting.c
// author:	Daniel Hamilton
// brief:	Basic settings for frame time and various rendering states
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology and CProcessing contributors
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include Files:
//------------------------------------------------------------------------------

#include "cprocessing.h"
#include "Internal_Image.h"
#include "Internal_System.h"
#include <stdbool.h>

//------------------------------------------------------------------------------
// How settings are kept:
//
// Each setting is stored in the current draw state (CP_DrawInfo) and, once
// the window is open, given to NanoVG. Settings made before CP_Engine_Run,
// when there's no NanoVG context yet, are only stored; CP_Settings_ApplyAll
// gives them all to NanoVG when the window opens. Transforms are different:
// they're reset at the start of every frame, so before CP_Engine_Run they do
// nothing.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Private Functions:
//------------------------------------------------------------------------------

static CP_BOOL isValidPositionMode(CP_POSITION_MODE mode)
{
	return (mode == CP_POSITION_CENTER || mode == CP_POSITION_CORNER) ? TRUE : FALSE;
}

static NVGcolor CP_ToNVGColor(CP_Color c)
{
	return nvgRGBA(c.r, c.g, c.b, c.a);
}

static int CP_ToNVGBlend(CP_BLEND_MODE blendMode)
{
	switch (blendMode)
	{
	case CP_BLEND_MAX:
		return NVG_BLEND_MAX;
	case CP_BLEND_MIN:
		return NVG_BLEND_MIN;
	case CP_BLEND_MULTIPLY:
		return NVG_BLEND_MULTIPLY;
	case CP_BLEND_SUBTRACT:
		return NVG_BLEND_SUBTRACT;
	case CP_BLEND_ADD:
		return NVG_BLEND_ADD;
	case CP_BLEND_ALPHA:
	default:
		return NVG_BLEND_ALPHA;
	}
}

//------------------------------------------------------------------------------
// Internal Functions:
//------------------------------------------------------------------------------

void CP_Settings_InitDefaults(CP_DrawInfoPtr DI)
{
	DI->rect_mode = CP_POSITION_CENTER;
	DI->ellipse_mode = CP_POSITION_CENTER;
	DI->image_mode = CP_POSITION_CENTER;
	DI->fill = TRUE;
	DI->stroke = TRUE;
	DI->camera = CP_Matrix_Identity();
	DI->fill_color = CP_Color_Create(200, 200, 200, 255);
	DI->stroke_color = CP_Color_Create(0, 0, 0, 255);
	DI->stroke_weight = 3.0f;
	DI->line_cap = CP_LINE_CAP_BUTT;
	DI->line_joint = CP_LINE_JOINT_BEVEL;
	DI->tint = CP_Color_Create(255, 255, 255, 255); // no tint
	DI->anti_alias = TRUE;
	DI->blend_mode = CP_BLEND_ALPHA;
	DI->filter_mode = CP_IMAGE_FILTER_LINEAR;
	DI->wrap_mode = CP_IMAGE_WRAP_CLAMP;
	DI->text_size = 16.0f; // NanoVG's default
	DI->text_align = (int)CP_TEXT_ALIGN_H_LEFT | (int)CP_TEXT_ALIGN_V_BASELINE;
	DI->text_blur = 0.0f;
	DI->text_letter_spacing = 0.0f;
	DI->text_line_spacing = 0.0f;
	DI->font = NULL;
	DI->fill_kind = CP_FILL_COLOR;
	DI->fill_image = 0;
	DI->fill_image_alpha = 1.0f;
}

void CP_Settings_ApplyAll(void)
{
	NVGcontext* vg = GetCPCore()->nvg;
	CP_DrawInfoPtr DI = GetDrawInfo();
	if (!vg)
		return;

	// CP's line, filter, wrap and alignment values are NanoVG's own. The fill
	// isn't given here: whatever fills sets it just before (see FillPath).
	nvgStrokeColor(vg, CP_ToNVGColor(DI->stroke_color));
	nvgStrokeWidth(vg, DI->stroke_weight);
	nvgLineCap(vg, DI->line_cap);
	nvgLineJoin(vg, DI->line_joint);
	nvgTintColor(vg, CP_ToNVGColor(DI->tint));
	nvgShapeAntiAlias(vg, DI->anti_alias);
	nvgGlobalCompositeOperation(vg, CP_ToNVGBlend(DI->blend_mode));
	nvgTextureFilter(vg, DI->filter_mode);
	nvgTextureWrap(vg, DI->wrap_mode);
	nvgFontSize(vg, DI->text_size);
	nvgTextAlign(vg, DI->text_align);
	nvgFontBlur(vg, DI->text_blur);
	nvgTextLetterSpacing(vg, DI->text_letter_spacing);
}

//------------------------------------------------------------------------------
// Public Functions:
//------------------------------------------------------------------------------

// NanoVG's fill is set only here and in CP_Settings_ApplyTextFill, from the
// DrawInfo, right before something is filled; CP_Settings_Fill, the
// gradients and FillImage only store the setting. Whatever else fills with
// NanoVG (images, points, the game state layers) sets its own fill and leaves
// it, so there's nothing to put back after it.
void CP_Settings_FillPath(void)
{
	NVGcontext* vg = GetCPCore()->nvg;
	CP_DrawInfoPtr DI = GetDrawInfo();
	if (!vg)
		return;

	const float* p = DI->fill_points;
	const NVGcolor c1 = CP_ToNVGColor(DI->gradient_colors[0]);
	const NVGcolor c2 = CP_ToNVGColor(DI->gradient_colors[1]);
	switch (DI->fill_kind)
	{
	case CP_FILL_LINEAR_GRADIENT:
		// nvgFillPaint places the gradient with the current transform, so its
		// numbers mean the same place the shape's numbers do
		nvgFillPaint(vg, nvgLinearGradient(vg, p[0], p[1], p[2], p[3], c1, c2));
		break;
	case CP_FILL_RADIAL_GRADIENT:
		nvgFillPaint(vg, nvgRadialGradient(vg, p[0], p[1], p[2] * 0.5f, p[3] * 0.5f, c1, c2));
		break;
	case CP_FILL_IMAGE:
	{
		// A freed image (NanoVG no longer knows its size) or a tile of no
		// size fills nothing. (For a freed image NanoVG would draw nothing
		// too, but only by giving up halfway through setting up the fill.)
		int w = 0, h = 0;
		nvgImageSize(vg, DI->fill_image, &w, &h);
		if (w <= 0 || h <= 0 || p[2] == 0.0f || p[3] == 0.0f)
			return;
		// One tile at (x, y), w by h, repeated past its edges whatever the
		// wrap mode (which is for CP_Image_Draw and DrawSubImage), then the
		// wrap mode put back
		nvgFillPaint(vg, nvgImagePattern(vg, p[0], p[1], p[2], p[3], 0.0f, DI->fill_image, DI->fill_image_alpha));
		nvgTextureWrap(vg, NVG_TEXTURE_WRAP_REPEAT);
		nvgFill(vg);
		nvgTextureWrap(vg, DI->wrap_mode);
		return;
	}
	case CP_FILL_COLOR:
	default:
		nvgFillColor(vg, CP_ToNVGColor(DI->fill_color));
		break;
	}
	nvgFill(vg);
}

// Text can't take a gradient: it's drawn in the plain fill color, which is a
// gradient's first color
void CP_Settings_ApplyTextFill(void)
{
	NVGcontext* vg = GetCPCore()->nvg;
	if (vg)
		nvgFillColor(vg, CP_ToNVGColor(GetDrawInfo()->fill_color));
}

CP_API void CP_Settings_Fill(CP_Color c)
{
	CP_DrawInfoPtr DI = GetDrawInfo();
	DI->fill_color = c;
	DI->fill = TRUE;
	DI->fill_kind = CP_FILL_COLOR;
}

// A gradient or an image is a fill: Fill, NoFill and the other fills replace
// it, and Save and Restore keep it. The numbers are placed when each shape is
// drawn, in the coordinates of that moment, like the shape's own numbers.
static void CP_Settings_FillPoints(CP_FILL_KIND kind, float a, float b, float c, float d)
{
	CP_DrawInfoPtr DI = GetDrawInfo();
	DI->fill = TRUE;
	DI->fill_kind = kind;
	DI->fill_points[0] = a;
	DI->fill_points[1] = b;
	DI->fill_points[2] = c;
	DI->fill_points[3] = d;
}

static void CP_Settings_FillGradient(CP_FILL_KIND kind, float a, float b, float c, float d, CP_Color color1, CP_Color color2)
{
	CP_DrawInfoPtr DI = GetDrawInfo();
	CP_Settings_FillPoints(kind, a, b, c, d);
	DI->gradient_colors[0] = color1;
	DI->gradient_colors[1] = color2;
	DI->fill_color = color1; // what text and GetFill use: text can't take a gradient
}

CP_API void CP_Settings_FillLinearGradient(float x1, float y1, CP_Color color1, float x2, float y2, CP_Color color2)
{
	CP_Settings_FillGradient(CP_FILL_LINEAR_GRADIENT, x1, y1, x2, y2, color1, color2);
}

CP_API void CP_Settings_FillRadialGradient(float x, float y, float innerDiameter, CP_Color innerColor, float outerDiameter, CP_Color outerColor)
{
	CP_Settings_FillGradient(CP_FILL_RADIAL_GRADIENT, x, y, innerDiameter, outerDiameter, innerColor, outerColor);
}

// Text can't take an image either, so the fill color stays what it was. A
// NULL image fills nothing, like CP_Image_Draw draws nothing.
CP_API void CP_Settings_FillImage(CP_Image img, float x, float y, float w, float h, int alpha)
{
	CP_DrawInfoPtr DI = GetDrawInfo();
	CP_Settings_FillPoints(CP_FILL_IMAGE, x, y, w, h);
	DI->fill_image = img ? img->handle : 0;
	DI->fill_image_alpha = CP_Math_ClampInt(alpha, 0, 255) / 255.0f;
}

CP_API void CP_Settings_NoFill(void)
{
	GetDrawInfo()->fill = FALSE;
}

CP_API void CP_Settings_Stroke(CP_Color c)
{
	CP_DrawInfoPtr DI = GetDrawInfo();
	NVGcontext* vg = GetCPCore()->nvg;
	DI->stroke_color = c;
	DI->stroke = TRUE;
	if (vg)
		nvgStrokeColor(vg, CP_ToNVGColor(c));
}

CP_API void CP_Settings_NoStroke(void)
{
	GetDrawInfo()->stroke = FALSE;
}

CP_API void CP_Settings_StrokeWeight(float weight)
{
	NVGcontext* vg = GetCPCore()->nvg;
	GetDrawInfo()->stroke_weight = weight;
	if (vg)
		nvgStrokeWidth(vg, weight);
}

CP_API void CP_Settings_Tint(CP_Color c)
{
	NVGcontext* vg = GetCPCore()->nvg;
	GetDrawInfo()->tint = c;
	if (vg)
		nvgTintColor(vg, CP_ToNVGColor(c));
}

CP_API void CP_Settings_NoTint(void)
{
	CP_Settings_Tint(CP_Color_Create(255, 255, 255, 255));
}

CP_API void CP_Settings_AntiAlias(CP_BOOL antiAlias)
{
	NVGcontext* vg = GetCPCore()->nvg;
	GetDrawInfo()->anti_alias = antiAlias ? TRUE : FALSE;
	if (vg)
		nvgShapeAntiAlias(vg, antiAlias ? 1 : 0);
}

CP_API void CP_Settings_LineCapMode(CP_LINE_CAP_MODE capMode)
{
	if (capMode == CP_LINE_CAP_BUTT ||
		capMode == CP_LINE_CAP_ROUND ||
		capMode == CP_LINE_CAP_SQUARE)
	{
		NVGcontext* vg = GetCPCore()->nvg;
		GetDrawInfo()->line_cap = capMode;
		if (vg)
			nvgLineCap(vg, capMode);
	}
}

CP_API void CP_Settings_LineJointMode(CP_LINE_JOINT_MODE jointMode)
{
	if (jointMode == CP_LINE_JOINT_ROUND ||
		jointMode == CP_LINE_JOINT_BEVEL ||
		jointMode == CP_LINE_JOINT_MITER)
	{
		NVGcontext* vg = GetCPCore()->nvg;
		GetDrawInfo()->line_joint = jointMode;
		if (vg)
			nvgLineJoin(vg, jointMode);
	}
}

CP_API void CP_Settings_RectMode(CP_POSITION_MODE mode)
{
	if (isValidPositionMode(mode))
		GetDrawInfo()->rect_mode = mode;
}

CP_API void CP_Settings_EllipseMode(CP_POSITION_MODE mode)
{
	if (isValidPositionMode(mode))
		GetDrawInfo()->ellipse_mode = mode;
}

CP_API void CP_Settings_ImageMode(CP_POSITION_MODE mode)
{
	if (isValidPositionMode(mode))
		GetDrawInfo()->image_mode = mode;
}

CP_API void CP_Settings_BlendMode(CP_BLEND_MODE blendMode)
{
	NVGcontext* vg = GetCPCore()->nvg;
	const int nvgBlend = CP_ToNVGBlend(blendMode);

	// An unknown mode means the usual alpha blending, as it always has
	GetDrawInfo()->blend_mode = nvgBlend == NVG_BLEND_ALPHA ? CP_BLEND_ALPHA : blendMode;
	if (vg)
		nvgGlobalCompositeOperation(vg, nvgBlend);
}

CP_API void CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_MODE filterMode)
{
	if (filterMode == CP_IMAGE_FILTER_NEAREST ||
		filterMode == CP_IMAGE_FILTER_LINEAR)
	{
		NVGcontext* vg = GetCPCore()->nvg;
		GetDrawInfo()->filter_mode = filterMode;
		if (vg)
			nvgTextureFilter(vg, filterMode);
	}
}

CP_API void CP_Settings_ImageWrapMode(CP_IMAGE_WRAP_MODE wrapMode)
{
	if (wrapMode == CP_IMAGE_WRAP_CLAMP ||
		wrapMode == CP_IMAGE_WRAP_CLAMP_EDGE ||
		wrapMode == CP_IMAGE_WRAP_REPEAT ||
		wrapMode == CP_IMAGE_WRAP_MIRROR)
	{
		NVGcontext* vg = GetCPCore()->nvg;
		GetDrawInfo()->wrap_mode = wrapMode;
		if (vg)
			nvgTextureWrap(vg, wrapMode);
	}
}

CP_API void CP_Settings_TextSize(float size)
{
	NVGcontext* vg = GetCPCore()->nvg;
	GetDrawInfo()->text_size = size;
	if (vg)
		nvgFontSize(vg, size);
}

CP_API void CP_Settings_TextAlignment(CP_TEXT_ALIGN_HORIZONTAL h, CP_TEXT_ALIGN_VERTICAL v)
{
	NVGcontext* vg = GetCPCore()->nvg;
	const int flags = (int)h | (int)v;
	GetDrawInfo()->text_align = flags;
	if (vg)
		nvgTextAlign(vg, flags);
}

// Fontstash blurs glyphs in whole steps, rounded (a CProcessing patch), and
// stops at 20. It can't draw a negative blur, so that's sharp.
CP_API void CP_Settings_TextBlur(float blur)
{
	NVGcontext* vg = GetCPCore()->nvg;
	if (!(blur > 0.0f))
		blur = 0.0f;
	GetDrawInfo()->text_blur = blur;
	if (vg)
		nvgFontBlur(vg, blur);
}

// Only between letters, not after the last one, so aligned text and its
// measured width stay right. Negative brings letters closer.
CP_API void CP_Settings_TextLetterSpacing(float spacing)
{
	NVGcontext* vg = GetCPCore()->nvg;
	GetDrawInfo()->text_letter_spacing = spacing;
	if (vg)
		nvgTextLetterSpacing(vg, spacing);
}

CP_API void CP_Settings_TextLineSpacing(float spacing)
{
	GetDrawInfo()->text_line_spacing = spacing;
}

CP_API void CP_Settings_Scale(float xScale, float yScale)
{
	NVGcontext* vg = GetCPCore()->nvg;
	if (!vg)
		return; // transforms are reset every frame; nothing to do yet

	nvgScale(vg, xScale, yScale);

	// World Mouse values are now invalid
	CP_Input_SetWorldMouseDirty();
}

CP_API void CP_Settings_Rotate(float degrees)
{
	NVGcontext* vg = GetCPCore()->nvg;
	if (!vg)
		return;

	nvgRotate(vg, CP_Math_Radians(degrees));

	// World Mouse values are now invalid
	CP_Input_SetWorldMouseDirty();
}

CP_API void CP_Settings_Translate(float x, float y)
{
	NVGcontext* vg = GetCPCore()->nvg;
	if (!vg)
		return;

	nvgTranslate(vg, x, y);

	// World Mouse values are now invalid
	CP_Input_SetWorldMouseDirty();
}

CP_API void CP_Settings_ApplyMatrix(CP_Matrix matrix)
{
	NVGcontext* vg = GetCPCore()->nvg;
	if (!vg)
		return;

	nvgTransform(vg, matrix.m00, matrix.m10, matrix.m01, matrix.m11, matrix.m02, matrix.m12);

	// World Mouse values are now invalid
	CP_Input_SetWorldMouseDirty();
}

CP_API void CP_Settings_ResetMatrix(void)
{
	NVGcontext* vg = GetCPCore()->nvg;
	if (!vg)
		return;

	nvgResetTransform(vg);

	// World Mouse values are now invalid
	CP_Input_SetWorldMouseDirty();
}

// Drawing is cut off outside this rectangle, the one CP_Graphics_DrawRect
// would draw with the same numbers. It's placed in the coordinates as they
// are now, so drawing moved by a later Translate moves under it, and a clip
// set after a Rotate is turned. A new clip replaces the old one. It's
// NanoVG's scissor, which Save and Restore keep, and which starts over every
// frame, in each game state, and in each image drawn into, like the transform.
CP_API void CP_Settings_Clip(float x, float y, float w, float h)
{
	NVGcontext* vg = GetCPCore()->nvg;
	if (!vg)
		return; // no frame yet, and the clip would end with the first one

	if (GetDrawInfo()->rect_mode == CP_POSITION_CENTER)
	{
		x -= w * 0.5f;
		y -= h * 0.5f;
	}
	// A negative size goes the other way from x or y, as a rectangle does;
	// NanoVG would take it as 0 and clip everything
	if (w < 0.0f)
	{
		x += w;
		w = -w;
	}
	if (h < 0.0f)
	{
		y += h;
		h = -h;
	}
	nvgScissor(vg, x, y, w, h);
}

CP_API void CP_Settings_NoClip(void)
{
	NVGcontext* vg = GetCPCore()->nvg;
	if (vg)
		nvgResetScissor(vg);
}

// The current settings, as the drawing functions use them. Save and Restore
// bring back what these return, like the settings themselves.

// The last fill color given, which is still what it is after NoFill
CP_API CP_Color CP_Settings_GetFill(void)
{
	return GetDrawInfo()->fill_color;
}

// The last stroke color given, which is still what it is after NoStroke
CP_API CP_Color CP_Settings_GetStroke(void)
{
	return GetDrawInfo()->stroke_color;
}

CP_API float CP_Settings_GetStrokeWeight(void)
{
	return GetDrawInfo()->stroke_weight;
}

CP_API float CP_Settings_GetTextSize(void)
{
	return GetDrawInfo()->text_size;
}

CP_API void CP_Settings_Save(void)
{
	CP_CorePtr CORE = GetCPCore();
	if (!CORE->nvg)
		return; // Save and Restore only matter while drawing

	nvgSave(CORE->nvg);

	// also save the DrawInfo details
	if (CORE->nstates >= CP_MAX_STATES)
		return;
	if (CORE->nstates > 0)
		memcpy(&CORE->states[CORE->nstates], &CORE->states[CORE->nstates - 1], sizeof(CP_DrawInfo));
	CORE->nstates++;
}

CP_API void CP_Settings_Restore(void)
{
	CP_CorePtr CORE = GetCPCore();
	if (!CORE->nvg)
		return;

	nvgRestore(CORE->nvg);

	// also restore the DrawInfo details
	if (CORE->nstates <= 1)
		return;
	CORE->nstates--;
}
