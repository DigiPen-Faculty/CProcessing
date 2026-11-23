//---------------------------------------------------------
// file:	sprite_animation.c
//
// Example: animating a character from a sprite sheet.
//
// Shows how to:
// - load an image once, in init, and free it in exit
// - draw one frame of a sprite sheet with CP_Image_DrawSubImage
// - change frames at a steady speed with a timer and CP_System_GetDt
// - flash the sprite white, or any color, when it's hurt: a white copy of
//   the sheet (MakeSilhouette below: copy it into your game) drawn over the
//   sprite, fading out, and colored with CP_Settings_Tint
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
#include <stdlib.h> // for malloc and free

// The sprite sheet: 8 frames in a row, each 64x64 pixels
CP_Image slimeSheet;
CP_Image slimeWhite; // the same sheet, every pixel white, for the flash
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

// A copy of an image with every pixel white, keeping how see-through each
// one is: the sprite's shape, and nothing else. It's made once, when the
// image is loaded, and freed with it.
// Drawn over the sprite with an alpha, it moves every pixel's color that
// far toward white. A tint colors it: a tint multiplies, and white times a
// color is that color. (A tint on the sprite itself can only darken it, so
// it can't flash it white, and red on a green slime only makes it dark.)
CP_Image MakeSilhouette(CP_Image image)
{
	const int width = CP_Image_GetWidth(image);
	const int height = CP_Image_GetHeight(image);
	if (width == 0 || height == 0)
	{
		return NULL; // the image didn't load
	}

	CP_Color* pixels = (CP_Color*)malloc((size_t)width * (size_t)height * sizeof(CP_Color));
	if (!pixels)
	{
		return NULL;
	}
	CP_Image_GetPixelData(image, pixels);
	for (int i = 0; i < width * height; ++i)
	{
		pixels[i].r = 255;
		pixels[i].g = 255;
		pixels[i].b = 255;
	}
	CP_Image silhouette = CP_Image_CreateFromData(width, height, pixels); // it copies the pixels
	free(pixels);
	return silhouette;
}

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	CP_System_SetWindowTitle("Sprite animation");

	// Load images once, here, not in game_update (which runs every frame)
	slimeSheet = CP_Image_Load("assets/SlimeSheet.png");
	slimeWhite = MakeSilhouette(slimeSheet);
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

	// Draw that frame big, 4 times its size, in the middle of the window
	CP_Image_DrawSubImage(slimeSheet, 400.0f, 340.0f, frameSize * 4.0f, frameSize * 4.0f, frameLeft, frameTop, frameRight, frameBottom, 255);

	// The flash: the same frame of the white sheet, in the same place, over
	// the sprite. Its alpha falls from 255 to 0 as the flash ends, so it
	// fades out. The tint gives it the flash's color, and NoTint after it
	// keeps the tint off everything drawn later.
	if (hurtTimer > 0.0f)
	{
		int alpha = (int)(255.0f * hurtTimer / hurtDuration);
		CP_Settings_Tint(hurtColor);
		CP_Image_DrawSubImage(slimeWhite, 400.0f, 340.0f, frameSize * 4.0f, frameSize * 4.0f, frameLeft, frameTop, frameRight, frameBottom, alpha);
		CP_Settings_NoTint();
	}

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
	CP_Image_Free(&slimeWhite);
}

int main(void)
{
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
