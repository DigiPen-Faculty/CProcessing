// Assertions against scalars tier2_capture.c already captured. See
// Scn_SystemEngineState in tier2_capture.c for why these have to be read
// during the engine run rather than after.
#include "unity.h"
#include "tier2_capture.h"

void test_tier2_system_window_size_matches_what_was_set(void)
{
    TEST_ASSERT_EQUAL_INT(TIER2_CANVAS_SIZE, tier2_scalars.windowWidthAfterSet);
    TEST_ASSERT_EQUAL_INT(TIER2_CANVAS_SIZE, tier2_scalars.windowHeightAfterSet);
}

void test_tier2_system_frame_count_advances(void)
{
    TEST_ASSERT_GREATER_THAN_UINT(tier2_scalars.frameCountEarly, tier2_scalars.frameCountLater);
}

void test_tier2_system_timing_getters_are_sane(void)
{
    TEST_ASSERT_GREATER_OR_EQUAL_FLOAT(0.0f, tier2_scalars.dtSample);
    TEST_ASSERT_GREATER_OR_EQUAL_FLOAT(0.0f, tier2_scalars.millisSample);
    TEST_ASSERT_GREATER_OR_EQUAL_FLOAT(0.0f, tier2_scalars.secondsSample);
    TEST_ASSERT_GREATER_THAN_FLOAT(0.0f, tier2_scalars.frameRateSample);
}

void test_tier2_system_window_focus_is_a_valid_bool(void)
{
    TEST_ASSERT_TRUE(tier2_scalars.windowFocusSample == TRUE || tier2_scalars.windowFocusSample == FALSE);
}

void test_tier2_system_display_info_is_positive(void)
{
    TEST_ASSERT_GREATER_THAN_INT(0, tier2_scalars.displayWidth);
    TEST_ASSERT_GREATER_THAN_INT(0, tier2_scalars.displayHeight);
    TEST_ASSERT_GREATER_THAN_INT(0, tier2_scalars.displayRefreshRate);
}

void test_tier2_engine_pre_and_post_update_hooks_fire(void)
{
    TEST_ASSERT_GREATER_THAN_INT(0, tier2_scalars.preUpdateHookCount);
    TEST_ASSERT_GREATER_THAN_INT(0, tier2_scalars.postUpdateHookCount);
}
