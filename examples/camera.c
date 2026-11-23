//---------------------------------------------------------
// file:	camera.c
//
// Example: a camera that moves around and zooms into a world bigger than
// the window.
//
// Shows how to:
// - draw the world through a camera with CP_Settings_Translate and
//   CP_Settings_Scale
// - find where the mouse is in the world with CP_Input_GetMouseWorldX/Y
// - draw text that stays put on the screen, with CP_Settings_ResetMatrix
// - find where a world position is in the window with CP_Math_WorldToScreen,
//   to label it at a size that doesn't zoom, or point to it from the edge
//   when it's off the screen
// - find the part of the world the window shows with CP_Math_ScreenToWorld
//
// WASD or the arrow keys move the camera, the mouse wheel zooms, and a
// click drops a marker where you clicked in the world.
//
// Try this:
// - Make the camera follow a player that moves with the keys.
// - Stop the camera at the edges of the world.
// - Skip drawing the landmarks outside the part of the world the window
//   shows. With thousands of things in a world, that saves a lot.
//
// See also: movement.c, for a player for the camera to follow; clipping.c,
// for a minimap of a world.
//
// To run it, copy this file over the template's main.c.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

// The world is 2000x2000 pixels, from (0, 0) to (2000, 2000)
const float worldSize = 2000.0f;

// The camera: the point of the world at the center of the window, and how
// much it's zoomed in (2 = everything twice as big)
CP_Vector cameraPosition;
float cameraZoom = 1.0f;
const float cameraSpeed = 500.0f; // screen pixels per second

// Markers dropped with the mouse, in world positions
#define MAX_MARKERS 50
CP_Vector markers[MAX_MARKERS];
int markerCount = 0;

// Where each marker is in the window, found while the camera is applied
CP_Vector markersOnScreen[MAX_MARKERS];

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Camera");

	cameraPosition = CP_Vector_Set(worldSize / 2.0f, worldSize / 2.0f);
}

// Move the camera with the keys and zoom with the mouse wheel
void UpdateCamera(void)
{
	CP_Vector direction = CP_Vector_Zero();
	if (CP_Input_KeyDown(CP_KEY_A) || CP_Input_KeyDown(CP_KEY_LEFT))
	{
		direction.x -= 1.0f;
	}
	if (CP_Input_KeyDown(CP_KEY_D) || CP_Input_KeyDown(CP_KEY_RIGHT))
	{
		direction.x += 1.0f;
	}
	if (CP_Input_KeyDown(CP_KEY_W) || CP_Input_KeyDown(CP_KEY_UP))
	{
		direction.y -= 1.0f;
	}
	if (CP_Input_KeyDown(CP_KEY_S) || CP_Input_KeyDown(CP_KEY_DOWN))
	{
		direction.y += 1.0f;
	}
	if (direction.x != 0.0f || direction.y != 0.0f)
	{
		direction = CP_Vector_Normalize(direction);
	}

	// Zoomed in, the same move in the world is more pixels on the screen, so
	// divide by the zoom to keep the screen speed the same
	float speed = cameraSpeed / cameraZoom * CP_System_GetDt();
	cameraPosition = CP_Vector_Add(cameraPosition, CP_Vector_Scale(direction, speed));

	// Each notch of the wheel zooms in or out by 10%, between 0.25x and 4x
	float wheel = CP_Input_MouseWheel();
	if (wheel > 0.0f)
	{
		cameraZoom *= 1.1f;
	}
	else if (wheel < 0.0f)
	{
		cameraZoom /= 1.1f;
	}
	cameraZoom = CP_Math_ClampFloat(cameraZoom, 0.25f, 4.0f);
}

// Set up the drawing so world positions land in the right place in the
// window. Read the three steps from the bottom up: move the camera's
// position to (0, 0), zoom, then move (0, 0) to the middle of the window.
void ApplyCamera(void)
{
	CP_Settings_ResetMatrix();
	CP_Settings_Translate(CP_System_GetWindowWidth() / 2.0f, CP_System_GetWindowHeight() / 2.0f);
	CP_Settings_Scale(cameraZoom, cameraZoom);
	CP_Settings_Translate(-cameraPosition.x, -cameraPosition.y);
}

// The world: a ground, grid lines every 100 pixels, and a few landmarks
void DrawWorld(void)
{
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(50, 90, 60, 255));
	CP_Graphics_DrawRect(0.0f, 0.0f, worldSize, worldSize);

	CP_Settings_Stroke(CP_Color_Create(70, 120, 80, 255));
	CP_Settings_StrokeWeight(2.0f);
	for (float line = 0.0f; line <= worldSize; line += 100.0f)
	{
		CP_Graphics_DrawLine(line, 0.0f, line, worldSize);
		CP_Graphics_DrawLine(0.0f, line, worldSize, line);
	}

	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(240, 200, 80, 255));
	CP_Graphics_DrawCircle(1000.0f, 1000.0f, 120.0f); // the middle of the world
	CP_Settings_Fill(CP_Color_Create(90, 160, 230, 255));
	CP_Graphics_DrawCircle(300.0f, 300.0f, 200.0f);
	CP_Graphics_DrawCircle(1700.0f, 400.0f, 150.0f);
	CP_Settings_Fill(CP_Color_Create(130, 100, 80, 255));
	CP_Graphics_DrawRect(1400.0f, 1400.0f, 300.0f, 200.0f);
	CP_Graphics_DrawRect(200.0f, 1500.0f, 150.0f, 350.0f);

	// the markers
	CP_Settings_Fill(CP_Color_Create(230, 60, 60, 255));
	for (int i = 0; i < markerCount; ++i)
	{
		CP_Graphics_DrawCircle(markers[i].x, markers[i].y, 20.0f);
	}
}

