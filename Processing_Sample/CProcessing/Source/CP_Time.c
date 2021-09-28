//------------------------------------------------------------------------------
// File:	CP_Time.c
// Author:	Justin Chambers
// Brief:	Timing file to manage frames and fps
//
// Copyright (C) 2021 DigiPen Institute of Technology, All rights reserved.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include Files:
//------------------------------------------------------------------------------

#include "stdafx.h" // This must be first
#include "cprocessing.h"
#include "Internal_System.h"
#include <stdbool.h>
#include "tinycthread.h"

//------------------------------------------------------------------------------
// Private Variables:
//------------------------------------------------------------------------------

// FrameRate Control
static double StartingTime, EndingTime, ElapsedSeconds;
static double _frametimeTarget = 0.033333; // 30 FPS
static double _frametime = _frametimeTarget;

// Frames since the start of the program
static unsigned long _frameCount;

//------------------------------------------------------------------------------
// Public Functions:
//------------------------------------------------------------------------------

void CP_IncFrameCount()
{
	++_frameCount;
}

void CP_FrameRate_Init()
{
	// start the app at frame zero
	_frameCount = 0;
}

void CP_FrameRate_FrameStart()
{
	StartingTime = glfwGetTime();

	// Update frame count
	CP_IncFrameCount();
}

double averageSleepCycles = 0.002;
double additionalBuffer = 0.001;	// Sleep() isn't very accurate and we want to make sure we don't over-sleep

void CP_FrameRate_FrameEnd()
{
	double prevSeconds = 0;
	double currSeconds = 0;

	do
	{
		CP_UpdateFrameTime();

		// compute remaining microseconds in the frame
		currSeconds = _frametimeTarget - ElapsedSeconds;

		// if our remaining microseconds this frame are greater than the Sleep function's
		// average margin of error (plus an additional buffer value) then go ahead and Sleep
		if (currSeconds > averageSleepCycles + additionalBuffer)
		{
			prevSeconds = currSeconds;

			// give back cycles to other processes if we don't need them
			thrd_sleep(&(struct timespec){.tv_nsec = 1000000}, NULL);	// sleep 1 millisecond

			// update the time after sleeping
			CP_UpdateFrameTime();

			// recompute remaining micros
			currSeconds = _frametimeTarget - ElapsedSeconds;

			// update the average sleep over 16 samples so we maintain awareness of the system's margin of error.
			averageSleepCycles = ((averageSleepCycles * 15.0) + (prevSeconds - currSeconds)) / 16.0;	// multiply by 15, add new sample, divide by 16
		}

	} while (ElapsedSeconds < _frametimeTarget);

	// Update time of the last frame
	_frametime = ElapsedSeconds;
}

void CP_UpdateFrameTime()
{
	EndingTime = glfwGetTime();
	ElapsedSeconds = EndingTime - StartingTime;
}

DLL_EXPORT unsigned long getframeCount()
{
	return _frameCount;
}

//------------------------------------------------------------------------------
// Private Functions:
//------------------------------------------------------------------------------
