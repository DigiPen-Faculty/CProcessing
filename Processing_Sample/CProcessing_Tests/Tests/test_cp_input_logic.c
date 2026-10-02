// Tier 1 coverage for the pure input logic behind CP_Input
// (Source/Internal_InputLogic.h): edge detection, double-click timing,
// gamepad button mapping, and the trigger/stick conditioning.
//
// This is the Phase F split from 06-test-suite-plan.md, done together with
// the XInput -> GLFW gamepad migration: the logic that turns raw device state
// into CP_Input_* answers now lives in static inline functions with no
// globals and no GLFW calls, so it can be exercised here with synthetic
// state transitions -- no window, OS input events or physical controller.
//
// The legacy_xinput_* helpers below are verbatim ports of the math the
// XInput implementation used (CP_Input.c before the migration). The
// *_matches_legacy_xinput tests pin the new code to it, so the move to GLFW
// can't silently change how sticks and triggers feel in existing games.
#include <math.h>
#include <string.h>
#include "unity.h"
#include "Internal_InputLogic.h"

// ---- legacy XInput math (reference implementation) ----

static float legacy_xinput_trigger(unsigned char byteValue)
{
    float value = (float)(byteValue - 30) / (255.0f - 30.0f); // XINPUT_GAMEPAD_TRIGGER_THRESHOLD = 30
    return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
}

static float legacy_xinput_stick(short stickValue)
{
    const float deadzone = 8689 / 32767.0f; // XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE
    float normStick = fmaxf(-1.0f, (float)stickValue / 32767.0f);
    return (fabsf(normStick) < deadzone ? 0 : (fabsf(normStick) - deadzone) * (normStick / fabsf(normStick))) / (1.0f - deadzone);
}

// ---- edge detection ----

void test_inputlogic_triggered_truth_table(void)
{
    TEST_ASSERT_TRUE(CP_InputLogic_Triggered(true, false));
    TEST_ASSERT_FALSE(CP_InputLogic_Triggered(true, true));   // held, not a new press
    TEST_ASSERT_FALSE(CP_InputLogic_Triggered(false, true));  // that's a release
    TEST_ASSERT_FALSE(CP_InputLogic_Triggered(false, false));
}

void test_inputlogic_released_truth_table(void)
{
    TEST_ASSERT_TRUE(CP_InputLogic_Released(false, true));
    TEST_ASSERT_FALSE(CP_InputLogic_Released(true, true));
    TEST_ASSERT_FALSE(CP_InputLogic_Released(true, false));
    TEST_ASSERT_FALSE(CP_InputLogic_Released(false, false));
}

void test_inputlogic_press_hold_release_sequence(void)
{
    // A button held for three frames then let go: triggered exactly once on
    // the first frame, released exactly once on the frame after.
    const bool frames[] = { false, true, true, true, false, false };
    int triggeredCount = 0, releasedCount = 0;
    int triggeredFrame = -1, releasedFrame = -1;
    for (int i = 1; i < (int)(sizeof(frames) / sizeof(frames[0])); ++i)
    {
        if (CP_InputLogic_Triggered(frames[i], frames[i - 1])) { ++triggeredCount; triggeredFrame = i; }
        if (CP_InputLogic_Released(frames[i], frames[i - 1])) { ++releasedCount; releasedFrame = i; }
    }
    TEST_ASSERT_EQUAL_INT(1, triggeredCount);
    TEST_ASSERT_EQUAL_INT(1, releasedCount);
    TEST_ASSERT_EQUAL_INT(1, triggeredFrame);
    TEST_ASSERT_EQUAL_INT(4, releasedFrame);
}

// ---- double click ----

void test_inputlogic_double_click_within_window(void)
{
    TEST_ASSERT_TRUE(CP_InputLogic_IsDoubleClick(10.0, 10.25));
}

void test_inputlogic_double_click_window_is_inclusive(void)
{
    TEST_ASSERT_TRUE(CP_InputLogic_IsDoubleClick(10.0, 10.0 + CP_DOUBLE_CLICK_TIME));
}

void test_inputlogic_double_click_too_slow(void)
{
    TEST_ASSERT_FALSE(CP_InputLogic_IsDoubleClick(10.0, 10.51));
}

