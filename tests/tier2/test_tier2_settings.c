// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors

// Assertions against the pixel buffers tier2_capture.c already captured.
// See tier2_capture.c for the worked coordinate geometry behind each
// sample point used here.
#include "unity.h"
#include "tier2_capture.h"

static void assertColorWithin(unsigned char tolerance, CP_Color expected, CP_Color actual)
{
	TEST_ASSERT_UINT8_WITHIN(tolerance, expected.r, actual.r);
	TEST_ASSERT_UINT8_WITHIN(tolerance, expected.g, actual.g);
	TEST_ASSERT_UINT8_WITHIN(tolerance, expected.b, actual.b);
}

// Fill and rect mode were set before CP_Engine_Run (tier2_RunCaptureOnce);
// see Scn_SettingsBeforeRun
void test_tier2_settings_made_before_run_are_kept(void)
{
	const CP_Color fill = CP_Color_Create(12, 200, 34, 255);
	const CP_Color blue = CP_Color_Create(0, 0, 255, 255);

	// The fill color set before CP_Engine_Run
	assertColorWithin(2, fill, tier2_SamplePixel(SCN_SETTINGS_BEFORE_RUN, 100, 100));
	// CORNER mode set before CP_Engine_Run: (50, 50) is the top left corner...
	assertColorWithin(2, fill, tier2_SamplePixel(SCN_SETTINGS_BEFORE_RUN, 140, 140));
	// ...not the center, which would have covered this point instead
	assertColorWithin(2, blue, tier2_SamplePixel(SCN_SETTINGS_BEFORE_RUN, 25, 25));
	// The default stroke, black and 3 wide, centered on the left edge (x = 50)
	assertColorWithin(2, CP_Color_Create(0, 0, 0, 255), tier2_SamplePixel(SCN_SETTINGS_BEFORE_RUN, 49, 100));
	assertColorWithin(2, fill, tier2_SamplePixel(SCN_SETTINGS_BEFORE_RUN, 54, 100));
}

void test_tier2_settings_default_wrap_mode_is_clamp_to_edge(void)
{
	// Past the bottom right corner of the image: its corner pixel, yellow,
	// stretched out. With CP_IMAGE_WRAP_CLAMP it would be see-through (the
	// blue background); with REPEAT or MIRROR, another color of the image.
	assertColorWithin(2, CP_Color_Create(255, 255, 0, 255), tier2_SamplePixel(SCN_SETTINGS_BEFORE_RUN, 188, 188));
	// Past the right edge beside the top row: green, the top right pixel
	assertColorWithin(2, CP_Color_Create(0, 255, 0, 255), tier2_SamplePixel(SCN_SETTINGS_BEFORE_RUN, 188, 158));
}

void test_tier2_settings_nofill_leaves_interior_untouched(void)
{
	// Interior: still the ClearBackground color, since fill is off.
	assertColorWithin(2, CP_Color_Create(0, 200, 0, 255), tier2_SamplePixel(SCN_SETTINGS_NOFILL, 100, 100));
	// Edge: the stroke is still drawn.
	assertColorWithin(2, CP_Color_Create(0, 0, 255, 255), tier2_SamplePixel(SCN_SETTINGS_NOFILL, 100, 58));
}

void test_tier2_settings_strokeweight_widens_the_line(void)
{
	// 12px off the line's centerline; only a 30-wide (half-width 15) stroke reaches this far.
	assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_SETTINGS_STROKEWEIGHT, 112, 100));
	assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_SETTINGS_STROKEWEIGHT, 140, 100));
}

void test_tier2_settings_rectmode_corner_changes_interpretation(void)
{
	assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_SETTINGS_RECTMODE_CORNER, 70, 70));
	// Would be inside if (x,y) were still being treated as the center.
	assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_SETTINGS_RECTMODE_CORNER, 30, 30));
}

void test_tier2_settings_ellipsemode_corner_changes_interpretation(void)
{
	// Only inside the circle of radius 40 centered at (90,90) -- the
	// CORNER-mode interpretation of DrawEllipse(50,50,80,80) -- and not
	// inside the CENTER-mode interpretation (center (50,50)).
	assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_SETTINGS_ELLIPSEMODE_CORNER, 115, 90));
	assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_SETTINGS_ELLIPSEMODE_CORNER, 10, 10));
}

void test_tier2_settings_translate_moves_subsequent_draws(void)
{
	assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_SETTINGS_TRANSLATE, 50, 50));
	assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_SETTINGS_TRANSLATE, 150, 150));
}

void test_tier2_settings_scale_resizes_subsequent_draws(void)
{
	assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_SETTINGS_SCALE, 115, 100));
	assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_SETTINGS_SCALE, 135, 100));
}

