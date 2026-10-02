// Tier 2 checks added with the cross-platform work: behavior every platform
// must share (drawing persisting through the offscreen canvas, screenshots of
// sub-regions, the native window handle), plus error paths that used to
// crash. See the matching Scn_* functions in tier2_capture.c.
#include "unity.h"
#include "tier2_capture.h"

static void assertColorWithin(unsigned char tolerance, CP_Color expected, CP_Color actual)
{
    TEST_ASSERT_UINT8_WITHIN(tolerance, expected.r, actual.r);
    TEST_ASSERT_UINT8_WITHIN(tolerance, expected.g, actual.g);
    TEST_ASSERT_UINT8_WITHIN(tolerance, expected.b, actual.b);
}

#define RED_C CP_Color_Create(255, 0, 0, 255)
#define GREEN_C CP_Color_Create(0, 200, 0, 255)
#define BLUE_C CP_Color_Create(0, 0, 255, 255)
#define YELLOW_C CP_Color_Create(255, 255, 0, 255)
#define WHITE_C CP_Color_Create(255, 255, 255, 255)

void test_tier2_drawing_persists_into_the_next_frame(void)
{
    // Frame 1 drew the red square; frame 2 drew only the blue one and did
    // not clear, so the red square must still be there.
    assertColorWithin(2, RED_C, tier2_SamplePixel(SCN_PERSIST_NEXT_FRAME, 50, 50));
    assertColorWithin(2, BLUE_C, tier2_SamplePixel(SCN_PERSIST_NEXT_FRAME, 150, 150));
    assertColorWithin(2, WHITE_C, tier2_SamplePixel(SCN_PERSIST_NEXT_FRAME, 150, 50));
}

void test_tier2_first_frame_did_not_contain_the_later_drawing(void)
{
    // Sanity check on the persistence test: the blue square really was
    // drawn only in the second frame.
    assertColorWithin(2, RED_C, tier2_SamplePixel(SCN_PERSIST_FIRST_FRAME, 50, 50));
    assertColorWithin(2, WHITE_C, tier2_SamplePixel(SCN_PERSIST_FIRST_FRAME, 150, 150));
}

void test_tier2_screenshot_subregion_has_requested_size(void)
{
    TEST_ASSERT_EQUAL_INT(100, tier2_scalars.subTopRightWidth);
    TEST_ASSERT_EQUAL_INT(100, tier2_scalars.subTopRightHeight);
}

void test_tier2_screenshot_subregion_reads_the_right_place(void)
{
    assertColorWithin(2, GREEN_C, tier2_scalars.subTopRightCenter);
    assertColorWithin(2, BLUE_C, tier2_scalars.subBottomLeftCenter);
}

void test_tier2_screenshot_subregion_is_not_flipped(void)
{
    // A region straddling all four quadrants keeps them in place: red top
    // left, green top right, blue bottom left, yellow bottom right.
    assertColorWithin(2, RED_C, tier2_scalars.straddleCorners[0]);
    assertColorWithin(2, GREEN_C, tier2_scalars.straddleCorners[1]);
    assertColorWithin(2, BLUE_C, tier2_scalars.straddleCorners[2]);
    assertColorWithin(2, YELLOW_C, tier2_scalars.straddleCorners[3]);
}

void test_tier2_missing_files_load_as_null(void)
{
    TEST_ASSERT_TRUE(tier2_scalars.missingImageIsNull);
    TEST_ASSERT_TRUE(tier2_scalars.missingFontIsNull);
    TEST_ASSERT_TRUE(tier2_scalars.missingSoundIsNull);
}

void test_tier2_null_handles_are_ignored(void)
{
    TEST_ASSERT_TRUE(tier2_scalars.survivedNullCalls);
}

void test_tier2_window_handle_is_available(void)
{
#if defined(_WIN32) || defined(__APPLE__)
    TEST_ASSERT_FALSE(tier2_scalars.windowHandleIsNull);
#else
    // Linux: the X11 window id under X11, but NULL under Wayland (there is
    // no global window handle there), so either is correct.
    TEST_PASS_MESSAGE("window handle may legitimately be NULL on Linux (Wayland)");
#endif
}
