// Tier 1 coverage for CP_Color -- all 7 functions are pure struct math
// with no GL dependency (CP_Color.c), despite being "graphics-adjacent".
#include "unity.h"
#include "cprocessing.h"

static void assertColorWithin(unsigned char tolerance, CP_Color expected, CP_Color actual)
{
    TEST_ASSERT_UINT8_WITHIN(tolerance, expected.r, actual.r);
    TEST_ASSERT_UINT8_WITHIN(tolerance, expected.g, actual.g);
    TEST_ASSERT_UINT8_WITHIN(tolerance, expected.b, actual.b);
    TEST_ASSERT_UINT8_WITHIN(tolerance, expected.a, actual.a);
}

static void assertColorEqual(CP_Color expected, CP_Color actual)
{
    assertColorWithin(0, expected, actual);
}

// ---- CP_Color_Create ----

void test_color_create_passes_through_in_range_values(void)
{
    CP_Color c = CP_Color_Create(10, 20, 30, 40);
    assertColorEqual((CP_Color) { .r = 10, .g = 20, .b = 30, .a = 40 }, c);
}

void test_color_create_clamps_below_zero(void)
{
    CP_Color c = CP_Color_Create(-5, -5, -5, -5);
    assertColorEqual((CP_Color) { .r = 0, .g = 0, .b = 0, .a = 0 }, c);
}

void test_color_create_clamps_above_255(void)
{
    CP_Color c = CP_Color_Create(300, 300, 300, 300);
    assertColorEqual((CP_Color) { .r = 255, .g = 255, .b = 255, .a = 255 }, c);
}

// ---- CP_Color_CreateHex ----

void test_color_createhex_red_opaque(void)
{
    CP_Color c = CP_Color_CreateHex(0xFF0000FF);
    assertColorEqual((CP_Color) { .r = 255, .g = 0, .b = 0, .a = 255 }, c);
}

void test_color_createhex_green_opaque(void)
{
    CP_Color c = CP_Color_CreateHex(0x00FF00FF);
    assertColorEqual((CP_Color) { .r = 0, .g = 255, .b = 0, .a = 255 }, c);
}

void test_color_createhex_half_alpha(void)
{
    CP_Color c = CP_Color_CreateHex(0x00000080);
    assertColorEqual((CP_Color) { .r = 0, .g = 0, .b = 0, .a = 128 }, c);
}

// ---- CP_Color_Lerp ----

void test_color_lerp_factor_zero_returns_a(void)
{
    CP_Color a = CP_Color_Create(0, 0, 0, 255);
    CP_Color b = CP_Color_Create(255, 255, 255, 255);
    assertColorEqual(a, CP_Color_Lerp(a, b, 0.0f));
}

void test_color_lerp_factor_one_returns_b(void)
{
    CP_Color a = CP_Color_Create(0, 0, 0, 255);
    CP_Color b = CP_Color_Create(255, 255, 255, 255);
    assertColorEqual(b, CP_Color_Lerp(a, b, 1.0f));
}

void test_color_lerp_factor_half(void)
{
    CP_Color a = CP_Color_Create(0, 0, 0, 255);
    CP_Color b = CP_Color_Create(200, 200, 200, 255);
    CP_Color result = CP_Color_Lerp(a, b, 0.5f);
    assertColorWithin(1, (CP_Color) { .r = 100, .g = 100, .b = 100, .a = 255 }, result);
}

void test_color_lerp_factor_out_of_range_clamps(void)
{
    CP_Color a = CP_Color_Create(0, 0, 0, 255);
    CP_Color b = CP_Color_Create(255, 255, 255, 255);
    assertColorEqual(a, CP_Color_Lerp(a, b, -1.0f));
    assertColorEqual(b, CP_Color_Lerp(a, b, 2.0f));
}

// ---- CP_ColorHSL_Create ----

void test_colorhsl_create_passes_through_in_range_values(void)
{
    CP_ColorHSL hsl = CP_ColorHSL_Create(180, 50, 50, 255);
    TEST_ASSERT_EQUAL_INT(180, hsl.h);
    TEST_ASSERT_EQUAL_INT(50, hsl.s);
    TEST_ASSERT_EQUAL_INT(50, hsl.l);
    TEST_ASSERT_EQUAL_INT(255, hsl.a);
}

void test_colorhsl_create_wraps_negative_hue(void)
{
    // clampH wraps rather than clamping, unlike every other channel here
    // (CP_Color.c:46-51) -- -10 degrees is the same hue as 350 degrees.
    CP_ColorHSL hsl = CP_ColorHSL_Create(-10, 50, 50, 255);
    TEST_ASSERT_EQUAL_INT(350, hsl.h);
}

void test_colorhsl_create_wraps_hue_above_360(void)
{
    CP_ColorHSL hsl = CP_ColorHSL_Create(370, 50, 50, 255);
    TEST_ASSERT_EQUAL_INT(10, hsl.h);
}

void test_colorhsl_create_clamps_saturation_and_lightness(void)
{
    CP_ColorHSL hsl = CP_ColorHSL_Create(0, -10, 150, 255);
    TEST_ASSERT_EQUAL_INT(0, hsl.s);
    TEST_ASSERT_EQUAL_INT(100, hsl.l);
}