void test_tier2_settings_rotate_rotates_subsequent_draws(void)
{
	assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_SETTINGS_ROTATE, 114, 114));
	// Where the rect would be if the rotation hadn't been applied.
	assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_SETTINGS_ROTATE, 125, 100));
}

void test_tier2_settings_resetmatrix_cancels_prior_transform(void)
{
	assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_SETTINGS_RESETMATRIX, 10, 10));
	// Where the rect would be if the translate had NOT been canceled.
	assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_SETTINGS_RESETMATRIX, 50, 50));
}

void test_tier2_settings_applymatrix_applies_the_given_matrix(void)
{
	assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_SETTINGS_APPLYMATRIX, 70, 70));
	assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_SETTINGS_APPLYMATRIX, 150, 150));
}

void test_tier2_settings_blendmode_add_sums_overlapping_colors(void)
{
	assertColorWithin(5, CP_Color_Create(100, 100, 0, 255), tier2_SamplePixel(SCN_SETTINGS_BLENDMODE_ADD, 100, 100));
	assertColorWithin(5, CP_Color_Create(100, 0, 0, 255), tier2_SamplePixel(SCN_SETTINGS_BLENDMODE_ADD, 60, 100));
	assertColorWithin(5, CP_Color_Create(0, 100, 0, 255), tier2_SamplePixel(SCN_SETTINGS_BLENDMODE_ADD, 140, 100));
}

void test_tier2_settings_blend_modes_with_equations(void)
{
	// See Scn_SettingsBlendModes: background (200, 100, 50), square (100, 100, 100)
	// Subtract: background minus square, clamped at 0
	assertColorWithin(3, CP_Color_Create(100, 0, 0, 255), tier2_SamplePixel(SCN_SETTINGS_BLEND_MODES, 40, 100));
	// Multiply: background * square / 255
	assertColorWithin(3, CP_Color_Create(78, 39, 20, 255), tier2_SamplePixel(SCN_SETTINGS_BLEND_MODES, 80, 100));
	// Min and max, per channel
	assertColorWithin(3, CP_Color_Create(100, 100, 50, 255), tier2_SamplePixel(SCN_SETTINGS_BLEND_MODES, 120, 100));
	assertColorWithin(3, CP_Color_Create(200, 100, 100, 255), tier2_SamplePixel(SCN_SETTINGS_BLEND_MODES, 160, 100));
}

void test_tier2_settings_tint_multiplies_shapes_and_images(void)
{
	// See Scn_SettingsTint
	assertColorWithin(3, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_SETTINGS_TINT, 50, 50));
	// Blue at alpha 128 tints halfway: 1 + (0 - 1) * 128/255 of red and green
	assertColorWithin(3, CP_Color_Create(127, 127, 255, 255), tier2_SamplePixel(SCN_SETTINGS_TINT, 150, 50));
	assertColorWithin(3, CP_Color_Create(0, 255, 0, 255), tier2_SamplePixel(SCN_SETTINGS_TINT, 100, 150));
}

void test_tier2_settings_carry_over_to_the_next_frame(void)
{
	// See Scn_SettingsCarryToNextFrame: the text size set in the previous
	// frame still applies, so the capital H is 150 * 690/1000 = 103.5
	// pixels tall
	int top = -1, bottom = -1;
	for (int y = 0; y < TIER2_CANVAS_SIZE; ++y)
	{
		for (int x = 0; x < TIER2_CANVAS_SIZE; ++x)
		{
			if (tier2_SamplePixel(SCN_SETTINGS_CARRY_TO_NEXT_FRAME, x, y).r < 128)
			{
				if (top < 0) top = y;
				bottom = y;
				break;
			}
		}
	}
	TEST_ASSERT_GREATER_OR_EQUAL_INT(0, top);
	TEST_ASSERT_INT_WITHIN(2, 104, bottom - top + 1);
}

void test_tier2_settings_save_restore_round_trips_fill_and_transform(void)
{
	// If fill weren't restored this would be blue; if the transform
	// weren't restored the rect would have moved to world (150,150),
	// making this background instead.
	assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_SETTINGS_SAVE_RESTORE, 100, 100));
	assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_SETTINGS_SAVE_RESTORE, 150, 150));
}

// ---- Getters ----

void test_tier2_settings_getters_give_back_what_was_set(void)
{
	TEST_ASSERT_TRUE(tier2_scalars.fillGetsWhatWasSet);
	TEST_ASSERT_TRUE(tier2_scalars.strokeGetsWhatWasSet);
	TEST_ASSERT_TRUE(tier2_scalars.strokeWeightGetsWhatWasSet);
	TEST_ASSERT_TRUE(tier2_scalars.textSizeGetsWhatWasSet);
}

