//---------------------------------------------------------
// file:	main.c
//
// Lesson 7: Images and animation.
// https://github.com/DigiPen-Faculty/CProcessing/wiki/Lesson-07-Images-and-Animation
//
// This is the finished program from the lesson: a slime that walks around
// and animates while it walks, and glowing orbs that glide to where you click.
//
//   WASD or the arrow keys   walk the slime (it only animates while it walks)
//   Click                    send an orb gliding from the slime to the mouse
//   1 to 5                   choose how the orb moves: linear, ease in,
//                            ease out, ease in and out, or bounce
//
// Two things are shown:
//   images      load once, draw a whole image, or one frame of a sprite sheet
//   easing      the same trip, with a different feel, from a curve applied
//               to the progress of the trip
//
// To run it, copy this file over the template's main.c. It needs
// SlimeSheet.png in the Assets folder, which the template includes.
//
// Try this:
// - Draw a second slime that follows the first, a little behind it.
// - Fade the orb out (the alpha of a color) as it arrives.
// - Draw a background picture under everything, with CP_Image_Draw.
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

//---------------------------------------------------------
// THE SPRITE SHEET: 8 frames of the slime in a row, each 64 x 64 pixels.
//---------------------------------------------------------
#define FRAME_COUNT 8
const float frameSize = 64.0f;
const float drawSize = 128.0f;		// the slime is drawn at twice its size

CP_Image slimeSheet;

CP_Vector slimePosition;
const float slimeSpeed = 220.0f;	// pixels per second
CP_BOOL facingLeft = FALSE;

// The animation: which frame shows now, and how long it has shown
int currentFrame = 0;
float frameTimer = 0.0f;
const float framesPerSecond = 12.0f;

//---------------------------------------------------------
// EASING. An orb makes a trip from a start to an end that takes `tripTime`
// seconds. `progress` is how far through the time we are, from 0 to 1.
// An easing function turns that into how far along the PATH we are, also
// from 0 to 1, but not at the same pace.
//---------------------------------------------------------
#define EASING_COUNT 5
const char* easingNames[EASING_COUNT] = { "linear", "ease in", "ease out", "ease in and out", "bounce" };
int easing = 3;

float ApplyEasing(int which, float progress)
{
	switch (which)
	{
	case 1:  return CP_Math_EaseInCubic(progress);
	case 2:  return CP_Math_EaseOutCubic(progress);
	case 3:  return CP_Math_EaseInOutCubic(progress);
	case 4:  return CP_Math_EaseOutBounce(progress);
	default: return progress;		// linear: no change
	}
}

const float tripTime = 1.4f;
CP_BOOL orbFlying = FALSE;
CP_BOOL orbLaunched = FALSE;		// has an orb been sent yet? (it stays where it landed)
CP_Vector orbStart;
CP_Vector orbEnd;
float orbTimer = 0.0f;
int orbEasing = 3;					// the easing this trip started with

// Where the orb is, a given way through its trip
CP_Vector OrbAt(float progress)
{
	return CP_Vector_Lerp(orbStart, orbEnd, ApplyEasing(orbEasing, progress));
}

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Lesson 7: Images and animation");
	CP_Settings_TextSize(20.0f);

	// Load an image ONCE, here. Loading in game_update would read the file
	// from the disk on every frame.
	slimeSheet = CP_Image_Load("Assets/SlimeSheet.png");

	slimePosition = CP_Vector_Set(180.0f, 400.0f);
}

