//------------------------------------------------------------------------------
// file:	CP_Graphics.c
// author:	Daniel Hamilton
// brief:	Manipulate and draw basic vector shapes 
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include Files:
//------------------------------------------------------------------------------

#include "cprocessing.h"
#include "Internal_System.h"

//------------------------------------------------------------------------------
// Defines and Internal Variables:
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static CP_BOOL firstVertex = FALSE;

//------------------------------------------------------------------------------
// Internal Functions:
//------------------------------------------------------------------------------

static void CP_Graphics_DrawRectInternal(float x, float y, float w, float h, float degrees, float cornerRadius)
{
	CP_CorePtr CORE = GetCPCore();
	CP_DrawInfoPtr DI = GetDrawInfo();

	if (!CORE->nvg)
		return;	// no window yet: CP_Engine_Run hasn't started

	// make sure corner radius isn't negative
	cornerRadius = cornerRadius < 0 ? 0 : cornerRadius;

	float pointOfRotationX = x;
	float pointOfRotationY = y;

	switch (DI->rect_mode)
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

	// rotation
	if (degrees != 0)
	{
		nvgSave(CORE->nvg);
		nvgTranslate(CORE->nvg, pointOfRotationX, pointOfRotationY);
		nvgRotate(CORE->nvg, CP_Math_Radians(degrees));
		nvgTranslate(CORE->nvg, -pointOfRotationX, -pointOfRotationY);
	}

	// Rectangle path
	nvgBeginPath(CORE->nvg);
	if (cornerRadius < 0.1f)
	{
		nvgRect(CORE->nvg, x, y, w, h);
	}
	else
	{
		nvgRoundedRect(CORE->nvg, x, y, w, h, cornerRadius);
	}

	if (DI->fill)
	{
		nvgFill(CORE->nvg);
	}
	if (DI->stroke)
	{
		nvgStroke(CORE->nvg);
	}

	if (degrees != 0)
	{
		nvgRestore(CORE->nvg);
	}
}

//------------------------------------------------------------------------------
// Library Functions:
//------------------------------------------------------------------------------

