// Assertions against data tier2_capture.c already captured. See
// Scn_ImageLoadAndDraw / Scn_ImageSubImage there for how Assets/quadrants.png
// (a 4x4 fixture: TL=red, TR=green, BL=blue, BR=yellow) was used.
#include "unity.h"
#include "tier2_capture.h"

static void assertColorWithin(unsigned char tolerance, CP_Color expected, CP_Color actual)
{
    TEST_ASSERT_UINT8_WITHIN(tolerance, expected.r, actual.r);
    TEST_ASSERT_UINT8_WITHIN(tolerance, expected.g, actual.g);
    TEST_ASSERT_UINT8_WITHIN(tolerance, expected.b, actual.b);
}

void test_tier2_image_load_reports_correct_dimensions(void)
{
    TEST_ASSERT_EQUAL_INT(4, tier2_scalars.quadImageWidth);
    TEST_ASSERT_EQUAL_INT(4, tier2_scalars.quadImageHeight);
}

void test_tier2_image_draw_places_quadrants_correctly(void)
{
    // Drawn as an 80x80 square centered at (100,100): each 40x40 quadrant's center.
    assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_IMAGE_LOAD_AND_DRAW, 80, 80));   // TL red
    assertColorWithin(2, CP_Color_Create(0, 255, 0, 255), tier2_SamplePixel(SCN_IMAGE_LOAD_AND_DRAW, 120, 80));  // TR green
    assertColorWithin(2, CP_Color_Create(0, 0, 255, 255), tier2_SamplePixel(SCN_IMAGE_LOAD_AND_DRAW, 80, 120));  // BL blue
    assertColorWithin(2, CP_Color_Create(255, 255, 0, 255), tier2_SamplePixel(SCN_IMAGE_LOAD_AND_DRAW, 120, 120)); // BR yellow
}

void test_tier2_image_createfromdata_roundtrips_exactly(void)
{
    CP_Color expected[4] = {
        CP_Color_Create(10, 20, 30, 255), CP_Color_Create(40, 50, 60, 255),
        CP_Color_Create(70, 80, 90, 255), CP_Color_Create(100, 110, 120, 255)
    };
    for (int i = 0; i < 4; ++i)
    {
        assertColorWithin(0, expected[i], tier2_scalars.createFromDataReadback[i]);
    }
}

void test_tier2_image_updatepixeldata_roundtrips_exactly(void)
{
    CP_Color expected[4] = {
        CP_Color_Create(200, 0, 0, 255), CP_Color_Create(0, 200, 0, 255),
        CP_Color_Create(0, 0, 200, 255), CP_Color_Create(200, 200, 0, 255)
    };
    for (int i = 0; i < 4; ++i)
    {
        assertColorWithin(0, expected[i], tier2_scalars.updatePixelDataReadback[i]);
    }
}

void test_tier2_image_drawsubimage_selects_correct_region(void)
{
    // u:[0.5,1.0] v:[0.0,0.5] selects only the top-right (green) quadrant.
    assertColorWithin(2, CP_Color_Create(0, 255, 0, 255), tier2_SamplePixel(SCN_IMAGE_SUBIMAGE, 100, 100));
    assertColorWithin(2, CP_Color_Create(0, 255, 0, 255), tier2_SamplePixel(SCN_IMAGE_SUBIMAGE, 75, 75));
    assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_IMAGE_SUBIMAGE, 20, 20));
}
