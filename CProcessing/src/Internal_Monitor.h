//------------------------------------------------------------------------------
// file:	Internal_Monitor.h
// brief:	Which monitor a window is on, and centering a window on one.
//
//			Pure functions on rectangles in screen coordinates (no GLFW), so
//			Tier 1 can test them with any monitor layout; CP_System.c feeds
//			them the real monitors.
//
// Internal header: not part of the public API.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology
//------------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CP_ScreenRect
{
	int x;
	int y;
	int width;
	int height;
} CP_ScreenRect;

// The part of two rectangles that overlaps, with a width and height of 0 if
// they don't overlap
static inline CP_ScreenRect CP_Monitor_Overlap(CP_ScreenRect a, CP_ScreenRect b)
{
	const int left = a.x > b.x ? a.x : b.x;
	const int top = a.y > b.y ? a.y : b.y;
	const int right = a.x + a.width < b.x + b.width ? a.x + a.width : b.x + b.width;
	const int bottom = a.y + a.height < b.y + b.height ? a.y + a.height : b.y + b.height;
	CP_ScreenRect overlap = { left, top, 0, 0 };
	if (right > left && bottom > top)
	{
		overlap.width = right - left;
		overlap.height = bottom - top;
	}
	return overlap;
}

// The index of the monitor with the largest part of the window on it, or -1
// if none of the window is on any monitor. A tie goes to the earlier monitor.
static inline int CP_Monitor_MostOfWindow(CP_ScreenRect window, const CP_ScreenRect* monitors, int count)
{
	int best = -1;
	long long bestArea = 0;
	for (int i = 0; i < count; ++i)
	{
		const CP_ScreenRect overlap = CP_Monitor_Overlap(window, monitors[i]);
		const long long area = (long long)overlap.width * (long long)overlap.height;
		if (area > bestArea)
		{
			bestArea = area;
			best = i;
		}
	}
	return best;
}

// The size of a window's title bar and borders on each side of its content
typedef struct CP_ScreenFrame
{
	int left;
	int top;
	int right;
	int bottom;
} CP_ScreenFrame;

// The position of a window's content that centers the whole window, title bar
// and borders included, in an area of the screen. A window too big for the
// area goes at the area's top or left edge instead, so its title bar is on
// the screen where it can be grabbed.
static inline void CP_Monitor_Center(CP_ScreenRect area, int width, int height, CP_ScreenFrame frame, int* x, int* y)
{
	const int outerWidth = frame.left + width + frame.right;
	const int outerHeight = frame.top + height + frame.bottom;
	int left = area.x + area.width / 2 - outerWidth / 2;
	int top = area.y + area.height / 2 - outerHeight / 2;
	left = left > area.x ? left : area.x;
	top = top > area.y ? top : area.y;
	*x = left + frame.left;
	*y = top + frame.top;
}

#ifdef __cplusplus
}
#endif
