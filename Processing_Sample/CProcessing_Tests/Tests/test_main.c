// Single entry point for the CProcessing_Tests executable. Unity expects
// exactly one setUp/tearDown pair linked into the binary, and one place
// that lists every RUN_TEST -- this is that place. Test functions
// themselves live alongside the module they cover (test_cp_math.c,
// test_cp_vector.c, ...); see test_runner.h for the full list.
#include "unity.h"
#include "test_runner.h"

void setUp(void)
{
}

void tearDown(void)
{
}

int main(void)
{
    UNITY_BEGIN();

    // test_smoke.c
    RUN_TEST(test_smoke_truth);
    RUN_TEST(test_smoke_cprocessing_link);

    // test_cp_math.c
    RUN_TEST(test_math_clampint_below_min_returns_min);
    RUN_TEST(test_math_clampint_above_max_returns_max);
    RUN_TEST(test_math_clampint_within_range_unchanged);
    RUN_TEST(test_math_clampint_boundaries_are_inclusive);
    RUN_TEST(test_math_clampfloat_below_min_returns_min);
    RUN_TEST(test_math_clampfloat_above_max_returns_max);
    RUN_TEST(test_math_clampfloat_within_range_unchanged);
    RUN_TEST(test_math_lerpint_factor_zero_returns_a);
    RUN_TEST(test_math_lerpint_factor_one_returns_b);
    RUN_TEST(test_math_lerpint_factor_half);
    RUN_TEST(test_math_lerpint_factor_below_zero_clamps);
    RUN_TEST(test_math_lerpint_factor_above_one_clamps);
    RUN_TEST(test_math_lerpfloat_factor_zero_returns_a);
    RUN_TEST(test_math_lerpfloat_factor_one_returns_b);
    RUN_TEST(test_math_lerpfloat_factor_half);
    RUN_TEST(test_math_lerpfloat_factor_out_of_range_clamps);
    RUN_TEST(test_math_square_positive);
    RUN_TEST(test_math_square_negative);
    RUN_TEST(test_math_square_zero);
    RUN_TEST(test_math_distance_same_point_is_zero);
    RUN_TEST(test_math_distance_3_4_5_triangle);
    RUN_TEST(test_math_distance_is_symmetric);
    RUN_TEST(test_math_degrees_zero);
    RUN_TEST(test_math_degrees_pi_is_180);
    RUN_TEST(test_math_degrees_half_pi_is_90);
    RUN_TEST(test_math_radians_zero);
    RUN_TEST(test_math_radians_180_is_pi);
    RUN_TEST(test_math_radians_90_is_half_pi);
    RUN_TEST(test_math_degrees_and_radians_are_inverses);

    // test_cp_vector.c
    RUN_TEST(test_vector_set);
    RUN_TEST(test_vector_zero);
    RUN_TEST(test_vector_negate);
    RUN_TEST(test_vector_add);
    RUN_TEST(test_vector_subtract);
    RUN_TEST(test_vector_scale);
    RUN_TEST(test_vector_normalize_unit_length);
    RUN_TEST(test_vector_normalize_zero_vector_stays_zero);
    RUN_TEST(test_vector_matrixmultiply_identity_is_noop);
    RUN_TEST(test_vector_matrixmultiply_treats_vector_as_point);
    RUN_TEST(test_vector_length);
    RUN_TEST(test_vector_length_zero_vector);
    RUN_TEST(test_vector_distance);
    RUN_TEST(test_vector_dotproduct_perpendicular_is_zero);
    RUN_TEST(test_vector_dotproduct_general);
    RUN_TEST(test_vector_crossproduct_general);
    RUN_TEST(test_vector_angle_same_direction_is_zero);
    RUN_TEST(test_vector_angle_perpendicular_is_90);
    RUN_TEST(test_vector_angle_opposite_is_180);
    RUN_TEST(test_vector_angle_is_unsigned_regardless_of_order);
    RUN_TEST(test_vector_anglecw_quarter_turn);
    RUN_TEST(test_vector_anglecw_is_not_symmetric);
    RUN_TEST(test_vector_anglecw_same_direction_is_zero);
    RUN_TEST(test_vector_anglecw_opposite_direction_is_180);
    RUN_TEST(test_vector_anglecw_stays_within_0_360);
    RUN_TEST(test_vector_angleccw_quarter_turn_is_negative);
    RUN_TEST(test_vector_angleccw_same_direction_stays_zero);
    RUN_TEST(test_vector_angleccw_stays_within_negative_360_to_0);
    RUN_TEST(test_vector_angleccw_equals_anglecw_minus_360);

    // test_cp_matrix.c
    RUN_TEST(test_matrix_set);
    RUN_TEST(test_matrix_identity);
    RUN_TEST(test_matrix_fromvector_places_columns);
    RUN_TEST(test_matrix_scale);
    RUN_TEST(test_matrix_translate);
    RUN_TEST(test_matrix_rotate_zero_is_identity);
    RUN_TEST(test_matrix_rotate_90_degrees);
    RUN_TEST(test_matrix_rotateradians_matches_rotate_degrees);
    RUN_TEST(test_matrix_transpose_swaps_off_diagonal);
    RUN_TEST(test_matrix_transpose_of_identity_is_identity);
    RUN_TEST(test_matrix_transpose_twice_returns_original);
    RUN_TEST(test_matrix_inverse_of_identity_is_identity);
    RUN_TEST(test_matrix_inverse_of_scale);
    RUN_TEST(test_matrix_inverse_times_original_is_identity);
    RUN_TEST(test_matrix_inverse_of_singular_matrix_is_not_a_number);
    RUN_TEST(test_matrix_multiply_by_identity_is_noop);
    RUN_TEST(test_matrix_multiply_composes_translations);

    // test_cp_color.c
    RUN_TEST(test_color_create_passes_through_in_range_values);
    RUN_TEST(test_color_create_clamps_below_zero);
    RUN_TEST(test_color_create_clamps_above_255);
    RUN_TEST(test_color_createhex_red_opaque);
    RUN_TEST(test_color_createhex_green_opaque);
    RUN_TEST(test_color_createhex_half_alpha);
    RUN_TEST(test_color_lerp_factor_zero_returns_a);
    RUN_TEST(test_color_lerp_factor_one_returns_b);
    RUN_TEST(test_color_lerp_factor_half);
    RUN_TEST(test_color_lerp_factor_out_of_range_clamps);
    RUN_TEST(test_colorhsl_create_passes_through_in_range_values);
    RUN_TEST(test_colorhsl_create_wraps_negative_hue);
    RUN_TEST(test_colorhsl_create_wraps_hue_above_360);
    RUN_TEST(test_colorhsl_create_clamps_saturation_and_lightness);
    RUN_TEST(test_colorhsl_lerp_factor_zero_returns_a);
    RUN_TEST(test_colorhsl_lerp_factor_one_returns_b);
    RUN_TEST(test_color_fromcolorhsl_red);
    RUN_TEST(test_color_fromcolorhsl_green);
    RUN_TEST(test_color_fromcolorhsl_blue);
    RUN_TEST(test_color_fromcolorhsl_white);
    RUN_TEST(test_color_fromcolorhsl_black);
    RUN_TEST(test_colorhsl_fromcolor_red);
    RUN_TEST(test_colorhsl_fromcolor_green);
    RUN_TEST(test_colorhsl_fromcolor_blue);
    RUN_TEST(test_colorhsl_fromcolor_white_has_zero_saturation);
    RUN_TEST(test_colorhsl_fromcolor_black_has_zero_lightness);
    RUN_TEST(test_colorhsl_fromcolor_preserves_alpha);

    // test_cp_random.c
    RUN_TEST(test_random_getbool_produces_both_values_over_many_samples);
    RUN_TEST(test_random_rangeint_respects_inclusive_bounds);
    RUN_TEST(test_random_rangeint_can_reach_both_boundaries);
    RUN_TEST(test_random_rangeint_swaps_reversed_bounds);
    RUN_TEST(test_random_rangeint_equal_bounds_returns_that_value);
    RUN_TEST(test_random_getfloat_stays_within_0_and_1);
    RUN_TEST(test_random_rangefloat_respects_bounds);
    RUN_TEST(test_random_rangefloat_swaps_reversed_bounds);
    RUN_TEST(test_random_seed_reproduces_sequence_on_this_platform);
    RUN_TEST(test_random_seed_different_seeds_diverge);
    RUN_TEST(test_random_getint_msvc_pinned_sequence_for_seed_42);
    RUN_TEST(test_random_gaussian_mean_is_roughly_zero);
    RUN_TEST(test_random_gaussian_stddev_is_roughly_one);
    RUN_TEST(test_noise_stays_within_0_and_1);
    RUN_TEST(test_noise_is_deterministic_for_same_seed_and_coordinates);
    RUN_TEST(test_noise_different_seeds_can_diverge);
    RUN_TEST(test_noise_same_coordinate_is_continuous_with_its_neighbor);

    return UNITY_END();
}
