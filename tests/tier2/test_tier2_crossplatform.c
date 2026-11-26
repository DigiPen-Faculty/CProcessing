// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors

// Tier 2 checks added with the cross-platform work: behavior every platform
// must share (drawing persisting through the offscreen canvas, screenshots of
// sub-regions, the native window handle), plus error paths that used to
// crash. See the matching Scn_* functions in tier2_capture.c.
#include "unity.h"
#include "tier2_capture.h"

static void assertColorWithin(unsigned char tolerance, CP_Color expected, CP_Color actual)
{
	TEST_ASSERT_UINT8_WITHIN(tolerance, expected.r, actual.r);
	TEST_ASSERT_UINT8_WITHIN(tolerance, expected.g, actual.g);
	TEST_ASSERT_UINT8_WITHIN(tolerance, expected.b, actual.b);
}

#define RED_C    CP_Color_Create(255, 0, 0, 255)
#define GREEN_C  CP_Color_Create(0, 200, 0, 255)
#define BLUE_C   CP_Color_Create(0, 0, 255, 255)
#define YELLOW_C CP_Color_Create(255, 255, 0, 255)
#define WHITE_C  CP_Color_Create(255, 255, 255, 255)

void test_tier2_drawing_persists_into_the_next_frame(void)
{
	// Frame 1 drew the red square; frame 2 drew only the blue one and did
	// not clear, so the red square must still be there.
	assertColorWithin(2, RED_C, tier2_SamplePixel(SCN_PERSIST_NEXT_FRAME, 50, 50));
	assertColorWithin(2, BLUE_C, tier2_SamplePixel(SCN_PERSIST_NEXT_FRAME, 150, 150));
	assertColorWithin(2, WHITE_C, tier2_SamplePixel(SCN_PERSIST_NEXT_FRAME, 150, 50));
}

void test_tier2_first_frame_did_not_contain_the_later_drawing(void)
{
	// Sanity check on the persistence test: the blue square really was
	// drawn only in the second frame.
	assertColorWithin(2, RED_C, tier2_SamplePixel(SCN_PERSIST_FIRST_FRAME, 50, 50));
	assertColorWithin(2, WHITE_C, tier2_SamplePixel(SCN_PERSIST_FIRST_FRAME, 150, 150));
}

void test_tier2_screenshot_subregion_has_requested_size(void)
{
	TEST_ASSERT_EQUAL_INT(100, tier2_scalars.subTopRightWidth);
	TEST_ASSERT_EQUAL_INT(100, tier2_scalars.subTopRightHeight);
}

void test_tier2_screenshot_subregion_reads_the_right_place(void)
{
	assertColorWithin(2, GREEN_C, tier2_scalars.subTopRightCenter);
	assertColorWithin(2, BLUE_C, tier2_scalars.subBottomLeftCenter);
}

void test_tier2_screenshot_subregion_is_not_flipped(void)
{
	// A region straddling all four quadrants keeps them in place: red top
	// left, green top right, blue bottom left, yellow bottom right.
	assertColorWithin(2, RED_C, tier2_scalars.straddleCorners[0]);
	assertColorWithin(2, GREEN_C, tier2_scalars.straddleCorners[1]);
	assertColorWithin(2, BLUE_C, tier2_scalars.straddleCorners[2]);
	assertColorWithin(2, YELLOW_C, tier2_scalars.straddleCorners[3]);
}

void test_tier2_missing_files_load_as_null(void)
{
	TEST_ASSERT_TRUE(tier2_scalars.missingImageIsNull);
	TEST_ASSERT_TRUE(tier2_scalars.missingFontIsNull);
	TEST_ASSERT_TRUE(tier2_scalars.missingSoundIsNull);
}

void test_tier2_null_handles_are_ignored(void)
{
	TEST_ASSERT_TRUE(tier2_scalars.survivedNullCalls);
}

// The messages (DESIGN.md, "Messages") are checked by their start, which
// names the function and the file, and by how many there are
#define ASSERT_SAID(times, messages, start) TEST_ASSERT_EQUAL_INT_MESSAGE(times, tier2_CountText(messages, start), messages)

void test_tier2_loads_before_run_say_so(void)
{
	const char* messages = tier2_scalars.messageLoadBeforeRun;
	TEST_ASSERT_TRUE(tier2_scalars.loadsBeforeRunAreNull);
	ASSERT_SAID(1, messages, "CProcessing: CP_Image_Load was called before CP_Engine_Run");
	ASSERT_SAID(1, messages, "CProcessing: CP_Sound_Load was called before CP_Engine_Run");
	ASSERT_SAID(1, messages, "CProcessing: CP_Sound_LoadStream was called before CP_Engine_Run");
	ASSERT_SAID(1, messages, "CProcessing: CP_Font_Load was called before CP_Engine_Run");
	ASSERT_SAID(4, messages, "CProcessing: ");
}