void test_tier2_settings_fill_and_stroke_colors_are_kept_by_nofill_and_nostroke(void)
{
	TEST_ASSERT_TRUE(tier2_scalars.fillKeptAfterNoFill);
	TEST_ASSERT_TRUE(tier2_scalars.strokeKeptAfterNoStroke);
}

void test_tier2_settings_restore_brings_back_what_the_getters_return(void)
{
	TEST_ASSERT_TRUE(tier2_scalars.settingsBackAfterRestore);
}

void test_tier2_settings_getfill_works_before_run(void)
{
	TEST_ASSERT_EQUAL_UINT8(12, tier2_scalars.fillBeforeRun.r);
	TEST_ASSERT_EQUAL_UINT8(200, tier2_scalars.fillBeforeRun.g);
	TEST_ASSERT_EQUAL_UINT8(34, tier2_scalars.fillBeforeRun.b);
}

// ---- Clip ----
// See the Clip scenarios in tier2_capture.c: each draws a red canvas through a
// clip, so red shows where the clip lets drawing through.

static const CP_Color kRed = { 255, 0, 0, 255 };
static const CP_Color kWhite = { 255, 255, 255, 255 };
static const CP_Color kBlue = { 0, 0, 255, 255 };

// Pixels in a region that are dark (text drawn in black)
static int countDark(int scenario, int x0, int y0, int x1, int y1)
{
	int count = 0;
	for (int y = y0; y <= y1; ++y)
	{
		for (int x = x0; x <= x1; ++x)
		{
			const CP_Color p = tier2_SamplePixel(scenario, x, y);
			if (p.r < 128 && p.g < 128 && p.b < 128)
			{
				++count;
			}
		}
	}
	return count;
}

void test_tier2_settings_clip_corner_cuts_off_drawing_outside(void)
{
	assertColorWithin(2, kRed, tier2_SamplePixel(SCN_CLIP_CORNER, 100, 100));
	assertColorWithin(2, kRed, tier2_SamplePixel(SCN_CLIP_CORNER, 53, 53));
	assertColorWithin(2, kRed, tier2_SamplePixel(SCN_CLIP_CORNER, 147, 147));
	assertColorWithin(2, kWhite, tier2_SamplePixel(SCN_CLIP_CORNER, 46, 100));
	assertColorWithin(2, kWhite, tier2_SamplePixel(SCN_CLIP_CORNER, 154, 100));
	assertColorWithin(2, kWhite, tier2_SamplePixel(SCN_CLIP_CORNER, 100, 46));
	assertColorWithin(2, kWhite, tier2_SamplePixel(SCN_CLIP_CORNER, 100, 154));
}

void test_tier2_settings_clip_follows_rectmode_center(void)
{
	assertColorWithin(2, kRed, tier2_SamplePixel(SCN_CLIP_CENTER, 100, 100));
	assertColorWithin(2, kRed, tier2_SamplePixel(SCN_CLIP_CENTER, 73, 73));
	assertColorWithin(2, kRed, tier2_SamplePixel(SCN_CLIP_CENTER, 127, 127));
	assertColorWithin(2, kWhite, tier2_SamplePixel(SCN_CLIP_CENTER, 66, 100));
	assertColorWithin(2, kWhite, tier2_SamplePixel(SCN_CLIP_CENTER, 134, 100));
}

void test_tier2_settings_clip_with_a_negative_size_is_the_same_rectangle(void)
{
	assertColorWithin(2, kRed, tier2_SamplePixel(SCN_CLIP_NEGATIVE_SIZE, 100, 100));
	assertColorWithin(2, kRed, tier2_SamplePixel(SCN_CLIP_NEGATIVE_SIZE, 53, 53));
	assertColorWithin(2, kWhite, tier2_SamplePixel(SCN_CLIP_NEGATIVE_SIZE, 46, 100));
	assertColorWithin(2, kWhite, tier2_SamplePixel(SCN_CLIP_NEGATIVE_SIZE, 154, 154));
}

void test_tier2_settings_clip_stays_put_while_drawing_moves_under_it(void)
{
	assertColorWithin(2, kRed, tier2_SamplePixel(SCN_CLIP_STAYS_PUT, 125, 100));   // moved drawing, inside the clip
	assertColorWithin(2, kWhite, tier2_SamplePixel(SCN_CLIP_STAYS_PUT, 75, 100));  // inside the clip, but the drawing moved away
	assertColorWithin(2, kWhite, tier2_SamplePixel(SCN_CLIP_STAYS_PUT, 165, 100)); // the drawing, but outside the clip
}

