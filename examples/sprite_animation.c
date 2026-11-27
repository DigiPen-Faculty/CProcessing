//---------------------------------------------------------
// file:	sprite_animation.c
//
// Example: animating a character from a sprite sheet.
//
// Shows how to:
// - load an image once, in init, and free it in exit
// - draw one frame of a sprite sheet with CP_Image_DrawSubImage
// - change frames at a steady speed with a timer and CP_System_GetDt
// - flash the sprite white, or any color, when it's hurt, fading out, with
//   CP_Settings_Overlay
//
// assets/SlimeSheet.png holds 8 frames in one row, each 64x64 pixels. The
// whole sheet is drawn at the top, with a box around the frame being shown.
// The Up and Down keys change the animation speed. Space hurts the slime,
// and it flashes white; R flashes it red.
//
// Try this:
// - Move the slime with the arrow keys, and only animate while it moves.
// - Make the animation play once when a key is pressed, then stop.
// - Flash the slime twice when it's hurt, or blink it on and off for a
//   second afterward, the way games show a player can't be hurt again yet.
//
// See also: rotation.c, for turning a sprite around its feet; lesson 7.
//
// To run it, copy this file over the template's main.c. It needs
// SlimeSheet.png in the assets folder, which the template includes.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
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

// The hurt flash: how long is left of it, in seconds, and its color
float hurtTimer = 0.0f;
const float hurtDuration = 0.4f;
CP_Color hurtColor;

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Sprite animation");

	// Load images once, here, not in game_update (which runs every frame)
	slimeSheet = CP_Image_Load("assets/SlimeSheet.png");
}

void game_update(void)
{
	// Up and Down change the speed, from 1 to 30 frames per second
	if (CP_Input_KeyTriggered(CP_KEY_UP))
	{
		framesPerSecond = CP_Math_ClampFloat(framesPerSecond + 1.0f, 1.0f, 30.0f);
	}
	if (CP_Input_KeyTriggered(CP_KEY_DOWN))
	{
		framesPerSecond = CP_Math_ClampFloat(framesPerSecond - 1.0f, 1.0f, 30.0f);
	}

	if (CP_Input_KeyTriggered(CP_KEY_SPACE))
	{
		hurtTimer = hurtDuration;
		hurtColor = CP_Color_Create(255, 255, 255, 255);
	}
	if (CP_Input_KeyTriggered(CP_KEY_R))
	{
		hurtTimer = hurtDuration;
		hurtColor = CP_Color_Create(255, 40, 40, 255);
	}
	if (hurtTimer > 0.0f)
	{
		hurtTimer -= CP_System_GetDt();
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

	// The part of the sheet to draw, as its edges in the sheet's pixels.
	// Frame 0 is 0 to 64 across, frame 1 is 64 to 128, and so on.
	float frameLeft = currentFrame * frameSize;
	float frameTop = 0.0f;
	float frameRight = frameLeft + frameSize;
	float frameBottom = frameSize;

	// The flash: while the slime is hurt, an overlay mixes the flash's color
	// into every pixel of it. The overlay's alpha is how much: it falls from
	// 255 to 0 as the flash ends, so the flash fades out. The see-through
	// pixels around the slime stay see-through. (A tint can't flash white: it
	// multiplies, so it can only darken, and red on a green slime is dark.)
	if (hurtTimer > 0.0f)
	{
		CP_Color flash = hurtColor;
		flash.a = (unsigned char)(255.0f * hurtTimer / hurtDuration);
		CP_Settings_Overlay(flash);
	}

	// Draw that frame big, 4 times its size, in the middle of the window
	CP_Image_DrawSubImage(slimeSheet, 400.0f, 340.0f, frameSize * 4.0f, frameSize * 4.0f, frameLeft, frameTop, frameRight, frameBottom, 255);

	// Like every setting, the overlay stays until it's changed: take it off,
	// so it doesn't flash everything drawn after the slime too
	CP_Settings_NoOverlay();

	// The whole sheet at the top, and a box around the current frame
	float sheetLeft = 400.0f - frameCount * frameSize / 2.0f;
	CP_Image_Draw(slimeSheet, 400.0f, 80.0f, frameCount * frameSize, frameSize, 255);
	CP_Settings_NoFill();
	CP_Settings_Stroke(CP_Color_Create(255, 200, 0, 255));
	CP_Graphics_DrawRect(sheetLeft + frameLeft + frameSize / 2.0f, 80.0f, frameSize, frameSize);

	// What's happening, at the bottom
	char text[100];
	snprintf(text, sizeof(text), "Frame %d", currentFrame);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Font_DrawText(text, 20.0f, 550.0f);
	snprintf(text, sizeof(text), "(frames are numbered 0 to %d), %.0f frames per second (Up and Down to change), Space or R: hurt",
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
