// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors

// Assertions about the game state stack, from Scn_GameStateStack in
// tier2_capture.c: states pushed on top of the harness, the picture they
// make together, and the order their inits and exits ran in.
#include <string.h>
#include "unity.h"
#include "tier2_capture.h"

static void assertColorWithin(unsigned char tolerance, CP_Color expected, CP_Color actual)
{
	TEST_ASSERT_UINT8_WITHIN(tolerance, expected.r, actual.r);
	TEST_ASSERT_UINT8_WITHIN(tolerance, expected.g, actual.g);
	TEST_ASSERT_UINT8_WITHIN(tolerance, expected.b, actual.b);
}

void test_tier2_gamestate_push_and_pop_run_inits_and_exits_in_order(void)
{
	// no exit for the harness when A is pushed, no init again when A pops;
	// A popping from under F takes F off first
	TEST_ASSERT_EQUAL_STRING("A.init B.init B.exit C.init C.exit F.init F.exit A.exit", tier2_scalars.stackLog);
	TEST_ASSERT_EQUAL_INT(2, tier2_scalars.stackDepthInPushedInit);
	TEST_ASSERT_EQUAL_INT(1, tier2_scalars.stackHarnessDepthAfter);
}

void test_tier2_gamestate_a_pushed_state_draws_over_the_frozen_one_below(void)
{
	// a black veil at alpha 128 over the harness's red, and a solid green square
	assertColorWithin(3, CP_Color_Create(127, 0, 0, 255), tier2_SamplePixel(SCN_GAMESTATE_STACK, 10, 10));
	assertColorWithin(3, CP_Color_Create(127, 0, 0, 255), tier2_SamplePixel(SCN_GAMESTATE_STACK, 190, 190));
	assertColorWithin(2, CP_Color_Create(0, 255, 0, 255), tier2_SamplePixel(SCN_GAMESTATE_STACK, 100, 100));
}

void test_tier2_gamestate_passing_none_through_hides_the_states_below(void)
{
	const CP_Color pixel = tier2_stackHiddenPixels[10 * TIER2_CANVAS_SIZE + 10];
	assertColorWithin(2, CP_Color_Create(0, 0, 0, 255), pixel);
}

void test_tier2_gamestate_queries_in_a_pushed_state(void)
{
	TEST_ASSERT_TRUE(tier2_scalars.stackPushedIsTop);
	TEST_ASSERT_GREATER_THAN_FLOAT(0.0f, tier2_scalars.stackPushedTime);
	TEST_ASSERT_TRUE(tier2_scalars.stackRunningStateIsTop);
}

void test_tier2_gamestate_passing_all_through_keeps_the_state_below_running_without_input(void)
{
	TEST_ASSERT_EQUAL_INT(2, tier2_scalars.stackUpdatesWhileRunningBelow); // under B, then under F
	TEST_ASSERT_TRUE(tier2_scalars.stackRunningBelowIsNotTop);
	TEST_ASSERT_TRUE(tier2_scalars.stackRunningBelowKeyMuted);
	// Each time A is covered, it sees the mouse where it was then, every frame
	TEST_ASSERT_EQUAL_FLOAT(tier2_scalars.stackMouseXWhenCovered, tier2_scalars.stackRunningBelowMouseX);
	TEST_ASSERT_FALSE(tier2_scalars.stackRunningBelowMouseMoved);
	// ...and in the world the same way: with its camera where it was then
	TEST_ASSERT_EQUAL_FLOAT(tier2_scalars.stackMouseXWhenBCovered - 37.0f, tier2_scalars.stackWorldMouseXWhenCovered);
	TEST_ASSERT_EQUAL_FLOAT(tier2_scalars.stackWorldMouseXWhenCovered, tier2_scalars.stackRunningBelowWorldMouseX);
}

void test_tier2_gamestate_a_frozen_state_is_not_updated(void)
{
	// A ran in frames 0, 1 (under B), 2, 4 and 5 (under F), not in 3 (under C)
	TEST_ASSERT_EQUAL_INT(5, tier2_scalars.stackUpdatesOfFirstPushed);
}

void test_tier2_gamestate_each_state_keeps_its_own_settings(void)
{
	TEST_ASSERT_TRUE(tier2_scalars.stackPushInheritsSettings);
	TEST_ASSERT_TRUE(tier2_scalars.stackSettingsBackAfterPop);
	TEST_ASSERT_TRUE(tier2_scalars.stackHarnessSettingsBack);
}

void test_tier2_gamestate_states_still_pushed_exit_when_the_program_ends(void)
{
	TEST_ASSERT_TRUE(tier2_scalars.stackShutdownExitRan);
}

void test_tier2_gamestate_a_screenshot_drawn_in_the_next_state_blends_over_it(void)
{
	TEST_ASSERT_TRUE(tier2_scalars.snapshotTaken);
	// D's green screenshot at half alpha over E's blue
	assertColorWithin(3, CP_Color_Create(0, 128, 127, 255), tier2_SamplePixel(SCN_GAMESTATE_SNAPSHOT, 100, 100));
	assertColorWithin(3, CP_Color_Create(0, 128, 127, 255), tier2_SamplePixel(SCN_GAMESTATE_SNAPSHOT, 10, 190));
	TEST_ASSERT_EQUAL_INT(1, tier2_scalars.snapshotHarnessDepthAfter);
}

void test_tier2_gamestate_a_screenshot_in_a_pushed_init_is_the_picture_below(void)
{
	// E's layer starts clear, so its init sees only D's green under it (and
	// nothing left in the layer by states pushed earlier)
	assertColorWithin(2, CP_Color_Create(0, 255, 0, 255), tier2_scalars.snapshotInPushedInitCenter);
	assertColorWithin(2, CP_Color_Create(0, 255, 0, 255), tier2_scalars.snapshotInPushedInitCorner);
}

void test_tier2_image_drawing_into_an_image(void)
{
	TEST_ASSERT_TRUE(tier2_scalars.drawIntoCreated);
	// drawn at 4x, centered on (100, 100): the red corner is top left
	assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_IMAGE_DRAW_INTO, 20, 20));
	// half-transparent blue over white
	assertColorWithin(4, CP_Color_Create(127, 127, 255, 255), tier2_SamplePixel(SCN_IMAGE_DRAW_INTO, 180, 180));
	// where nothing was drawn, the image is transparent: white shows
	assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_IMAGE_DRAW_INTO, 180, 20));
	TEST_ASSERT_TRUE(tier2_scalars.drawIntoSettingsBack);
}

void test_tier2_image_drawn_into_reads_back_top_row_first_with_plain_alpha(void)
{
	assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_scalars.drawIntoTopLeft);
	TEST_ASSERT_UINT8_WITHIN(2, 255, tier2_scalars.drawIntoBottomRight.b);
	TEST_ASSERT_UINT8_WITHIN(2, 128, tier2_scalars.drawIntoBottomRight.a);
	assertColorWithin(2, CP_Color_Create(10, 200, 30, 255), tier2_scalars.drawIntoAfterUpdate);
	TEST_ASSERT_TRUE(tier2_scalars.drawIntoLoadedRefused);
}
