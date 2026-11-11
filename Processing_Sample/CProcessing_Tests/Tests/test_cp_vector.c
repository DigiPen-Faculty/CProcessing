// Tier 1 coverage for CP_Vector -- all 15 functions are pure.
//
// CP_Vector_Angle/AngleCW/AngleCCW look similar from their signatures but
// have distinct, precisely documented contracts (per the wiki and
// CP_Math.c:139-164):
//   - Angle    always returns the unsigned angle between the vectors, [0, 180].
//   - AngleCW  returns the clockwise angle from `from` to `to`,        [0, 360).
//   - AngleCCW returns AngleCW - 360 (0 stays 0),                    (-360, 0].
#include "unity.h"
#include "cprocessing.h"

static void assertVectorEqual(CP_Vector expected, CP_Vector actual)
{
    TEST_ASSERT_EQUAL_FLOAT(expected.x, actual.x);
    TEST_ASSERT_EQUAL_FLOAT(expected.y, actual.y);
}

// ---- Construction ----

void test_vector_set(void)
{
    CP_Vector v = CP_Vector_Set(3.0f, -4.0f);
    TEST_ASSERT_EQUAL_FLOAT(3.0f, v.x);
    TEST_ASSERT_EQUAL_FLOAT(-4.0f, v.y);
}

void test_vector_zero(void)
{
    assertVectorEqual(CP_Vector_Set(0.0f, 0.0f), CP_Vector_Zero());
}

void test_vector_negate(void)
{
    assertVectorEqual(CP_Vector_Set(-3.0f, 4.0f), CP_Vector_Negate(CP_Vector_Set(3.0f, -4.0f)));
}

// ---- Arithmetic ----

void test_vector_add(void)
{
    CP_Vector result = CP_Vector_Add(CP_Vector_Set(1.0f, 2.0f), CP_Vector_Set(3.0f, 4.0f));
    assertVectorEqual(CP_Vector_Set(4.0f, 6.0f), result);
}

void test_vector_subtract(void)
{
    CP_Vector result = CP_Vector_Subtract(CP_Vector_Set(5.0f, 7.0f), CP_Vector_Set(2.0f, 1.0f));
    assertVectorEqual(CP_Vector_Set(3.0f, 6.0f), result);
}

void test_vector_scale(void)
{
    CP_Vector result = CP_Vector_Scale(CP_Vector_Set(2.0f, -3.0f), 2.5f);
    assertVectorEqual(CP_Vector_Set(5.0f, -7.5f), result);
}

// ---- Normalize ----

void test_vector_normalize_unit_length(void)
{
    CP_Vector result = CP_Vector_Normalize(CP_Vector_Set(3.0f, 4.0f));
    assertVectorEqual(CP_Vector_Set(0.6f, 0.8f), result);
}

void test_vector_normalize_zero_vector_stays_zero(void)
{
    // Documented current behavior: the zero vector has no direction to
    // normalize to, so CP_Vector_Normalize special-cases it rather than
    // dividing by zero (CP_Math.c:115-123).
    assertVectorEqual(CP_Vector_Zero(), CP_Vector_Normalize(CP_Vector_Zero()));
}

// ---- Matrix interaction ----

void test_vector_matrixmultiply_identity_is_noop(void)
{
    CP_Vector v = CP_Vector_Set(7.0f, -2.0f);
    CP_Vector result = CP_Vector_MatrixMultiply(CP_Matrix_Identity(), v);
    assertVectorEqual(v, result);
}

void test_vector_matrixmultiply_treats_vector_as_point(void)
{
    // Translation only affects a point (third homogeneous component == 1),
    // which is what CP_Vector_MatrixMultiply assumes (CP_Math.c:125-132).
    CP_Matrix translate = CP_Matrix_Translate(CP_Vector_Set(5.0f, 7.0f));
    CP_Vector result = CP_Vector_MatrixMultiply(translate, CP_Vector_Set(1.0f, 1.0f));
    assertVectorEqual(CP_Vector_Set(6.0f, 8.0f), result);
}

// ---- Scalar measurements ----

void test_vector_length(void)
{
    TEST_ASSERT_EQUAL_FLOAT(5.0f, CP_Vector_Length(CP_Vector_Set(3.0f, 4.0f)));
}

void test_vector_length_zero_vector(void)
{
    TEST_ASSERT_EQUAL_FLOAT(0.0f, CP_Vector_Length(CP_Vector_Zero()));
}

