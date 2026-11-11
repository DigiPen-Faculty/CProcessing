//---------------------------------------------------------
// file:	main.c
//
// Lesson 2: Shapes and the coordinate system.
// https://github.com/DigiPen-Faculty/CProcessing/wiki/Lesson-02-Shapes-and-Coordinates
//
// This is the finished program from the lesson: a little house built from
// shapes, which you can paint on. The scene is drawn ONCE, and then the
// window keeps it: whatever you paint stays until you press C, which draws
// the scene again.
//
//   hold the left mouse button   paint
//   1, 2, 3, 4                   choose a paint color
//   mouse wheel                  change the brush size
//   C                            clear the painting
//
// To run it, copy this file over the template's main.c.
//
// Try this:
// - Add a second tree, a moon, or a car on the path.
// - Make the fence longer, or turn it into a row of tall and short pickets.
// - Draw the sun with CP_Graphics_DrawPie and rays around it.
//---------------------------------------------------------

#include "cprocessing.h"

// Paint colors, chosen with the keys 1 to 4
CP_Color brushColors[4];
int brushColor = 0;
float brushSize = 20.0f;

// TRUE when the scene needs to be drawn: at the start, and after C
CP_BOOL needsScene = TRUE;

// A cloud is three overlapping ellipses. (x, y) is the middle of the middle one.
void DrawCloud(float x, float y)
{
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 235));
	CP_Graphics_DrawEllipse(x - 40.0f, y + 6.0f, 80.0f, 40.0f);
	CP_Graphics_DrawEllipse(x + 40.0f, y + 6.0f, 90.0f, 40.0f);
	CP_Graphics_DrawEllipse(x, y, 100.0f, 56.0f);
}

// The house is placed by its top left corner, because that makes the
// numbers easy: every part is measured from the same corner.
void DrawHouse(float left, float top)
{
	CP_Settings_Stroke(CP_Color_Create(70, 40, 30, 255));
	CP_Settings_StrokeWeight(4.0f);

	// A chimney, drawn first so the roof covers its bottom
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(CP_Color_Create(150, 80, 70, 255));
	CP_Graphics_DrawRect(left + 190.0f, top - 90.0f, 30.0f, 80.0f);

	// The walls
	CP_Settings_Fill(CP_Color_Create(235, 205, 150, 255));
	CP_Graphics_DrawRect(left, top, 260.0f, 170.0f);

	// The roof: a triangle takes its three corners
	CP_Settings_Fill(CP_Color_Create(190, 60, 55, 255));
	CP_Graphics_DrawTriangle(left - 20.0f, top, left + 130.0f, top - 110.0f, left + 280.0f, top);

	// The door, standing on the bottom of the walls
	CP_Settings_Fill(CP_Color_Create(120, 75, 40, 255));
	CP_Graphics_DrawRect(left + 110.0f, top + 80.0f, 56.0f, 90.0f);
	CP_Settings_Fill(CP_Color_Create(250, 220, 90, 255));
	CP_Graphics_DrawCircle(left + 154.0f, top + 128.0f, 8.0f);

	// The windows are easier to place by their middles, so use the middle
	// mode for them. (Rectangles are placed by the middle unless you ask
	// for the corner.)
	CP_Settings_RectMode(CP_POSITION_CENTER);
	CP_Settings_Fill(CP_Color_Create(170, 220, 245, 255));
	CP_Graphics_DrawRect(left + 55.0f, top + 60.0f, 60.0f, 60.0f);
	CP_Graphics_DrawRect(left + 205.0f, top + 60.0f, 60.0f, 60.0f);
	// A cross of lines divides each window into panes
	CP_Settings_StrokeWeight(3.0f);
	CP_Graphics_DrawLine(left + 25.0f, top + 60.0f, left + 85.0f, top + 60.0f);
	CP_Graphics_DrawLine(left + 55.0f, top + 30.0f, left + 55.0f, top + 90.0f);
	CP_Graphics_DrawLine(left + 175.0f, top + 60.0f, left + 235.0f, top + 60.0f);
	CP_Graphics_DrawLine(left + 205.0f, top + 30.0f, left + 205.0f, top + 90.0f);
}

void DrawTree(float x, float groundY)
{
	CP_Settings_RectMode(CP_POSITION_CENTER);
	CP_Settings_Stroke(CP_Color_Create(70, 40, 30, 255));
	CP_Settings_StrokeWeight(4.0f);
	CP_Settings_Fill(CP_Color_Create(130, 85, 50, 255));
	CP_Graphics_DrawRect(x, groundY - 60.0f, 30.0f, 120.0f);

	// Leaves with no outline are softer
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(50, 140, 60, 255));
	CP_Graphics_DrawCircle(x, groundY - 150.0f, 130.0f);
	CP_Graphics_DrawCircle(x - 45.0f, groundY - 110.0f, 90.0f);
	CP_Graphics_DrawCircle(x + 45.0f, groundY - 110.0f, 90.0f);
}

// A row of pickets: the same rectangle, again and again, each one further
// along. A loop does the repeating, and arithmetic does the spacing.
void DrawFence(float left, float groundY, int pickets)
{
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Stroke(CP_Color_Create(90, 70, 50, 255));
	CP_Settings_StrokeWeight(2.0f);
	CP_Settings_Fill(CP_Color_Create(245, 240, 225, 255));
	for (int i = 0; i < pickets; i++)
	{
		CP_Graphics_DrawRect(left + i * 24.0f, groundY - 70.0f, 14.0f, 70.0f);
	}
	// Two rails across the pickets
	CP_Graphics_DrawRect(left - 4.0f, groundY - 55.0f, pickets * 24.0f - 6.0f, 8.0f);
	CP_Graphics_DrawRect(left - 4.0f, groundY - 25.0f, pickets * 24.0f - 6.0f, 8.0f);
}

