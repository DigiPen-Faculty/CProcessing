//------------------------------------------------------------------------------
// file:	CP_Setting.c
// author:	Daniel Hamilton
// brief:	Basic settings for frame time and various rendering states
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include Files:
//------------------------------------------------------------------------------

#include "cprocessing.h"
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
	DI->tint = CP_Color_Create(255, 255, 255, 255);	// no tint
	DI->anti_alias = TRUE;
	DI->blend_mode = CP_BLEND_ALPHA;
	DI->filter_mode = CP_IMAGE_FILTER_LINEAR;
	DI->wrap_mode = CP_IMAGE_WRAP_CLAMP;
	DI->text_size = 16.0f;	// NanoVG's default
	DI->text_align = (int)CP_TEXT_ALIGN_H_LEFT | (int)CP_TEXT_ALIGN_V_BASELINE;
}

void CP_Settings_ApplyAll(void)
{
	NVGcontext* vg = GetCPCore()->nvg;
	CP_DrawInfoPtr DI = GetDrawInfo();
	if (!vg)
		return;

	// CP's line, filter, wrap and alignment values are NanoVG's own
	nvgFillColor(vg, CP_ToNVGColor(DI->fill_color));
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
}

//------------------------------------------------------------------------------
// Public Functions:
//------------------------------------------------------------------------------

CP_API void CP_Settings_Fill(CP_Color c)
{
	CP_DrawInfoPtr DI = GetDrawInfo();
	NVGcontext* vg = GetCPCore()->nvg;
	DI->fill_color = c;
	DI->fill = TRUE;
	if (vg)
		nvgFillColor(vg, CP_ToNVGColor(c));
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

CP_API void CP_Settings_Scale(float xScale, float yScale)
{
	NVGcontext* vg = GetCPCore()->nvg;
	if (!vg)
		return;	// transforms are reset every frame; nothing to do yet

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

CP_API void CP_Settings_ApplyMatrix(CP_Matrix m)
{
	NVGcontext* vg = GetCPCore()->nvg;
	if (!vg)
		return;

	nvgTransform(vg, m.m00, m.m10, m.m01, m.m11, m.m02, m.m12);

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

CP_API void CP_Settings_Save(void)
{
	CP_CorePtr CORE = GetCPCore();
	if (!CORE->nvg)
		return;	// Save and Restore only matter while drawing

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