void test_vector_distance(void)
{
    TEST_ASSERT_EQUAL_FLOAT(5.0f, CP_Vector_Distance(CP_Vector_Set(0.0f, 0.0f), CP_Vector_Set(3.0f, 4.0f)));
}

void test_vector_dotproduct_perpendicular_is_zero(void)
{
    TEST_ASSERT_EQUAL_FLOAT(0.0f, CP_Vector_DotProduct(CP_Vector_Set(1.0f, 0.0f), CP_Vector_Set(0.0f, 1.0f)));
}

void test_vector_dotproduct_general(void)
{
    TEST_ASSERT_EQUAL_FLOAT(23.0f, CP_Vector_DotProduct(CP_Vector_Set(2.0f, 3.0f), CP_Vector_Set(4.0f, 5.0f)));
}

void test_vector_crossproduct_general(void)
{
    TEST_ASSERT_EQUAL_FLOAT(-2.0f, CP_Vector_CrossProduct(CP_Vector_Set(2.0f, 3.0f), CP_Vector_Set(4.0f, 5.0f)));
}

// ---- Angle family ----

void test_vector_angle_same_direction_is_zero(void)
{
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, CP_Vector_Angle(CP_Vector_Set(1.0f, 0.0f), CP_Vector_Set(1.0f, 0.0f)));
}

void test_vector_angle_perpendicular_is_90(void)
{
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 90.0f, CP_Vector_Angle(CP_Vector_Set(1.0f, 0.0f), CP_Vector_Set(0.0f, 1.0f)));
}

void test_vector_angle_opposite_is_180(void)
{
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 180.0f, CP_Vector_Angle(CP_Vector_Set(1.0f, 0.0f), CP_Vector_Set(-1.0f, 0.0f)));
}

void test_vector_angle_is_unsigned_regardless_of_order(void)
{
    float ab = CP_Vector_Angle(CP_Vector_Set(1.0f, 0.0f), CP_Vector_Set(0.0f, 1.0f));
    float ba = CP_Vector_Angle(CP_Vector_Set(0.0f, 1.0f), CP_Vector_Set(1.0f, 0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, ab, ba);
}

void test_vector_anglecw_quarter_turn(void)
{
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 90.0f, CP_Vector_AngleCW(CP_Vector_Set(1.0f, 0.0f), CP_Vector_Set(0.0f, 1.0f)));
}

void test_vector_anglecw_is_not_symmetric(void)
{
    // Reversing from/to flips which side of the half-turn you land on:
    // 90 deg one way is 270 the other, unlike the unsigned CP_Vector_Angle.
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 270.0f, CP_Vector_AngleCW(CP_Vector_Set(0.0f, 1.0f), CP_Vector_Set(1.0f, 0.0f)));
}

void test_vector_anglecw_same_direction_is_zero(void)
{
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, CP_Vector_AngleCW(CP_Vector_Set(1.0f, 0.0f), CP_Vector_Set(1.0f, 0.0f)));
}

void test_vector_anglecw_opposite_direction_is_180(void)
{
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 180.0f, CP_Vector_AngleCW(CP_Vector_Set(1.0f, 0.0f), CP_Vector_Set(-1.0f, 0.0f)));
}

void test_vector_anglecw_stays_within_0_360(void)
{
    float result = CP_Vector_AngleCW(CP_Vector_Set(0.0f, 1.0f), CP_Vector_Set(1.0f, 0.0f));
    TEST_ASSERT_GREATER_OR_EQUAL_FLOAT(0.0f, result);
    TEST_ASSERT_LESS_THAN_FLOAT(360.0f, result);
}

void test_vector_angleccw_quarter_turn_is_negative(void)
{
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -270.0f, CP_Vector_AngleCCW(CP_Vector_Set(1.0f, 0.0f), CP_Vector_Set(0.0f, 1.0f)));
}

void test_vector_angleccw_same_direction_stays_zero(void)
{
    // The documented (-360, 0] range includes 0 itself: the implementation
    // special-cases an AngleCW of exactly 0 so it doesn't wrap to -360
    // (CP_Math.c:160-164).
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, CP_Vector_AngleCCW(CP_Vector_Set(1.0f, 0.0f), CP_Vector_Set(1.0f, 0.0f)));
}

void test_vector_angleccw_stays_within_negative_360_to_0(void)
{
    float result = CP_Vector_AngleCCW(CP_Vector_Set(0.0f, 1.0f), CP_Vector_Set(1.0f, 0.0f));
    TEST_ASSERT_GREATER_THAN_FLOAT(-360.0f, result);
    TEST_ASSERT_LESS_OR_EQUAL_FLOAT(0.0f, result);
}