// ---- CP_ColorHSL_Lerp ----

void test_colorhsl_lerp_factor_zero_returns_a(void)
{
    CP_ColorHSL a = CP_ColorHSL_Create(0, 0, 0, 255);
    CP_ColorHSL b = CP_ColorHSL_Create(300, 100, 100, 255);
    CP_ColorHSL result = CP_ColorHSL_Lerp(a, b, 0.0f);
    TEST_ASSERT_EQUAL_INT(a.h, result.h);
    TEST_ASSERT_EQUAL_INT(a.s, result.s);
    TEST_ASSERT_EQUAL_INT(a.l, result.l);
}

void test_colorhsl_lerp_factor_one_returns_b(void)
{
    CP_ColorHSL a = CP_ColorHSL_Create(0, 0, 0, 255);
    CP_ColorHSL b = CP_ColorHSL_Create(300, 100, 100, 255);
    CP_ColorHSL result = CP_ColorHSL_Lerp(a, b, 1.0f);
    TEST_ASSERT_EQUAL_INT(b.h, result.h);
    TEST_ASSERT_EQUAL_INT(b.s, result.s);
    TEST_ASSERT_EQUAL_INT(b.l, result.l);
}

// ---- CP_Color_FromColorHSL ----

void test_color_fromcolorhsl_red(void)
{
    CP_Color c = CP_Color_FromColorHSL(CP_ColorHSL_Create(0, 100, 50, 255));
    assertColorWithin(1, (CP_Color) { .r = 255, .g = 0, .b = 0, .a = 255 }, c);
}

void test_color_fromcolorhsl_green(void)
{
    CP_Color c = CP_Color_FromColorHSL(CP_ColorHSL_Create(120, 100, 50, 255));
    assertColorWithin(1, (CP_Color) { .r = 0, .g = 255, .b = 0, .a = 255 }, c);
}

void test_color_fromcolorhsl_blue(void)
{
    CP_Color c = CP_Color_FromColorHSL(CP_ColorHSL_Create(240, 100, 50, 255));
    assertColorWithin(1, (CP_Color) { .r = 0, .g = 0, .b = 255, .a = 255 }, c);
}

void test_color_fromcolorhsl_white(void)
{
    CP_Color c = CP_Color_FromColorHSL(CP_ColorHSL_Create(0, 0, 100, 255));
    assertColorWithin(1, (CP_Color) { .r = 255, .g = 255, .b = 255, .a = 255 }, c);
}

void test_color_fromcolorhsl_black(void)
{
    CP_Color c = CP_Color_FromColorHSL(CP_ColorHSL_Create(0, 0, 0, 255));
    assertColorWithin(1, (CP_Color) { .r = 0, .g = 0, .b = 0, .a = 255 }, c);
}

// ---- CP_ColorHSL_FromColor ----

void test_colorhsl_fromcolor_red(void)
{
    CP_ColorHSL hsl = CP_ColorHSL_FromColor(CP_Color_Create(255, 0, 0, 255));
    TEST_ASSERT_EQUAL_INT(0, hsl.h);
    TEST_ASSERT_EQUAL_INT(100, hsl.s);
    TEST_ASSERT_EQUAL_INT(50, hsl.l);
}

void test_colorhsl_fromcolor_green(void)
{
    CP_ColorHSL hsl = CP_ColorHSL_FromColor(CP_Color_Create(0, 255, 0, 255));
    TEST_ASSERT_EQUAL_INT(120, hsl.h);
    TEST_ASSERT_EQUAL_INT(100, hsl.s);
    TEST_ASSERT_EQUAL_INT(50, hsl.l);
}

void test_colorhsl_fromcolor_blue(void)
{
    CP_ColorHSL hsl = CP_ColorHSL_FromColor(CP_Color_Create(0, 0, 255, 255));
    TEST_ASSERT_EQUAL_INT(240, hsl.h);
    TEST_ASSERT_EQUAL_INT(100, hsl.s);
    TEST_ASSERT_EQUAL_INT(50, hsl.l);
}

void test_colorhsl_fromcolor_white_has_zero_saturation(void)
{
    CP_ColorHSL hsl = CP_ColorHSL_FromColor(CP_Color_Create(255, 255, 255, 255));
    TEST_ASSERT_EQUAL_INT(0, hsl.s);
    TEST_ASSERT_EQUAL_INT(100, hsl.l);
}

void test_colorhsl_fromcolor_black_has_zero_lightness(void)
{
    CP_ColorHSL hsl = CP_ColorHSL_FromColor(CP_Color_Create(0, 0, 0, 255));
    TEST_ASSERT_EQUAL_INT(0, hsl.s);
    TEST_ASSERT_EQUAL_INT(0, hsl.l);
}

void test_colorhsl_fromcolor_preserves_alpha(void)
{
    CP_ColorHSL hsl = CP_ColorHSL_FromColor(CP_Color_Create(100, 150, 200, 77));
    TEST_ASSERT_EQUAL_INT(77, hsl.a);
}
