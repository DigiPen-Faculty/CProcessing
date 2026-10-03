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

void test_tier2_font_text_size_is_the_em_size(void)
{
    // See Scn_FontEmSize: a capital H at size 100 is 69 pixels tall
    int top = -1, bottom = -1;
    for (int y = 0; y < TIER2_CANVAS_SIZE; ++y)
    {
        for (int x = 0; x < TIER2_CANVAS_SIZE; ++x)
        {
            if (IsInk(tier2_SamplePixel(SCN_FONT_EM_SIZE, x, y)))
            {
                if (top < 0) top = y;
                bottom = y;
                break;
            }
        }
    }
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, top);
    TEST_ASSERT_INT_WITHIN(2, 69, bottom - top + 1);
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
            else           // flipped about y = 167
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
