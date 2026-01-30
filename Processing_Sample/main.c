//---------------------------------------------------------
// file:	main.c
//
// brief:	Main entry point for the demo project
//			of the CProcessing library
//
// documentation link:
// https://github.com/DigiPen-Faculty/CProcessing/wiki
//
// Copyright © 2025 DigiPen, All rights reserved.
//---------------------------------------------------------

#include "cprocessing.h"
#include "DemoManager.h"

int main(void)
{
	DemoManagerInit();
	CP_Engine_Run();
	return 0;
}
