// Entry point for the CProcessing_Tests_Tier2 executable.
//
// tier2_RunCaptureOnce() drives the ONE AND ONLY CP_Engine_Run() call this
// process makes, scripting through every scenario in tier2_capture.c and
// capturing a screenshot per scenario. That has to finish -- engine shut
// down, window closed -- before UNITY_BEGIN(), so that every RUN_TEST
// below only ever touches already-captured pixel data. See tier2_capture.h
// for the full reasoning.
#include "unity.h"
#include "tier2_capture.h"
#include "test_runner_tier2.h"

void setUp(void)
{
}

void tearDown(void)
{
}

int main(void)
{
    tier2_RunCaptureOnce();

    UNITY_BEGIN();

    // test_tier2_graphics.c
    RUN_TEST(test_tier2_graphics_clearbackground_fills_whole_canvas);
    RUN_TEST(test_tier2_graphics_drawpoint_is_colored_by_stroke_not_fill);
    RUN_TEST(test_tier2_graphics_drawline);
    RUN_TEST(test_tier2_graphics_drawlineadvanced_rotates_90_degrees);
    RUN_TEST(test_tier2_graphics_drawrect);
    RUN_TEST(test_tier2_graphics_drawrectadvanced_rotates_45_degrees);
    RUN_TEST(test_tier2_graphics_drawcircle);
    RUN_TEST(test_tier2_graphics_drawellipse_is_not_circular);
    RUN_TEST(test_tier2_graphics_drawellipseadvanced_rotates_90_degrees);
    RUN_TEST(test_tier2_graphics_drawtriangle);
    RUN_TEST(test_tier2_graphics_drawtriangleadvanced_rotates_180_degrees);
    RUN_TEST(test_tier2_graphics_drawquad);
    RUN_TEST(test_tier2_graphics_drawquadadvanced_rotates_45_degrees);
    RUN_TEST(test_tier2_graphics_customshape_beginend);

    // test_tier2_settings.c
    RUN_TEST(test_tier2_settings_nofill_leaves_interior_untouched);
    RUN_TEST(test_tier2_settings_strokeweight_widens_the_line);
    RUN_TEST(test_tier2_settings_rectmode_corner_changes_interpretation);
    RUN_TEST(test_tier2_settings_ellipsemode_corner_changes_interpretation);
    RUN_TEST(test_tier2_settings_translate_moves_subsequent_draws);
    RUN_TEST(test_tier2_settings_scale_resizes_subsequent_draws);
    RUN_TEST(test_tier2_settings_rotate_rotates_subsequent_draws);
    RUN_TEST(test_tier2_settings_resetmatrix_cancels_prior_transform);
    RUN_TEST(test_tier2_settings_applymatrix_applies_the_given_matrix);
    RUN_TEST(test_tier2_settings_blendmode_add_sums_overlapping_colors);
    RUN_TEST(test_tier2_settings_save_restore_round_trips_fill_and_transform);

    return UNITY_END();
}
