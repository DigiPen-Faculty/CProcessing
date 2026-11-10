//---------------------------------------------------------
// file:	ScreenshotDemo.c
//
// CProcessing features in this demo:
// - Take screenshots every frame, of the whole window and of a small region
//   (CP_Image_Screenshot), and draw them scaled (CP_Image_Draw)
// - Free each screenshot when it's no longer needed (CP_Image_Free)
//
// Controls (also listed on screen, top right):
// - A screenshot of the window, at 1/8 size, follows the mouse
// - Hold the left mouse button for a 2x magnifier
// - Space switches tunnel mode on and off: each screenshot is taken after the
//   previous one was drawn, so it contains it, and so on
//---------------------------------------------------------

#include "cprocessing.h"
#include "ScreenshotDemo.h"
#include "DemoManager.h"
#include <stdio.h>

//-------------------------
// BALLS: something moving to take screenshots of

#define BALL_COUNT 24

typedef struct Ball
{
	CP_Vector position;
	CP_Vector velocity;		// pixels per second
	float diameter;
	CP_Color color;
} Ball;

static Ball balls[BALL_COUNT];

static void CreateBalls(void)
{
	const float width = (float)CP_System_GetWindowWidth();
	const float height = (float)CP_System_GetWindowHeight();
	for (int i = 0; i < BALL_COUNT; ++i)
	{
		balls[i].diameter = CP_Random_RangeFloat(30.0f, 120.0f);
		balls[i].position = CP_Vector_Set(CP_Random_RangeFloat(60.0f, width - 60.0f), CP_Random_RangeFloat(60.0f, height - 60.0f));
		balls[i].velocity = CP_Vector_Set(CP_Random_RangeFloat(-250.0f, 250.0f), CP_Random_RangeFloat(-250.0f, 250.0f));
		balls[i].color = CP_Color_FromColorHSL(CP_ColorHSL_Create(CP_Random_RangeInt(0, 359), 70, 55, 255));
	}
}

// Move each ball, and bounce it off the edges of the window
static void MoveBalls(void)
{
	const float width = (float)CP_System_GetWindowWidth();
	const float height = (float)CP_System_GetWindowHeight();
	const float dt = CP_System_GetDt();
	for (int i = 0; i < BALL_COUNT; ++i)
	{
		Ball* ball = &balls[i];
		const float radius = ball->diameter * 0.5f;
		ball->position = CP_Vector_Add(ball->position, CP_Vector_Scale(ball->velocity, dt));
		if ((ball->position.x < radius && ball->velocity.x < 0) || (ball->position.x > width - radius && ball->velocity.x > 0))
		{
			ball->velocity.x = -ball->velocity.x;
		}
		if ((ball->position.y < radius && ball->velocity.y < 0) || (ball->position.y > height - radius && ball->velocity.y > 0))
		{
			ball->velocity.y = -ball->velocity.y;
		}
	}
}

static void DrawBalls(void)
{
	CP_Settings_NoStroke();
	for (int i = 0; i < BALL_COUNT; ++i)
	{
		CP_Settings_Fill(balls[i].color);
		CP_Graphics_DrawCircle(balls[i].position.x, balls[i].position.y, balls[i].diameter);
	}
}

//-------------------------
// SCREENSHOTS

#define THUMBNAIL_SCALE 8		// the window screenshot is drawn at 1/8 size
#define LENS_SIZE 80			// the magnifier shows an 80 x 80 region...
#define LENS_ZOOM 2				// ...at 2x

static CP_BOOL tunnelMode = FALSE;
static CP_Image tunnelScreenshot = NULL;	// in tunnel mode, the previous frame's screenshot
static unsigned screenshotsTaken = 0;

static CP_Image TakeScreenshot(int x, int y, int width, int height)
{
	++screenshotsTaken;
	return CP_Image_Screenshot(x, y, width, height);
}

static int ClampInt(int value, int low, int high)
{
	return value < low ? low : (value > high ? high : value);
}

