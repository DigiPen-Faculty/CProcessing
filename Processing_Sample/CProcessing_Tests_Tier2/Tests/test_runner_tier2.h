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

// test_tier2_image.c -- Tier 2: CP_Image.
void test_tier2_image_load_reports_correct_dimensions(void);
void test_tier2_image_draw_places_quadrants_correctly(void);
void test_tier2_image_createfromdata_roundtrips_exactly(void);
void test_tier2_image_updatepixeldata_roundtrips_exactly(void);
void test_tier2_image_drawsubimage_selects_correct_region(void);

// test_tier2_font.c -- Tier 2: CP_Font (bounding-box/occupancy checks).
void test_tier2_font_drawtext_occupies_expected_region(void);
void test_tier2_font_load_free_renders_with_custom_font(void);

// test_tier2_system_engine.c -- Tier 2: CP_System + CP_Engine lifecycle.
void test_tier2_system_window_size_matches_what_was_set(void);
void test_tier2_system_frame_count_advances(void);
void test_tier2_system_timing_getters_are_sane(void);
void test_tier2_system_window_focus_is_a_valid_bool(void);
void test_tier2_system_display_info_is_positive(void);
void test_tier2_engine_pre_and_post_update_hooks_fire(void);

// test_tier2_sound.c -- Tier 2: CP_Sound (crash/round-trip checks, not audio correctness).
void test_tier2_sound_group_volume_roundtrips(void);
void test_tier2_sound_group_pitch_roundtrips(void);
