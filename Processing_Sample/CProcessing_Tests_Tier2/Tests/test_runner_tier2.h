// Prototypes for every Tier 2 Unity test function, grouped by the
// source file that defines them. See tier2_capture.h for why these
// never call into the engine themselves.
#pragma once

// test_tier2_graphics.c -- Tier 2: CP_Graphics (shape primitives).
void test_tier2_graphics_clearbackground_fills_whole_canvas(void);
void test_tier2_graphics_drawpoint_is_colored_by_stroke_not_fill(void);
void test_tier2_graphics_drawline(void);
void test_tier2_graphics_drawlineadvanced_rotates_90_degrees(void);
void test_tier2_graphics_drawrect(void);
void test_tier2_graphics_drawrectadvanced_rotates_45_degrees(void);
void test_tier2_graphics_drawcircle(void);
void test_tier2_graphics_drawellipse_is_not_circular(void);
void test_tier2_graphics_drawellipseadvanced_rotates_90_degrees(void);
void test_tier2_graphics_drawtriangle(void);
void test_tier2_graphics_drawtriangleadvanced_rotates_180_degrees(void);
void test_tier2_graphics_drawquad(void);
void test_tier2_graphics_drawquadadvanced_rotates_45_degrees(void);
void test_tier2_graphics_customshape_beginend(void);

// test_tier2_settings.c -- Tier 2: CP_Settings (state affecting draw output).
void test_tier2_settings_nofill_leaves_interior_untouched(void);
void test_tier2_settings_strokeweight_widens_the_line(void);
void test_tier2_settings_rectmode_corner_changes_interpretation(void);
void test_tier2_settings_ellipsemode_corner_changes_interpretation(void);
void test_tier2_settings_translate_moves_subsequent_draws(void);
void test_tier2_settings_scale_resizes_subsequent_draws(void);
void test_tier2_settings_rotate_rotates_subsequent_draws(void);
void test_tier2_settings_resetmatrix_cancels_prior_transform(void);
void test_tier2_settings_applymatrix_applies_the_given_matrix(void);
void test_tier2_settings_blendmode_add_sums_overlapping_colors(void);
void test_tier2_settings_save_restore_round_trips_fill_and_transform(void);
