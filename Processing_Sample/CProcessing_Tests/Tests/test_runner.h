// Prototypes for every Unity test function in the suite, grouped by the
// source file that defines them. test_main.c includes this header and
// RUN_TESTs each one; setUp/tearDown live in test_main.c since Unity
// expects exactly one definition of each, shared by every test file.
#pragma once

// test_smoke.c -- Phase A: proves the harness itself works end to end.
void test_smoke_truth(void);
void test_smoke_cprocessing_link(void);

// test_cp_math.c -- Tier 1: CP_Math (pure functions only).
void test_math_clampint_below_min_returns_min(void);
void test_math_clampint_above_max_returns_max(void);
void test_math_clampint_within_range_unchanged(void);
void test_math_clampint_boundaries_are_inclusive(void);
void test_math_clampfloat_below_min_returns_min(void);
void test_math_clampfloat_above_max_returns_max(void);
void test_math_clampfloat_within_range_unchanged(void);
void test_math_lerpint_factor_zero_returns_a(void);
void test_math_lerpint_factor_one_returns_b(void);
void test_math_lerpint_factor_half(void);
void test_math_lerpint_factor_below_zero_clamps(void);
void test_math_lerpint_factor_above_one_clamps(void);
void test_math_lerpfloat_factor_zero_returns_a(void);
void test_math_lerpfloat_factor_one_returns_b(void);
void test_math_lerpfloat_factor_half(void);
void test_math_lerpfloat_factor_out_of_range_clamps(void);
void test_math_square_positive(void);
void test_math_square_negative(void);
void test_math_square_zero(void);
void test_math_distance_same_point_is_zero(void);
void test_math_distance_3_4_5_triangle(void);
void test_math_distance_is_symmetric(void);
void test_math_degrees_zero(void);
void test_math_degrees_pi_is_180(void);
void test_math_degrees_half_pi_is_90(void);
void test_math_radians_zero(void);
void test_math_radians_180_is_pi(void);
void test_math_radians_90_is_half_pi(void);
void test_math_degrees_and_radians_are_inverses(void);

// test_cp_vector.c -- Tier 1: CP_Vector (all 15 functions are pure).
void test_vector_set(void);
void test_vector_zero(void);
void test_vector_negate(void);
void test_vector_add(void);
void test_vector_subtract(void);
void test_vector_scale(void);
void test_vector_normalize_unit_length(void);
void test_vector_normalize_zero_vector_stays_zero(void);
void test_vector_matrixmultiply_identity_is_noop(void);
void test_vector_matrixmultiply_treats_vector_as_point(void);
void test_vector_length(void);
void test_vector_length_zero_vector(void);
void test_vector_distance(void);
void test_vector_dotproduct_perpendicular_is_zero(void);
void test_vector_dotproduct_general(void);
void test_vector_crossproduct_general(void);
void test_vector_angle_same_direction_is_zero(void);
void test_vector_angle_perpendicular_is_90(void);
void test_vector_angle_opposite_is_180(void);
void test_vector_angle_is_unsigned_regardless_of_order(void);
void test_vector_anglecw_quarter_turn(void);
void test_vector_anglecw_is_not_symmetric(void);
void test_vector_anglecw_same_direction_is_zero(void);
void test_vector_anglecw_opposite_direction_is_180(void);
void test_vector_anglecw_stays_within_0_360(void);
void test_vector_angleccw_quarter_turn_is_negative(void);
void test_vector_angleccw_same_direction_stays_zero(void);
void test_vector_angleccw_stays_within_negative_360_to_0(void);
void test_vector_angleccw_equals_anglecw_minus_360(void);

// test_cp_matrix.c -- Tier 1: CP_Matrix (all 10 functions are pure).
void test_matrix_set(void);
void test_matrix_identity(void);
void test_matrix_fromvector_places_columns(void);
void test_matrix_scale(void);
void test_matrix_translate(void);
void test_matrix_rotate_zero_is_identity(void);
void test_matrix_rotate_90_degrees(void);
void test_matrix_rotateradians_matches_rotate_degrees(void);
void test_matrix_transpose_swaps_off_diagonal(void);
void test_matrix_transpose_of_identity_is_identity(void);
void test_matrix_transpose_twice_returns_original(void);
void test_matrix_inverse_of_identity_is_identity(void);
void test_matrix_inverse_of_scale(void);
void test_matrix_inverse_times_original_is_identity(void);
void test_matrix_inverse_of_singular_matrix_is_not_a_number(void);
void test_matrix_multiply_by_identity_is_noop(void);
void test_matrix_multiply_composes_translations(void);

// test_cp_color.c -- Tier 1: CP_Color (all 7 functions are pure).
void test_color_create_passes_through_in_range_values(void);
void test_color_create_clamps_below_zero(void);
void test_color_create_clamps_above_255(void);
void test_color_createhex_red_opaque(void);
void test_color_createhex_green_opaque(void);
void test_color_createhex_half_alpha(void);
void test_color_lerp_factor_zero_returns_a(void);
void test_color_lerp_factor_one_returns_b(void);
void test_color_lerp_factor_half(void);
void test_color_lerp_factor_out_of_range_clamps(void);
void test_colorhsl_create_passes_through_in_range_values(void);
void test_colorhsl_create_wraps_negative_hue(void);
void test_colorhsl_create_wraps_hue_above_360(void);
void test_colorhsl_create_clamps_saturation_and_lightness(void);
void test_colorhsl_lerp_factor_zero_returns_a(void);
void test_colorhsl_lerp_factor_one_returns_b(void);
void test_color_fromcolorhsl_red(void);
void test_color_fromcolorhsl_green(void);
void test_color_fromcolorhsl_blue(void);
void test_color_fromcolorhsl_white(void);
void test_color_fromcolorhsl_black(void);
void test_colorhsl_fromcolor_red(void);
void test_colorhsl_fromcolor_green(void);
void test_colorhsl_fromcolor_blue(void);
void test_colorhsl_fromcolor_white_has_zero_saturation(void);
void test_colorhsl_fromcolor_black_has_zero_lightness(void);
void test_colorhsl_fromcolor_preserves_alpha(void);

// test_cp_random.c -- Tier 1: CP_Random + CP_Noise.
void test_random_getbool_produces_both_values_over_many_samples(void);
void test_random_rangeint_respects_inclusive_bounds(void);
void test_random_rangeint_can_reach_both_boundaries(void);
void test_random_rangeint_swaps_reversed_bounds(void);
void test_random_rangeint_equal_bounds_returns_that_value(void);
void test_random_getfloat_stays_within_0_and_1(void);
void test_random_rangefloat_respects_bounds(void);
void test_random_rangefloat_swaps_reversed_bounds(void);
void test_random_seed_reproduces_sequence_on_this_platform(void);
void test_random_seed_different_seeds_diverge(void);
void test_random_getint_msvc_pinned_sequence_for_seed_42(void);
void test_random_gaussian_mean_is_roughly_zero(void);
void test_random_gaussian_stddev_is_roughly_one(void);
void test_noise_stays_within_0_and_1(void);
void test_noise_is_deterministic_for_same_seed_and_coordinates(void);
void test_noise_different_seeds_can_diverge(void);
void test_noise_same_coordinate_is_continuous_with_its_neighbor(void);
