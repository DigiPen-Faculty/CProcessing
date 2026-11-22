// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors

// Bounding-box/occupancy checks rather than pixel-perfect glyph
// comparisons -- see Scn_FontDrawText / Scn_FontLoadFree in
// tier2_capture.c and the Tier 2 notes in 06-test-suite-plan.md for why.
#include <stdio.h>
#include "unity.h"
#include "tier2_capture.h"

static CP_BOOL RegionHasInk(Tier2Scenario scenario, int x0, int y0, int x1, int y1)
{
	for (int y = y0; y < y1; ++y)
	{
		for (int x = x0; x < x1; ++x)
		{
			CP_Color c = tier2_SamplePixel(scenario, x, y);
			if (c.r < 250 || c.g < 250 || c.b < 250)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

void test_tier2_font_drawtext_occupies_expected_region(void)
{
	// "I" drawn at (40,40), size 60, top-left aligned -- somewhere in a
	// generous box around that origin should have ink.
	TEST_ASSERT_TRUE(RegionHasInk(SCN_FONT_DRAWTEXT, 35, 40, 75, 100));
	// Nowhere near the far corner should.
	TEST_ASSERT_FALSE(RegionHasInk(SCN_FONT_DRAWTEXT, 150, 150, 195, 195));
}

void test_tier2_font_load_free_renders_with_custom_font(void)
{
	TEST_ASSERT_TRUE(RegionHasInk(SCN_FONT_LOAD_FREE, 35, 40, 75, 100));
	TEST_ASSERT_FALSE(RegionHasInk(SCN_FONT_LOAD_FREE, 150, 150, 195, 195));
}

void test_tier2_font_free_keeps_other_fonts_working(void)
{
	// See Scn_FontFreeKeepsOthers
	TEST_ASSERT_EQUAL_INT(3, tier2_scalars.fontFreeLoaded);
	TEST_ASSERT_GREATER_THAN_INT(300, tier2_scalars.fontFreeInkBefore);
	// Text in the two fonts loaded after the freed one looks the same
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, tier2_scalars.fontFreeMismatched,
		"pixels changed after freeing an earlier font");
	// And a font loaded after the free draws too
	TEST_ASSERT_TRUE(RegionHasInk(SCN_FONT_FREE_KEEPS_OTHERS, 40, 80, 160, 120));
}

// See Scn_FontSizeSweep: every frame of text animated through new sizes
// must look exactly like an immediate redraw of the same frame.
static void AssertSweepDrewEveryFrame(const Tier2TextSweep* sweep)
{
	char message[160];
	snprintf(message, sizeof message,
		"%d of %d frames drew the text wrong (first: frame %d, up to %d pixels off)",
		sweep->badFrames, sweep->frames, sweep->firstBadFrame, sweep->worstPixels);
	TEST_ASSERT_GREATER_THAN_INT(0, sweep->frames);
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, sweep->blankFrames, "the redraw itself had no text");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, sweep->badFrames, message);
}

void test_tier2_font_text_animated_by_size_draws_every_frame(void)
{
	AssertSweepDrewEveryFrame(&tier2_scalars.textSizeSweep);
}

void test_tier2_font_text_animated_by_scale_draws_every_frame(void)
{
	AssertSweepDrewEveryFrame(&tier2_scalars.textScaleSweep);
}

static CP_BOOL IsInk(CP_Color c)
{
	return c.r < 128;
}

// The height in pixels of the ink in a scenario's capture: the rows from
// the first with any ink to the last. 0 if there is none.
static int InkHeight(Tier2Scenario scenario)
{
	int top = -1, bottom = -1;
	for (int y = 0; y < TIER2_CANVAS_SIZE; ++y)
	{
		for (int x = 0; x < TIER2_CANVAS_SIZE; ++x)
		{
			if (IsInk(tier2_SamplePixel(scenario, x, y)))
			{
				if (top < 0) top = y;
				bottom = y;
				break;
			}
		}
	}
	return top < 0 ? 0 : bottom - top + 1;
}

void test_tier2_font_text_size_is_the_em_size(void)
{
	// See Scn_FontEmSize: a capital H at size 100 is 69 pixels tall
	TEST_ASSERT_INT_WITHIN(2, 69, InkHeight(SCN_FONT_EM_SIZE));
}

