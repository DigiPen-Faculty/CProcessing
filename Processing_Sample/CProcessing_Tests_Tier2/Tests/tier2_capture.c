// See tier2_capture.h for the overall approach. Each Scn_* function below
// issues exactly one scenario's worth of draw calls (ClearBackground plus
// whatever it wants to test); RunScenario() wraps it with a screenshot and
// a settings reset so scenarios can't leak state into each other, since
// CP_Settings state (fill/stroke color, rect/ellipse mode, blend mode, ...)
// is *not* reset automatically each frame the way the transform is
// (CP_System.c's CP_Update calls nvgResetTransform every frame, but
// nothing resets fill/stroke/mode state).
//
// All coordinates below were computed by hand against the actual source
// (CP_Graphics.c, CP_Setting.c) rather than assumed -- see the Phase D
// commit message for the worked geometry on the rotation/transform cases.
#include "tier2_capture.h"

CP_Color tier2_snapshots[SCN_COUNT][TIER2_CANVAS_SIZE * TIER2_CANVAS_SIZE];
Tier2Scalars tier2_scalars = { 0 };

#define WHITE CP_Color_Create(255, 255, 255, 255)
#define BLACK CP_Color_Create(0, 0, 0, 255)
#define RED CP_Color_Create(255, 0, 0, 255)
#define BLUE CP_Color_Create(0, 0, 255, 255)
#define GREEN CP_Color_Create(0, 200, 0, 255)

static int frameCount = 0;
static int firstScenarioFrame = -1;
// The deferred window resize requested in HarnessInit has to actually
// settle at the OS/compositor level before any scenario is captured. How
// long that takes varies: it is synchronous on Windows but asynchronous on
// X11/Wayland/macOS (observed taking anywhere from ~10 to 40+ frames under
// WSLg), so a fixed frame count was flaky. Instead, wait at least
// WARMUP_FRAMES and then until the canvas reports the requested size, giving
// up after MAX_WARMUP_FRAMES (the size test then fails with a clear message
// instead of every pixel test failing mysteriously).
#define WARMUP_FRAMES 15
#define MAX_WARMUP_FRAMES 600

static void CaptureCurrentFrame(Tier2Scenario scenario)
{
    CP_Image shot = CP_Image_Screenshot(0, 0, TIER2_CANVAS_SIZE, TIER2_CANVAS_SIZE);
    CP_Image_GetPixelData(shot, tier2_snapshots[scenario]);
    CP_Image_Free(&shot);
}

// Known-good baseline so each scenario starts from the same state
// regardless of what the previous scenario left behind.
static void ResetToBaseline(void)
{
    CP_Settings_ResetMatrix();
    CP_Settings_Fill(WHITE);
    CP_Settings_Stroke(BLACK);
    CP_Settings_StrokeWeight(3.0f);
    CP_Settings_RectMode(CP_POSITION_CENTER);
    CP_Settings_EllipseMode(CP_POSITION_CENTER);
    CP_Settings_BlendMode(CP_BLEND_ALPHA);
}

static void Scn_ClearBackground(void)
{
    CP_Graphics_ClearBackground(CP_Color_Create(10, 20, 30, 255));
    // Captured here (the first scenario) and again in Scn_SystemEngineState
    // (one of the last) so the test suite can confirm frame count actually
    // advances across the scripted run.
    tier2_scalars.frameCountEarly = CP_System_GetFrameCount();
}

static void Scn_DrawPoint(void)
{
    CP_Graphics_ClearBackground(WHITE);
    // CP_Graphics_DrawPoint is gated on DI->fill (must be enabled for
    // anything to draw at all -- CP_Graphics.c:100-112), but nanovg's
    // nvgFillPoint renders it with useStrokePaint=1 (nanovg.c:2331-2334),
    // i.e. the pixels that actually appear are colored by the current
    // *stroke* color/width, not the fill color. Undocumented, verified by
    // running this scenario against both colors and checking which one
    // shows up -- asserted explicitly in test_tier2_graphics_drawpoint.
    CP_Settings_Fill(CP_Color_Create(0, 255, 0, 255)); // must be enabled, but should NOT be the rendered color
    CP_Settings_Stroke(RED); // this is the color that actually renders
    CP_Settings_StrokeWeight(20.0f); // nvgPoint sizes itself off strokeWidth
    CP_Graphics_DrawPoint(100, 100);
}