// ---- gamepad buttons ----

void test_inputlogic_gamepad_button_validity(void)
{
    TEST_ASSERT_TRUE(CP_InputLogic_GamepadButtonIsValid(GAMEPAD_DPAD_UP));
    TEST_ASSERT_TRUE(CP_InputLogic_GamepadButtonIsValid(GAMEPAD_Y));
    TEST_ASSERT_FALSE(CP_InputLogic_GamepadButtonIsValid((CP_GAMEPAD)-1));
    TEST_ASSERT_FALSE(CP_InputLogic_GamepadButtonIsValid((CP_GAMEPAD)(GAMEPAD_Y + 1)));
}

void test_inputlogic_gamepad_button_mask_is_one_bit_per_button(void)
{
    unsigned seen = 0;
    for (int b = GAMEPAD_DPAD_UP; b <= GAMEPAD_Y; ++b)
    {
        unsigned mask = CP_InputLogic_GamepadButtonMask((CP_GAMEPAD)b);
        TEST_ASSERT_NOT_EQUAL(0u, mask);
        TEST_ASSERT_EQUAL_UINT(0u, mask & (mask - 1)); // exactly one bit
        TEST_ASSERT_EQUAL_UINT(0u, seen & mask);       // no two buttons share a bit
        seen |= mask;
    }
}

void test_inputlogic_gamepad_button_mask_invalid_is_zero(void)
{
    TEST_ASSERT_EQUAL_UINT(0u, CP_InputLogic_GamepadButtonMask((CP_GAMEPAD)-1));
    TEST_ASSERT_EQUAL_UINT(0u, CP_InputLogic_GamepadButtonMask((CP_GAMEPAD)99));
}

void test_inputlogic_gamepad_button_down_reads_only_its_bit(void)
{
    unsigned buttons = CP_InputLogic_GamepadButtonMask(GAMEPAD_A) | CP_InputLogic_GamepadButtonMask(GAMEPAD_START);
    TEST_ASSERT_TRUE(CP_InputLogic_GamepadButtonDown(buttons, GAMEPAD_A));
    TEST_ASSERT_TRUE(CP_InputLogic_GamepadButtonDown(buttons, GAMEPAD_START));
    TEST_ASSERT_FALSE(CP_InputLogic_GamepadButtonDown(buttons, GAMEPAD_B));
    TEST_ASSERT_FALSE(CP_InputLogic_GamepadButtonDown(buttons, GAMEPAD_BACK));
    TEST_ASSERT_FALSE(CP_InputLogic_GamepadButtonDown(buttons, (CP_GAMEPAD)-1));
}

void test_inputlogic_gamepad_to_glfw_button_mapping(void)
{
    // Xbox layout, matching the XINPUT_GAMEPAD_* bits the enum used to map to
    TEST_ASSERT_EQUAL_INT(GLFW_GAMEPAD_BUTTON_DPAD_UP, CP_InputLogic_GamepadToGLFWButton(GAMEPAD_DPAD_UP));
    TEST_ASSERT_EQUAL_INT(GLFW_GAMEPAD_BUTTON_DPAD_DOWN, CP_InputLogic_GamepadToGLFWButton(GAMEPAD_DPAD_DOWN));
    TEST_ASSERT_EQUAL_INT(GLFW_GAMEPAD_BUTTON_DPAD_LEFT, CP_InputLogic_GamepadToGLFWButton(GAMEPAD_DPAD_LEFT));
    TEST_ASSERT_EQUAL_INT(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, CP_InputLogic_GamepadToGLFWButton(GAMEPAD_DPAD_RIGHT));
    TEST_ASSERT_EQUAL_INT(GLFW_GAMEPAD_BUTTON_START, CP_InputLogic_GamepadToGLFWButton(GAMEPAD_START));
    TEST_ASSERT_EQUAL_INT(GLFW_GAMEPAD_BUTTON_BACK, CP_InputLogic_GamepadToGLFWButton(GAMEPAD_BACK));
    TEST_ASSERT_EQUAL_INT(GLFW_GAMEPAD_BUTTON_LEFT_THUMB, CP_InputLogic_GamepadToGLFWButton(GAMEPAD_LEFT_THUMB));
    TEST_ASSERT_EQUAL_INT(GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, CP_InputLogic_GamepadToGLFWButton(GAMEPAD_RIGHT_THUMB));
    TEST_ASSERT_EQUAL_INT(GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, CP_InputLogic_GamepadToGLFWButton(GAMEPAD_LEFT_SHOULDER));
    TEST_ASSERT_EQUAL_INT(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, CP_InputLogic_GamepadToGLFWButton(GAMEPAD_RIGHT_SHOULDER));
    TEST_ASSERT_EQUAL_INT(GLFW_GAMEPAD_BUTTON_A, CP_InputLogic_GamepadToGLFWButton(GAMEPAD_A));
    TEST_ASSERT_EQUAL_INT(GLFW_GAMEPAD_BUTTON_B, CP_InputLogic_GamepadToGLFWButton(GAMEPAD_B));
    TEST_ASSERT_EQUAL_INT(GLFW_GAMEPAD_BUTTON_X, CP_InputLogic_GamepadToGLFWButton(GAMEPAD_X));
    TEST_ASSERT_EQUAL_INT(GLFW_GAMEPAD_BUTTON_Y, CP_InputLogic_GamepadToGLFWButton(GAMEPAD_Y));
    TEST_ASSERT_EQUAL_INT(-1, CP_InputLogic_GamepadToGLFWButton((CP_GAMEPAD)42));
}

