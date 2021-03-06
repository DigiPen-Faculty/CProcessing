//------------------------------------------------------------------------------
// file:	dllmain.c
// author:	Justin Chambers
// brief:	Defines the entry point for the DLL application.
//
// Copyright © 2019 DigiPen, All rights reserved.
//------------------------------------------------------------------------------

#if defined _WIN32

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN			// Exclude rarely-used stuff from Windows headers
#include <windows.h>

static BOOL valuesSet = FALSE;
void SetCPCoreValues(void);			// from CP_System.c

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD ul_reason_for_call,
	LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(hModule);
	UNREFERENCED_PARAMETER(lpReserved);

	// Make sure the initial memory state for the Core struct is set
	if (!valuesSet)
	{
		SetCPCoreValues();
		valuesSet = TRUE;
	}

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}

#endif //_WIN32
