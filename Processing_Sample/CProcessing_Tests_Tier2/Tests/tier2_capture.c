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

#define WHITE CP_Color_Create(255, 255, 255, 255)
#define BLACK CP_Color_Create(0, 0, 0, 255)
#define RED CP_Color_Create(255, 0, 0, 255)
#define BLUE CP_Color_Create(0, 0, 255, 255)
#define GREEN CP_Color_Create(0, 200, 0, 255)

static int frameCount = 0;
#define WARMUP_FRAMES 2

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
};

static void HarnessInit(void)
{
    CP_System_SetWindowSize(TIER2_CANVAS_SIZE, TIER2_CANVAS_SIZE);
    CP_System_SetWindowTitle("CProcessing Tier 2 Test Capture");
    frameCount = 0;
}

static void HarnessUpdate(void)
{
    int scenarioIndex = frameCount - WARMUP_FRAMES;
    ++frameCount;

    // Let the deferred window resize (requested in HarnessInit) take effect
    // before drawing/capturing anything.
    if (scenarioIndex < 0)
    {
        return;
    }

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