// ---- GLFW state conversion ----

static GLFWgamepadstate RestingGLFWState(void)
{
    GLFWgamepadstate state;
    memset(&state, 0, sizeof(state));
    state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] = -1.0f;  // GLFW triggers rest at -1
    state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] = -1.0f;
    return state;
}

void test_inputlogic_from_glfw_null_is_disconnected(void)
{
    CP_GamepadRawState raw = CP_InputLogic_FromGLFWGamepad(NULL);
    TEST_ASSERT_FALSE(raw.connected);
    TEST_ASSERT_EQUAL_UINT(0u, raw.buttons);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, raw.left_trigger);
}

void test_inputlogic_from_glfw_resting_pad(void)
{
    GLFWgamepadstate state = RestingGLFWState();
    CP_GamepadRawState raw = CP_InputLogic_FromGLFWGamepad(&state);
    TEST_ASSERT_TRUE(raw.connected);
    TEST_ASSERT_EQUAL_UINT(0u, raw.buttons);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, raw.left_trigger);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, raw.right_trigger);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, raw.left_x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, raw.left_y);
}

void test_inputlogic_from_glfw_maps_pressed_buttons(void)
{
    GLFWgamepadstate state = RestingGLFWState();
    state.buttons[GLFW_GAMEPAD_BUTTON_A] = GLFW_PRESS;
    state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] = GLFW_PRESS;
    state.buttons[GLFW_GAMEPAD_BUTTON_GUIDE] = GLFW_PRESS; // no CP_GAMEPAD equivalent: ignored
    CP_GamepadRawState raw = CP_InputLogic_FromGLFWGamepad(&state);
    TEST_ASSERT_EQUAL_UINT(CP_InputLogic_GamepadButtonMask(GAMEPAD_A) | CP_InputLogic_GamepadButtonMask(GAMEPAD_DPAD_LEFT), raw.buttons);
}

void test_inputlogic_from_glfw_triggers_rescaled_to_0_1(void)
{
    GLFWgamepadstate state = RestingGLFWState();
    state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] = 1.0f;  // fully pressed
    state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] = 0.0f; // half way
    CP_GamepadRawState raw = CP_InputLogic_FromGLFWGamepad(&state);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, raw.left_trigger);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, raw.right_trigger);
}

void test_inputlogic_from_glfw_stick_y_is_up_positive(void)
{
    // GLFW (SDL convention) reports pushing a stick up as -1; CProcessing has
    // always reported up as +1 (XInput convention).
    GLFWgamepadstate state = RestingGLFWState();
    state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] = -1.0f;
    state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] = 0.5f;
    state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X] = 0.25f;
    CP_GamepadRawState raw = CP_InputLogic_FromGLFWGamepad(&state);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, raw.left_y);
    TEST_ASSERT_EQUAL_FLOAT(-0.5f, raw.right_y);
    TEST_ASSERT_EQUAL_FLOAT(0.25f, raw.right_x); // x is not flipped
}

