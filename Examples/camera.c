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
//
// WASD or the arrow keys move the camera, the mouse wheel zooms, and a
// click drops a marker where you clicked in the world.
//
// Try this:
// - Make the camera follow a player that moves with the keys.
// - Stop the camera at the edges of the world.
//
// To run it, copy this file over the template's main.c.
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

// The world is 2000x2000 pixels, from (0, 0) to (2000, 2000)
const float worldSize = 2000.0f;

// The camera: the point of the world at the center of the window, and how
// much it's zoomed in (2 = everything twice as big)
CP_Vector cameraPosition;
float cameraZoom = 1.0f;
const float cameraSpeed = 500.0f;	// screen pixels per second

// Markers dropped with the mouse, in world positions
#define MAX_MARKERS 50
CP_Vector markers[MAX_MARKERS];
int markerCount = 0;

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
	if (CP_Input_KeyDown(KEY_A) || CP_Input_KeyDown(KEY_LEFT))
	{
		direction.x -= 1.0f;
	}
	if (CP_Input_KeyDown(KEY_D) || CP_Input_KeyDown(KEY_RIGHT))
	{
		direction.x += 1.0f;
	}
	if (CP_Input_KeyDown(KEY_W) || CP_Input_KeyDown(KEY_UP))
	{
		direction.y -= 1.0f;
	}
	if (CP_Input_KeyDown(KEY_S) || CP_Input_KeyDown(KEY_DOWN))
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
	CP_Graphics_DrawCircle(1000.0f, 1000.0f, 120.0f);	// the middle of the world
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

void game_update(void)
{
	UpdateCamera();
	ApplyCamera();

	// With the camera applied, CP_Input_GetMouseWorldX/Y say where the mouse
	// is in the world (CP_Input_GetMouseX/Y are still window positions)
	float mouseWorldX = CP_Input_GetMouseWorldX();
	float mouseWorldY = CP_Input_GetMouseWorldY();
	if (CP_Input_MouseTriggered(MOUSE_BUTTON_LEFT) && markerCount < MAX_MARKERS)
	{
		markers[markerCount] = CP_Vector_Set(mouseWorldX, mouseWorldY);
		++markerCount;
	}

	CP_Graphics_ClearBackground(CP_Color_Create(20, 20, 20, 255));
	DrawWorld();

	// Text on the screen, not in the world: undo the camera first
	CP_Settings_ResetMatrix();
	char text[120];
	snprintf(text, sizeof(text), "Camera at (%.0f, %.0f), zoom %.2fx    Mouse in the world: (%.0f, %.0f)",
		cameraPosition.x, cameraPosition.y, cameraZoom, mouseWorldX, mouseWorldY);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Font_DrawText(text, 20.0f, 30.0f);
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
