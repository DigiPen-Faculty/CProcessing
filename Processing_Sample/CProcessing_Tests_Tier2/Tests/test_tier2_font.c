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