void test_vector_angleccw_equals_anglecw_minus_360(void)
{
    CP_Vector from = CP_Vector_Set(1.0f, 0.0f);
    CP_Vector to = CP_Vector_Set(-1.0f, 2.0f);
    float cw = CP_Vector_AngleCW(from, to);
    float ccw = CP_Vector_AngleCCW(from, to);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, cw - 360.0f, ccw);
}

// ---- Lerp, Rotate, FromAngle and Limit ----

void test_vector_lerp_ends_and_middle(void)
{
    CP_Vector a = CP_Vector_Set(0.0f, 10.0f);
    CP_Vector b = CP_Vector_Set(10.0f, 30.0f);
    assertVectorEqual(a, CP_Vector_Lerp(a, b, 0.0f));
    assertVectorEqual(b, CP_Vector_Lerp(a, b, 1.0f));
    assertVectorEqual(CP_Vector_Set(5.0f, 20.0f), CP_Vector_Lerp(a, b, 0.5f));
}

void test_vector_lerp_factor_is_clamped(void)
{
    CP_Vector a = CP_Vector_Set(0.0f, 0.0f);
    CP_Vector b = CP_Vector_Set(10.0f, 10.0f);
    assertVectorEqual(a, CP_Vector_Lerp(a, b, -1.0f));
    assertVectorEqual(b, CP_Vector_Lerp(a, b, 2.0f));
}

void test_vector_rotate_a_quarter_turn_goes_clockwise_on_screen(void)
{
    // y points down, so right turned 90 degrees points down the screen
    CP_Vector v = CP_Vector_Rotate(CP_Vector_Set(1.0f, 0.0f), 90.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, v.x);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, v.y);
}

void test_vector_rotate_keeps_length_and_matches_the_matrix(void)
{
    CP_Vector v = CP_Vector_Set(3.0f, 4.0f);
    CP_Vector turned = CP_Vector_Rotate(v, 37.0f);
    CP_Vector viaMatrix = CP_Vector_MatrixMultiply(CP_Matrix_Rotate(37.0f), v);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 5.0f, CP_Vector_Length(turned));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, viaMatrix.x, turned.x);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, viaMatrix.y, turned.y);
}

void test_vector_rotate_a_full_turn_is_the_same_vector(void)
{
    CP_Vector v = CP_Vector_Rotate(CP_Vector_Set(3.0f, 4.0f), 360.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 3.0f, v.x);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 4.0f, v.y);
}

void test_vector_fromangle_zero_points_right_and_ninety_points_down(void)
{
    CP_Vector right = CP_Vector_FromAngle(0.0f);
    CP_Vector down = CP_Vector_FromAngle(90.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, right.x);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, right.y);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, down.x);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, down.y);
}

void test_vector_fromangle_is_a_unit_vector_that_rotate_agrees_with(void)
{
    CP_Vector v = CP_Vector_FromAngle(123.0f);
    CP_Vector r = CP_Vector_Rotate(CP_Vector_Set(1.0f, 0.0f), 123.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, CP_Vector_Length(v));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, r.x, v.x);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, r.y, v.y);
}

void test_vector_limit_shortens_a_long_vector_keeping_direction(void)
{
    CP_Vector v = CP_Vector_Limit(CP_Vector_Set(30.0f, 40.0f), 5.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 3.0f, v.x);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 4.0f, v.y);
}

void test_vector_limit_leaves_a_short_vector_alone(void)
{
    assertVectorEqual(CP_Vector_Set(3.0f, 4.0f), CP_Vector_Limit(CP_Vector_Set(3.0f, 4.0f), 10.0f));
    assertVectorEqual(CP_Vector_Set(3.0f, 4.0f), CP_Vector_Limit(CP_Vector_Set(3.0f, 4.0f), 5.0f));
}

void test_vector_limit_zero_or_negative_gives_zero(void)
{
    assertVectorEqual(CP_Vector_Zero(), CP_Vector_Limit(CP_Vector_Set(3.0f, 4.0f), 0.0f));
    assertVectorEqual(CP_Vector_Zero(), CP_Vector_Limit(CP_Vector_Set(3.0f, 4.0f), -2.0f));
    assertVectorEqual(CP_Vector_Zero(), CP_Vector_Limit(CP_Vector_Zero(), 3.0f));
}
