// Tier 1 coverage for Source/Internal_Monitor.h: which monitor a window is
// on (fullscreen, the display size and centering use it) and centering a
// window on a monitor. Pure rectangle math, so any monitor layout can be
// tested without the monitors.
#include "unity.h"
#include "Internal_Monitor.h"

// A 1920x1080 primary monitor with a 2560x1440 one to its right
static const CP_ScreenRect kSideBySide[2] = {
    { 0, 0, 1920, 1080 },
    { 1920, 0, 2560, 1440 },
};

static CP_ScreenRect Rect(int x, int y, int width, int height)
{
    CP_ScreenRect rect = { x, y, width, height };
    return rect;
}

void test_monitor_window_inside_one_monitor(void)
{
    TEST_ASSERT_EQUAL_INT(0, CP_Monitor_MostOfWindow(Rect(100, 100, 800, 600), kSideBySide, 2));
    TEST_ASSERT_EQUAL_INT(1, CP_Monitor_MostOfWindow(Rect(2500, 300, 800, 600), kSideBySide, 2));
}

void test_monitor_window_across_two_monitors_picks_the_larger_part(void)
{
    // 1720..2520: 200 px on the first monitor, 600 on the second
    TEST_ASSERT_EQUAL_INT(1, CP_Monitor_MostOfWindow(Rect(1720, 100, 800, 600), kSideBySide, 2));
    // 1320..2120: 600 px on the first monitor, 200 on the second
    TEST_ASSERT_EQUAL_INT(0, CP_Monitor_MostOfWindow(Rect(1320, 100, 800, 600), kSideBySide, 2));
}

void test_monitor_window_split_evenly_picks_the_earlier_monitor(void)
{
    TEST_ASSERT_EQUAL_INT(0, CP_Monitor_MostOfWindow(Rect(1520, 100, 800, 600), kSideBySide, 2));
}

void test_monitor_window_below_a_shorter_monitor(void)
{
    // y 1200..1400 is below the 1080-tall first monitor but on the second
    TEST_ASSERT_EQUAL_INT(1, CP_Monitor_MostOfWindow(Rect(1800, 1200, 400, 200), kSideBySide, 2));
}

void test_monitor_window_on_no_monitor(void)
{
    TEST_ASSERT_EQUAL_INT(-1, CP_Monitor_MostOfWindow(Rect(-900, 100, 800, 600), kSideBySide, 2));
    TEST_ASSERT_EQUAL_INT(-1, CP_Monitor_MostOfWindow(Rect(100, 100, 0, 0), kSideBySide, 2));
    TEST_ASSERT_EQUAL_INT(-1, CP_Monitor_MostOfWindow(Rect(100, 100, 800, 600), kSideBySide, 0));
}

void test_monitor_left_of_the_primary_has_negative_coordinates(void)
{
    const CP_ScreenRect leftAndPrimary[2] = {
        { 0, 0, 1920, 1080 },
        { -1280, 0, 1280, 1024 },
    };
    TEST_ASSERT_EQUAL_INT(1, CP_Monitor_MostOfWindow(Rect(-1000, 200, 640, 480), leftAndPrimary, 2));
}

void test_monitor_center_on_the_primary_monitor_matches_the_old_formula(void)
{
    // Before 3.0.2: native / 2 - size / 2, on the primary monitor at (0, 0)
    int x = 0, y = 0;
    CP_Monitor_Center(kSideBySide[0], 1281, 721, &x, &y);
    TEST_ASSERT_EQUAL_INT(1920 / 2 - 1281 / 2, x);
    TEST_ASSERT_EQUAL_INT(1080 / 2 - 721 / 2, y);
}

void test_monitor_center_on_a_second_monitor(void)
{
    int x = 0, y = 0;
    CP_Monitor_Center(kSideBySide[1], 1280, 720, &x, &y);
    TEST_ASSERT_EQUAL_INT(1920 + 640, x);
    TEST_ASSERT_EQUAL_INT(360, y);
}
