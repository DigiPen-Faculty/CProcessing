// Tier 1 coverage for CP_Matrix -- all 10 functions are pure.
#include <math.h>
#include "unity.h"
#include "cprocessing.h"

static void assertMatrixWithin(float tolerance, CP_Matrix expected, CP_Matrix actual)
{
    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected.m00, actual.m00);
    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected.m01, actual.m01);
    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected.m02, actual.m02);
    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected.m10, actual.m10);
    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected.m11, actual.m11);
    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected.m12, actual.m12);
    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected.m20, actual.m20);
    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected.m21, actual.m21);
    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected.m22, actual.m22);
}

static void assertMatrixEqual(CP_Matrix expected, CP_Matrix actual)
{
    assertMatrixWithin(0.0001f, expected, actual);
}

// ---- Construction ----

void test_matrix_set(void)
{
    CP_Matrix m = CP_Matrix_Set(1, 2, 3, 4, 5, 6, 7, 8, 9);
    TEST_ASSERT_EQUAL_FLOAT(1, m.m00); TEST_ASSERT_EQUAL_FLOAT(2, m.m01); TEST_ASSERT_EQUAL_FLOAT(3, m.m02);
    TEST_ASSERT_EQUAL_FLOAT(4, m.m10); TEST_ASSERT_EQUAL_FLOAT(5, m.m11); TEST_ASSERT_EQUAL_FLOAT(6, m.m12);
    TEST_ASSERT_EQUAL_FLOAT(7, m.m20); TEST_ASSERT_EQUAL_FLOAT(8, m.m21); TEST_ASSERT_EQUAL_FLOAT(9, m.m22);
}

void test_matrix_identity(void)
{
    CP_Matrix expected = CP_Matrix_Set(1, 0, 0, 0, 1, 0, 0, 0, 1);
    assertMatrixEqual(expected, CP_Matrix_Identity());
}

void test_matrix_fromvector_places_columns(void)
{
    CP_Matrix m = CP_Matrix_FromVector(CP_Vector_Set(1, 2), CP_Vector_Set(3, 4), CP_Vector_Set(5, 6));
    CP_Matrix expected = CP_Matrix_Set(1, 3, 5, 2, 4, 6, 0, 0, 1);
    assertMatrixEqual(expected, m);
}

void test_matrix_scale(void)
{
    CP_Matrix expected = CP_Matrix_Set(2, 0, 0, 0, 3, 0, 0, 0, 1);
    assertMatrixEqual(expected, CP_Matrix_Scale(CP_Vector_Set(2, 3)));
}

void test_matrix_translate(void)
{
    CP_Matrix expected = CP_Matrix_Set(1, 0, 5, 0, 1, 7, 0, 0, 1);
    assertMatrixEqual(expected, CP_Matrix_Translate(CP_Vector_Set(5, 7)));
}

// ---- Rotation ----

void test_matrix_rotate_zero_is_identity(void)
{
    assertMatrixWithin(0.0001f, CP_Matrix_Identity(), CP_Matrix_Rotate(0.0f));
}

void test_matrix_rotate_90_degrees(void)
{
    CP_Matrix expected = CP_Matrix_Set(0, -1, 0, 1, 0, 0, 0, 0, 1);
    assertMatrixWithin(0.001f, expected, CP_Matrix_Rotate(90.0f));
}

void test_matrix_rotateradians_matches_rotate_degrees(void)
{
    CP_Matrix viaDegrees = CP_Matrix_Rotate(45.0f);
    CP_Matrix viaRadians = CP_Matrix_RotateRadians(CP_Math_Radians(45.0f));
    assertMatrixWithin(0.0001f, viaDegrees, viaRadians);
}

// ---- Transpose ----

void test_matrix_transpose_swaps_off_diagonal(void)
{
    CP_Matrix m = CP_Matrix_Set(1, 2, 3, 4, 5, 6, 7, 8, 9);
    CP_Matrix expected = CP_Matrix_Set(1, 4, 7, 2, 5, 8, 3, 6, 9);
    assertMatrixEqual(expected, CP_Matrix_Transpose(m));
}

void test_matrix_transpose_of_identity_is_identity(void)
{
    assertMatrixEqual(CP_Matrix_Identity(), CP_Matrix_Transpose(CP_Matrix_Identity()));
}

void test_matrix_transpose_twice_returns_original(void)
{
    CP_Matrix m = CP_Matrix_Set(1, 2, 3, 4, 5, 6, 7, 8, 9);
    assertMatrixEqual(m, CP_Matrix_Transpose(CP_Matrix_Transpose(m)));
}

// ---- Inverse ----

void test_matrix_inverse_of_identity_is_identity(void)
{
    assertMatrixWithin(0.0001f, CP_Matrix_Identity(), CP_Matrix_Inverse(CP_Matrix_Identity()));
}

void test_matrix_inverse_of_scale(void)
{
    CP_Matrix scale = CP_Matrix_Scale(CP_Vector_Set(2.0f, 4.0f));
    CP_Matrix expected = CP_Matrix_Scale(CP_Vector_Set(0.5f, 0.25f));
    assertMatrixWithin(0.0001f, expected, CP_Matrix_Inverse(scale));
}

void test_matrix_inverse_times_original_is_identity(void)
{
    CP_Matrix m = CP_Matrix_Multiply(CP_Matrix_Rotate(37.0f), CP_Matrix_Translate(CP_Vector_Set(5.0f, -2.0f)));
    CP_Matrix roundTrip = CP_Matrix_Multiply(CP_Matrix_Inverse(m), m);
    assertMatrixWithin(0.001f, CP_Matrix_Identity(), roundTrip);
}

void test_matrix_inverse_of_singular_matrix_is_not_a_number(void)
{
    // Current, undocumented behavior: CP_Matrix_Inverse divides by the
    // determinant with no singularity check (CP_Math.c:248-268), so a
    // matrix with determinant 0 produces 0/0 (NaN) rather than an error.
    // Asserted explicitly so a future change to this behavior is caught
    // rather than silently tolerated (06-test-suite-plan.md Tier 1 notes).
    CP_Matrix singular = CP_Matrix_Set(1, 1, 1, 1, 1, 1, 1, 1, 1);
    CP_Matrix result = CP_Matrix_Inverse(singular);
    TEST_ASSERT_TRUE(isnan(result.m00));
    TEST_ASSERT_TRUE(isnan(result.m11));
    TEST_ASSERT_TRUE(isnan(result.m22));
}

// ---- Multiply ----

void test_matrix_multiply_by_identity_is_noop(void)
{
    CP_Matrix m = CP_Matrix_Set(1, 2, 3, 4, 5, 6, 7, 8, 9);
    assertMatrixEqual(m, CP_Matrix_Multiply(m, CP_Matrix_Identity()));
    assertMatrixEqual(m, CP_Matrix_Multiply(CP_Matrix_Identity(), m));
}

void test_matrix_multiply_composes_translations(void)
{
    CP_Matrix a = CP_Matrix_Translate(CP_Vector_Set(1.0f, 0.0f));
    CP_Matrix b = CP_Matrix_Translate(CP_Vector_Set(0.0f, 1.0f));
    CP_Matrix expected = CP_Matrix_Translate(CP_Vector_Set(1.0f, 1.0f));
    assertMatrixEqual(expected, CP_Matrix_Multiply(a, b));
}