void test_tier2_font_measuring_text(void)
{
	// See Scn_FontEmSize: measured at size 100, in the same units as drawing
	TEST_ASSERT_GREATER_THAN_FLOAT(30.0f, tier2_scalars.textWidthH);
	TEST_ASSERT_FLOAT_WITHIN(2.0f, 2.0f * tier2_scalars.textWidthH, tier2_scalars.textWidthHH);
	// A space at the end still counts: it moves where the next text starts
	TEST_ASSERT_GREATER_THAN_FLOAT(tier2_scalars.textWidthH + 5.0f, tier2_scalars.textWidthHSpace);
	// Twice the size, twice the width
	TEST_ASSERT_FLOAT_WITHIN(2.0f, 2.0f * tier2_scalars.textWidthH, tier2_scalars.textWidthH200);
	// GetTextSize is the width and a line's height, the same for every string
	TEST_ASSERT_EQUAL_FLOAT(tier2_scalars.textWidthH, tier2_scalars.textSizeH.x);
	TEST_ASSERT_GREATER_THAN_FLOAT(69.0f, tier2_scalars.textSizeH.y);
	TEST_ASSERT_EQUAL_FLOAT(tier2_scalars.textSizeH.y, tier2_scalars.textSizeLong.y);
	TEST_ASSERT_GREATER_THAN_FLOAT(tier2_scalars.textSizeH.x, tier2_scalars.textSizeLong.x);
}

void test_tier2_font_measuring_a_text_box_counts_wrapped_lines(void)
{
	// One line in a wide box; the long text in a 300 wide box wraps onto
	// several lines, never wider than the box
	TEST_ASSERT_GREATER_THAN_FLOAT(0.0f, tier2_scalars.textBoxOneLine.x);
	TEST_ASSERT_GREATER_THAN_FLOAT(2.0f * tier2_scalars.textBoxOneLine.y, tier2_scalars.textBoxWrapped.y);
	TEST_ASSERT_LESS_OR_EQUAL_FLOAT(300.0f, tier2_scalars.textBoxWrapped.x);
}

void test_tier2_font_measuring_null_text_is_zero(void)
{
	TEST_ASSERT_EQUAL_FLOAT(0.0f, tier2_scalars.textWidthNull);
}

void test_tier2_font_opentype_cff_font_loads_and_draws(void)
{
	// See Scn_FontOpenType: the .otf build of Exo 2 draws like the .ttf
	TEST_ASSERT_TRUE(tier2_scalars.openTypeFontLoaded);
	TEST_ASSERT_INT_WITHIN(2, 69, InkHeight(SCN_FONT_OPENTYPE));
}

void test_tier2_font_large_text_draws(void)
{
	// See Scn_FontLarge: the middle of a 600-pixel @ covers the canvas
	int ink = 0;
	for (int y = 0; y < TIER2_CANVAS_SIZE; ++y)
		for (int x = 0; x < TIER2_CANVAS_SIZE; ++x)
			ink += IsInk(tier2_SamplePixel(SCN_FONT_LARGE, x, y));
	TEST_ASSERT_GREATER_THAN_INT(2000, ink);
}

// Counts the ink in a band of Scn_FontMirrored, and the pixels where the
// band doesn't match the plain text (rows 0-66) mirrored or flipped.
static void CompareBand(int band, int* ink, int* mismatched)
{
	*ink = 0;
	*mismatched = 0;
	for (int y = 0; y < 67; ++y)
	{
		for (int x = 0; x < TIER2_CANVAS_SIZE; ++x)
		{
			CP_BOOL plain = IsInk(tier2_SamplePixel(SCN_FONT_MIRRORED, x, y));
			CP_BOOL other;
			if (band == 1) // mirrored about x = 100, centered on y = 100
				other = IsInk(tier2_SamplePixel(SCN_FONT_MIRRORED, 199 - x, y + 67));
			else // flipped about y = 167
				other = IsInk(tier2_SamplePixel(SCN_FONT_MIRRORED, x, 199 - y));
			*ink += other;
			*mismatched += plain != other;
		}
	}
}

void test_tier2_font_text_mirrored_and_flipped_draws(void)
{
	// See Scn_FontMirrored. The plain text has ink...
	int plainInk = 0;
	for (int y = 0; y < 67; ++y)
		for (int x = 0; x < TIER2_CANVAS_SIZE; ++x)
			plainInk += IsInk(tier2_SamplePixel(SCN_FONT_MIRRORED, x, y));
	TEST_ASSERT_GREATER_THAN_INT(300, plainInk);

	// ...and the mirrored and flipped copies are its mirror images, give or
	// take antialiasing at the edges. (They used to have no ink at all.)
	for (int band = 1; band <= 2; ++band)
	{
		int ink, mismatched;
		CompareBand(band, &ink, &mismatched);
		char message[96];
		snprintf(message, sizeof message, "band %d: %d ink pixels, %d differ from the plain text (%d ink)",
			band, ink, mismatched, plainInk);
		TEST_ASSERT_GREATER_THAN_INT_MESSAGE(plainInk * 9 / 10, ink, message);
		TEST_ASSERT_LESS_THAN_INT_MESSAGE(plainInk / 10, mismatched, message);
	}
}

