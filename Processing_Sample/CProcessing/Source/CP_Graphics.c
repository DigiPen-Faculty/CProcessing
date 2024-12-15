//------------------------------------------------------------------------------
// file:	CP_Graphics.c
// author:	Daniel Hamilton
// brief:	Manipulate and draw basic vector shapes 
//
// Copyright © 2019 DigiPen, All rights reserved.
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

	// TODO: Should this null check be added to all draw functions?
	if (!CORE || !CORE->nvg || !DI)
		return;

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
	// Set the background color
	nvgCancelFrame(GetCPCore()->nvg);	// also wipe any prior render calls this frame
	glClearColor(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

CP_API void CP_Graphics_DrawPoint(float x, float y)
{
	CP_CorePtr CORE = GetCPCore();
	CP_DrawInfoPtr DI = GetDrawInfo();

	// Point path and fill
	if (DI->fill)
	{
		nvgBeginPath(CORE->nvg);
		nvgPoint(CORE->nvg, x, y);
		nvgFillPoint(CORE->nvg);
	}
}

CP_API void CP_Graphics_DrawLine(float x1, float y1, float x2, float y2)
{
	CP_CorePtr CORE = GetCPCore();
	CP_DrawInfoPtr DI = GetDrawInfo();

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

CP_API void CP_Graphics_DrawCircle(float x, float y, float d)
{
	CP_Graphics_DrawEllipse(x, y, d, d);
}

CP_API void CP_Graphics_DrawEllipse(float x, float y, float w, float h)
{
	CP_CorePtr CORE = GetCPCore();
	CP_DrawInfoPtr DI = GetDrawInfo();

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

CP_API void CP_Graphics_DrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3)
{
	CP_CorePtr CORE = GetCPCore();
	CP_DrawInfoPtr DI = GetDrawInfo();

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
	nvgBeginPath(CORE->nvg);
	firstVertex = TRUE;
}

CP_API void CP_Graphics_AddVertex(float x, float y)
{
	CP_CorePtr CORE = GetCPCore();

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
