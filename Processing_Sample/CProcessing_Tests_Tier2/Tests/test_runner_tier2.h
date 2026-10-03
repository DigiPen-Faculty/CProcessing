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
void test_tier2_image_many_images_load_draw_and_free(void);

// test_tier2_font.c -- Tier 2: CP_Font (bounding-box/occupancy checks).
void test_tier2_font_drawtext_occupies_expected_region(void);
void test_tier2_font_load_free_renders_with_custom_font(void);
void test_tier2_font_text_animated_by_size_draws_every_frame(void);
void test_tier2_font_text_animated_by_scale_draws_every_frame(void);

// test_tier2_system_engine.c -- Tier 2: CP_System + CP_Engine lifecycle.
void test_tier2_system_window_size_matches_what_was_set(void);
void test_tier2_system_frame_count_advances(void);
void test_tier2_system_timing_getters_are_sane(void);
void test_tier2_system_window_focus_is_a_valid_bool(void);
void test_tier2_system_display_info_is_positive(void);
void test_tier2_engine_pre_and_post_update_hooks_fire(void);
void test_tier2_system_window_queries_before_run_report_no_window(void);
void test_tier2_system_cursor_hidden_before_run_stays_hidden(void);
void test_tier2_system_cursor_getter_follows_showcursor(void);
void test_tier2_system_console_hide_then_show_round_trips(void);
void test_tier2_system_console_printf_keeps_working(void);

// test_tier2_sound.c -- Tier 2: CP_Sound (crash/round-trip checks, not audio correctness).
void test_tier2_sound_group_volume_roundtrips(void);
void test_tier2_sound_group_pitch_roundtrips(void);

// test_tier2_input.c -- Tier 2: CP_Input (quiescent-state defaults only).
void test_tier2_input_no_gamepad_attached_reports_disconnected(void);
void test_tier2_input_mouse_wheel_is_zero_at_rest(void);
void test_tier2_input_no_clicks_or_keys_in_a_quiescent_frame(void);

// test_tier2_crossplatform.c -- Tier 2: behavior every platform must share.
void test_tier2_drawing_persists_into_the_next_frame(void);
void test_tier2_first_frame_did_not_contain_the_later_drawing(void);
void test_tier2_screenshot_subregion_has_requested_size(void);
void test_tier2_screenshot_subregion_reads_the_right_place(void);
void test_tier2_screenshot_subregion_is_not_flipped(void);
void test_tier2_missing_files_load_as_null(void);
void test_tier2_null_handles_are_ignored(void);
void test_tier2_window_handle_is_available(void);
