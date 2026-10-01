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

void test_tier2_settings_save_restore_round_trips_fill_and_transform(void)
{
    // If fill weren't restored this would be blue; if the transform
    // weren't restored the rect would have moved to world (150,150),
    // making this background instead.
    assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_SETTINGS_SAVE_RESTORE, 100, 100));
    assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_SETTINGS_SAVE_RESTORE, 150, 150));
}
