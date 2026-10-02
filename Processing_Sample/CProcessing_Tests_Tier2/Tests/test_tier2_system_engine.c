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
    // 0 means "unknown": virtual displays (Xvfb, some VMs/remote sessions)
    // don't report a refresh rate, and GLFW passes that through as 0.
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, tier2_scalars.displayRefreshRate);
    TEST_ASSERT_LESS_OR_EQUAL_INT(1000, tier2_scalars.displayRefreshRate);
}

void test_tier2_engine_pre_and_post_update_hooks_fire(void)
{
    TEST_ASSERT_GREATER_THAN_INT(0, tier2_scalars.preUpdateHookCount);
    TEST_ASSERT_GREATER_THAN_INT(0, tier2_scalars.postUpdateHookCount);
}

void test_tier2_system_window_queries_before_run_report_no_window(void)
{
    TEST_ASSERT_EQUAL_UINT(FALSE, tier2_scalars.windowFocusBeforeRun);
    TEST_ASSERT_EQUAL_INT(0, tier2_scalars.displayRefreshRateBeforeRun);
}

void test_tier2_system_cursor_hidden_before_run_stays_hidden(void)
{
    TEST_ASSERT_EQUAL_UINT(FALSE, tier2_scalars.cursorVisibleBeforeRun);
    TEST_ASSERT_EQUAL_UINT(FALSE, tier2_scalars.cursorVisibleAtStart);
}

void test_tier2_system_cursor_getter_follows_showcursor(void)
{
    TEST_ASSERT_EQUAL_UINT(TRUE, tier2_scalars.cursorVisibleAfterShow);
    TEST_ASSERT_EQUAL_UINT(FALSE, tier2_scalars.cursorVisibleAfterHide);
}

void test_tier2_system_console_hide_then_show_round_trips(void)
{
    TEST_ASSERT_TRUE(tier2_scalars.consoleVisibleBeforeRun == TRUE || tier2_scalars.consoleVisibleBeforeRun == FALSE);
    TEST_ASSERT_TRUE(tier2_scalars.consoleVisibleAfterHide == TRUE || tier2_scalars.consoleVisibleAfterHide == FALSE);
    TEST_ASSERT_EQUAL_UINT(tier2_scalars.consoleVisibleBeforeRun, tier2_scalars.consoleVisibleAfterReshow);
}

void test_tier2_system_console_printf_keeps_working(void)
{
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, tier2_scalars.printfAfterShowConsole);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, tier2_scalars.printfAfterHideConsole);
}
