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

#include <stdbool.h>
#include <stddef.h>
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
    SCN_SETTINGS_BLEND_MODES,
    SCN_SETTINGS_TINT,
    SCN_IMAGE_FILTER_MODES,
    SCN_IMAGE_WRAP_MODES,
    // These two must stay in this order: one frame, then the next
    SCN_SETTINGS_LAST_FRAME_SETS_TEXT_SIZE,
    SCN_SETTINGS_CARRY_TO_NEXT_FRAME,
    SCN_SETTINGS_SAVE_RESTORE,
    SCN_IMAGE_LOAD_AND_DRAW,
    SCN_IMAGE_SUBIMAGE,
    SCN_IMAGE_16_BIT,
    SCN_IMAGE_MANY,
    SCN_FONT_DRAWTEXT,
    SCN_FONT_LOAD_FREE,
    SCN_FONT_FREE_KEEPS_OTHERS,
    SCN_FONT_OPENTYPE,
    SCN_FONT_SIZE_SWEEP,
    SCN_FONT_SCALE_SWEEP,
    SCN_FONT_EM_SIZE,
    SCN_FONT_LARGE,
    SCN_FONT_MIRRORED,
    SCN_SYSTEM_ENGINE_STATE,
    SCN_SOUND_ROUNDTRIP,
    SCN_INPUT_QUIESCENT_DEFAULTS,
    // Cross-platform work: these two must stay consecutive -- the second
    // one draws on top of the first one's frame without clearing.
    SCN_PERSIST_FIRST_FRAME,
    SCN_PERSIST_NEXT_FRAME,
    SCN_SCREENSHOT_SUBREGION,
    SCN_ERROR_PATHS,
    SCN_SYSTEM_WINDOW_SETTINGS,
    SCN_ASSETS_NEXT_TO_PROGRAM,
    SCN_COUNT
} Tier2Scenario;

extern CP_Color tier2_snapshots[SCN_COUNT][TIER2_CANVAS_SIZE * TIER2_CANVAS_SIZE];

// Results of one animated-text sweep (see Scn_FontSizeSweep)
typedef struct
{
    int frames;
    int badFrames;          // the text didn't match an immediate redraw
    int blankFrames;        // the redraw had no text on the canvas at all
    int firstBadFrame;      // -1 when there were none
    int worstPixels;        // most pixels that differed in any one frame
} Tier2TextSweep;

// Non-pixel results: CP_Image round-trips, CP_System/CP_Engine state, and
// CP_Sound round-trips that don't fit the screenshot model. Populated
// during the same single scripted engine run as tier2_snapshots, for the
// same reason (see the file banner above) -- most of these touch GLFW
// handles that are torn down once CP_Engine_Run() returns, so they have to
// be read *during* the run, not after.
typedef struct
{
    // CP_Image
    int quadImageWidth;
    int quadImageHeight;
    CP_Color createFromDataReadback[4];
    CP_Color updatePixelDataReadback[4];
    int manyImagesCreated;          // see Scn_ImageMany
    int manyImagesFreed;            // handles set to NULL by CP_Image_Free
    int manyImagesFileWidth;
    int image16BitWidth;            // see Scn_Image16Bit

    // CP_System / CP_Engine
    int windowWidthAfterSet;
    int windowHeightAfterSet;
    unsigned frameCountEarly;
    unsigned frameCountLater;
    float dtSample;
    float millisSample;
    float secondsSample;
    float frameRateSample;
    CP_BOOL windowFocusSample;
    int displayWidth;
    int displayHeight;
    int displayRefreshRate;
    int preUpdateHookCount;
    int postUpdateHookCount;

    // CP_Sound
    float volumeAfterSet;
    float pitchAfterSet;

    // CP_Input (quiescent-state defaults only -- see Scn_InputQuiescentDefaults)
    CP_BOOL gamepadConnected;
    CP_BOOL gamepad0ConnectedAdvanced;
    float mouseWheel;
    CP_BOOL mouseDoubleClicked;
    CP_BOOL keyADown;
    CP_BOOL mouseLeftDown;

    // CP_System_GetWindowHandle
    CP_BOOL windowHandleIsNull;

    // Freeing a font (see Scn_FontFreeKeepsOthers)
    int fontFreeLoaded;             // fonts loaded, of 3
    int fontFreeInkBefore;          // ink pixels before freeing the first
    int fontFreeMismatched;         // pixels that changed after freeing it
    CP_BOOL openTypeFontLoaded;     // see Scn_FontOpenType

    // Text animated by size and by scale (see Scn_FontSizeSweep)
    Tier2TextSweep textSizeSweep;
    Tier2TextSweep textScaleSweep;

    // CP_Image_Screenshot of sub-regions (see Scn_ScreenshotSubregion)
    int subTopRightWidth;
    int subTopRightHeight;
    CP_Color subTopRightCenter;     // region (100,0) 100x100 -> green quadrant
    CP_Color subBottomLeftCenter;   // region (0,100) 100x100 -> blue quadrant
    CP_Color straddleCorners[4];    // region (75,75) 50x50: TL, TR, BL, BR samples

    // Error paths (see Scn_ErrorPaths)
    CP_BOOL missingImageIsNull;
    CP_BOOL missingFontIsNull;
    CP_BOOL missingSoundIsNull;
    CP_BOOL survivedNullCalls;

    // Window settings and queries before CP_Engine_Run, and the cursor and
    // console during it (see tier2_RunCaptureOnce and Scn_SystemWindowSettings)
    CP_BOOL cursorVisibleBeforeRun;
    CP_BOOL cursorVisibleAtStart;
    CP_BOOL cursorVisibleAfterShow;
    CP_BOOL cursorVisibleAfterHide;
    CP_BOOL windowFocusBeforeRun;
    int displayRefreshRateBeforeRun;
    CP_BOOL consoleVisibleBeforeRun;
    CP_BOOL consoleVisibleAfterHide;
    CP_BOOL consoleVisibleAfterReshow;
    int printfAfterShowConsole;     // printf's return value: negative on failure
    int printfAfterHideConsole;

    // Loading assets from another folder (see Scn_AssetsNextToProgram)
    CP_BOOL assetFolderEntered;
    CP_BOOL assetImageLoaded;
    CP_BOOL assetFontLoaded;
    CP_BOOL assetSoundAvailable;    // sound loads at all (not with no audio device)
    CP_BOOL assetSoundLoaded;

    // How many scenarios were captured: SCN_COUNT unless the engine couldn't
    // start (no window or OpenGL context) or stopped early
    int scenariosCaptured;
} Tier2Scalars;

extern Tier2Scalars tier2_scalars;

// Runs the one and only CP_Engine_Run() call for this process, scripting
// through every scenario in Tier2Scenario and filling tier2_snapshots.
// Must be called exactly once, before any RUN_TEST.
void tier2_RunCaptureOnce(void);

static inline CP_Color tier2_SamplePixel(Tier2Scenario scenario, int x, int y)
{
    return tier2_snapshots[scenario][(y * TIER2_CANVAS_SIZE) + x];
}