static void Scn_DrawLine(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_Stroke(RED);
    CP_Settings_StrokeWeight(10.0f);
    CP_Graphics_DrawLine(20, 100, 180, 100);
}

static void Scn_DrawLineAdvanced(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_Stroke(RED);
    CP_Settings_StrokeWeight(10.0f);
    // Rotated 90 degrees about its own midpoint (100,100): a horizontal
    // line becomes vertical, so the original endpoint is no longer covered.
    CP_Graphics_DrawLineAdvanced(20, 100, 180, 100, 90);
}

static void Scn_DrawRect(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_NoStroke();
    CP_Settings_Fill(RED);
    CP_Graphics_DrawRect(100, 100, 80, 80);
}

static void Scn_DrawRectAdvanced(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_NoStroke();
    CP_Settings_Fill(RED);
    // An 80x20 rect rotated 45 degrees about its center (100,100).
    CP_Graphics_DrawRectAdvanced(100, 100, 80, 20, 45, 0);
}

static void Scn_DrawCircle(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_NoStroke();
    CP_Settings_Fill(RED);
    CP_Graphics_DrawCircle(100, 100, 80);
}

static void Scn_DrawEllipse(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_NoStroke();
    CP_Settings_Fill(RED);
    CP_Graphics_DrawEllipse(100, 100, 120, 40); // half-w 60, half-h 20
}

static void Scn_DrawEllipseAdvanced(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_NoStroke();
    CP_Settings_Fill(RED);
    // Same 120x40 ellipse, rotated 90 degrees: the wide axis becomes vertical.
    CP_Graphics_DrawEllipseAdvanced(100, 100, 120, 40, 90);
}

static void Scn_DrawTriangle(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_NoStroke();
    CP_Settings_Fill(RED);
    // Centroid is exactly (100,100): ((100+143+57)/3, (50+125+125)/3).
    CP_Graphics_DrawTriangle(100, 50, 143, 125, 57, 125);
}

static void Scn_DrawTriangleAdvanced(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_NoStroke();
    CP_Settings_Fill(RED);
    // Same triangle, rotated 180 degrees about its centroid (100,100):
    // the apex that was at the top (100,50) is now at the bottom (100,150).
    CP_Graphics_DrawTriangleAdvanced(100, 50, 143, 125, 57, 125, 180);
}

static void Scn_DrawQuad(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_NoStroke();
    CP_Settings_Fill(RED);
    // An axis-aligned 60x60 square (half-size 30) centered at (100,100).
    CP_Graphics_DrawQuad(70, 70, 130, 70, 130, 130, 70, 130);
}

static void Scn_DrawQuadAdvanced(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_NoStroke();
    CP_Settings_Fill(RED);
    // Same square, rotated 45 degrees about its center: a diamond whose
    // reach along the pure-vertical/horizontal axes grows from 30 to
    // 30*sqrt(2) =~ 42.4, covering points the unrotated square did not.
    CP_Graphics_DrawQuadAdvanced(70, 70, 130, 70, 130, 130, 70, 130, 45);
}

static void Scn_CustomShape(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_NoStroke();
    CP_Settings_Fill(RED);
    CP_Graphics_BeginShape();
    CP_Graphics_AddVertex(70, 70);
    CP_Graphics_AddVertex(130, 70);
    CP_Graphics_AddVertex(130, 130);
    CP_Graphics_AddVertex(70, 130);
    CP_Graphics_EndShape();
}

static void Scn_SettingsNoFill(void)
{
    CP_Graphics_ClearBackground(GREEN);
    CP_Settings_NoFill();
    CP_Settings_Stroke(BLUE);
    CP_Settings_StrokeWeight(16.0f);
    CP_Graphics_DrawRect(100, 100, 80, 80); // edges at y=60 and y=140
}

static void Scn_SettingsStrokeWeight(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_NoFill();
    CP_Settings_Stroke(RED);
    CP_Settings_StrokeWeight(30.0f); // half-width 15
    CP_Graphics_DrawLine(100, 20, 100, 180);
}

static void Scn_SettingsRectModeCorner(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_NoStroke();
    CP_Settings_Fill(RED);
    CP_Settings_RectMode(CP_POSITION_CORNER);
    CP_Graphics_DrawRect(50, 50, 60, 60); // CORNER: spans [50,110] x [50,110]
}

