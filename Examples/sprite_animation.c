//---------------------------------------------------------
// file:	sprite_animation.c
//
// Example: animating a character from a sprite sheet.
//
// Shows how to:
// - load an image once, in init, and free it in exit
// - draw one frame of a sprite sheet with CP_Image_DrawSubImage
// - change frames at a steady speed with a timer and CP_System_GetDt
//
// Assets/SlimeSheet.png holds 8 frames in one row, each 64x64 pixels. The
// whole sheet is drawn at the top, with a box around the frame being shown.
// The Up and Down keys change the animation speed.
//
// Try this:
// - Move the slime with the arrow keys, and only animate while it moves.
// - Make the animation play once when a key is pressed, then stop.
//
// To run it, copy this file over the template's main.c. It needs
// SlimeSheet.png in the Assets folder, which the template includes.
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

// The sprite sheet: 8 frames in a row, each 64x64 pixels
CP_Image slimeSheet;
const int frameCount = 8;
const float frameSize = 64.0f;

// The animation: which frame is showing, and for how long it has shown
int currentFrame = 0;
float frameTimer = 0.0f;
float framesPerSecond = 10.0f;

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Sprite animation");

	// Load images once, here, not in game_update (which runs every frame)
	slimeSheet = CP_Image_Load("Assets/SlimeSheet.png");
}

void game_update(void)
{
	// Up and Down change the speed, from 1 to 30 frames per second
	if (CP_Input_KeyTriggered(KEY_UP))
	{
		framesPerSecond = CP_Math_ClampFloat(framesPerSecond + 1.0f, 1.0f, 30.0f);
	}
	if (CP_Input_KeyTriggered(KEY_DOWN))
	{
		framesPerSecond = CP_Math_ClampFloat(framesPerSecond - 1.0f, 1.0f, 30.0f);
	}

	// Each frame shows for 1 / framesPerSecond seconds. Add up the time
	// since the last frame, and when it's enough, move on to the next frame,
	// back to the first after the last.
	frameTimer += CP_System_GetDt();
	float frameDuration = 1.0f / framesPerSecond;
	while (frameTimer >= frameDuration)
	{
		frameTimer -= frameDuration;
		currentFrame = (currentFrame + 1) % frameCount;
	}

	CP_Graphics_ClearBackground(CP_Color_Create(40, 40, 40, 255));

	// The part of the sheet to draw, in the sheet's pixels: from (u0, v0) at
	// its top left to (u1, v1) at its bottom right. Frame 0 is 0 to 64
	// across, frame 1 is 64 to 128, and so on.
	float u0 = currentFrame * frameSize;
	float v0 = 0.0f;
	float u1 = u0 + frameSize;
	float v1 = frameSize;

	// Draw that frame big, 4 times its size, in the middle of the window
	CP_Image_DrawSubImage(slimeSheet, 400.0f, 340.0f, frameSize * 4.0f, frameSize * 4.0f, u0, v0, u1, v1, 255);

	// The whole sheet at the top, and a box around the current frame
	float sheetLeft = 400.0f - frameCount * frameSize / 2.0f;
	CP_Image_Draw(slimeSheet, 400.0f, 80.0f, frameCount * frameSize, frameSize, 255);
	CP_Settings_NoFill();
	CP_Settings_Stroke(CP_Color_Create(255, 200, 0, 255));
	CP_Graphics_DrawRect(sheetLeft + u0 + frameSize / 2.0f, 80.0f, frameSize, frameSize);

	// What's happening, at the bottom
	char text[100];
	snprintf(text, sizeof(text), "Frame %d", currentFrame);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Font_DrawText(text, 20.0f, 550.0f);
	snprintf(text, sizeof(text), "(frames are numbered 0 to %d), %.0f frames per second (Up and Down to change)",
		frameCount - 1, framesPerSecond);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Font_DrawText(text, 20.0f, 570.0f);
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
