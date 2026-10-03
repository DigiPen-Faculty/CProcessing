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

void test_tier2_image_16_bit_png_loads_and_draws(void)
{
    // See Scn_Image16Bit: drawn like quadrants.png, 80x80 centered
    TEST_ASSERT_EQUAL_INT(4, tier2_scalars.image16BitWidth);
    assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_IMAGE_16_BIT, 80, 80));
    assertColorWithin(2, CP_Color_Create(0, 255, 0, 255), tier2_SamplePixel(SCN_IMAGE_16_BIT, 120, 80));
    assertColorWithin(2, CP_Color_Create(0, 0, 255, 255), tier2_SamplePixel(SCN_IMAGE_16_BIT, 80, 120));
    assertColorWithin(2, CP_Color_Create(255, 255, 0, 255), tier2_SamplePixel(SCN_IMAGE_16_BIT, 120, 120));
}

void test_tier2_image_filter_mode_nearest_and_linear(void)
{
    // See Scn_ImageFilterModes. Nearest: four solid colors meeting in the
    // middle of the left half.
    assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_IMAGE_FILTER_MODES, 45, 95));
    assertColorWithin(2, CP_Color_Create(0, 255, 0, 255), tier2_SamplePixel(SCN_IMAGE_FILTER_MODES, 55, 95));
    assertColorWithin(2, CP_Color_Create(0, 0, 255, 255), tier2_SamplePixel(SCN_IMAGE_FILTER_MODES, 45, 105));
    assertColorWithin(2, CP_Color_Create(255, 255, 0, 255), tier2_SamplePixel(SCN_IMAGE_FILTER_MODES, 55, 105));
    // Linear: the middle of the right half averages all four
    assertColorWithin(10, CP_Color_Create(128, 128, 64, 255), tier2_SamplePixel(SCN_IMAGE_FILTER_MODES, 150, 100));
}

void test_tier2_image_wrap_modes(void)
{
    // See Scn_ImageWrapModes. Each source pixel is 20x20 on screen; the
    // samples are in the top row, 2.5 and 3.5 source pixels across, past
    // the image's right edge (top row: red, green).
    const CP_Color red = CP_Color_Create(255, 0, 0, 255);
    const CP_Color green = CP_Color_Create(0, 255, 0, 255);
    const CP_Color white = CP_Color_Create(255, 255, 255, 255);
    struct { int x, y; CP_Color past, farther; } tiles[4] = {
        { 10, 10, white, white },     // clamp: nothing past the edge
        { 110, 10, green, green },    // clamp to edge: the edge pixel repeats
        { 10, 110, red, green },      // repeat: the image again
        { 110, 110, green, red },     // mirror: the image reversed
    };
    for (int i = 0; i < 4; ++i)
    {
        assertColorWithin(2, red, tier2_SamplePixel(SCN_IMAGE_WRAP_MODES, tiles[i].x + 10, tiles[i].y + 10));
        assertColorWithin(2, tiles[i].past, tier2_SamplePixel(SCN_IMAGE_WRAP_MODES, tiles[i].x + 50, tiles[i].y + 10));
        assertColorWithin(2, tiles[i].farther, tier2_SamplePixel(SCN_IMAGE_WRAP_MODES, tiles[i].x + 70, tiles[i].y + 10));
    }
}

void test_tier2_image_many_images_load_draw_and_free(void)
{
    // See Scn_ImageMany: 40 images, more than the image list starts with.
    TEST_ASSERT_EQUAL_INT(40, tier2_scalars.manyImagesCreated);
    TEST_ASSERT_EQUAL_INT(4, tier2_scalars.manyImagesFileWidth);
    TEST_ASSERT_EQUAL_INT(40, tier2_scalars.manyImagesFreed);
    // The first and the last were drawn: 10x10 squares at (0,0) and (140,80)
    assertColorWithin(2, CP_Color_Create(0, 0, 255, 255), tier2_SamplePixel(SCN_IMAGE_MANY, 5, 5));
    assertColorWithin(2, CP_Color_Create(0, 0, 255, 255), tier2_SamplePixel(SCN_IMAGE_MANY, 145, 85));
}