static void Scn_SettingsEllipseModeCorner(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_NoStroke();
    CP_Settings_Fill(RED);
    CP_Settings_EllipseMode(CP_POSITION_CORNER);
    CP_Graphics_DrawEllipse(50, 50, 80, 80); // CORNER: center (90,90), r=40
}

static void Scn_SettingsTranslate(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_NoStroke();
    CP_Settings_Fill(RED);
    CP_Settings_Translate(50, 50);
    CP_Graphics_DrawRect(0, 0, 40, 40); // ends up centered at world (50,50)
}

static void Scn_SettingsScale(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_NoStroke();
    CP_Settings_Fill(RED);
    CP_Settings_Translate(100, 100);
    CP_Settings_Scale(2.0f, 2.0f);
    CP_Graphics_DrawRect(0, 0, 20, 20); // half-size 10 * scale 2 = 20 in world space
}

static void Scn_SettingsRotate(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_NoStroke();
    CP_Settings_Fill(RED);
    CP_Settings_Translate(100, 100);
    CP_Settings_Rotate(45);
    CP_Graphics_DrawRect(0, 0, 60, 10); // local half-extents x:+-30, y:+-5
}

static void Scn_SettingsResetMatrix(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_NoStroke();
    CP_Settings_Fill(RED);
    CP_Settings_Translate(50, 50);
    CP_Settings_ResetMatrix(); // cancels the translate above
    CP_Graphics_DrawRect(0, 0, 40, 40); // back at world origin, mostly off-canvas
}

static void Scn_SettingsApplyMatrix(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_NoStroke();
    CP_Settings_Fill(RED);
    CP_Settings_ApplyMatrix(CP_Matrix_Translate(CP_Vector_Set(70, 70)));
    CP_Graphics_DrawRect(0, 0, 40, 40); // centered at world (70,70)
}

static void Scn_SettingsBlendModeAdd(void)
{
    CP_Graphics_ClearBackground(BLACK);
    CP_Settings_BlendMode(CP_BLEND_ADD);
    CP_Settings_NoStroke();
    CP_Settings_Fill(CP_Color_Create(100, 0, 0, 255));
    CP_Graphics_DrawRect(80, 100, 60, 60); // spans x:[50,110]
    CP_Settings_Fill(CP_Color_Create(0, 100, 0, 255));
    CP_Graphics_DrawRect(120, 100, 60, 60); // spans x:[90,150]; overlap x:[90,110]
}

static void Scn_SettingsSaveRestore(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_Fill(RED);
    CP_Settings_Save();
    CP_Settings_Fill(BLUE);
    CP_Settings_Translate(50, 50);
    CP_Settings_Restore(); // should undo both the fill change and the translate
    CP_Settings_NoStroke();
    CP_Graphics_DrawRect(100, 100, 40, 40);
}

// ---- CP_Image (Phase E) ----
// Test fixture: Assets/quadrants.png, a 4x4 image with four distinct,
// solid-colored 2x2 quadrants (TL=red, TR=green, BL=blue, BR=yellow) --
// see the Phase E commit message for how it was generated. A tiny
// synthetic fixture with exact, known pixel values is more testable than
// a photo, and keeps this project's test assets independent of the demo
// app's Assets folder.

static void Scn_ImageLoadAndDraw(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_NEAREST); // crisp quadrant edges, no bilinear bleed
    CP_Settings_ImageMode(CP_POSITION_CENTER);

    CP_Image img = CP_Image_Load("Assets/quadrants.png");
    tier2_scalars.quadImageWidth = CP_Image_GetWidth(img);
    tier2_scalars.quadImageHeight = CP_Image_GetHeight(img);

    // Drawn as an 80x80 square centered on the canvas: spans x:[60,140], y:[60,140].
    CP_Image_Draw(img, 100, 100, 80, 80, 255);
    CP_Image_Free(&img);

    // CP_Image_CreateFromData / GetPixelData / UpdatePixelData round-trip,
    // entirely independent of any file -- a 2x2 synthetic buffer.
    CP_Color source[4] = {
        CP_Color_Create(10, 20, 30, 255), CP_Color_Create(40, 50, 60, 255),
        CP_Color_Create(70, 80, 90, 255), CP_Color_Create(100, 110, 120, 255)
    };
    CP_Image synthetic = CP_Image_CreateFromData(2, 2, (unsigned char*)source);
    CP_Image_GetPixelData(synthetic, tier2_scalars.createFromDataReadback);

    CP_Color updated[4] = {
        CP_Color_Create(200, 0, 0, 255), CP_Color_Create(0, 200, 0, 255),
        CP_Color_Create(0, 0, 200, 255), CP_Color_Create(200, 200, 0, 255)
    };
    CP_Image_UpdatePixelData(synthetic, updated);
    CP_Image_GetPixelData(synthetic, tier2_scalars.updatePixelDataReadback);
    CP_Image_Free(&synthetic);
}