void test_tier2_failed_loads_say_why_once_per_file(void)
{
	const char* messages = tier2_scalars.messageErrorPaths;
	const int sound = tier2_scalars.errorPathsSoundAvailable ? 1 : 0;
	TEST_ASSERT_TRUE(tier2_scalars.unreadableFilesAreNull);

	// Not there: the image was loaded twice, and is named once
	ASSERT_SAID(1, messages, "CProcessing: CP_Image_Load couldn't find \"assets/this_file_does_not_exist.png\"");
	ASSERT_SAID(1, messages, "CProcessing: CP_Font_Load couldn't find \"assets/this_file_does_not_exist.ttf\"");
	ASSERT_SAID(sound, messages, "CProcessing: CP_Sound_Load couldn't find \"assets/this_file_does_not_exist.wav\"");
	ASSERT_SAID(2 + sound, messages, "Check the spelling, upper and lower case included");

	// No name (the image's was said in its own scenario, for "")
	ASSERT_SAID(1, messages, "CProcessing: CP_Font_Load was given no file name");
	ASSERT_SAID(sound, messages, "CProcessing: CP_Sound_Load was given no file name");
	ASSERT_SAID(sound, messages, "CProcessing: CP_Sound_LoadStream was given no file name");
	ASSERT_SAID(0, messages, "CProcessing: CP_Image_Load was given no file name");

	// There, but not the kind asked for
	ASSERT_SAID(1, messages, "CProcessing: CP_Image_Load found \"assets/beep.wav\" but couldn't read it as an image");
	ASSERT_SAID(1, messages, "CProcessing: CP_Font_Load found \"assets/quadrants.png\" but couldn't read it as a font");
	ASSERT_SAID(sound, messages, "CProcessing: CP_Sound_Load found \"assets/quadrants.png\" but couldn't read it as a sound");
}

void test_tier2_null_handles_say_so_once(void)
{
	const char* messages = tier2_scalars.messageErrorPaths;
	const int sound = tier2_scalars.errorPathsSoundAvailable ? 1 : 0;
	ASSERT_SAID(1, messages, "CProcessing: CP_Image_Draw was given no image (NULL)");
	ASSERT_SAID(1, messages, "CProcessing: CP_Font_Set was given no font (NULL)");
	ASSERT_SAID(sound, messages, "CProcessing: CP_Sound_Play was given no sound (NULL)");
	ASSERT_SAID(sound, messages, "CProcessing: CP_Sound_PlayAdvanced was given no sound (NULL)");

	// Freeing NULL is fine (a Free sets the variable to NULL, so freeing
	// twice is too): every message in the scenario is one of those above
	ASSERT_SAID(1, messages, "CProcessing: CP_Font_Load was given no file name");
	ASSERT_SAID(7 + sound * 6, messages, "CProcessing: ");
}

void test_tier2_drawing_before_run_says_so(void)
{
	const char* messages = tier2_scalars.messageDrawBeforeRun;
	ASSERT_SAID(1, messages, "CProcessing: CP_Settings_Translate was called before CP_Engine_Run");
	ASSERT_SAID(1, messages, "CProcessing: CP_Settings_Rotate was called before CP_Engine_Run");
	ASSERT_SAID(1, messages, "CProcessing: CP_Graphics_ClearBackground was called before CP_Engine_Run");
	ASSERT_SAID(1, messages, "CProcessing: CP_Graphics_DrawRect was called before CP_Engine_Run");
	ASSERT_SAID(1, messages, "CProcessing: CP_Graphics_DrawCircle was called before CP_Engine_Run");
	ASSERT_SAID(1, messages, "CProcessing: CP_Graphics_BeginShape was called before CP_Engine_Run");
	// the points and EndShape belong to the shape BeginShape already spoke
	// for; Save and Restore before the window opens are harmless
	ASSERT_SAID(6, messages, "CProcessing: ");
}

void test_tier2_nan_numbers_are_ignored_and_said(void)
{
	const char* messages = tier2_scalars.messageDrawing;
	ASSERT_SAID(1, messages, "CProcessing: CP_Graphics_DrawRect's y isn't a number (NaN), so the call was ignored");
	ASSERT_SAID(1, messages, "CProcessing: CP_Graphics_DrawCircle's x is infinite, so the call was ignored");
	ASSERT_SAID(1, messages, "CProcessing: CP_Settings_Translate's x isn't a number (NaN)");
	ASSERT_SAID(1, messages, "CProcessing: CP_Settings_StrokeWeight's weight isn't a number (NaN)");
	ASSERT_SAID(4, messages, "division by zero");

	// The NaN stroke weight was ignored, and so was the NaN translate: the
	// square drawn after it is still in the middle
	TEST_ASSERT_EQUAL_FLOAT(3.0f, tier2_scalars.drawingStrokeWeightKept);
	const CP_Color middle = tier2_SamplePixel(SCN_DRAWING_MESSAGES, 100, 100);
	TEST_ASSERT_EQUAL_UINT8(0, middle.r);
	TEST_ASSERT_EQUAL_UINT8(255, middle.b);
	const CP_Color outside = tier2_SamplePixel(SCN_DRAWING_MESSAGES, 100, 60);
	TEST_ASSERT_EQUAL_UINT8(255, outside.r);
}