// ---- trigger threshold ----

void test_inputlogic_trigger_rest_and_full(void)
{
    TEST_ASSERT_EQUAL_FLOAT(0.0f, CP_InputLogic_ApplyTriggerThreshold(0.0f));
    TEST_ASSERT_EQUAL_FLOAT(1.0f, CP_InputLogic_ApplyTriggerThreshold(1.0f));
}

void test_inputlogic_trigger_below_threshold_is_zero(void)
{
    TEST_ASSERT_EQUAL_FLOAT(0.0f, CP_InputLogic_ApplyTriggerThreshold(29.0f / 255.0f));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, CP_InputLogic_ApplyTriggerThreshold(30.0f / 255.0f));
}

void test_inputlogic_trigger_out_of_range_clamps(void)
{
    TEST_ASSERT_EQUAL_FLOAT(0.0f, CP_InputLogic_ApplyTriggerThreshold(-0.5f));
    TEST_ASSERT_EQUAL_FLOAT(1.0f, CP_InputLogic_ApplyTriggerThreshold(1.5f));
}

void test_inputlogic_trigger_matches_legacy_xinput(void)
{
    for (int b = 0; b <= 255; ++b)
    {
        float expected = legacy_xinput_trigger((unsigned char)b);
        float actual = CP_InputLogic_ApplyTriggerThreshold((float)b / 255.0f);
        TEST_ASSERT_FLOAT_WITHIN(1e-5f, expected, actual);
    }
}

// ---- stick deadzone ----

void test_inputlogic_stick_center_is_zero(void)
{
    TEST_ASSERT_EQUAL_FLOAT(0.0f, CP_InputLogic_ApplyStickDeadzone(0.0f));
}

void test_inputlogic_stick_inside_deadzone_is_zero(void)
{
    TEST_ASSERT_EQUAL_FLOAT(0.0f, CP_InputLogic_ApplyStickDeadzone(0.2f));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, CP_InputLogic_ApplyStickDeadzone(-0.2f));
}

void test_inputlogic_stick_full_deflection_reaches_one(void)
{
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 1.0f, CP_InputLogic_ApplyStickDeadzone(1.0f));
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, -1.0f, CP_InputLogic_ApplyStickDeadzone(-1.0f));
}

void test_inputlogic_stick_is_odd_symmetric(void)
{
    for (float v = 0.0f; v <= 1.0f; v += 0.05f)
    {
        TEST_ASSERT_EQUAL_FLOAT(-CP_InputLogic_ApplyStickDeadzone(v), CP_InputLogic_ApplyStickDeadzone(-v));
    }
}

void test_inputlogic_stick_is_monotonic(void)
{
    float previous = CP_InputLogic_ApplyStickDeadzone(-1.0f);
    for (float v = -1.0f; v <= 1.0f; v += 0.01f)
    {
        float current = CP_InputLogic_ApplyStickDeadzone(v);
        TEST_ASSERT_TRUE(current >= previous);
        previous = current;
    }
}

void test_inputlogic_stick_out_of_range_clamps(void)
{
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 1.0f, CP_InputLogic_ApplyStickDeadzone(1.5f));
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, -1.0f, CP_InputLogic_ApplyStickDeadzone(-3.0f));
}

void test_inputlogic_stick_matches_legacy_xinput(void)
{
    // every 97th short value across the whole range, plus the edges and the
    // values either side of the deadzone boundary
    for (int s = -32767; s <= 32767; s += 97)
    {
        float expected = legacy_xinput_stick((short)s);
        float actual = CP_InputLogic_ApplyStickDeadzone((float)s / 32767.0f);
        TEST_ASSERT_FLOAT_WITHIN(1e-5f, expected, actual);
    }
    const short edges[] = { -32768, -32767, -8690, -8689, -8688, 0, 8688, 8689, 8690, 32767 };
    for (unsigned i = 0; i < sizeof(edges) / sizeof(edges[0]); ++i)
    {
        float expected = legacy_xinput_stick(edges[i]);
        float actual = CP_InputLogic_ApplyStickDeadzone((float)edges[i] / 32767.0f);
        TEST_ASSERT_FLOAT_WITHIN(1e-5f, expected, actual);
    }
}