static void Scn_ImageSubImage(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_NEAREST);
    CP_Settings_ImageMode(CP_POSITION_CENTER);

    CP_Image img = CP_Image_Load("Assets/quadrants.png");
    // s0/t0/s1/t1 are *pixel* coordinates into the source image, not
    // normalized [0,1] UVs (CP_Image.c:102-145, confirmed against the
    // wiki's Image page). (2,0)-(4,2) selects just the top-right (green)
    // quadrant of the 4x4 fixture; drawn as a 60x60 square centered on the
    // canvas.
    CP_Image_DrawSubImage(img, 100, 100, 60, 60, 2, 0, 4, 2, 255);
    CP_Image_Free(&img);
}

// ---- CP_Font (Phase E) ----
// Bounding-box/occupancy checks rather than pixel-perfect glyph
// comparisons, per 06-test-suite-plan.md's Tier 2 notes -- exact glyph
// rendering is too brittle to assert against directly.

static void Scn_FontDrawText(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_Fill(BLACK);
    CP_Font_Set(CP_Font_GetDefault());
    CP_Settings_TextSize(60.0f);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
    CP_Font_DrawText("I", 40, 40); // a single bold vertical stroke, cheap to bound reliably
}

static void Scn_FontLoadFree(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_Fill(BLACK);
    CP_Font customFont = CP_Font_Load("Assets/Exo2-Regular.ttf");
    CP_Font_Set(customFont);
    CP_Settings_TextSize(60.0f);
    CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
    CP_Font_DrawText("I", 40, 40);
    CP_Font_Free(&customFont);
}

// ---- CP_System / CP_Engine (Phase E) ----
// These read state via the public getters rather than touching _CORE
// directly, and must run *during* the single CP_Engine_Run() call: once it
// returns, CP_Shutdown() has already torn down the GLFW window/context,
// and several of these getters (ShowCursor, GetWindowFocus, ...) call
// straight into GLFW with no null-check guard.

static int preUpdateHookCount = 0;
static int postUpdateHookCount = 0;

static void PreUpdateHook(void) { ++preUpdateHookCount; }
static void PostUpdateHook(void) { ++postUpdateHookCount; }

static void Scn_SystemEngineState(void)
{
    CP_Graphics_ClearBackground(WHITE);

    tier2_scalars.windowWidthAfterSet = CP_System_GetWindowWidth();
    tier2_scalars.windowHeightAfterSet = CP_System_GetWindowHeight();
    tier2_scalars.frameCountLater = CP_System_GetFrameCount();
    tier2_scalars.dtSample = CP_System_GetDt();
    tier2_scalars.millisSample = CP_System_GetMillis();
    tier2_scalars.secondsSample = CP_System_GetSeconds();
    tier2_scalars.frameRateSample = CP_System_GetFrameRate();
    tier2_scalars.windowFocusSample = CP_System_GetWindowFocus();
    tier2_scalars.displayWidth = CP_System_GetDisplayWidth();
    tier2_scalars.displayHeight = CP_System_GetDisplayHeight();
    tier2_scalars.displayRefreshRate = CP_System_GetDisplayRefreshRate();
    tier2_scalars.preUpdateHookCount = preUpdateHookCount;
    tier2_scalars.postUpdateHookCount = postUpdateHookCount;
    tier2_scalars.windowHandleIsNull = CP_System_GetWindowHandle() == NULL;

    CP_System_SetWindowTitle("CProcessing Tier 2 Test Capture"); // just must not crash
    CP_System_ShowCursor(TRUE); // just must not crash
}

// ---- CP_Sound (Phase E) ----
// Per 06-test-suite-plan.md: no reliable way to assert "it sounds
// correct" automatically, so the realistic goal is load/free/play/pause/
// stop not crashing or leaking, plus group volume/pitch getters
// round-tripping what was set.

