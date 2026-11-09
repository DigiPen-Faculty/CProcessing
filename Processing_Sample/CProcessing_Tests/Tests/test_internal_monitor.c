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

static const CP_ScreenFrame kNoFrame = { 0, 0, 0, 0 };

void test_monitor_center_on_the_primary_monitor_matches_the_old_formula(void)
{
    // Before 3.0.2: native / 2 - size / 2, on the primary monitor at (0, 0)
    int x = 0, y = 0;
    CP_Monitor_Center(kSideBySide[0], 1281, 721, kNoFrame, &x, &y);
    TEST_ASSERT_EQUAL_INT(1920 / 2 - 1281 / 2, x);
    TEST_ASSERT_EQUAL_INT(1080 / 2 - 721 / 2, y);
}

void test_monitor_center_on_a_second_monitor(void)
{
    int x = 0, y = 0;
    CP_Monitor_Center(kSideBySide[1], 1280, 720, kNoFrame, &x, &y);
    TEST_ASSERT_EQUAL_INT(1920 + 640, x);
    TEST_ASSERT_EQUAL_INT(360, y);
}

void test_monitor_center_includes_the_title_bar(void)
{
    // A Mac's 1440x900 screen with a 33 point menu bar, and a window with a
    // 28 point title bar: the 748 point tall window is centered below the
    // menu bar, and its content starts under the title bar
    const CP_ScreenRect belowMenuBar = { 0, 33, 1440, 867 };
    const CP_ScreenFrame titleBar = { 0, 28, 0, 0 };
    int x = 0, y = 0;
    CP_Monitor_Center(belowMenuBar, 1280, 720, titleBar, &x, &y);
    TEST_ASSERT_EQUAL_INT(80, x);
    TEST_ASSERT_EQUAL_INT(33 + 867 / 2 - 748 / 2 + 28, y);
}

void test_monitor_center_too_big_keeps_the_title_bar_on_screen(void)
{
    // Windows-style borders on a window taller and wider than the area: it
    // goes at the area's top left, with its title bar and borders on screen
    const CP_ScreenRect aboveTaskbar = { 0, 0, 1920, 1032 };
    const CP_ScreenFrame borders = { 8, 31, 8, 8 };
    int x = 0, y = 0;
    CP_Monitor_Center(aboveTaskbar, 1920, 1080, borders, &x, &y);
    TEST_ASSERT_EQUAL_INT(8, x);
    TEST_ASSERT_EQUAL_INT(31, y);
}

void test_monitor_overlap(void)
{
    // A work area still sized for a 1920x1200 fullscreen mode, on a monitor
    // back at 1440x900: only the part on the monitor counts
    const CP_ScreenRect overlap = CP_Monitor_Overlap(Rect(0, -300, 1920, 1200), Rect(0, 0, 1440, 900));
    TEST_ASSERT_EQUAL_INT(0, overlap.x);
    TEST_ASSERT_EQUAL_INT(0, overlap.y);
    TEST_ASSERT_EQUAL_INT(1440, overlap.width);
    TEST_ASSERT_EQUAL_INT(900, overlap.height);

    const CP_ScreenRect apart = CP_Monitor_Overlap(kSideBySide[0], Rect(-900, 100, 800, 600));
    TEST_ASSERT_EQUAL_INT(0, apart.width);
    TEST_ASSERT_EQUAL_INT(0, apart.height);
}
