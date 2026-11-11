// Assertions against the pixel buffers tier2_capture.c already captured.
// These never touch the engine -- see tier2_capture.h for why.
#include "unity.h"
#include "tier2_capture.h"

static void assertColorWithin(unsigned char tolerance, CP_Color expected, CP_Color actual)
{
    TEST_ASSERT_UINT8_WITHIN(tolerance, expected.r, actual.r);
    TEST_ASSERT_UINT8_WITHIN(tolerance, expected.g, actual.g);
    TEST_ASSERT_UINT8_WITHIN(tolerance, expected.b, actual.b);
}

void test_tier2_graphics_clearbackground_fills_whole_canvas(void)
{
    CP_Color expected = CP_Color_Create(10, 20, 30, 255);
    assertColorWithin(2, expected, tier2_SamplePixel(SCN_CLEAR_BACKGROUND, 100, 100));
    assertColorWithin(2, expected, tier2_SamplePixel(SCN_CLEAR_BACKGROUND, 5, 5));
}

void test_tier2_graphics_drawpoint_follows_stroke_settings(void)
{
    // See Scn_DrawPoint in tier2_capture.c
    const CP_Color red = CP_Color_Create(255, 0, 0, 255);
    const CP_Color white = CP_Color_Create(255, 255, 255, 255);

    // Round cap: the middle is drawn, the corner of its bounding square isn't
    assertColorWithin(2, red, tier2_SamplePixel(SCN_DRAW_POINT, 50, 50));
    assertColorWithin(2, white, tier2_SamplePixel(SCN_DRAW_POINT, 59, 59));
    // Square cap: the corner is drawn too
    assertColorWithin(2, red, tier2_SamplePixel(SCN_DRAW_POINT, 150, 50));
    assertColorWithin(2, red, tier2_SamplePixel(SCN_DRAW_POINT, 158, 58));
    // The rect drawn after the points still uses the fill color
    assertColorWithin(2, CP_Color_Create(0, 255, 0, 255), tier2_SamplePixel(SCN_DRAW_POINT, 100, 100));
    // Hidden by no stroke, not by no fill
    assertColorWithin(2, white, tier2_SamplePixel(SCN_DRAW_POINT, 100, 150));
    assertColorWithin(2, red, tier2_SamplePixel(SCN_DRAW_POINT, 50, 150));
    // Red again after CP_Settings_Restore, not blue
    assertColorWithin(2, red, tier2_SamplePixel(SCN_DRAW_POINT, 150, 150));
    assertColorWithin(2, white, tier2_SamplePixel(SCN_DRAW_POINT, 10, 10));
}

void test_tier2_graphics_drawline(void)
{
    assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_DRAW_LINE, 100, 100));
    assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_DRAW_LINE, 100, 20));
}

void test_tier2_graphics_drawlineadvanced_rotates_90_degrees(void)
{
    assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_DRAW_LINE_ADVANCED, 100, 100));
    // The unrotated line would have passed through here; after a 90 degree
    // rotation about its midpoint it no longer does.
    assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_DRAW_LINE_ADVANCED, 20, 100));
}

void test_tier2_graphics_drawrect(void)
{
    assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_DRAW_RECT, 100, 100));
    assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_DRAW_RECT, 10, 10));
}

void test_tier2_graphics_drawrectadvanced_rotates_45_degrees(void)
{
    // Along the rotated long axis: still inside.
    assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_DRAW_RECT_ADVANCED, 125, 125));
    // Along the *original*, unrotated long axis: now outside.
    assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_DRAW_RECT_ADVANCED, 135, 100));
}

void test_tier2_graphics_drawcircle(void)
{
    assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_DRAW_CIRCLE, 100, 100));
    assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_DRAW_CIRCLE, 10, 10));
}

void test_tier2_graphics_drawellipse_is_not_circular(void)
{
    assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_DRAW_ELLIPSE, 100, 100));
    // 35px above center: inside the half-width-60 horizontal reach but
    // outside the half-height-20 vertical reach -- proves w and h aren't
    // both being treated as the same radius.
    assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_DRAW_ELLIPSE, 100, 135));
    assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_DRAW_ELLIPSE, 150, 100));
}

void test_tier2_graphics_drawellipseadvanced_rotates_90_degrees(void)
{
    // Exactly swapped from the unrotated case above: the wide axis is now vertical.
    assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_DRAW_ELLIPSE_ADVANCED, 100, 135));
    assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_DRAW_ELLIPSE_ADVANCED, 150, 100));
}