static void Scn_SoundRoundTrip(void)
{
    CP_Graphics_ClearBackground(WHITE);

    CP_Sound sound = CP_Sound_Load("Assets/beep.wav");

    CP_Sound_SetGroupVolume(CP_SOUND_GROUP_SFX, 0.3f);
    tier2_scalars.volumeAfterSet = CP_Sound_GetGroupVolume(CP_SOUND_GROUP_SFX);
    CP_Sound_SetGroupPitch(CP_SOUND_GROUP_SFX, 1.5f);
    tier2_scalars.pitchAfterSet = CP_Sound_GetGroupPitch(CP_SOUND_GROUP_SFX);

    CP_Sound_PlayAdvanced(sound, 0.1f, 1.0f, FALSE, CP_SOUND_GROUP_SFX);
    CP_Sound_PauseGroup(CP_SOUND_GROUP_SFX);
    CP_Sound_ResumeGroup(CP_SOUND_GROUP_SFX);
    CP_Sound_PauseAll();
    CP_Sound_ResumeAll();
    CP_Sound_StopGroup(CP_SOUND_GROUP_SFX);
    CP_Sound_Play(sound);
    CP_Sound_StopAll();

    CP_Sound_Free(&sound);
}

// ---- CP_Input (Phase F, partial -- see the Phase F commit message) ----
// This covers only the slice of CP_Input that's deterministically
// testable without synthesizing OS input events or hardware: default
// values in a quiescent frame where nothing was pressed, moved, or
// plugged in. The edge-detection/deadzone/mapping logic itself was split
// out into Source/Internal_InputLogic.h together with the XInput -> GLFW
// gamepad migration, and is unit-tested directly in the Tier 1 project
// (CProcessing_Tests/Tests/test_cp_input_logic.c).
static void Scn_InputQuiescentDefaults(void)
{
    CP_Graphics_ClearBackground(WHITE);

    // Assumes no gamepad is actually attached to the machine running this;
    // true in CI once Phase G lands, and was true on the dev machine this
    // was verified on, but is a real caveat for local runs elsewhere.
    tier2_scalars.gamepadConnected = CP_Input_GamepadConnected();
    tier2_scalars.gamepad0ConnectedAdvanced = CP_Input_GamepadConnectedAdvanced(0);
    tier2_scalars.mouseWheel = CP_Input_MouseWheel();
    tier2_scalars.mouseDoubleClicked = CP_Input_MouseDoubleClicked();
    tier2_scalars.keyADown = CP_Input_KeyDown(KEY_A);
    tier2_scalars.mouseLeftDown = CP_Input_MouseDown(MOUSE_BUTTON_LEFT);
}

// ---- Drawing persistence (cross-platform work) ----
// CProcessing never clears the screen between frames on its own: whatever
// was drawn stays until drawn over, via the offscreen canvas framebuffer
// (CP_System.c).
// The first scenario draws a red square; the next frame draws only a blue
// square without clearing, so both must be in the second capture.

static void Scn_PersistFirstFrame(void)
{
    CP_Graphics_ClearBackground(WHITE);
    CP_Settings_NoStroke();
    CP_Settings_Fill(RED);
    CP_Graphics_DrawRect(50, 50, 40, 40);
}

static void Scn_PersistNextFrame(void)
{
    // deliberately no ClearBackground
    CP_Settings_NoStroke();
    CP_Settings_Fill(BLUE);
    CP_Graphics_DrawRect(150, 150, 40, 40);
}

// ---- CP_Image_Screenshot of sub-regions (cross-platform work) ----
// The scenario captures always screenshot the whole canvas from (0,0); this
// checks offset regions too, i.e. the window-to-framebuffer coordinate
// conversion (y is flipped for glReadPixels).

static CP_Color SampleImage(CP_Image image, int x, int y)
{
    static CP_Color pixels[TIER2_CANVAS_SIZE * TIER2_CANVAS_SIZE];
    int w = CP_Image_GetWidth(image);
    CP_Image_GetPixelData(image, pixels);
    return pixels[y * w + x];
}

