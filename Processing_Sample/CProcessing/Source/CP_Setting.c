//------------------------------------------------------------------------------
// file:	CP_Setting.c
// author:	Daniel Hamilton
// brief:	Basic settings for frame time and various rendering states 
//
// Copyright © 2019 DigiPen, All rights reserved.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include Files:
//------------------------------------------------------------------------------

#include "cprocessing.h"
#include "Internal_System.h"
#include <stdbool.h>

//------------------------------------------------------------------------------
// Defines:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Private Consts:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Private Structures:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Variables:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Private Variables:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Private Function Declarations:
//------------------------------------------------------------------------------

int isValidPositionMode(CP_POSITION_MODE mode)
{
	return (mode == CP_POSITION_CENTER || mode == CP_POSITION_CORNER) ? TRUE : FALSE;
}

//------------------------------------------------------------------------------
// Public Functions:
//------------------------------------------------------------------------------

CP_API void CP_Settings_Fill(CP_Color c)
{
	nvgFillColor(GetCPCore()->nvg, nvgRGBA(c.r, c.g, c.b, c.a));

	// Enable fill
	GetDrawInfo()->fill = TRUE;
}

CP_API void CP_Settings_NoFill(void)
{
	GetDrawInfo()->fill = FALSE;
}

CP_API void CP_Settings_Stroke(CP_Color c)
{
	nvgStrokeColor(GetCPCore()->nvg, nvgRGBA(c.r, c.g, c.b, c.a));

	// Enable stroke
	GetDrawInfo()->stroke = TRUE;
}

CP_API void CP_Settings_NoStroke(void)
{
	GetDrawInfo()->stroke = FALSE;
}

CP_API void CP_Settings_StrokeWeight(float weight)
{
	nvgStrokeWidth(GetCPCore()->nvg, weight);
}

CP_API void CP_Settings_Tint(CP_Color c)
{
	nvgTintColor(GetCPCore()->nvg, nvgRGBA(c.r, c.g, c.b, c.a));
}

CP_API void CP_Settings_NoTint(void)
{
	CP_Settings_Tint(CP_Color_Create(255, 255, 255, 255));
}

CP_API void CP_Settings_AntiAlias(CP_BOOL antiAlias)
{
	nvgShapeAntiAlias(GetCPCore()->nvg, antiAlias);
}

CP_API void CP_Settings_LineCapMode(CP_LINE_CAP_MODE capMode)
{
	if (capMode == CP_LINE_CAP_BUTT ||
		capMode == CP_LINE_CAP_ROUND ||
		capMode == CP_LINE_CAP_SQUARE)
	{
		nvgLineCap(GetCPCore()->nvg, capMode);
	}
}

CP_API void CP_Settings_LineJointMode(CP_LINE_JOINT_MODE jointMode)
{
	if (jointMode == CP_LINE_JOINT_ROUND ||
		jointMode == CP_LINE_JOINT_BEVEL ||
		jointMode == CP_LINE_JOINT_MITER)
	{
		nvgLineJoin(GetCPCore()->nvg, jointMode);
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
	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg)
		return; // break?

	switch (blendMode)
	{
	case CP_BLEND_MAX:
		nvgGlobalCompositeOperation(CORE->nvg, NVG_BLEND_MAX);
		break;
	case CP_BLEND_MIN:
		nvgGlobalCompositeOperation(CORE->nvg, NVG_BLEND_MIN);
		break;
	case CP_BLEND_MULTIPLY:
		nvgGlobalCompositeOperation(CORE->nvg, NVG_BLEND_MULTIPLY);
		break;
	case CP_BLEND_SUBTRACT:
		nvgGlobalCompositeOperation(CORE->nvg, NVG_BLEND_SUBTRACT);
		break;
	case CP_BLEND_ADD:
		nvgGlobalCompositeOperation(CORE->nvg, NVG_BLEND_ADD);
		break;
	case CP_BLEND_ALPHA:
	default:
		nvgGlobalCompositeOperation(CORE->nvg, NVG_BLEND_ALPHA);
		break;
	}
}

CP_API void CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_MODE filterMode)
{
	nvgTextureFilter(GetCPCore()->nvg, filterMode);
}

CP_API void CP_Settings_ImageWrapMode(CP_IMAGE_WRAP_MODE wrapMode)
{
	nvgTextureWrap(GetCPCore()->nvg, wrapMode);
}

CP_API void CP_Settings_TextSize(float size)
{
	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg) return;
	nvgFontSize(CORE->nvg, size);
}

CP_API void CP_Settings_TextAlignment(CP_TEXT_ALIGN_HORIZONTAL h, CP_TEXT_ALIGN_VERTICAL v)
{
	int flags = h | v;
	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg) return;
	nvgTextAlign(CORE->nvg, flags);
}

CP_API void CP_Settings_Scale(float xScale, float yScale)
{
	nvgScale(GetCPCore()->nvg, xScale, yScale);

	// World Mouse values are now invalid
	CP_Input_SetWorldMouseDirty();
}

CP_API void CP_Settings_Rotate(float degrees)
{
	nvgRotate(GetCPCore()->nvg, CP_Math_Radians(degrees));

	// World Mouse values are now invalid
	CP_Input_SetWorldMouseDirty();
}

CP_API void CP_Settings_Translate(float x, float y)
{
	nvgTranslate(GetCPCore()->nvg, x, y);

	// World Mouse values are now invalid
	CP_Input_SetWorldMouseDirty();
}

CP_API void CP_Settings_ApplyMatrix(CP_Matrix m)
{
	nvgTransform(GetCPCore()->nvg, m.m00, m.m10, m.m01, m.m11, m.m02, m.m12);

	// World Mouse values are now invalid
	CP_Input_SetWorldMouseDirty();
}

CP_API void CP_Settings_ResetMatrix(void)
{
	nvgResetTransform(GetCPCore()->nvg);

	// World Mouse values are now invalid
	CP_Input_SetWorldMouseDirty();
}

CP_API void CP_Settings_Save(void)
{
	CP_CorePtr CORE = GetCPCore();
	if (!CORE || !CORE->nvg)
		return; // break?

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
	if (!CORE || !CORE->nvg)
		return; // break?

	nvgRestore(CORE->nvg);

	// also restore the DrawInfo details
	if (CORE->nstates <= 1)
		return;
	CORE->nstates--;
}
