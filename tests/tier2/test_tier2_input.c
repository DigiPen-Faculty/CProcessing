// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors

// Covers only the slice of CP_Input that's deterministically testable
// without synthesizing OS input events or hardware -- see the comment on
// Scn_InputQuiescentDefaults in tier2_capture.c for why the edge-detection
// logic itself (triggered/released/down transitions) isn't covered here.
#include "unity.h"
#include "tier2_capture.h"

void test_tier2_input_no_gamepad_attached_reports_disconnected(void)
{
	TEST_ASSERT_FALSE(tier2_scalars.gamepadConnected);
	TEST_ASSERT_FALSE(tier2_scalars.gamepad0ConnectedAdvanced);
}

void test_tier2_input_mouse_wheel_is_zero_at_rest(void)
{
	TEST_ASSERT_EQUAL_FLOAT(0.0f, tier2_scalars.mouseWheel);
}

void test_tier2_input_no_clicks_or_keys_in_a_quiescent_frame(void)
{
	TEST_ASSERT_FALSE(tier2_scalars.mouseDoubleClicked);
	TEST_ASSERT_FALSE(tier2_scalars.keyADown);
	TEST_ASSERT_FALSE(tier2_scalars.mouseLeftDown);
	// and no text was typed
	TEST_ASSERT_TRUE(tier2_scalars.typedTextIsEmpty);
}

void test_tier2_input_no_key_is_triggered_and_no_key_is_never_down(void)
{
	// Nothing was pressed, so GetKeyTriggered gives CP_KEY_NONE, and
	// CP_KEY_NONE (a CP_KEY left at zero) is never down, triggered or released
	TEST_ASSERT_EQUAL_INT(CP_KEY_NONE, tier2_scalars.keyTriggered);
	TEST_ASSERT_FALSE(tier2_scalars.keyNoneEverOn);
}