// Draw an image with its top left corner at (x, y), framed in white
static void DrawFramed(CP_Image image, float x, float y, float width, float height)
{
	CP_Settings_Save();
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_Image_Draw(image, x, y, width, height, 255);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_NoFill();
	CP_Settings_Stroke(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_StrokeWeight(2.0f);
	CP_Graphics_DrawRect(x, y, width, height);
	CP_Settings_Restore();
}

static void DrawScreenshots(void)
{
	const int width = CP_System_GetWindowWidth();
	const int height = CP_System_GetWindowHeight();
	const float mouseX = CP_Input_GetMouseX();
	const float mouseY = CP_Input_GetMouseY();
	const float thumbnailWidth = (float)width / THUMBNAIL_SCALE;
	const float thumbnailHeight = (float)height / THUMBNAIL_SCALE;

	if (CP_Input_KeyTriggered(KEY_SPACE))
	{
		tunnelMode = !tunnelMode;
		if (!tunnelMode)
		{
			CP_Image_Free(&tunnelScreenshot);
		}
	}

	// The magnifier's region: centered on the mouse, but kept inside the
	// window. It's captured before anything is drawn over it.
	CP_Image lens = NULL;
	int lensX = ClampInt((int)mouseX - LENS_SIZE / 2, 0, width - LENS_SIZE);
	int lensY = ClampInt((int)mouseY - LENS_SIZE / 2, 0, height - LENS_SIZE);
	if (CP_Input_MouseDown(MOUSE_BUTTON_LEFT))
	{
		lens = TakeScreenshot(lensX, lensY, LENS_SIZE, LENS_SIZE);
	}

	if (tunnelMode)
	{
		// last frame's screenshot, which shows last frame's thumbnail, which
		// shows the one before it, and so on
		if (tunnelScreenshot)
		{
			DrawFramed(tunnelScreenshot, mouseX, mouseY, thumbnailWidth, thumbnailHeight);
		}
	}
	else
	{
		// the window as it is now
		CP_Image screenshot = TakeScreenshot(0, 0, width, height);
		DrawFramed(screenshot, mouseX, mouseY, thumbnailWidth, thumbnailHeight);
		CP_Image_Free(&screenshot);
	}

	if (lens)
	{
		// drawn centered over the region it shows, with a cross at its center
		const float zoomedSize = (float)(LENS_SIZE * LENS_ZOOM);
		const float centerX = lensX + LENS_SIZE * 0.5f;
		const float centerY = lensY + LENS_SIZE * 0.5f;
		DrawFramed(lens, centerX - zoomedSize * 0.5f, centerY - zoomedSize * 0.5f, zoomedSize, zoomedSize);
		CP_Image_Free(&lens);

		CP_Settings_Save();
		CP_Settings_Stroke(CP_Color_Create(255, 0, 0, 255));
		CP_Settings_StrokeWeight(1.0f);
		CP_Graphics_DrawLine(centerX - 8.0f, centerY, centerX + 8.0f, centerY);
		CP_Graphics_DrawLine(centerX, centerY - 8.0f, centerX, centerY + 8.0f);
		CP_Settings_Restore();
	}

	if (tunnelMode)
	{
		// taken after everything is drawn, so the next thumbnail contains this
		// frame's thumbnail
		CP_Image_Free(&tunnelScreenshot);
		tunnelScreenshot = TakeScreenshot(0, 0, width, height);
	}
}

// The controls and what's happening, in the top right corner
static void DrawInfo(void)
{
	CP_Settings_Save();
	CP_Settings_TextSize(16.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_RIGHT, CP_TEXT_ALIGN_V_TOP);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));

	const float x = CP_System_GetWindowWidth() - 15.0f;
	const float lineHeight = 19.0f;
	float y = 10.0f;
	char line[96];

	CP_Font_DrawText("SCREENSHOTS: the window at 1/8 size follows the mouse", x, y); y += lineHeight;
	CP_Font_DrawText("Hold the left mouse button: 2x magnifier", x, y); y += lineHeight;
	snprintf(line, sizeof(line), "Space: tunnel mode (%s)", tunnelMode ? "ON" : "off");
	CP_Font_DrawText(line, x, y); y += lineHeight;
	snprintf(line, sizeof(line), "%.0f FPS, %u screenshots taken", CP_System_GetFrameRate(), screenshotsTaken);
	CP_Font_DrawText(line, x, y);

	CP_Settings_Restore();
}

void ScreenshotDemoInit(void)
{
	CreateBalls();
	tunnelMode = FALSE;
	screenshotsTaken = 0;
}

void ScreenshotDemoUpdate(void)
{
	CP_Graphics_ClearBackground(GetCommonBackgroundColor());
	MoveBalls();
	DrawBalls();
	DrawInfo();

	// last, so the screenshots show everything above
	DrawScreenshots();
}

void ScreenshotDemoExit(void)
{
	CP_Image_Free(&tunnelScreenshot);
}