// ---- CP_Font_GetCurrent ----

void test_tier2_font_getcurrent_is_the_default_until_a_font_is_set(void)
{
	TEST_ASSERT_TRUE(tier2_scalars.fontStartsAsDefault);
}

void test_tier2_font_getcurrent_is_the_font_that_was_set_and_follows_save_restore(void)
{
	TEST_ASSERT_TRUE(tier2_scalars.fontSetIsCurrent);
	TEST_ASSERT_TRUE(tier2_scalars.fontBackAfterRestore);
}

void test_tier2_font_getcurrent_falls_back_to_the_default_when_the_font_is_freed(void)
{
	TEST_ASSERT_TRUE(tier2_scalars.freedFontFallsBackToDefault);
}

// ---- Text effects ----

// Pixels in a region with any ink at all, light or dark
static int InkCount(Tier2Scenario scenario, int x0, int y0, int x1, int y1)
{
	int count = 0;
	for (int y = y0; y < y1; ++y)
	{
		for (int x = x0; x < x1; ++x)
		{
			CP_Color c = tier2_SamplePixel(scenario, x, y);
			if (c.r < 250 || c.g < 250 || c.b < 250)
			{
				++count;
			}
		}
	}
	return count;
}

// The darkest red value in a region
static int Darkest(Tier2Scenario scenario, int x0, int y0, int x1, int y1)
{
	int darkest = 255;
	for (int y = y0; y < y1; ++y)
	{
		for (int x = x0; x < x1; ++x)
		{
			CP_Color c = tier2_SamplePixel(scenario, x, y);
			if (c.r < darkest)
			{
				darkest = c.r;
			}
		}
	}
	return darkest;
}

// The pixels of two 100x100 quarters of a capture that differ by more than
// a little (quarters 0 to 3: top left, top right, bottom left, bottom right)
static int QuarterDifferences(Tier2Scenario scenario, int a, int b)
{
	int differences = 0;
	for (int y = 0; y < 100; ++y)
	{
		for (int x = 0; x < 100; ++x)
		{
			CP_Color ca = tier2_SamplePixel(scenario, x + (a % 2) * 100, y + (a / 2) * 100);
			CP_Color cb = tier2_SamplePixel(scenario, x + (b % 2) * 100, y + (b / 2) * 100);
			int dr = ca.r - cb.r;
			if (dr > 8 || dr < -8)
			{
				++differences;
			}
		}
	}
	return differences;
}

// Where the dark ink is in a region: its leftmost and rightmost columns and
// its top and bottom rows. All -1 if there is none.
typedef struct
{
	int left, right, top, bottom;
} InkBox;

static InkBox FindInk(Tier2Scenario scenario, int x0, int y0, int x1, int y1)
{
	InkBox box = { -1, -1, -1, -1 };
	for (int y = y0; y < y1; ++y)
	{
		for (int x = x0; x < x1; ++x)
		{
			if (IsInk(tier2_SamplePixel(scenario, x, y)))
			{
				if (box.left < 0 || x < box.left) box.left = x;
				if (x > box.right) box.right = x;
				if (box.top < 0) box.top = y;
				box.bottom = y;
			}
		}
	}
	return box;
}

void test_tier2_font_text_blur_spreads_the_letters_and_follows_save_restore(void)
{
	// The blurred I covers more pixels than the sharp one, and none of it
	// is as dark: the blur spreads the same ink out
	int sharpInk = InkCount(SCN_TEXT_BLUR, 0, 0, 100, 100);
	int blurredInk = InkCount(SCN_TEXT_BLUR, 100, 0, 200, 100);
	int sharpDarkest = Darkest(SCN_TEXT_BLUR, 0, 0, 100, 100);
	TEST_ASSERT_GREATER_THAN_INT(100, sharpInk);
	TEST_ASSERT_GREATER_THAN_INT(sharpInk * 3 / 2, blurredInk);
	TEST_ASSERT_LESS_THAN_INT(40, sharpDarkest);
	TEST_ASSERT_GREATER_THAN_INT(sharpDarkest + 40, Darkest(SCN_TEXT_BLUR, 100, 0, 200, 100));
}