// A bird from the side is two arcs: the top half of a small circle, twice
void DrawBird(float x, float y)
{
	CP_Settings_NoFill();
	CP_Settings_Stroke(CP_Color_Create(40, 40, 60, 255));
	CP_Settings_StrokeWeight(3.0f);
	CP_Graphics_DrawArc(x - 12.0f, y, 24.0f, 180.0f, 180.0f);
	CP_Graphics_DrawArc(x + 12.0f, y, 24.0f, 180.0f, 180.0f);
}

void DrawScene(void)
{
	const float windowWidth = (float)CP_System_GetWindowWidth();
	const float windowHeight = (float)CP_System_GetWindowHeight();

	// Every drawing function uses the settings that were last made, so the
	// scene starts by choosing the ones it needs
	CP_Graphics_ClearBackground(CP_Color_Create(135, 206, 235, 255));
	CP_Settings_NoStroke();
	CP_Settings_EllipseMode(CP_POSITION_CENTER);

	// The sun, in the top right corner
	CP_Settings_Fill(CP_Color_Create(255, 220, 80, 255));
	CP_Graphics_DrawCircle(windowWidth - 120.0f, 110.0f, 110.0f);

	DrawCloud(160.0f, 100.0f);
	DrawCloud(470.0f, 170.0f);

	// Two hills: ellipses that are mostly below the bottom of the window
	CP_Settings_Fill(CP_Color_Create(90, 170, 90, 255));
	CP_Graphics_DrawEllipse(200.0f, windowHeight - 50.0f, 800.0f, 330.0f);
	CP_Settings_Fill(CP_Color_Create(110, 190, 100, 255));
	CP_Graphics_DrawEllipse(windowWidth - 150.0f, windowHeight - 40.0f, 700.0f, 280.0f);

	// The ground: a rectangle from the left edge, 120 tall, at the bottom.
	// The corner mode puts its top left corner at (0, windowHeight - 120).
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(CP_Color_Create(125, 200, 105, 255));
	CP_Graphics_DrawRect(0.0f, windowHeight - 120.0f, windowWidth, 120.0f);

	// A path to the door: four corners, in order around the shape
	CP_Settings_Fill(CP_Color_Create(200, 185, 150, 255));
	CP_Graphics_DrawQuad(352.0f, 500.0f, 408.0f, 500.0f, 480.0f, windowHeight, 280.0f, windowHeight);

	DrawTree(110.0f, 510.0f);
	DrawHouse(240.0f, 330.0f);
	DrawFence(540.0f, 520.0f, 10);

	DrawBird(520.0f, 90.0f);
	DrawBird(600.0f, 130.0f);
	DrawBird(350.0f, 60.0f);

	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(30, 60, 30, 255));
	CP_Font_DrawText("Hold the left mouse button to paint. 1-4: color, wheel: brush size, C: clear.", 20.0f, windowHeight - 14.0f);
}

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Lesson 2: Shapes and the coordinate system");
	CP_Settings_TextSize(20.0f);

	brushColors[0] = CP_Color_Create(240, 70, 70, 255);
	brushColors[1] = CP_Color_Create(250, 180, 40, 255);
	brushColors[2] = CP_Color_Create(60, 120, 240, 255);
	brushColors[3] = CP_Color_Create(250, 250, 250, 255);
}

void game_update(void)
{
	// The scene is drawn once. Unlike lesson 1, nothing clears the window
	// every frame, so what was drawn is still there next frame.
	if (needsScene)
	{
		DrawScene();
		needsScene = FALSE;
	}
	if (CP_Input_KeyTriggered(KEY_C))
	{
		needsScene = TRUE;
	}

	// Keys 1 to 4 choose the brush color. KEY_1 + 1 is KEY_2, and so on.
	for (int i = 0; i < 4; i++)
	{
		if (CP_Input_KeyTriggered((CP_KEY)(KEY_1 + i)))
		{
			brushColor = i;
		}
	}

	// The wheel makes the brush bigger and smaller, within limits
	brushSize = CP_Math_ClampFloat(brushSize + CP_Input_MouseWheel() * 3.0f, 4.0f, 80.0f);

	// While the button is held, a thick line from where the mouse was last
	// frame to where it is now, with round ends, makes a smooth stroke.
	// (A circle each frame would leave gaps when the mouse moves fast.)
	if (CP_Input_MouseDown(MOUSE_BUTTON_LEFT))
	{
		CP_Settings_Stroke(brushColors[brushColor]);
		CP_Settings_StrokeWeight(brushSize);
		CP_Settings_LineCapMode(CP_LINE_CAP_ROUND);
		CP_Graphics_DrawLine(CP_Input_GetMousePreviousX(), CP_Input_GetMousePreviousY(),
			CP_Input_GetMouseX(), CP_Input_GetMouseY());
		CP_Settings_LineCapMode(CP_LINE_CAP_BUTT);
	}
}

void game_exit(void)
{
	// This program loaded nothing to free.
}

int main(void)
{
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