CP_API void CP_Graphics_ClearBackground(CP_Color c)
{
	CP_CorePtr CORE = GetCPCore();

	if (!CORE->nvg)
		return;	// no window yet: CP_Engine_Run hasn't started

	// Set the background color
	nvgCancelFrame(CORE->nvg);	// also wipe any prior render calls this frame
	const float a = c.a / 255.0f;
	if (CP_Canvas_IsPremultiplied())
	{
		// a state pushed on top of another draws on a layer that is blended
		// over the ones below, with premultiplied alpha, as NanoVG draws
		glClearColor(c.r / 255.0f * a, c.g / 255.0f * a, c.b / 255.0f * a, a);
	}
	else
	{
		glClearColor(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, a);
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

CP_API void CP_Graphics_DrawPoint(float x, float y)
{
	CP_CorePtr CORE = GetCPCore();
	CP_DrawInfoPtr DI = GetDrawInfo();

	if (!CORE->nvg)
		return;	// no window yet: CP_Engine_Run hasn't started

	// A point follows the line settings, like CP_Graphics_DrawLine: the
	// stroke color, the stroke weight as its size, and the line cap as its
	// shape (round caps make a circle, butt and square caps a square).
	if (DI->stroke)
	{
		const CP_Color stroke = DI->stroke_color;
		const CP_Color fill = DI->fill_color;
		const float size = DI->stroke_weight;

		nvgBeginPath(CORE->nvg);
		if (DI->line_cap == CP_LINE_CAP_ROUND)
		{
			nvgCircle(CORE->nvg, x + 0.5f, y + 0.5f, size * 0.5f);
		}
		else
		{
			nvgRect(CORE->nvg, x + 0.5f - size * 0.5f, y + 0.5f - size * 0.5f, size, size);
		}

		// Fill the point with the stroke color, then put the fill color back
		nvgFillColor(CORE->nvg, nvgRGBA(stroke.r, stroke.g, stroke.b, stroke.a));
		nvgFill(CORE->nvg);
		nvgFillColor(CORE->nvg, nvgRGBA(fill.r, fill.g, fill.b, fill.a));
	}
}

CP_API void CP_Graphics_DrawLine(float x1, float y1, float x2, float y2)
{
	CP_CorePtr CORE = GetCPCore();
	CP_DrawInfoPtr DI = GetDrawInfo();

	if (!CORE->nvg)
		return;	// no window yet: CP_Engine_Run hasn't started

	// Line stroke
	if (DI->stroke)
	{
		nvgBeginPath(CORE->nvg);
		nvgMoveTo(CORE->nvg, x1, y1);
		nvgLineTo(CORE->nvg, x2, y2);
		nvgStroke(CORE->nvg);
	}
}

CP_API void CP_Graphics_DrawLineAdvanced(float x1, float y1, float x2, float y2, float degrees)
{
	CP_CorePtr CORE = GetCPCore();
	CP_DrawInfoPtr DI = GetDrawInfo();

	if (!CORE->nvg)
		return;	// no window yet: CP_Engine_Run hasn't started

	// rotation
	nvgSave(CORE->nvg);
	nvgTranslate(CORE->nvg, ((x1 + x2) / 2.0f), ((y1 + y2) / 2.0f));
	nvgRotate(CORE->nvg, CP_Math_Radians(degrees));
	nvgTranslate(CORE->nvg, -((x1 + x2) / 2.0f), -((y1 + y2) / 2.0f));

	// Line stroke
	if (DI->stroke)
	{
		nvgBeginPath(CORE->nvg);
		nvgMoveTo(CORE->nvg, x1, y1);
		nvgLineTo(CORE->nvg, x2, y2);
		nvgStroke(CORE->nvg);
	}

	nvgRestore(CORE->nvg);
}

CP_API void CP_Graphics_DrawRect(float x, float y, float w, float h)
{
	CP_Graphics_DrawRectInternal(x, y, w, h, 0, 0);
}

CP_API void CP_Graphics_DrawRectAdvanced(float x, float y, float w, float h, float degrees, float cornerRadius)
{
	CP_Graphics_DrawRectInternal(x, y, w, h, degrees, cornerRadius);
}

CP_API void CP_Graphics_DrawCircle(float x, float y, float diameter)
{
	CP_Graphics_DrawEllipse(x, y, diameter, diameter);
}

CP_API void CP_Graphics_DrawEllipse(float x, float y, float w, float h)
{
	CP_CorePtr CORE = GetCPCore();
	CP_DrawInfoPtr DI = GetDrawInfo();

	if (!CORE->nvg)
		return;	// no window yet: CP_Engine_Run hasn't started

	// nanoVG wants a radius
	float rw = w * 0.5f;
	float rh = h * 0.5f;

	switch (DI->ellipse_mode)
	{
	case CP_POSITION_CORNER:
		x += rw;
		y += rh;
		break;
	case CP_POSITION_CENTER:
		// this is the default of NanoVG
	default:
		break;
	}

	// Ellipse path
	nvgBeginPath(CORE->nvg);
	nvgEllipse(CORE->nvg, x, y, rw, rh);

	if (DI->fill)
	{
		nvgFill(CORE->nvg);
	}
	if (DI->stroke)
	{
		nvgStroke(CORE->nvg);
	}
}

CP_API void CP_Graphics_DrawEllipseAdvanced(float x, float y, float w, float h, float degrees)
{
	CP_CorePtr CORE = GetCPCore();

	if (!CORE->nvg)
		return;	// no window yet: CP_Engine_Run hasn't started

	float pointOfRotationX = x;
	float pointOfRotationY = y;

	// rotation
	nvgSave(CORE->nvg);
	nvgTranslate(CORE->nvg, pointOfRotationX, pointOfRotationY);
	nvgRotate(CORE->nvg, CP_Math_Radians(degrees));
	nvgTranslate(CORE->nvg, -pointOfRotationX, -pointOfRotationY);

	CP_Graphics_DrawEllipse(x, y, w, h);

	nvgRestore(CORE->nvg);
}

// The circle that an arc or a pie is part of: centered on (x, y) with
// diameter, or with its top left at (x, y) in CP_POSITION_CORNER ellipse mode, as for
// CP_Graphics_DrawCircle. Angles are in degrees, 0 is to the right and a
// positive sweep goes clockwise (y points down), like CP_Vector_FromAngle.
static void ArcPath(CP_CorePtr CORE, CP_DrawInfoPtr DI, float x, float y, float diameter, float startDegrees, float sweepDegrees, CP_BOOL pie)
{
	const float radius = diameter * 0.5f;
	if (DI->ellipse_mode == CP_POSITION_CORNER)
	{
		x += radius;
		y += radius;
	}

	nvgBeginPath(CORE->nvg);
	if (pie)
	{
		nvgMoveTo(CORE->nvg, x, y);
	}
	nvgArc(CORE->nvg, x, y, radius, CP_Math_Radians(startDegrees), CP_Math_Radians(startDegrees + sweepDegrees),
		sweepDegrees >= 0.0f ? NVG_CW : NVG_CCW);
	if (pie)
	{
		nvgClosePath(CORE->nvg);
	}
}

CP_API void CP_Graphics_DrawArc(float x, float y, float diameter, float startDegrees, float sweepDegrees)
{
	CP_CorePtr CORE = GetCPCore();
	CP_DrawInfoPtr DI = GetDrawInfo();

	if (!CORE->nvg)
		return;	// no window yet: CP_Engine_Run hasn't started

	// Just the curved line, like DrawLine: it has no fill
	if (DI->stroke)
	{
		ArcPath(CORE, DI, x, y, diameter, startDegrees, sweepDegrees, FALSE);
		nvgStroke(CORE->nvg);
	}
}

CP_API void CP_Graphics_DrawPie(float x, float y, float diameter, float startDegrees, float sweepDegrees)
{
	CP_CorePtr CORE = GetCPCore();
	CP_DrawInfoPtr DI = GetDrawInfo();

	if (!CORE->nvg)
		return;	// no window yet: CP_Engine_Run hasn't started

	ArcPath(CORE, DI, x, y, diameter, startDegrees, sweepDegrees, TRUE);
	if (DI->fill)
	{
		nvgFill(CORE->nvg);
	}
	if (DI->stroke)
	{
		nvgStroke(CORE->nvg);
	}
}

CP_API void CP_Graphics_DrawBezier(float x1, float y1, float controlX1, float controlY1, float controlX2, float controlY2, float x2, float y2)
{
	CP_CorePtr CORE = GetCPCore();
	CP_DrawInfoPtr DI = GetDrawInfo();

	if (!CORE->nvg)
		return;	// no window yet: CP_Engine_Run hasn't started

	// A curve is a line: stroke only, no fill
	if (DI->stroke)
	{
		nvgBeginPath(CORE->nvg);
		nvgMoveTo(CORE->nvg, x1, y1);
		nvgBezierTo(CORE->nvg, controlX1, controlY1, controlX2, controlY2, x2, y2);
		nvgStroke(CORE->nvg);
	}
}

CP_API void CP_Graphics_DrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3)
{
	CP_CorePtr CORE = GetCPCore();
	CP_DrawInfoPtr DI = GetDrawInfo();

	if (!CORE->nvg)
		return;	// no window yet: CP_Engine_Run hasn't started

	// Triangle path
	nvgBeginPath(CORE->nvg);
	nvgMoveTo(CORE->nvg, x1, y1);
	nvgLineTo(CORE->nvg, x2, y2);
	nvgLineTo(CORE->nvg, x3, y3);
	nvgClosePath(CORE->nvg);

	if (DI->fill)
	{
		nvgFill(CORE->nvg);
	}
	if (DI->stroke)
	{
		nvgStroke(CORE->nvg);
	}
}

CP_API void CP_Graphics_DrawTriangleAdvanced(float x1, float y1, float x2, float y2, float x3, float y3, float degrees)
{
	CP_CorePtr CORE = GetCPCore();

	if (!CORE->nvg)
		return;	// no window yet: CP_Engine_Run hasn't started

	// rotation
	nvgSave(CORE->nvg);
	nvgTranslate(CORE->nvg, ((x1 + x2 + x3) / 3.0f), ((y1 + y2 + y3) / 3.0f));
	nvgRotate(CORE->nvg, CP_Math_Radians(degrees));
	nvgTranslate(CORE->nvg, -((x1 + x2 + x3) / 3.0f), -((y1 + y2 + y3) / 3.0f));

	CP_Graphics_DrawTriangle(x1, y1, x2, y2, x3, y3);

	nvgRestore(CORE->nvg);
}

//////////////////////////////////////////////////////////////////////////////////////
//  QUAD                                                                            //
//                                                                                  //
//  A quad is a quadrilateral, a four sided polygon. It is similar to a rectangle,  //
//  but the angles between its edges are not constrained to ninety degrees. The     //
//  first pair of parameters (x1,y1) sets the first vertex and the subsequent pairs //
//  should proceed clockwise or counter-clockwise around the defined shape.         //
//////////////////////////////////////////////////////////////////////////////////////
CP_API void CP_Graphics_DrawQuad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
{
	CP_CorePtr CORE = GetCPCore();
	CP_DrawInfoPtr DI = GetDrawInfo();

	if (!CORE->nvg)
		return;	// no window yet: CP_Engine_Run hasn't started

	// Quad path
	nvgBeginPath(CORE->nvg);
	nvgMoveTo(CORE->nvg, x1, y1);
	nvgLineTo(CORE->nvg, x2, y2);
	nvgLineTo(CORE->nvg, x3, y3);
	nvgLineTo(CORE->nvg, x4, y4);
	nvgClosePath(CORE->nvg);

	if (DI->fill)
	{
		nvgFill(CORE->nvg);
	}
	if (DI->stroke)
	{
		nvgStroke(CORE->nvg);
	}
}

CP_API void CP_Graphics_DrawQuadAdvanced(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float degrees)
{
	CP_CorePtr CORE = GetCPCore();

	if (!CORE->nvg)
		return;	// no window yet: CP_Engine_Run hasn't started

	// rotation
	nvgSave(CORE->nvg);
	nvgTranslate(CORE->nvg, ((x1 + x2 + x3 + x4) / 4.0f), ((y1 + y2 + y3 + y4) / 4.0f));
	nvgRotate(CORE->nvg, CP_Math_Radians(degrees));
	nvgTranslate(CORE->nvg, -((x1 + x2 + x3 + x4) / 4.0f), -((y1 + y2 + y3 + y4) / 4.0f));

	CP_Graphics_DrawQuad(x1, y1, x2, y2, x3, y3, x4, y4);

	nvgRestore(CORE->nvg);
}

CP_API void CP_Graphics_BeginShape(void)
{
	CP_CorePtr CORE = GetCPCore();

	if (!CORE->nvg)
		return;	// no window yet: CP_Engine_Run hasn't started
	nvgBeginPath(CORE->nvg);
	firstVertex = TRUE;
}

CP_API void CP_Graphics_AddVertex(float x, float y)
{
	CP_CorePtr CORE = GetCPCore();

	if (!CORE->nvg)
		return;	// no window yet: CP_Engine_Run hasn't started

	if (firstVertex)
	{
		nvgMoveTo(CORE->nvg, x, y);

		// Update tracking
		firstVertex = FALSE;
	}
	else
	{
		nvgLineTo(CORE->nvg, x, y);
	}
}

CP_API void CP_Graphics_EndShape(void)
{
	CP_CorePtr CORE = GetCPCore();
	CP_DrawInfoPtr DI = GetDrawInfo();

	if (!CORE->nvg)
		return;	// no window yet: CP_Engine_Run hasn't started

	// Shape fill
	if (DI->fill)
	{
		// if the user wants a filled shape then make sure to close it so the stroke is correct
		nvgClosePath(CORE->nvg);
		nvgFill(CORE->nvg);
	}

	// Shape stroke
	if (DI->stroke)
	{
		nvgStroke(CORE->nvg);
	}
}