void test_tier2_settings_mistakes_are_said(void)
{
	const char* messages = tier2_scalars.messageDrawing;
	TEST_ASSERT_TRUE(tier2_scalars.drawingZeroSizeImageIsNull);
	ASSERT_SAID(1, messages, "CProcessing: CP_Settings_RectMode was given 7, which isn't a position mode");
	ASSERT_SAID(1, messages, "CProcessing: CP_Settings_LineCapMode was given 99, which isn't one of its modes");
	ASSERT_SAID(1, messages, "CProcessing: CP_Settings_BlendMode was given 99, which isn't a blend mode");
	ASSERT_SAID(1, messages, "CProcessing: CP_Image_Create was given a size of 0 x 10, so it made no image (NULL)");
	ASSERT_SAID(1, messages, "CProcessing: CP_Settings_Restore was called without a CP_Settings_Save before it");
	ASSERT_SAID(1, messages, "CProcessing: CP_Font_DrawText was given no text (NULL)");
	// with the 4 NaN messages, that's all of them
	ASSERT_SAID(10, messages, "CProcessing: ");
}

void test_tier2_a_save_left_open_is_said_at_the_end_of_the_frame(void)
{
	const char* messages = tier2_scalars.messageSaveOpen;
	ASSERT_SAID(1, messages, "CProcessing: CP_Settings_Save had no CP_Settings_Restore by the end of the frame (1 Save open)");
	ASSERT_SAID(1, messages, "CProcessing: ");
}

void test_tier2_keys_and_buttons_that_arent_say_so(void)
{
	const char* messages = tier2_scalars.messageInput;
	TEST_ASSERT_TRUE(tier2_scalars.inputMistakesAllFalse);
	ASSERT_SAID(1, messages, "CProcessing: CP_Input_KeyDown was given 'a' (97), which isn't a key, so it gave false. A letter key is named by its capital: use CP_KEY_A.");
	ASSERT_SAID(1, messages, "CProcessing: CP_Input_KeyTriggered was given 500, which isn't a key");
	ASSERT_SAID(1, messages, "CProcessing: CP_Input_MouseDown was given 99, which isn't a mouse button");
	ASSERT_SAID(1, messages, "CProcessing: CP_Input_GamepadDown was given 99, which isn't a gamepad button");
	ASSERT_SAID(1, messages, "CProcessing: CP_Input_GamepadDownAdvanced was given gamepad 7, but gamepads are numbered 0 to 3");
	// CP_KEY_NONE said nothing
	ASSERT_SAID(5, messages, "CProcessing: ");

	const char* pastLast = tier2_scalars.messageGamepadPastLast;
	ASSERT_SAID(1, pastLast, "CProcessing: CP_Input_GamepadConnectedAdvanced was given gamepad 4, but gamepads are numbered 0 to 3");
	ASSERT_SAID(1, pastLast, "CProcessing: ");
}

void test_tier2_setting_the_running_state_says_to_use_forced(void)
{
	TEST_ASSERT_TRUE_MESSAGE(tier2_scalars.sameStateAsked, "the harness wasn't the only state");
	const char* messages = tier2_scalars.messageSameState;
	ASSERT_SAID(1, messages, "CProcessing: CP_Engine_SetNextGameState was given the state that is already running, so nothing happened");
	ASSERT_SAID(1, messages, "CP_Engine_SetNextGameStateForced");
	ASSERT_SAID(1, messages, "CProcessing: ");
}

void test_tier2_a_file_loaded_1000_times_says_so(void)
{
	const char* messages = tier2_scalars.messageLoadedOften;
	ASSERT_SAID(1, messages, "CProcessing: CP_Image_Load has loaded \"assets/quadrants.png\" 1000 times without a CP_Image_Free");
	ASSERT_SAID(1, messages, "CProcessing: ");
}

void test_tier2_assets_load_from_next_to_the_program(void)
{
	// See Scn_AssetsNextToProgram: relative paths, from an empty folder
	TEST_ASSERT_TRUE_MESSAGE(tier2_scalars.assetFolderEntered, "couldn't move to an empty folder");
	TEST_ASSERT_TRUE_MESSAGE(tier2_scalars.assetImageLoaded, "image");
	TEST_ASSERT_TRUE_MESSAGE(tier2_scalars.assetFontLoaded, "font");
	if (tier2_scalars.assetSoundAvailable)
	{
		TEST_ASSERT_TRUE_MESSAGE(tier2_scalars.assetSoundLoaded, "sound");
	}
}

void test_tier2_window_handle_is_available(void)
{
#if defined(_WIN32) || defined(__APPLE__)
	TEST_ASSERT_FALSE(tier2_scalars.windowHandleIsNull);
#else
	// Linux: the X11 window id under X11, but NULL under Wayland (there is
	// no global window handle there), so either is correct.
	TEST_PASS_MESSAGE("window handle may legitimately be NULL on Linux (Wayland)");
#endif
}
