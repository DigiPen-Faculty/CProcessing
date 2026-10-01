// Tier 2 engine harness (06-test-suite-plan.md Phase D/E).
//
// CP_Engine_Run() can only be called once per process (CP_System.c:97-101
// guards against re-entry via a static _isInitialized that's never reset),
// so this executable cannot call it once per Unity test the way a normal
// test runner would. Instead, tier2_RunCaptureOnce() drives a single real
// engine run that scripts through every scenario below -- one real window,
// one real GL context, one frame per scenario -- and screenshots each
// scenario's rendered output into tier2_snapshots before the engine shuts
// down. All of that happens *before* UNITY_BEGIN(). The Unity test
// functions that follow never touch the engine: they just index into the
// already-captured pixel buffers and assert, which keeps Unity's
// setjmp/longjmp failure handling (TEST_ASSERT_* et al) safely outside the
// engine's own call stack.
#pragma once

#include "cprocessing.h"

#define TIER2_CANVAS_SIZE 200

typedef enum
{
    SCN_CLEAR_BACKGROUND,
    SCN_DRAW_POINT,
    SCN_DRAW_LINE,
    SCN_DRAW_LINE_ADVANCED,
    SCN_DRAW_RECT,
    SCN_DRAW_RECT_ADVANCED,
    SCN_DRAW_CIRCLE,
    SCN_DRAW_ELLIPSE,
    SCN_DRAW_ELLIPSE_ADVANCED,
    SCN_DRAW_TRIANGLE,
    SCN_DRAW_TRIANGLE_ADVANCED,
    SCN_DRAW_QUAD,
    SCN_DRAW_QUAD_ADVANCED,
    SCN_CUSTOM_SHAPE,
    SCN_SETTINGS_NOFILL,
    SCN_SETTINGS_STROKEWEIGHT,
    SCN_SETTINGS_RECTMODE_CORNER,
    SCN_SETTINGS_ELLIPSEMODE_CORNER,
    SCN_SETTINGS_TRANSLATE,
    SCN_SETTINGS_SCALE,
    SCN_SETTINGS_ROTATE,
    SCN_SETTINGS_RESETMATRIX,
    SCN_SETTINGS_APPLYMATRIX,
    SCN_SETTINGS_BLENDMODE_ADD,
    SCN_SETTINGS_SAVE_RESTORE,
    SCN_COUNT
} Tier2Scenario;

extern CP_Color tier2_snapshots[SCN_COUNT][TIER2_CANVAS_SIZE * TIER2_CANVAS_SIZE];

// Runs the one and only CP_Engine_Run() call for this process, scripting
// through every scenario in Tier2Scenario and filling tier2_snapshots.
// Must be called exactly once, before any RUN_TEST.
void tier2_RunCaptureOnce(void);

static inline CP_Color tier2_SamplePixel(Tier2Scenario scenario, int x, int y)
{
    return tier2_snapshots[scenario][(y * TIER2_CANVAS_SIZE) + x];
}