static void Scn_ScreenshotSubregion(void)
{
    CP_Settings_NoStroke();
    CP_Settings_RectMode(CP_POSITION_CORNER);
    CP_Settings_Fill(RED);
    CP_Graphics_DrawRect(0, 0, 100, 100);
    CP_Settings_Fill(GREEN);
    CP_Graphics_DrawRect(100, 0, 100, 100);
    CP_Settings_Fill(BLUE);
    CP_Graphics_DrawRect(0, 100, 100, 100);
    CP_Settings_Fill(CP_Color_Create(255, 255, 0, 255));
    CP_Graphics_DrawRect(100, 100, 100, 100);

    CP_Image topRight = CP_Image_Screenshot(100, 0, 100, 100);
    tier2_scalars.subTopRightWidth = CP_Image_GetWidth(topRight);
    tier2_scalars.subTopRightHeight = CP_Image_GetHeight(topRight);
    tier2_scalars.subTopRightCenter = SampleImage(topRight, 50, 50);
    CP_Image_Free(&topRight);

    CP_Image bottomLeft = CP_Image_Screenshot(0, 100, 100, 100);
    tier2_scalars.subBottomLeftCenter = SampleImage(bottomLeft, 50, 50);
    CP_Image_Free(&bottomLeft);

    // 50x50 region centered on the point where all four quadrants meet
    CP_Image straddle = CP_Image_Screenshot(75, 75, 50, 50);
    tier2_scalars.straddleCorners[0] = SampleImage(straddle, 5, 5);
    tier2_scalars.straddleCorners[1] = SampleImage(straddle, 44, 5);
    tier2_scalars.straddleCorners[2] = SampleImage(straddle, 5, 44);
    tier2_scalars.straddleCorners[3] = SampleImage(straddle, 44, 44);
    CP_Image_Free(&straddle);
}

// ---- Error paths (cross-platform work) ----
// Missing files and NULL handles must fail quietly (NULL / no-op) rather
// than crash -- including when there is no audio device at all, which is
// the normal case on CI runners.

static void Scn_ErrorPaths(void)
{
    CP_Graphics_ClearBackground(WHITE);

    CP_Image missingImage = CP_Image_Load("Assets/this_file_does_not_exist.png");
    tier2_scalars.missingImageIsNull = missingImage == NULL;
    CP_Font missingFont = CP_Font_Load("Assets/this_file_does_not_exist.ttf");
    tier2_scalars.missingFontIsNull = missingFont == NULL;
    CP_Sound missingSound = CP_Sound_Load("Assets/this_file_does_not_exist.wav");
    tier2_scalars.missingSoundIsNull = missingSound == NULL;
    // a NULL path is treated like a missing file
    tier2_scalars.missingImageIsNull = tier2_scalars.missingImageIsNull && CP_Image_Load(NULL) == NULL;
    tier2_scalars.missingFontIsNull = tier2_scalars.missingFontIsNull && CP_Font_Load(NULL) == NULL;
    tier2_scalars.missingSoundIsNull = tier2_scalars.missingSoundIsNull && CP_Sound_Load(NULL) == NULL
        && CP_Sound_LoadStream(NULL) == NULL;

    // None of these may crash
    CP_Image nullImage = NULL;
    CP_Sound nullSound = NULL;
    CP_Font nullFont = NULL;
    CP_Image_Draw(NULL, 10, 10, 10, 10, 255);
    CP_Image_Free(NULL);
    CP_Image_Free(&nullImage);
    CP_Sound_Play(NULL);
    CP_Sound_PlayAdvanced(NULL, 1.0f, 1.0f, FALSE, CP_SOUND_GROUP_SFX);
    CP_Sound_Free(NULL);
    CP_Sound_Free(&nullSound);
    CP_Font_Set(NULL);
    CP_Font_Free(NULL);
    CP_Font_Free(&nullFont);
    tier2_scalars.survivedNullCalls = TRUE;
}

typedef void (*ScenarioFunc)(void);