void game_update(void)
{
	const float dt = CP_System_GetDt();

	//-----------------------------------------------------
	// Walk the slime (lesson 5)
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
	const CP_BOOL walking = direction.x != 0.0f || direction.y != 0.0f;
	if (walking)
	{
		direction = CP_Vector_Normalize(direction);
		slimePosition = CP_Vector_Add(slimePosition, CP_Vector_Scale(direction, slimeSpeed * dt));
		slimePosition.x = CP_Math_ClampFloat(slimePosition.x, drawSize / 2.0f, CP_System_GetWindowWidth() - drawSize / 2.0f);
		slimePosition.y = CP_Math_ClampFloat(slimePosition.y, drawSize / 2.0f + 60.0f, CP_System_GetWindowHeight() - drawSize / 2.0f);
		if (direction.x != 0.0f)
		{
			facingLeft = direction.x < 0.0f;
		}

		// Animate: add up time, and each time a frame's worth has passed,
		// go on to the next frame, back to the first after the last
		frameTimer += dt;
		const float frameDuration = 1.0f / framesPerSecond;
		while (frameTimer >= frameDuration)
		{
			frameTimer -= frameDuration;
			currentFrame = (currentFrame + 1) % FRAME_COUNT;
		}
	}
	else
	{
		// Standing still: the first frame, the slime resting on the ground
		currentFrame = 0;
		frameTimer = 0.0f;
	}

	//-----------------------------------------------------
	// Easing: choose with 1 to 5, launch with a click
	if (CP_Input_KeyTriggered(KEY_1)) { easing = 0; }
	if (CP_Input_KeyTriggered(KEY_2)) { easing = 1; }
	if (CP_Input_KeyTriggered(KEY_3)) { easing = 2; }
	if (CP_Input_KeyTriggered(KEY_4)) { easing = 3; }
	if (CP_Input_KeyTriggered(KEY_5)) { easing = 4; }

	if (CP_Input_MouseTriggered(MOUSE_BUTTON_LEFT))
	{
		orbStart = CP_Vector_Set(slimePosition.x, slimePosition.y - 20.0f);
		orbEnd = CP_Vector_Set(CP_Input_GetMouseX(), CP_Input_GetMouseY());
		orbTimer = 0.0f;
		orbEasing = easing;
		orbFlying = TRUE;
		orbLaunched = TRUE;
	}

	float progress = 1.0f;
	if (orbFlying)
	{
		orbTimer += dt;
		progress = CP_Math_ClampFloat(orbTimer / tripTime, 0.0f, 1.0f);
		if (progress >= 1.0f)
		{
			orbFlying = FALSE;
		}
	}

	//-----------------------------------------------------
	// Draw
	CP_Graphics_ClearBackground(CP_Color_Create(30, 30, 50, 255));
	CP_Settings_NoStroke();

	// The ground
	CP_Settings_Fill(CP_Color_Create(45, 60, 70, 255));
	CP_Graphics_DrawRect(400.0f, 560.0f, 800.0f, 80.0f);

	// The trip, drawn as ghosts. Each ghost is where the orb is at a
	// regular beat in TIME (every tenth of the trip), so how far apart they
	// are shows the easing: wide gaps are fast, close ones slow.
	if (orbLaunched)
	{
		for (int i = 0; i <= 10; i++)
		{
			const CP_Vector ghost = OrbAt(i / 10.0f);
			CP_Settings_Fill(CP_Color_Create(255, 220, 120, 70));
			CP_Graphics_DrawCircle(ghost.x, ghost.y, 14.0f);
		}
	}

	// The slime: one frame of the sheet. Frame n is the part of the sheet
	// from n * 64 to n * 64 + 64 across, and all 64 down.
	const float u0 = currentFrame * frameSize;
	const float u1 = u0 + frameSize;
	if (facingLeft)
	{
		// Asking for the frame's edges the other way round draws it flipped
		CP_Image_DrawSubImage(slimeSheet, slimePosition.x, slimePosition.y, drawSize, drawSize, u1, 0.0f, u0, frameSize, 255);
	}
	else
	{
		CP_Image_DrawSubImage(slimeSheet, slimePosition.x, slimePosition.y, drawSize, drawSize, u0, 0.0f, u1, frameSize, 255);
	}

	// The orb, on top
	const CP_Vector orb = OrbAt(progress);
	if (orbLaunched)
	{
		CP_Settings_Fill(CP_Color_Create(255, 220, 120, 255));
		CP_Graphics_DrawCircle(orb.x, orb.y, 30.0f);
		CP_Settings_Fill(CP_Color_Create(255, 250, 220, 255));
		CP_Graphics_DrawCircle(orb.x - 5.0f, orb.y - 5.0f, 10.0f);
	}

	// The whole sheet, small, in the corner, with a box on the frame in use
	const float sheetWidth = FRAME_COUNT * frameSize / 2.0f;
	const float sheetLeft = 800.0f - sheetWidth - 16.0f;
	CP_Image_Draw(slimeSheet, sheetLeft + sheetWidth / 2.0f, 16.0f + frameSize / 4.0f, sheetWidth, frameSize / 2.0f, 255);
	CP_Settings_NoFill();
	CP_Settings_Stroke(CP_Color_Create(255, 200, 0, 255));
	CP_Graphics_DrawRect(sheetLeft + (currentFrame + 0.5f) * frameSize / 2.0f, 16.0f + frameSize / 4.0f, frameSize / 2.0f, frameSize / 2.0f);
	CP_Settings_NoStroke();

	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	char text[100];
	snprintf(text, sizeof(text), "Frame %d of %d", currentFrame, FRAME_COUNT - 1);
	CP_Font_DrawText(text, 20.0f, 30.0f);
	snprintf(text, sizeof(text), "Easing (1 to 5): %s", easingNames[easing]);
	CP_Font_DrawText(text, 20.0f, 60.0f);
	CP_Settings_Fill(CP_Color_Create(200, 200, 200, 255));
	CP_Font_DrawText("Walk: WASD    Click: send an orb", 20.0f, 585.0f);
}

void game_exit(void)
{
	// Free what init loaded
	CP_Image_Free(&slimeSheet);
}

int main(void)
{
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
