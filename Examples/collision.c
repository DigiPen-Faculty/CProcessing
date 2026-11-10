//---------------------------------------------------------
// file:	collision.c
//
// Example: checking whether shapes touch.
//
// Shows how to check:
// - a point against a rectangle (the mouse over a button)
// - a circle against a circle
// - a rectangle against a rectangle (axis-aligned, not rotated)
// - a circle against a rectangle
//
// Move the mouse to move the player. Click the button, or press Space, to
// switch the player between a circle and a rectangle. Shapes the player
// touches turn red.
//
// Try this:
// - Make the player bounce off the walls instead of passing through.
// - Count how many shapes the player touches and show the number.
//
// To run it, copy this file over the template's main.c.
//---------------------------------------------------------

#include "cprocessing.h"

// Shapes are stored by their center, as CProcessing draws them by default
// (CP_POSITION_CENTER).
typedef struct Box
{
	CP_Vector center;
	float width;
	float height;
} Box;

typedef struct Circle
{
	CP_Vector center;
	float radius;
} Circle;

Box MakeBox(float x, float y, float width, float height)
{
	Box box;
	box.center = CP_Vector_Set(x, y);
	box.width = width;
	box.height = height;
	return box;
}

Circle MakeCircle(float x, float y, float radius)
{
	Circle circle;
	circle.center = CP_Vector_Set(x, y);
	circle.radius = radius;
	return circle;
}

//-------------------------
// THE CHECKS

// Is the point inside the box? Inside means between the left and right
// edges, and between the top and bottom edges.
CP_BOOL PointInBox(CP_Vector point, Box box)
{
	float left = box.center.x - box.width / 2.0f;
	float right = box.center.x + box.width / 2.0f;
	float top = box.center.y - box.height / 2.0f;
	float bottom = box.center.y + box.height / 2.0f;
	return point.x >= left && point.x <= right && point.y >= top && point.y <= bottom;
}

// Two circles touch when their centers are closer than their two radii
// added together.
CP_BOOL CirclesTouch(Circle a, Circle b)
{
	return CP_Vector_Distance(a.center, b.center) <= a.radius + b.radius;
}

// Two boxes touch when they overlap both across (x) and down (y). They
// overlap across when the distance between their centers is less than half
// their widths added together; the same for down.
CP_BOOL BoxesTouch(Box a, Box b)
{
	float distanceX = a.center.x - b.center.x;
	float distanceY = a.center.y - b.center.y;

	// only how far apart they are matters, not which is first
	if (distanceX < 0.0f)
	{
		distanceX = -distanceX;
	}
	if (distanceY < 0.0f)
	{
		distanceY = -distanceY;
	}
	return distanceX <= (a.width + b.width) / 2.0f && distanceY <= (a.height + b.height) / 2.0f;
}

// A circle touches a box when the box's closest point to the circle's
// center is inside the circle. The closest point is the center, clamped to
// the box's edges.
CP_BOOL CircleTouchesBox(Circle circle, Box box)
{
	CP_Vector closest;
	closest.x = CP_Math_ClampFloat(circle.center.x, box.center.x - box.width / 2.0f, box.center.x + box.width / 2.0f);
	closest.y = CP_Math_ClampFloat(circle.center.y, box.center.y - box.height / 2.0f, box.center.y + box.height / 2.0f);
	return CP_Vector_Distance(circle.center, closest) <= circle.radius;
}

//-------------------------
// THE SCENE

#define BOX_COUNT 3
#define CIRCLE_COUNT 3

Box boxes[BOX_COUNT];
Circle circles[CIRCLE_COUNT];
Box button;

// The player is a circle or a box, at the mouse
CP_BOOL playerIsCircle = TRUE;
Circle playerCircle;
Box playerBox;

CP_Color grey;
CP_Color red;

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Collision");

	boxes[0] = MakeBox(200.0f, 200.0f, 160.0f, 80.0f);
	boxes[1] = MakeBox(600.0f, 180.0f, 60.0f, 200.0f);
	boxes[2] = MakeBox(420.0f, 470.0f, 260.0f, 50.0f);
	circles[0] = MakeCircle(400.0f, 260.0f, 60.0f);
	circles[1] = MakeCircle(160.0f, 430.0f, 40.0f);
	circles[2] = MakeCircle(660.0f, 430.0f, 80.0f);
	button = MakeBox(110.0f, 45.0f, 180.0f, 40.0f);

	playerCircle.radius = 35.0f;
	playerBox.width = 90.0f;
	playerBox.height = 60.0f;

	grey = CP_Color_Create(150, 150, 150, 255);
	red = CP_Color_Create(230, 60, 60, 255);
}

void game_update(void)
{
	CP_Vector mouse = CP_Vector_Set(CP_Input_GetMouseX(), CP_Input_GetMouseY());

	// Point in a box: is the mouse over the button when it's clicked?
	CP_BOOL mouseOverButton = PointInBox(mouse, button);
	if ((mouseOverButton && CP_Input_MouseTriggered(MOUSE_BUTTON_LEFT)) || CP_Input_KeyTriggered(KEY_SPACE))
	{
		playerIsCircle = !playerIsCircle;
	}

	// The player follows the mouse
	playerCircle.center = mouse;
	playerBox.center = mouse;

	CP_Graphics_ClearBackground(CP_Color_Create(40, 40, 40, 255));
	CP_Settings_NoStroke();

	// Each shape is red if the player touches it. Which check to use depends
	// on the two shapes.
	for (int i = 0; i < BOX_COUNT; ++i)
	{
		CP_BOOL touching = playerIsCircle ? CircleTouchesBox(playerCircle, boxes[i]) : BoxesTouch(playerBox, boxes[i]);
		CP_Settings_Fill(touching ? red : grey);
		CP_Graphics_DrawRect(boxes[i].center.x, boxes[i].center.y, boxes[i].width, boxes[i].height);
	}
	for (int i = 0; i < CIRCLE_COUNT; ++i)
	{
		CP_BOOL touching = playerIsCircle ? CirclesTouch(playerCircle, circles[i]) : CircleTouchesBox(circles[i], playerBox);
		CP_Settings_Fill(touching ? red : grey);
		CP_Graphics_DrawCircle(circles[i].center.x, circles[i].center.y, circles[i].radius * 2.0f);	// DrawCircle takes the diameter
	}

	// The player, see-through so the shapes under it show
	CP_Settings_Fill(CP_Color_Create(80, 200, 255, 160));
	if (playerIsCircle)
	{
		CP_Graphics_DrawCircle(playerCircle.center.x, playerCircle.center.y, playerCircle.radius * 2.0f);
	}
	else
	{
		CP_Graphics_DrawRect(playerBox.center.x, playerBox.center.y, playerBox.width, playerBox.height);
	}

	// The button, lighter while the mouse is over it
	CP_Settings_Fill(mouseOverButton ? CP_Color_Create(110, 110, 160, 255) : CP_Color_Create(80, 80, 120, 255));
	CP_Graphics_DrawRect(button.center.x, button.center.y, button.width, button.height);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Font_DrawText(playerIsCircle ? "Switch to a box" : "Switch to a circle", button.center.x, button.center.y);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);
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
