//------------------------------------------------------------------------------
// file:	Internal_Monitor.h
// brief:	Which monitor a window is on, and centering a window on one.
//
//			Pure functions on rectangles in screen coordinates (no GLFW), so
//			Tier 1 can test them with any monitor layout; CP_System.c feeds
//			them the real monitors.
//
// INTERNAL USE ONLY, DO NOT DISTRIBUTE
//
// Copyright (c) 2026 DigiPen, All rights reserved.
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

// The index of the monitor with the largest part of the window on it, or -1
// if none of the window is on any monitor. A tie goes to the earlier monitor.
static inline int CP_Monitor_MostOfWindow(CP_ScreenRect window, const CP_ScreenRect* monitors, int count)
{
	int best = -1;
	long long bestArea = 0;
	for (int i = 0; i < count; ++i)
	{
		const CP_ScreenRect m = monitors[i];
		const int left = window.x > m.x ? window.x : m.x;
		const int top = window.y > m.y ? window.y : m.y;
		const int right = window.x + window.width < m.x + m.width ? window.x + window.width : m.x + m.width;
		const int bottom = window.y + window.height < m.y + m.height ? window.y + window.height : m.y + m.height;
		if (right > left && bottom > top)
		{
			const long long area = (long long)(right - left) * (long long)(bottom - top);
			if (area > bestArea)
			{
				bestArea = area;
				best = i;
			}
		}
	}
	return best;
}

// The position that centers a window of the given size on a monitor
static inline void CP_Monitor_Center(CP_ScreenRect monitor, int width, int height, int* x, int* y)
{
	*x = monitor.x + monitor.width / 2 - width / 2;
	*y = monitor.y + monitor.height / 2 - height / 2;
}

#ifdef __cplusplus
}
#endif
