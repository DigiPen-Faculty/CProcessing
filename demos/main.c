//---------------------------------------------------------
// file:	main.c
//
// brief:	Main entry point for the demo project
//			of the CProcessing library
//
// documentation link:
// https://github.com/DigiPen-Faculty/CProcessing/wiki
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include "demo_manager.h"

int main(void)
{
	DemoManagerInit();
	CP_Engine_Run();
	return 0;
}