void test_tier2_graphics_drawarc_goes_clockwise_from_the_right_and_has_no_fill(void)
{
    const CP_Color black = CP_Color_Create(0, 0, 0, 255);
    const CP_Color white = CP_Color_Create(255, 255, 255, 255);
    // Starts to the right of the center, ends below it, and is on the circle
    // half way between (just inside each end, where the line is not cut off)
    assertColorWithin(2, black, tier2_SamplePixel(SCN_DRAW_ARC, 180, 104));
    assertColorWithin(2, black, tier2_SamplePixel(SCN_DRAW_ARC, 104, 180));
    assertColorWithin(2, black, tier2_SamplePixel(SCN_DRAW_ARC, 156, 156));
    // The other three quarters of the circle are not drawn
    assertColorWithin(2, white, tier2_SamplePixel(SCN_DRAW_ARC, 100, 20));
    assertColorWithin(2, white, tier2_SamplePixel(SCN_DRAW_ARC, 20, 100));
    // And it is only a line, even with a red fill set
    assertColorWithin(2, white, tier2_SamplePixel(SCN_DRAW_ARC, 140, 140));
}

void test_tier2_graphics_drawarc_with_a_negative_sweep_goes_counterclockwise(void)
{
    const CP_Color black = CP_Color_Create(0, 0, 0, 255);
    const CP_Color white = CP_Color_Create(255, 255, 255, 255);
    assertColorWithin(2, black, tier2_SamplePixel(SCN_DRAW_ARC_COUNTERCLOCKWISE, 180, 95));
    assertColorWithin(2, black, tier2_SamplePixel(SCN_DRAW_ARC_COUNTERCLOCKWISE, 156, 44));
    assertColorWithin(2, white, tier2_SamplePixel(SCN_DRAW_ARC_COUNTERCLOCKWISE, 156, 156));
    assertColorWithin(2, white, tier2_SamplePixel(SCN_DRAW_ARC_COUNTERCLOCKWISE, 104, 180));
}

void test_tier2_graphics_drawpie_fills_only_its_slice(void)
{
    const CP_Color red = CP_Color_Create(255, 0, 0, 255);
    const CP_Color white = CP_Color_Create(255, 255, 255, 255);
    assertColorWithin(2, red, tier2_SamplePixel(SCN_DRAW_PIE, 140, 140));
    assertColorWithin(2, white, tier2_SamplePixel(SCN_DRAW_PIE, 60, 60));
    assertColorWithin(2, white, tier2_SamplePixel(SCN_DRAW_PIE, 140, 60));
    assertColorWithin(2, white, tier2_SamplePixel(SCN_DRAW_PIE, 60, 140));
}

void test_tier2_graphics_drawpie_in_corner_mode_uses_the_top_left(void)
{
    const CP_Color red = CP_Color_Create(255, 0, 0, 255);
    const CP_Color white = CP_Color_Create(255, 255, 255, 255);
    assertColorWithin(2, red, tier2_SamplePixel(SCN_DRAW_PIE_CORNER_MODE, 140, 140));
    assertColorWithin(2, white, tier2_SamplePixel(SCN_DRAW_PIE_CORNER_MODE, 60, 60));
}

void test_tier2_graphics_drawbezier_bends_toward_its_control_points(void)
{
    const CP_Color black = CP_Color_Create(0, 0, 0, 255);
    const CP_Color white = CP_Color_Create(255, 255, 255, 255);
    // A straight line between the ends would be along y = 150
    assertColorWithin(2, black, tier2_SamplePixel(SCN_DRAW_BEZIER, 100, 52));
    assertColorWithin(2, white, tier2_SamplePixel(SCN_DRAW_BEZIER, 100, 150));
    // Under the curve is empty: it is a line, with no fill
    assertColorWithin(2, white, tier2_SamplePixel(SCN_DRAW_BEZIER, 100, 100));
}

void test_tier2_graphics_drawtriangle(void)
{
    assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_DRAW_TRIANGLE, 100, 100));
    assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_DRAW_TRIANGLE, 100, 20));
}

void test_tier2_graphics_drawtriangleadvanced_rotates_180_degrees(void)
{
    // Centroid stays inside any rotation about itself.
    assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_DRAW_TRIANGLE_ADVANCED, 100, 100));
    // The original apex was at y=50; after a 180 degree flip about the
    // centroid, the triangle's topmost edge is at y=75, so y=55 is now
    // above the entire shape.
    assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_DRAW_TRIANGLE_ADVANCED, 100, 55));
}

void test_tier2_graphics_drawquad(void)
{
    assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_DRAW_QUAD, 100, 100));
    // 35px above center: outside the axis-aligned square's half-size of 30.
    assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_DRAW_QUAD, 100, 65));
}

void test_tier2_graphics_drawquadadvanced_rotates_45_degrees(void)
{
    assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_DRAW_QUAD_ADVANCED, 100, 100));
    // Same point that was *outside* the unrotated square (35 > half-size
    // 30) is now *inside* the rotated diamond, whose reach along the pure
    // vertical axis grew to 30*sqrt(2) =~ 42.4.
    assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_DRAW_QUAD_ADVANCED, 100, 65));
    assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_DRAW_QUAD_ADVANCED, 100, 30));
}

void test_tier2_graphics_customshape_beginend(void)
{
    assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_CUSTOM_SHAPE, 100, 100));
    assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_CUSTOM_SHAPE, 10, 10));
}