void test_tier2_settings_clip_replaces_the_last_one_and_noclip_removes_it(void)
{
	assertColorWithin(2, kWhite, tier2_SamplePixel(SCN_CLIP_REPLACE_AND_NOCLIP, 50, 50)); // the first clip
	assertColorWithin(2, kRed, tier2_SamplePixel(SCN_CLIP_REPLACE_AND_NOCLIP, 150, 150)); // the second
	assertColorWithin(2, kWhite, tier2_SamplePixel(SCN_CLIP_REPLACE_AND_NOCLIP, 50, 150));
	assertColorWithin(2, kBlue, tier2_SamplePixel(SCN_CLIP_REPLACE_AND_NOCLIP, 175, 25)); // after NoClip
}

void test_tier2_settings_restore_brings_back_the_clip(void)
{
	assertColorWithin(2, kRed, tier2_SamplePixel(SCN_CLIP_SAVE_RESTORE, 50, 100));
	assertColorWithin(2, kWhite, tier2_SamplePixel(SCN_CLIP_SAVE_RESTORE, 150, 100));
}

void test_tier2_settings_clip_set_after_rotate_is_turned(void)
{
	assertColorWithin(2, kRed, tier2_SamplePixel(SCN_CLIP_TURNED, 100, 100));
	assertColorWithin(2, kRed, tier2_SamplePixel(SCN_CLIP_TURNED, 100, 165)); // toward a corner of the diamond
	assertColorWithin(2, kRed, tier2_SamplePixel(SCN_CLIP_TURNED, 35, 100));
	assertColorWithin(2, kWhite, tier2_SamplePixel(SCN_CLIP_TURNED, 145, 145)); // inside the square unturned
	assertColorWithin(2, kWhite, tier2_SamplePixel(SCN_CLIP_TURNED, 100, 178));
}

void test_tier2_settings_clip_cuts_images_and_text(void)
{
	assertColorWithin(4, kRed, tier2_SamplePixel(SCN_CLIP_TEXT_AND_IMAGE, 50, 25));    // the image's red quarter
	assertColorWithin(2, kWhite, tier2_SamplePixel(SCN_CLIP_TEXT_AND_IMAGE, 150, 25)); // its green quarter, cut off
	assertColorWithin(2, kWhite, tier2_SamplePixel(SCN_CLIP_TEXT_AND_IMAGE, 150, 75)); // its yellow quarter, cut off
	TEST_ASSERT_TRUE(countDark(SCN_CLIP_TEXT_AND_IMAGE, 0, 100, 99, 199) > 100);       // the first H
	TEST_ASSERT_EQUAL_INT(0, countDark(SCN_CLIP_TEXT_AND_IMAGE, 103, 100, 199, 199));  // the second, cut off
}

void test_tier2_settings_clearbackground_clears_the_whole_window_inside_a_clip(void)
{
	assertColorWithin(2, kRed, tier2_SamplePixel(SCN_CLIP_CLEAR_BACKGROUND, 50, 100));
	assertColorWithin(2, kBlue, tier2_SamplePixel(SCN_CLIP_CLEAR_BACKGROUND, 150, 100));
}

void test_tier2_settings_drawing_into_an_image_is_unclipped_and_the_clip_comes_back(void)
{
	assertColorWithin(2, kRed, tier2_SamplePixel(SCN_CLIP_IMAGE_DRAWING, 145, 45)); // all of the image is red
	assertColorWithin(2, kRed, tier2_SamplePixel(SCN_CLIP_IMAGE_DRAWING, 165, 65));
	assertColorWithin(2, kWhite, tier2_SamplePixel(SCN_CLIP_IMAGE_DRAWING, 45, 45)); // clipped again after EndDrawing
}

void test_tier2_settings_clip_is_gone_in_the_next_frame(void)
{
	assertColorWithin(2, kRed, tier2_SamplePixel(SCN_CLIP_LEFT_SET, 10, 10));
	assertColorWithin(2, kWhite, tier2_SamplePixel(SCN_CLIP_LEFT_SET, 100, 100));
	assertColorWithin(2, kRed, tier2_SamplePixel(SCN_CLIP_GONE_NEXT_FRAME, 100, 100));
	assertColorWithin(2, kRed, tier2_SamplePixel(SCN_CLIP_GONE_NEXT_FRAME, 190, 190));
}

void test_tier2_settings_clip_doesnt_reach_the_next_game_state(void)
{
	TEST_ASSERT_TRUE(tier2_scalars.clipStateBelowRanFirst);
	assertColorWithin(2, kRed, tier2_SamplePixel(SCN_CLIP_NEW_STATE, 10, 100));
	assertColorWithin(2, kRed, tier2_SamplePixel(SCN_CLIP_NEW_STATE, 100, 100));
}