void test_tier2_font_text_blur_below_zero_is_sharp(void)
{
	TEST_ASSERT_EQUAL_INT(0, QuarterDifferences(SCN_TEXT_BLUR, 0, 2));
}

void test_tier2_font_text_blur_rounds_to_whole_steps(void)
{
	// 2.8 like 3, 2.2 like 2, and 3 unlike 2
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, QuarterDifferences(SCN_TEXT_BLUR_ROUNDS, 0, 1), "a blur of 2.8 didn't draw like 3");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, QuarterDifferences(SCN_TEXT_BLUR_ROUNDS, 2, 3), "a blur of 2.2 didn't draw like 2");
	TEST_ASSERT_GREATER_THAN_INT(20, QuarterDifferences(SCN_TEXT_BLUR_ROUNDS, 1, 3));
}

void test_tier2_font_letter_spacing_widens_measured_and_drawn_text(void)
{
	// Three gaps between four letters: 30 wider with 10, 15 narrower with -5
	TEST_ASSERT_GREATER_THAN_FLOAT(60.0f, tier2_scalars.letterWidthPlain);
	TEST_ASSERT_FLOAT_WITHIN(1.0f, tier2_scalars.letterWidthPlain + 30.0f, tier2_scalars.letterWidthSpaced);
	TEST_ASSERT_EQUAL_FLOAT(tier2_scalars.letterWidthSpaced, tier2_scalars.letterSizeSpaced);
	TEST_ASSERT_FLOAT_WITHIN(1.0f, tier2_scalars.letterWidthPlain - 15.0f, tier2_scalars.letterWidthTight);

	InkBox plain = FindInk(SCN_TEXT_LETTER_SPACING, 0, 0, 200, 60);
	InkBox spaced = FindInk(SCN_TEXT_LETTER_SPACING, 0, 60, 200, 120);
	TEST_ASSERT_GREATER_THAN_INT(0, plain.left);
	TEST_ASSERT_INT_WITHIN(1, plain.left, spaced.left);
	TEST_ASSERT_INT_WITHIN(2, plain.right + 30, spaced.right);
}

void test_tier2_font_letter_spacing_keeps_centered_text_centered(void)
{
	// The spacing goes only between letters, so "HH" centered on x = 100
	// with a spacing of 30 is still centered there
	InkBox centered = FindInk(SCN_TEXT_LETTER_SPACING, 0, 120, 200, 200);
	TEST_ASSERT_GREATER_THAN_INT(0, centered.left);
	TEST_ASSERT_GREATER_THAN_INT(40, centered.right - centered.left);
	TEST_ASSERT_INT_WITHIN(2, 200, centered.left + centered.right);
}

void test_tier2_font_line_spacing_adds_a_gap_between_lines(void)
{
	// Three lines: two gaps of 20
	TEST_ASSERT_GREATER_THAN_FLOAT(2.0f * tier2_scalars.lineBoxOneLinePlain.y, tier2_scalars.lineBoxPlain.y);
	TEST_ASSERT_FLOAT_WITHIN(1.0f, tier2_scalars.lineBoxPlain.y + 40.0f, tier2_scalars.lineBoxSpaced.y);
	TEST_ASSERT_EQUAL_FLOAT(tier2_scalars.lineBoxPlain.x, tier2_scalars.lineBoxSpaced.x);

	InkBox plain = FindInk(SCN_TEXT_LINE_SPACING, 0, 0, 100, 200);
	InkBox spaced = FindInk(SCN_TEXT_LINE_SPACING, 100, 0, 200, 200);
	TEST_ASSERT_GREATER_THAN_INT(0, plain.top);
	TEST_ASSERT_INT_WITHIN(1, plain.top, spaced.top);
	TEST_ASSERT_INT_WITHIN(2, plain.bottom + 40, spaced.bottom);
}

void test_tier2_font_line_spacing_leaves_one_line_alone(void)
{
	TEST_ASSERT_EQUAL_FLOAT(tier2_scalars.lineBoxOneLinePlain.y, tier2_scalars.lineBoxOneLineSpaced.y);
	TEST_ASSERT_EQUAL_FLOAT(tier2_scalars.lineTextHeightPlain, tier2_scalars.lineTextHeightSpaced);
}