// A number for each marker, drawn after the camera is undone, so it's the
// same size at any zoom. A marker off the screen gets an arrow at the edge
// of the window instead, pointing the way to it.
void DrawMarkerLabels(void)
{
	const float width = (float)CP_System_GetWindowWidth();
	const float height = (float)CP_System_GetWindowHeight();
	CP_Settings_Save(); // and Restore at the end, so the text after keeps its settings
	CP_Settings_TextSize(18.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	for (int i = 0; i < markerCount; ++i)
	{
		char number[12]; // room for any int, so no compiler warns it might not fit
		snprintf(number, sizeof(number), "%d", i + 1);
		const CP_Vector onScreen = markersOnScreen[i];

		if (onScreen.x >= 0.0f && onScreen.x <= width && onScreen.y >= 0.0f && onScreen.y <= height)
		{
			CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
			CP_Font_DrawText(number, onScreen.x, onScreen.y - 24.0f);
			continue;
		}

		// Off the screen: the same point kept inside the window, away from
		// the text at the top and bottom, and an arrow from there toward
		// the marker, with its number
		CP_Vector edge;
		edge.x = CP_Math_ClampFloat(onScreen.x, 30.0f, width - 30.0f);
		edge.y = CP_Math_ClampFloat(onScreen.y, 75.0f, height - 50.0f);
		const CP_Vector way = CP_Vector_Normalize(CP_Vector_Subtract(onScreen, edge));
		const CP_Vector side = CP_Vector_Set(-way.y, way.x);
		const CP_Vector tip = CP_Vector_Add(edge, CP_Vector_Scale(way, 14.0f));
		const CP_Vector back = CP_Vector_Subtract(edge, CP_Vector_Scale(way, 8.0f));
		CP_Settings_NoStroke();
		CP_Settings_Fill(CP_Color_Create(230, 60, 60, 255));
		CP_Graphics_DrawTriangle(tip.x, tip.y, back.x + side.x * 10.0f, back.y + side.y * 10.0f, back.x - side.x * 10.0f,
			back.y - side.y * 10.0f);
		CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
		CP_Font_DrawText(number, edge.x - way.x * 22.0f, edge.y - way.y * 22.0f);
	}
	CP_Settings_Restore();
}

void game_update(void)
{
	UpdateCamera();
	ApplyCamera();

	// With the camera applied, CP_Input_GetMouseWorldX/Y say where the mouse
	// is in the world (CP_Input_GetMouseX/Y are still window positions)
	float mouseWorldX = CP_Input_GetMouseWorldX();
	float mouseWorldY = CP_Input_GetMouseWorldY();
	if (CP_Input_MouseTriggered(CP_MOUSE_LEFT) && markerCount < MAX_MARKERS)
	{
		markers[markerCount] = CP_Vector_Set(mouseWorldX, mouseWorldY);
		++markerCount;
	}

	CP_Graphics_ClearBackground(CP_Color_Create(20, 20, 20, 255));
	DrawWorld();

	// While the camera is still applied, CP_Math_WorldToScreen gives where
	// a world position is in the window...
	for (int i = 0; i < markerCount; ++i)
	{
		CP_Math_WorldToScreen(markers[i].x, markers[i].y, &markersOnScreen[i].x, &markersOnScreen[i].y);
	}
	// ...and CP_Math_ScreenToWorld goes the other way: the window's top left
	// and bottom right corners, in the world, are the part of it you see
	float viewLeft, viewTop, viewRight, viewBottom;
	CP_Math_ScreenToWorld(0.0f, 0.0f, &viewLeft, &viewTop);
	CP_Math_ScreenToWorld((float)CP_System_GetWindowWidth(), (float)CP_System_GetWindowHeight(), &viewRight, &viewBottom);

	// Text on the screen, not in the world: undo the camera first
	CP_Settings_ResetMatrix();
	DrawMarkerLabels();
	char text[120];
	snprintf(text, sizeof(text), "Camera at (%.0f, %.0f), zoom %.2fx    Mouse in the world: (%.0f, %.0f)",
		cameraPosition.x, cameraPosition.y, cameraZoom, mouseWorldX, mouseWorldY);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Font_DrawText(text, 20.0f, 30.0f);
	snprintf(text, sizeof(text), "The window shows the world from (%.0f, %.0f) to (%.0f, %.0f)", viewLeft, viewTop,
		viewRight, viewBottom);
	CP_Font_DrawText(text, 20.0f, 52.0f);
	CP_Font_DrawText("WASD or arrows: move    Mouse wheel: zoom    Click: drop a marker", 20.0f, 580.0f);
}

void game_exit(void)
{
}

int main(void)
{
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
