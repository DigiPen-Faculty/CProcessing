// Entry point for the CProcessing_Tests_Tier2 executable.
//
// tier2_RunCaptureOnce() drives the ONE AND ONLY CP_Engine_Run() call this
// process makes, scripting through every scenario in tier2_capture.c and
// capturing a screenshot per scenario. That has to finish -- engine shut
// down, window closed -- before UNITY_BEGIN(), so that every RUN_TEST
// below only ever touches already-captured pixel data. See tier2_capture.h
// for the full reasoning.
#include <stdio.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <crtdbg.h>
#endif
#include "unity.h"
#include "tier2_capture.h"
#include "test_runner_tier2.h"

// A failed assert or abort() in a Debug build normally opens a dialog box,
// which nobody can click on a CI runner, so the job would hang. Print the
// report and exit instead. (The library uses the same CRT DLL, so this
// covers asserts inside CProcessing and its third-party code too.)
static void ReportCrashesWithoutDialogs(void)
{
#ifdef _MSC_VER
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _set_error_mode(_OUT_TO_STDERR);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
}

void setUp(void)
{
}

void tearDown(void)
{
}

int main(void)
{
    ReportCrashesWithoutDialogs();
    tier2_RunCaptureOnce();

    // If the engine couldn't start (CProcessing prints the reason above) or
    // stopped early, the captures are empty: every test would fail on zeros
    // and a few would pass by accident. Report that once, clearly, instead.
    if (tier2_scalars.scenariosCaptured != SCN_COUNT)
    {
        printf("Tier 2 could not run: %d of %d scenarios were captured.\n"
               "It needs a window with an OpenGL 3.2 or newer context.\n",
               tier2_scalars.scenariosCaptured, SCN_COUNT);
        return 1;
    }

    UNITY_BEGIN();

    // test_tier2_graphics.c
    RUN_TEST(test_tier2_graphics_clearbackground_fills_whole_canvas);
    RUN_TEST(test_tier2_graphics_drawpoint_follows_stroke_settings);
    RUN_TEST(test_tier2_graphics_drawline);
    RUN_TEST(test_tier2_graphics_drawlineadvanced_rotates_90_degrees);
    RUN_TEST(test_tier2_graphics_drawrect);
    RUN_TEST(test_tier2_graphics_drawrectadvanced_rotates_45_degrees);
    RUN_TEST(test_tier2_graphics_drawcircle);
    RUN_TEST(test_tier2_graphics_drawellipse_is_not_circular);
    RUN_TEST(test_tier2_graphics_drawellipseadvanced_rotates_90_degrees);
    RUN_TEST(test_tier2_graphics_drawarc_goes_clockwise_from_the_right_and_has_no_fill);
    RUN_TEST(test_tier2_graphics_drawarc_with_a_negative_sweep_goes_counterclockwise);
    RUN_TEST(test_tier2_graphics_drawpie_fills_only_its_slice);
    RUN_TEST(test_tier2_graphics_drawpie_in_corner_mode_uses_the_top_left);
    RUN_TEST(test_tier2_graphics_drawbezier_bends_toward_its_control_points);
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
    RUN_TEST(test_tier2_settings_blend_modes_with_equations);
    RUN_TEST(test_tier2_settings_tint_multiplies_shapes_and_images);
    RUN_TEST(test_tier2_settings_carry_over_to_the_next_frame);
    RUN_TEST(test_tier2_settings_made_before_run_are_kept);

    // test_tier2_image.c
    RUN_TEST(test_tier2_image_load_reports_correct_dimensions);
    RUN_TEST(test_tier2_image_draw_places_quadrants_correctly);
    RUN_TEST(test_tier2_image_createfromdata_roundtrips_exactly);
    RUN_TEST(test_tier2_image_updatepixeldata_roundtrips_exactly);
    RUN_TEST(test_tier2_image_drawsubimage_selects_correct_region);
    RUN_TEST(test_tier2_image_many_images_load_draw_and_free);
    RUN_TEST(test_tier2_image_16_bit_png_loads_and_draws);
    RUN_TEST(test_tier2_image_filter_mode_nearest_and_linear);
    RUN_TEST(test_tier2_image_wrap_modes);
    RUN_TEST(test_tier2_shared_image_survives_freeing_one_load);
    RUN_TEST(test_tier2_shared_font_and_sound_survive_freeing_one_load);
    RUN_TEST(test_tier2_assets_still_loaded_are_reported_at_exit);

    // test_tier2_font.c
    RUN_TEST(test_tier2_font_drawtext_occupies_expected_region);
    RUN_TEST(test_tier2_font_load_free_renders_with_custom_font);
    RUN_TEST(test_tier2_font_free_keeps_other_fonts_working);
    RUN_TEST(test_tier2_font_text_animated_by_size_draws_every_frame);
    RUN_TEST(test_tier2_font_text_animated_by_scale_draws_every_frame);
    RUN_TEST(test_tier2_font_text_size_is_the_em_size);
    RUN_TEST(test_tier2_font_measuring_text);
    RUN_TEST(test_tier2_font_measuring_a_text_box_counts_wrapped_lines);
    RUN_TEST(test_tier2_font_measuring_null_text_is_zero);
    RUN_TEST(test_tier2_font_opentype_cff_font_loads_and_draws);
    RUN_TEST(test_tier2_font_large_text_draws);
    RUN_TEST(test_tier2_font_text_mirrored_and_flipped_draws);

    // test_tier2_gamestates.c
    RUN_TEST(test_tier2_gamestate_push_and_pop_run_inits_and_exits_in_order);
    RUN_TEST(test_tier2_gamestate_a_pushed_state_draws_over_the_frozen_one_below);
    RUN_TEST(test_tier2_gamestate_hidden_mode_hides_the_states_below);
    RUN_TEST(test_tier2_gamestate_queries_in_a_pushed_state);
    RUN_TEST(test_tier2_gamestate_running_mode_keeps_the_state_below_running_without_input);
    RUN_TEST(test_tier2_gamestate_a_frozen_state_is_not_updated);
    RUN_TEST(test_tier2_gamestate_each_state_keeps_its_own_settings);
    RUN_TEST(test_tier2_gamestate_states_still_pushed_exit_when_the_program_ends);

    // test_tier2_system_engine.c
    RUN_TEST(test_tier2_system_window_size_matches_what_was_set);
    RUN_TEST(test_tier2_system_window_size_chosen_in_init_is_there_for_the_first_update);
    RUN_TEST(test_tier2_system_frame_count_advances);
    RUN_TEST(test_tier2_system_frame_rate_zero_means_no_limit);
    RUN_TEST(test_tier2_system_vsync_and_resizable_chosen_before_run_are_kept);
    RUN_TEST(test_tier2_system_vsync_and_resizable_getters_follow_the_setters);
    RUN_TEST(test_tier2_system_is_fullscreen_is_false_in_a_window);
    RUN_TEST(test_tier2_system_target_frame_rate_gives_the_limit_and_0_for_none);
    RUN_TEST(test_tier2_settings_getters_give_back_what_was_set);
    RUN_TEST(test_tier2_settings_fill_and_stroke_colors_are_kept_by_nofill_and_nostroke);
    RUN_TEST(test_tier2_settings_restore_brings_back_what_the_getters_return);
    RUN_TEST(test_tier2_settings_getfill_works_before_run);
    RUN_TEST(test_tier2_font_getcurrent_is_the_default_until_a_font_is_set);
    RUN_TEST(test_tier2_font_getcurrent_is_the_font_that_was_set_and_follows_save_restore);
    RUN_TEST(test_tier2_font_getcurrent_falls_back_to_the_default_when_the_font_is_freed);
    RUN_TEST(test_tier2_system_timing_getters_are_sane);
    RUN_TEST(test_tier2_system_window_focus_is_a_valid_bool);
    RUN_TEST(test_tier2_system_display_info_is_positive);
    RUN_TEST(test_tier2_engine_pre_and_post_update_hooks_fire);
    RUN_TEST(test_tier2_system_window_queries_before_run_report_no_window);
    RUN_TEST(test_tier2_system_cursor_hidden_before_run_stays_hidden);
    RUN_TEST(test_tier2_system_cursor_getter_follows_showcursor);
    RUN_TEST(test_tier2_system_cursor_mode_getters_follow_setcursormode);
    RUN_TEST(test_tier2_system_console_hide_then_show_round_trips);
    RUN_TEST(test_tier2_system_console_printf_keeps_working);

    // test_tier2_sound.c
    RUN_TEST(test_tier2_sound_group_volume_roundtrips);
    RUN_TEST(test_tier2_sound_group_pitch_roundtrips);
    RUN_TEST(test_tier2_sound_play_returns_an_instance_that_stops_one_sound);
    RUN_TEST(test_tier2_sound_no_instance_is_not_playing_and_is_safe);

    // test_tier2_input.c
    RUN_TEST(test_tier2_input_no_gamepad_attached_reports_disconnected);
    RUN_TEST(test_tier2_input_mouse_wheel_is_zero_at_rest);
    RUN_TEST(test_tier2_input_no_clicks_or_keys_in_a_quiescent_frame);

    // test_tier2_crossplatform.c
    RUN_TEST(test_tier2_drawing_persists_into_the_next_frame);
    RUN_TEST(test_tier2_first_frame_did_not_contain_the_later_drawing);
    RUN_TEST(test_tier2_screenshot_subregion_has_requested_size);
    RUN_TEST(test_tier2_screenshot_subregion_reads_the_right_place);
    RUN_TEST(test_tier2_screenshot_subregion_is_not_flipped);
    RUN_TEST(test_tier2_missing_files_load_as_null);
    RUN_TEST(test_tier2_null_handles_are_ignored);
    RUN_TEST(test_tier2_window_handle_is_available);
    RUN_TEST(test_tier2_assets_load_from_next_to_the_program);

    return UNITY_END();
}