static const ScenarioFunc kScenarios[SCN_COUNT] = {
    [SCN_CLEAR_BACKGROUND] = Scn_ClearBackground,
    [SCN_DRAW_POINT] = Scn_DrawPoint,
    [SCN_DRAW_LINE] = Scn_DrawLine,
    [SCN_DRAW_LINE_ADVANCED] = Scn_DrawLineAdvanced,
    [SCN_DRAW_RECT] = Scn_DrawRect,
    [SCN_DRAW_RECT_ADVANCED] = Scn_DrawRectAdvanced,
    [SCN_DRAW_CIRCLE] = Scn_DrawCircle,
    [SCN_DRAW_ELLIPSE] = Scn_DrawEllipse,
    [SCN_DRAW_ELLIPSE_ADVANCED] = Scn_DrawEllipseAdvanced,
    [SCN_DRAW_TRIANGLE] = Scn_DrawTriangle,
    [SCN_DRAW_TRIANGLE_ADVANCED] = Scn_DrawTriangleAdvanced,
    [SCN_DRAW_QUAD] = Scn_DrawQuad,
    [SCN_DRAW_QUAD_ADVANCED] = Scn_DrawQuadAdvanced,
    [SCN_CUSTOM_SHAPE] = Scn_CustomShape,
    [SCN_SETTINGS_NOFILL] = Scn_SettingsNoFill,
    [SCN_SETTINGS_STROKEWEIGHT] = Scn_SettingsStrokeWeight,
    [SCN_SETTINGS_RECTMODE_CORNER] = Scn_SettingsRectModeCorner,
    [SCN_SETTINGS_ELLIPSEMODE_CORNER] = Scn_SettingsEllipseModeCorner,
    [SCN_SETTINGS_TRANSLATE] = Scn_SettingsTranslate,
    [SCN_SETTINGS_SCALE] = Scn_SettingsScale,
    [SCN_SETTINGS_ROTATE] = Scn_SettingsRotate,
    [SCN_SETTINGS_RESETMATRIX] = Scn_SettingsResetMatrix,
    [SCN_SETTINGS_APPLYMATRIX] = Scn_SettingsApplyMatrix,
    [SCN_SETTINGS_BLENDMODE_ADD] = Scn_SettingsBlendModeAdd,
    [SCN_SETTINGS_SAVE_RESTORE] = Scn_SettingsSaveRestore,
    [SCN_IMAGE_LOAD_AND_DRAW] = Scn_ImageLoadAndDraw,
    [SCN_IMAGE_SUBIMAGE] = Scn_ImageSubImage,
    [SCN_FONT_DRAWTEXT] = Scn_FontDrawText,
    [SCN_FONT_LOAD_FREE] = Scn_FontLoadFree,
    [SCN_SYSTEM_ENGINE_STATE] = Scn_SystemEngineState,
    [SCN_SOUND_ROUNDTRIP] = Scn_SoundRoundTrip,
    [SCN_INPUT_QUIESCENT_DEFAULTS] = Scn_InputQuiescentDefaults,
    [SCN_PERSIST_FIRST_FRAME] = Scn_PersistFirstFrame,
    [SCN_PERSIST_NEXT_FRAME] = Scn_PersistNextFrame,
    [SCN_SCREENSHOT_SUBREGION] = Scn_ScreenshotSubregion,
    [SCN_ERROR_PATHS] = Scn_ErrorPaths,
};

static void HarnessInit(void)
{
    CP_System_SetWindowSize(TIER2_CANVAS_SIZE, TIER2_CANVAS_SIZE);
    CP_System_SetWindowTitle("CProcessing Tier 2 Test Capture");
    CP_Engine_SetPreUpdateFunction(PreUpdateHook);
    CP_Engine_SetPostUpdateFunction(PostUpdateHook);
    frameCount = 0;
    firstScenarioFrame = -1;
}

static void HarnessUpdate(void)
{
    // Let the deferred window resize (requested in HarnessInit) take effect
    // before drawing/capturing anything.
    if (firstScenarioFrame < 0)
    {
        bool sizeSettled = CP_System_GetWindowWidth() == TIER2_CANVAS_SIZE
            && CP_System_GetWindowHeight() == TIER2_CANVAS_SIZE;
        if ((frameCount >= WARMUP_FRAMES && sizeSettled) || frameCount >= MAX_WARMUP_FRAMES)
        {
            firstScenarioFrame = frameCount;
        }
        else
        {
            ++frameCount;
            return;
        }
    }

    int scenarioIndex = frameCount - firstScenarioFrame;
    ++frameCount;

    if (scenarioIndex < SCN_COUNT)
    {
        ResetToBaseline();
        kScenarios[scenarioIndex]();
        CaptureCurrentFrame((Tier2Scenario)scenarioIndex);
    }

    if (scenarioIndex >= SCN_COUNT - 1)
    {
        CP_Engine_Terminate();
    }
}

void tier2_RunCaptureOnce(void)
{
    CP_Engine_SetNextGameState(HarnessInit, HarnessUpdate, NULL);
    CP_Engine_Run();
}
