// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors

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
#define _CRT_SECURE_NO_WARNINGS // fopen, in the exit report capture
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tier2_capture.h"
#if defined(_WIN32)
	#include <io.h>
	#define tier2_dup    _dup
	#define tier2_dup2   _dup2
	#define tier2_close  _close
	#define tier2_fileno _fileno
#else
	#include <unistd.h>
	#define tier2_dup    dup
	#define tier2_dup2   dup2
	#define tier2_close  close
	#define tier2_fileno fileno
#endif

// For Scn_AssetsNextToProgram, which runs in another folder for a moment
#if defined(_WIN32)
	#include <direct.h>
	#define tier2_getcwd      _getcwd
	#define tier2_chdir       _chdir
	#define tier2_mkdir(path) _mkdir(path)
	#define tier2_rmdir       _rmdir
#else
	#include <sys/stat.h>
	#include <unistd.h>
	#define tier2_getcwd      getcwd
	#define tier2_chdir       chdir
	#define tier2_mkdir(path) mkdir(path, 0755)
	#define tier2_rmdir       rmdir
#endif

CP_Color tier2_snapshots[SCN_COUNT][TIER2_CANVAS_SIZE * TIER2_CANVAS_SIZE];
Tier2Scalars tier2_scalars = { 0 };

// CProcessing's messages go to stderr (DESIGN.md, "Messages"). BeginMessages
// sends stderr to a file while calls that should print a message run, and
// EndMessages adds what they printed to text, puts stderr back, and shows it
// in the log too. It adds rather than replaces, because a scenario can run on
// more than one frame, and a message printed once is only there the first
// time. (The library and the tests share the C runtime, so this catches the
// library's stderr.)
static FILE* messageFile = NULL;
static int messageSavedStderr = -1;

static void BeginMessages(void)
{
	fflush(stderr);
	messageFile = fopen("tier2_messages.txt", "w+");
	if (messageFile)
	{
		messageSavedStderr = tier2_dup(tier2_fileno(stderr));
		tier2_dup2(tier2_fileno(messageFile), tier2_fileno(stderr));
	}
}

static void EndMessages(char* text, size_t size)
{
	if (!messageFile)
	{
		return;
	}
	fflush(stderr);
	tier2_dup2(messageSavedStderr, tier2_fileno(stderr));
	tier2_close(messageSavedStderr);
	rewind(messageFile);
	const size_t start = strlen(text);
	const size_t length = fread(text + start, 1, size - 1 - start, messageFile);
	text[start + length] = '\0';
	fclose(messageFile);
	messageFile = NULL;
	remove("tier2_messages.txt");
	fputs(text + start, stderr);
}

int tier2_CountText(const char* text, const char* part)
{
	int count = 0;
	for (const char* at = strstr(text, part); at; at = strstr(at + 1, part))
	{
		++count;
	}
	return count;
}

#define WHITE CP_Color_Create(255, 255, 255, 255)
#define BLACK CP_Color_Create(0, 0, 0, 255)
#define RED   CP_Color_Create(255, 0, 0, 255)
#define BLUE  CP_Color_Create(0, 0, 255, 255)
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
#define WARMUP_FRAMES     15
#define MAX_WARMUP_FRAMES 600

static void CaptureInto(CP_Color* pixels)
{
	CP_Image shot = CP_Image_Screenshot(0, 0, TIER2_CANVAS_SIZE, TIER2_CANVAS_SIZE);
	CP_Image_GetPixelData(shot, pixels);
	CP_Image_Free(&shot);
}

static void CaptureCurrentFrame(Tier2Scenario scenario)
{
	CaptureInto(tier2_snapshots[scenario]);
}

// Known-good baseline so each scenario starts from the same state
// regardless of what the previous scenario left behind.
static void ResetToBaseline(void)
{
	CP_Settings_ResetMatrix();
	CP_Settings_Fill(WHITE);
	CP_Settings_Stroke(BLACK);
	CP_Settings_StrokeWeight(3.0f);
	CP_Settings_LineCapMode(CP_LINE_CAP_BUTT);
	CP_Settings_RectMode(CP_POSITION_CENTER);
	CP_Settings_EllipseMode(CP_POSITION_CENTER);
	CP_Settings_ImageMode(CP_POSITION_CENTER);
	CP_Settings_BlendMode(CP_BLEND_ALPHA);
	CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_LINEAR);
	CP_Settings_ImageWrapMode(CP_IMAGE_WRAP_CLAMP_EDGE);
	CP_Settings_TextBlur(0.0f);
	CP_Settings_TextLetterSpacing(0.0f);
	CP_Settings_TextLineSpacing(0.0f);
	CP_System_SetFrameRate(60.0f);
}

// Draws with the settings made before CP_Engine_Run (tier2_RunCaptureOnce):
// a fill of (12, 200, 34) and CORNER rects, kept from before the window
// existed, and the default stroke, black and 3 wide.
// Bottom right, with the default image settings (CENTER, linear, clamp to
// edge): a 2x2 image (red, green / blue, yellow) as a sub-image twice its
// size, so the image fills [155,175] x [155,175] and past its edges, to 195,
// are its edge pixels stretched out.
static void Scn_SettingsBeforeRun(void)
{
	CP_Graphics_ClearBackground(BLUE);
	CP_Graphics_DrawRect(50, 50, 100, 100); // CORNER: spans [50,150] x [50,150]

	const CP_Color pixels[4] = {
		CP_Color_Create(255, 0, 0, 255),
		CP_Color_Create(0, 255, 0, 255),
		CP_Color_Create(0, 0, 255, 255),
		CP_Color_Create(255, 255, 0, 255),
	};
	CP_Image image = CP_Image_CreateFromData(2, 2, pixels);
	CP_Image_DrawSubImage(image, 175, 175, 40, 40, 0, 0, 4, 4, 255);
	CP_Image_Free(&image);
}

// Vsync and a resizable window were turned on before CP_Engine_Run
// (tier2_RunCaptureOnce): they are still on with the window open. They go
// back off here, so the other scenarios run as they always have.
static void Scn_SystemGetters(void)
{
	CP_Graphics_ClearBackground(WHITE);

	tier2_scalars.vsyncInRun = CP_System_GetVSync();
	tier2_scalars.resizableInRun = CP_System_GetWindowResizable();
	tier2_scalars.fullscreenInRun = CP_System_IsFullscreen();
	CP_System_SetVSync(false);
	CP_System_SetWindowResizable(false);
	tier2_scalars.vsyncSetOff = !CP_System_GetVSync();
	tier2_scalars.resizableSetOff = !CP_System_GetWindowResizable();

	// ResetToBaseline set 60 frames a second
	tier2_scalars.targetFrameRateDefault = CP_System_GetTargetFrameRate();
	CP_System_SetFrameRate(30.0f);
	tier2_scalars.targetFrameRateSet30 = CP_System_GetTargetFrameRate();
	CP_System_SetFrameRate(0.0f);
	tier2_scalars.targetFrameRateSet0 = CP_System_GetTargetFrameRate();
	CP_System_SetFrameRate(-30.0f);
	tier2_scalars.targetFrameRateSetNegative = CP_System_GetTargetFrameRate();
	CP_System_SetFrameRate(60.0f);
}

// Each getter gives back what was set, and Save and Restore bring them back
static void Scn_SettingsGetters(void)
{
	CP_Graphics_ClearBackground(WHITE);

	const CP_Color fill = CP_Color_Create(10, 20, 30, 40);
	const CP_Color stroke = CP_Color_Create(50, 60, 70, 80);
	CP_Settings_Fill(fill);
	CP_Settings_Stroke(stroke);
	CP_Settings_StrokeWeight(7.5f);
	CP_Settings_TextSize(33.0f);
	CP_Color gotFill = CP_Settings_GetFill();
	CP_Color gotStroke = CP_Settings_GetStroke();
	tier2_scalars.fillGetsWhatWasSet = gotFill.r == 10 && gotFill.g == 20 && gotFill.b == 30 && gotFill.a == 40;
	tier2_scalars.strokeGetsWhatWasSet = gotStroke.r == 50 && gotStroke.g == 60 && gotStroke.b == 70 && gotStroke.a == 80;
	tier2_scalars.strokeWeightGetsWhatWasSet = CP_Settings_GetStrokeWeight() == 7.5f;
	tier2_scalars.textSizeGetsWhatWasSet = CP_Settings_GetTextSize() == 33.0f;

	// The colors stay what they were, though there is no fill or stroke now
	CP_Settings_NoFill();
	CP_Settings_NoStroke();
	gotFill = CP_Settings_GetFill();
	gotStroke = CP_Settings_GetStroke();
	tier2_scalars.fillKeptAfterNoFill = gotFill.r == 10 && gotFill.a == 40;
	tier2_scalars.strokeKeptAfterNoStroke = gotStroke.r == 50 && gotStroke.a == 80;

	// Changes made after Save are undone by Restore
	CP_Settings_Save();
	CP_Settings_Fill(CP_Color_Create(1, 2, 3, 4));
	CP_Settings_Stroke(CP_Color_Create(5, 6, 7, 8));
	CP_Settings_StrokeWeight(1.0f);
	CP_Settings_TextSize(10.0f);
	CP_Settings_Restore();
	gotFill = CP_Settings_GetFill();
	gotStroke = CP_Settings_GetStroke();
	tier2_scalars.settingsBackAfterRestore = gotFill.r == 10 && gotStroke.r == 50 &&
											 CP_Settings_GetStrokeWeight() == 7.5f && CP_Settings_GetTextSize() == 33.0f;
}

// The font starts as the default one, is the one that CP_Font_Set chose, goes
// back with Restore, and is the default again if it is freed while in use
static void Scn_FontGetCurrent(void)
{
	CP_Graphics_ClearBackground(WHITE);

	const CP_Font defaultFont = CP_Font_GetDefault();
	tier2_scalars.fontStartsAsDefault = CP_Font_GetCurrent() == defaultFont;

	CP_Font custom = CP_Font_Load("assets/Exo2-Regular.otf");
	CP_Font_Set(custom);
	tier2_scalars.fontSetIsCurrent = custom != NULL && CP_Font_GetCurrent() == custom;

	CP_Settings_Save();
	CP_Font_Set(defaultFont);
	CP_Settings_Restore();
	tier2_scalars.fontBackAfterRestore = CP_Font_GetCurrent() == custom;

	CP_Font_Free(&custom);
	tier2_scalars.freedFontFallsBackToDefault = CP_Font_GetCurrent() == defaultFont;
	CP_Font_Set(defaultFont);
}

static void Scn_ClearBackground(void)
{
	CP_Graphics_ClearBackground(CP_Color_Create(10, 20, 30, 255));
	// Captured here (one of the first scenarios) and again in Scn_SystemEngineState
	// (one of the last) so the test suite can confirm frame count actually
	// advances across the scripted run.
	tier2_scalars.frameCountEarly = CP_System_GetFrameCount();
}

// A point follows the line settings, as the wiki documents: the stroke
// color, the stroke weight as its size, and the line cap as its shape. It
// has no fill. See test_tier2_graphics_drawpoint_follows_stroke_settings.
static void Scn_DrawPoint(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_Fill(CP_Color_Create(0, 255, 0, 255));
	CP_Settings_Stroke(RED);
	CP_Settings_StrokeWeight(20.0f);

	// Round cap: a circle 20 across. Square cap: a 20x20 square.
	CP_Settings_LineCapMode(CP_LINE_CAP_ROUND);
	CP_Graphics_DrawPoint(50, 50);
	CP_Settings_LineCapMode(CP_LINE_CAP_SQUARE);
	CP_Graphics_DrawPoint(150, 50);

	// Drawing a point leaves the fill color alone
	CP_Settings_NoStroke();
	CP_Graphics_DrawRect(100, 100, 20, 20);

	// No stroke hides a point; no fill doesn't
	CP_Graphics_DrawPoint(100, 150);
	CP_Settings_Stroke(RED);
	CP_Settings_NoFill();
	CP_Graphics_DrawPoint(50, 150);

	// A stroke color undone by CP_Settings_Restore is undone for points too
	CP_Settings_Save();
	CP_Settings_Stroke(CP_Color_Create(0, 0, 255, 255));
	CP_Settings_Restore();
	CP_Graphics_DrawPoint(150, 150);
}

static void Scn_DrawLine(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_Stroke(RED);
	CP_Settings_StrokeWeight(10.0f);
	CP_Graphics_DrawLine(20, 100, 180, 100);
}

static void Scn_DrawLineRotated(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_Stroke(RED);
	CP_Settings_StrokeWeight(10.0f);
	// Turned 90 degrees about its own midpoint (100,100), the way the docs
	// teach it: move (0, 0) there, rotate, draw around (0, 0). A horizontal
	// line becomes vertical, so the original endpoint is no longer covered.
	CP_Settings_Save();
	CP_Settings_Translate(100, 100);
	CP_Settings_Rotate(90);
	CP_Graphics_DrawLine(-80, 0, 80, 0);
	CP_Settings_Restore();
}

static void Scn_DrawRect(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_Fill(RED);
	CP_Graphics_DrawRect(100, 100, 80, 80);
}

static void Scn_DrawRectRotated(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_Fill(RED);
	// An 80x20 rect turned 45 degrees about its center (100,100).
	CP_Settings_Save();
	CP_Settings_Translate(100, 100);
	CP_Settings_Rotate(45);
	CP_Graphics_DrawRect(0, 0, 80, 20);
	CP_Settings_Restore();
}

// An 80x80 square around (100, 100) with corners of radius 20: the very
// corner is cut away, the middle of each edge is still there
static void Scn_DrawRoundedRect(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_Fill(RED);
	CP_Graphics_DrawRoundedRect(100, 100, 80, 80, 20);
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

static void Scn_DrawEllipseRotated(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_Fill(RED);
	// Same 120x40 ellipse, turned 90 degrees: the wide axis becomes vertical.
	CP_Settings_Save();
	CP_Settings_Translate(100, 100);
	CP_Settings_Rotate(90);
	CP_Graphics_DrawEllipse(0, 0, 120, 40);
	CP_Settings_Restore();
}

// A quarter of a circle of diameter 160 around (100, 100), from the angle 0
// (to the right) clockwise, so down: it passes (180, 100) and (100, 180).
static void Scn_DrawArc(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_Fill(RED); // an arc is a line: this must not fill it
	CP_Graphics_DrawArc(100, 100, 160, 0, 90);
}

// The same quarter, but a negative sweep goes the other way: up
static void Scn_DrawArcCounterclockwise(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Graphics_DrawArc(100, 100, 160, 0, -90);
}

// A filled quarter of the circle: the bottom right
static void Scn_DrawPie(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_Fill(RED);
	CP_Graphics_DrawPie(100, 100, 160, 0, 90);
}

// In CORNER ellipse mode the circle's top left is the position: this is the
// bottom right quarter of the circle centered on (100, 100), as above
static void Scn_DrawPieCornerMode(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_Fill(RED);
	CP_Settings_EllipseMode(CP_POSITION_CORNER);
	CP_Graphics_DrawPie(20, 20, 160, 0, 90);
}

// A curve from (20, 150) to (180, 150) pulled up by both control points; at
// half way along it is at (100, 52.5)
static void Scn_DrawBezier(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_Fill(RED); // a curve is a line: this must not fill it
	CP_Graphics_DrawBezier(20, 150, 20, 20, 180, 20, 180, 150);
}

static void Scn_DrawTriangle(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_Fill(RED);
	// Centroid is exactly (100,100): ((100+143+57)/3, (50+125+125)/3).
	CP_Graphics_DrawTriangle(100, 50, 143, 125, 57, 125);
}

static void Scn_DrawTriangleRotated(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_Fill(RED);
	// Same triangle, turned 180 degrees about its centroid (100,100): the
	// corners are given around (0, 0), and the apex that was at the top
	// (100,50) is now at the bottom (100,150).
	CP_Settings_Save();
	CP_Settings_Translate(100, 100);
	CP_Settings_Rotate(180);
	CP_Graphics_DrawTriangle(0, -50, 43, 25, -43, 25);
	CP_Settings_Restore();
}

static void Scn_DrawQuad(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_Fill(RED);
	// An axis-aligned 60x60 square (half-size 30) centered at (100,100).
	CP_Graphics_DrawQuad(70, 70, 130, 70, 130, 130, 70, 130);
}

static void Scn_DrawQuadRotated(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_Fill(RED);
	// Same square, turned 45 degrees about its center: a diamond whose
	// reach along the pure-vertical/horizontal axes grows from 30 to
	// 30*sqrt(2) =~ 42.4, covering points the unrotated square did not.
	CP_Settings_Save();
	CP_Settings_Translate(100, 100);
	CP_Settings_Rotate(45);
	CP_Graphics_DrawQuad(-30, -30, 30, -30, 30, 30, -30, 30);
	CP_Settings_Restore();
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

// A U of three edges, outline only: EndShape closes it, so the fourth edge
// (across the top, from (130, 70) back to (70, 70)) is drawn
static void Scn_ShapeOutlineClosed(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoFill();
	CP_Settings_Stroke(RED);
	CP_Settings_StrokeWeight(6.0f);
	CP_Graphics_BeginShape();
	CP_Graphics_AddVertex(70, 70);
	CP_Graphics_AddVertex(70, 130);
	CP_Graphics_AddVertex(130, 130);
	CP_Graphics_AddVertex(130, 70);
	CP_Graphics_EndShape();
}

// The same vertices with EndShapeOpen: the top edge is missing
static void Scn_ShapeOutlineOpen(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoFill();
	CP_Settings_Stroke(RED);
	CP_Settings_StrokeWeight(6.0f);
	CP_Graphics_BeginShape();
	CP_Graphics_AddVertex(70, 70);
	CP_Graphics_AddVertex(70, 130);
	CP_Graphics_AddVertex(130, 130);
	CP_Graphics_AddVertex(130, 70);
	CP_Graphics_EndShapeOpen();
}

// ---- Curves and holes in shapes ----

// A red square of the given corners, as a shape's outline or a hole
static void AddRectVertices(float left, float top, float right, float bottom)
{
	CP_Graphics_AddVertex(left, top);
	CP_Graphics_AddVertex(right, top);
	CP_Graphics_AddVertex(right, bottom);
	CP_Graphics_AddVertex(left, bottom);
}

// From (20, 150) to (180, 150), bending toward (100, 10): the middle of the
// curve is at (100, 80), and the shape closes along y = 150. It's given 10
// left and 20 up, under a Translate(10, 20), so the points move with it.
static void Scn_ShapeQuadratic(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_Fill(RED);
	CP_Settings_Translate(10, 20);
	CP_Graphics_BeginShape();
	CP_Graphics_AddVertex(10, 130);
	CP_Graphics_AddQuadraticVertex(90, -10, 170, 130);
	CP_Graphics_EndShape();
}

// An S curve from (20, 100) to (180, 100), leaving toward (70, 0) and
// arriving from (130, 200), then down and back along y = 190. A quarter of
// the way along, the curve is at about (59, 72), above the straight line;
// three quarters along, at about (141, 128), below it.
static void Scn_ShapeBezier(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_Fill(RED);
	CP_Graphics_BeginShape();
	CP_Graphics_AddVertex(20, 100);
	CP_Graphics_AddBezierVertex(70, 0, 130, 200, 180, 100);
	CP_Graphics_AddVertex(180, 190);
	CP_Graphics_AddVertex(20, 190);
	CP_Graphics_EndShape();
}

// A curve as the first point starts the shape at its end point, (50, 50):
// the shape is the square [50,150]. (NanoVG alone would drop it, leaving a
// triangle.)
static void Scn_ShapeCurveFirst(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_Fill(RED);
	CP_Graphics_BeginShape();
	CP_Graphics_AddQuadraticVertex(0, 0, 50, 50);
	CP_Graphics_AddVertex(150, 50);
	CP_Graphics_AddVertex(150, 150);
	CP_Graphics_AddVertex(50, 150);
	CP_Graphics_EndShape();
}

// The quadratic curve's outline alone, open: no line along y = 150
static void Scn_ShapeCurveOpen(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoFill();
	CP_Settings_Stroke(RED);
	CP_Settings_StrokeWeight(6.0f);
	CP_Graphics_BeginShape();
	CP_Graphics_AddVertex(20, 150);
	CP_Graphics_AddQuadraticVertex(100, 10, 180, 150);
	CP_Graphics_EndShapeOpen();
}

// Two squares with a square hole each. On the left the hole's points go the
// same way round as the outline's, on the right the other way: both holes.
static void Scn_ShapeHoleEitherWay(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_Fill(RED);
	CP_Graphics_BeginShape();
	AddRectVertices(10, 50, 95, 150);
	CP_Graphics_BeginHole();
	AddRectVertices(35, 85, 70, 115);
	CP_Graphics_EndHole();
	CP_Graphics_EndShape();

	CP_Graphics_BeginShape();
	AddRectVertices(105, 50, 190, 150);
	CP_Graphics_BeginHole();
	CP_Graphics_AddVertex(130, 85);
	CP_Graphics_AddVertex(130, 115);
	CP_Graphics_AddVertex(165, 115);
	CP_Graphics_AddVertex(165, 85);
	CP_Graphics_EndHole();
	CP_Graphics_EndShape();
}

// The hole before the outline: the same as after it
static void Scn_ShapeHoleFirst(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_Fill(RED);
	CP_Graphics_BeginShape();
	CP_Graphics_BeginHole();
	AddRectVertices(70, 70, 130, 130);
	CP_Graphics_EndHole();
	AddRectVertices(20, 20, 180, 180);
	CP_Graphics_EndShape();
}

// Two holes with an outline 4 wide, and no EndHole: the second BeginHole
// ends the first hole, and EndShape the second. Every edge is outlined,
// including the closing edges, x = 20 (the outline's, closed by the first
// BeginHole) and x = 120 (the second hole's, closed by EndShape).
static void Scn_ShapeHolesOutlined(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_Fill(RED);
	CP_Settings_Stroke(BLACK);
	CP_Settings_StrokeWeight(4.0f);
	CP_Graphics_BeginShape();
	AddRectVertices(20, 60, 180, 140);
	CP_Graphics_BeginHole();
	AddRectVertices(40, 80, 80, 120);
	CP_Graphics_BeginHole();
	AddRectVertices(120, 80, 160, 120);
	CP_Graphics_EndShape();
}

// The outline can't go on after a hole: its points there are left out (with
// a message). Added,
// they'd have made a piece of their own: the triangle to the right of the
// square.
static void Scn_ShapeOutlineAfterHole(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_Fill(RED);
	CP_Graphics_BeginShape();
	AddRectVertices(20, 20, 180, 180);
	CP_Graphics_BeginHole();
	AddRectVertices(70, 70, 130, 130);
	CP_Graphics_EndHole();
	CP_Graphics_AddVertex(184, 20);
	CP_Graphics_AddVertex(198, 20);
	CP_Graphics_AddQuadraticVertex(199, 70, 198, 120);
	CP_Graphics_EndShape();
}

static void Scn_ShapeHoleOutside(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_Fill(RED);
	CP_Graphics_BeginShape();
	AddRectVertices(20, 50, 120, 150);
	CP_Graphics_BeginHole();
	AddRectVertices(90, 80, 150, 120);
	CP_Graphics_EndHole();
	CP_Graphics_EndShape();
}

// Two holes that overlap in [90,110]: the overlap is filled again (1 - 1 - 1)
static void Scn_ShapeHolesOverlap(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_Fill(RED);
	CP_Graphics_BeginShape();
	AddRectVertices(20, 50, 180, 150);
	CP_Graphics_BeginHole();
	AddRectVertices(50, 80, 110, 120);
	CP_Graphics_EndHole();
	CP_Graphics_BeginHole();
	AddRectVertices(90, 80, 150, 120);
	CP_Graphics_EndHole();
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
	CP_Settings_ResetMatrix();          // cancels the translate above
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

// ---- Settings that CProcessing patches into NanoVG ----
// See CProcessing/third_party/nanovg/CPROCESSING.md. Each of these
// guards a patch, so an update of NanoVG that loses one fails here.

// The blend modes that need a blend equation, over a (200, 100, 50)
// background, each with a (100, 100, 100) square:
// subtract (background minus square), multiply, min and max.
static void Scn_SettingsBlendModes(void)
{
	CP_Graphics_ClearBackground(CP_Color_Create(200, 100, 50, 255));
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(100, 100, 100, 255));
	const CP_BLEND_MODE modes[4] = { CP_BLEND_SUBTRACT, CP_BLEND_MULTIPLY, CP_BLEND_MIN, CP_BLEND_MAX };
	for (int i = 0; i < 4; ++i)
	{
		CP_Settings_BlendMode(modes[i]);
		CP_Graphics_DrawRect(40.0f + 40.0f * i, 100, 30, 30);
	}
}

// Tint multiplies what is drawn, weighted by the tint's alpha: white
// squares tinted red and half-strength blue, and a white image tinted green.
static void Scn_SettingsTint(void)
{
	CP_Graphics_ClearBackground(BLACK);
	CP_Settings_NoStroke();
	CP_Settings_Fill(WHITE);
	CP_Settings_Tint(CP_Color_Create(255, 0, 0, 255));
	CP_Graphics_DrawRect(50, 50, 40, 40);
	CP_Settings_Tint(CP_Color_Create(0, 0, 255, 128));
	CP_Graphics_DrawRect(150, 50, 40, 40);

	CP_Color white[4] = { WHITE, WHITE, WHITE, WHITE };
	CP_Image image = CP_Image_CreateFromData(2, 2, white);
	CP_Settings_Tint(CP_Color_Create(0, 255, 0, 255));
	CP_Image_Draw(image, 100, 150, 40, 40, 255);
	CP_Image_Free(&image);
	CP_Settings_NoTint();
}

// The overlay mixes a color in after the tint, keeping each pixel's alpha:
// shapes, an image with see-through pixels, a gradient and text
static void Scn_SettingsOverlay(void)
{
	const CP_Color red = CP_Color_Create(255, 0, 0, 255);
	CP_Settings_Save();
	CP_Graphics_ClearBackground(BLACK);
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CENTER);

	// Row 1: red in white; red in half white; white tinted red, then half blue
	CP_Settings_Fill(red);
	CP_Settings_Overlay(WHITE);
	CP_Graphics_DrawRect(30, 30, 40, 40);
	CP_Settings_Overlay(CP_Color_Create(255, 255, 255, 128));
	CP_Graphics_DrawRect(100, 30, 40, 40);
	CP_Settings_Fill(WHITE);
	CP_Settings_Tint(red);
	CP_Settings_Overlay(CP_Color_Create(0, 0, 255, 128));
	CP_Graphics_DrawRect(170, 30, 40, 40);
	CP_Settings_NoTint();

	// Row 2: an image (red, see-through / half see-through red, green) in
	// white, at full alpha and at 128; a red to blue gradient in green
	CP_Color pixels[4] = { red, CP_Color_Create(0, 0, 0, 0), CP_Color_Create(255, 0, 0, 128), CP_Color_Create(0, 255, 0, 255) };
	CP_Image image = CP_Image_CreateFromData(2, 2, pixels);
	CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_NEAREST);
	CP_Settings_Overlay(WHITE);
	CP_Image_Draw(image, 30, 100, 40, 40, 255);
	CP_Image_Draw(image, 100, 100, 40, 40, 128);
	CP_Image_Free(&image);
	CP_Settings_FillLinearGradient(150, 0, red, 190, 0, CP_Color_Create(0, 0, 255, 255));
	CP_Settings_Overlay(CP_Color_Create(0, 255, 0, 255));
	CP_Graphics_DrawRect(170, 100, 40, 40);

	// Row 3: red text in white; then, with no overlay, a red square
	CP_Settings_Fill(red);
	CP_Settings_Overlay(WHITE);
	CP_Settings_TextSize(60.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);
	CP_Font_DrawText("H", 10, 190);
	CP_Settings_NoOverlay();
	CP_Graphics_DrawRect(170, 170, 40, 40);
	CP_Settings_Restore();
}

// A 2x2 image: red, green / blue, yellow
static CP_Image CreateFourColorImage(void)
{
	CP_Color pixels[4] = {
		CP_Color_Create(255, 0, 0, 255),
		CP_Color_Create(0, 255, 0, 255),
		CP_Color_Create(0, 0, 255, 255),
		CP_Color_Create(255, 255, 0, 255),
	};
	return CP_Image_CreateFromData(2, 2, pixels);
}

// ---- Gradient fills (CP_Settings_FillLinearGradient, FillRadialGradient) ----

// Red at x = 20 to blue at x = 180, over a rect that covers the canvas: the
// ends keep their colors past the gradient's two points
static void Scn_GradientLinear(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_FillLinearGradient(20, 0, RED, 180, 0, BLUE);
	CP_Graphics_DrawRect(100, 100, 200, 200);
}

// Green inside a 40-wide circle at the middle, blue outside a 160-wide one
static void Scn_GradientRadial(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_FillRadialGradient(100, 100, 40, GREEN, 160, BLUE);
	CP_Graphics_DrawRect(100, 100, 200, 200);
}

// The gradient's numbers are placed when the shape is drawn, so they turn
// and move with it: red at local x = 0 to blue at local x = 40, set before
// the Translate and Rotate. Turned 90 degrees, local x runs down the screen.
static void Scn_GradientTurned(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_FillLinearGradient(0, 0, RED, 40, 0, BLUE);
	CP_Settings_Translate(100, 100);
	CP_Settings_Rotate(90);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Graphics_DrawRect(0, -20, 40, 40); // on screen: x 80 to 120, y 100 to 140
}

// Fill replaces a gradient, and Save and Restore bring back whichever fill
// there was: the left half is green (the gradient was undone by Restore),
// the right half is the gradient again (Fill was undone by Restore)
static void Scn_GradientFillSaveRestore(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);

	CP_Settings_Fill(GREEN);
	CP_Settings_Save();
	CP_Settings_FillLinearGradient(0, 0, RED, 200, 0, BLUE);
	CP_Settings_Restore();
	CP_Graphics_DrawRect(0, 0, 100, 200);

	CP_Settings_FillLinearGradient(0, 0, RED, 200, 0, BLUE);
	CP_Settings_Save();
	CP_Settings_Fill(GREEN);
	CP_Settings_Restore();
	CP_Graphics_DrawRect(100, 0, 100, 200);
}

// Text and outlines don't take a gradient: text is the gradient's first
// color, and GetFill gives it; the outline keeps the stroke color
static void Scn_GradientTextAndStroke(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_FillLinearGradient(0, 0, RED, 200, 0, BLUE);
	const CP_Color fill = CP_Settings_GetFill();
	tier2_scalars.gradientGetFillIsFirstColor = fill.r == 255 && fill.g == 0 && fill.b == 0 && fill.a == 255;

	CP_Settings_Stroke(BLACK);
	CP_Settings_StrokeWeight(8.0f);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Graphics_DrawRect(10, 10, 40, 40);

	CP_Font_Set(CP_Font_GetDefault());
	CP_Settings_TextSize(120.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Font_DrawText("I", 100, 120); // in the middle, where the gradient is purple
}

// Red fading to transparent black, over white: blended with premultiplied
// alpha the middle is a light pink, with no dark band
static void Scn_GradientFadeToTransparent(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_FillLinearGradient(0, 0, RED, 200, 0, CP_Color_Create(0, 0, 0, 0));
	CP_Graphics_DrawRect(100, 100, 200, 200);
}

// ---- Image fills (CP_Settings_FillImage) ----
// The four color image (red, green / blue, yellow) as 40x40 tiles, so each of
// its pixels is a 20x20 square: the color at (x, y) is picked by whether
// (x - tile x) / 20 and (y - tile y) / 20 are even or odd. Nearest filtering
// keeps the squares sharp.

// Top half: tiles from (0, 0) across a 200x100 rect, with the default wrap
// mode (clamp to edge, which doesn't repeat an image drawn on its own).
// Then strips of the same width with the tiles moved: a square to the right
// (y 100 to 150, tiles from (20, 100)), and whole tiles to the left
// (y 150 to 200, tiles from (-400, 150)), which looks the same as from 0.
static void Scn_ImageFillTiles(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_NEAREST);
	CP_Image image = CreateFourColorImage();

	CP_Settings_FillImage(image, 0, 0, 40, 40, 255);
	CP_Graphics_DrawRect(0, 0, 200, 100);
	CP_Settings_FillImage(image, 20, 100, 40, 40, 255);
	CP_Graphics_DrawRect(0, 100, 200, 50);
	CP_Settings_FillImage(image, -400, 150, 40, 40, 255);
	CP_Graphics_DrawRect(0, 150, 200, 50);

	CP_Image_Free(&image);
}

// Only the shape is filled. Top left: a circle 80 wide at (50, 50). Top
// right: a square from (110, 10) to (190, 90) with a square hole from
// (130, 30) to (170, 70). Bottom left: rects that fill nothing, with a NULL
// image (y 100 to 150) and a tile of no size (y 150 to 200).
static void Scn_ImageFillShapes(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_NEAREST);
	CP_Image image = CreateFourColorImage();
	CP_Settings_FillImage(image, 0, 0, 40, 40, 255);
	CP_Graphics_DrawCircle(50, 50, 80);

	CP_Graphics_BeginShape();
	CP_Graphics_AddVertex(110, 10);
	CP_Graphics_AddVertex(190, 10);
	CP_Graphics_AddVertex(190, 90);
	CP_Graphics_AddVertex(110, 90);
	CP_Graphics_BeginHole();
	CP_Graphics_AddVertex(130, 30);
	CP_Graphics_AddVertex(130, 70);
	CP_Graphics_AddVertex(170, 70);
	CP_Graphics_AddVertex(170, 30);
	CP_Graphics_EndHole();
	CP_Graphics_EndShape();

	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_FillImage(NULL, 0, 0, 40, 40, 255);
	CP_Graphics_DrawRect(0, 100, 100, 50);
	CP_Settings_FillImage(image, 0, 0, 0, 40, 255);
	CP_Graphics_DrawRect(0, 150, 100, 50);
	CP_Image_Free(&image);
}

// A fill set to an image that is then freed. CP_Image_Free deletes the image
// at the end of the frame, so it's the next frame that fills with an image
// that's gone: it fills nothing, and doesn't crash.
static void Scn_ImageFillFreed(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Image image = CreateFourColorImage();
	CP_Settings_FillImage(image, 0, 0, 40, 40, 255);
	CP_Image_Free(&image);
}

static void Scn_ImageFillFreedNextFrame(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Graphics_DrawRect(0, 0, 200, 200);
}

// Left: the tiles at half alpha over white (0 to 100, all the way down).
// Right: the tiles follow the transform like the shape's numbers do: 20x20
// tiles from (0, 0), with (0, 0) moved to (100, 0) and everything twice the
// size, so on screen the tiles are 40x40 from (100, 0) and each image pixel
// is a 20x20 square again.
static void Scn_ImageFillTransformed(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_NEAREST);
	CP_Image image = CreateFourColorImage();

	CP_Settings_FillImage(image, 0, 0, 40, 40, 128);
	CP_Graphics_DrawRect(0, 0, 100, 200);

	CP_Settings_FillImage(image, 0, 0, 20, 20, 255);
	CP_Settings_Translate(100, 0);
	CP_Settings_Scale(2, 2);
	CP_Graphics_DrawRect(0, 0, 50, 100);

	CP_Image_Free(&image);
}

// Top left: Fill was set, then FillImage between Save and Restore: green.
// Top right: FillImage, then Fill between Save and Restore: the tiles, from
// (0, 0). Bottom left: text drawn with FillImage set, after Fill(BLUE): blue,
// and GetFill gives blue. Bottom right: the image drawn as a sub-image twice
// its size right after a tiled shape, with the default wrap mode (clamp to
// edge): the fill's repeat doesn't stay, so past the image's edge are its
// edge pixels stretched out, not the image again.
static void Scn_ImageFillSettings(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_NEAREST);
	CP_Image image = CreateFourColorImage();

	CP_Settings_Fill(GREEN);
	CP_Settings_Save();
	CP_Settings_FillImage(image, 0, 0, 40, 40, 255);
	CP_Settings_Restore();
	CP_Graphics_DrawRect(0, 0, 100, 100);

	CP_Settings_FillImage(image, 0, 0, 40, 40, 255);
	CP_Settings_Save();
	CP_Settings_Fill(GREEN);
	CP_Settings_Restore();
	CP_Graphics_DrawRect(100, 0, 100, 100);

	CP_Settings_Fill(BLUE);
	CP_Settings_FillImage(image, 0, 0, 40, 40, 255);
	const CP_Color fill = CP_Settings_GetFill();
	tier2_scalars.imageFillGetFillKept = fill.r == 0 && fill.g == 0 && fill.b == 255 && fill.a == 255;
	CP_Font_Set(CP_Font_GetDefault());
	CP_Settings_TextSize(120.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Font_DrawText("I", 50, 150);

	CP_Graphics_DrawRect(100, 100, 10, 10); // a tiled shape, then the sub-image
	CP_Image_DrawSubImage(image, 110, 110, 80, 80, 0, 0, 4, 4, 255);

	CP_Image_Free(&image);
}

// Drawing an image used to leave NanoVG's fill set to the image, so a shape
// drawn after it, with no CP_Settings_Fill in between, showed the image (or
// nothing) and text came out white. Both are red now.
static void Scn_FillAfterImage(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_NoStroke();
	CP_Settings_Fill(RED);
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_Image image = CreateFourColorImage();
	CP_Image_Draw(image, 0, 0, 40, 40, 255);

	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Graphics_DrawRect(120, 20, 60, 60);

	CP_Image_Draw(image, 0, 100, 40, 40, 255);
	CP_Image_Free(&image);
	CP_Font_Set(CP_Font_GetDefault());
	CP_Settings_TextSize(120.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Font_DrawText("I", 140, 150);
}


// ---- Clip (CP_Settings_Clip, CP_Settings_NoClip) ----
// Each draws a red rect over the whole canvas after setting a clip, so the
// red shows exactly where the clip lets drawing through.

static void FillCanvasRed(void)
{
	CP_Settings_NoStroke();
	CP_Settings_Fill(RED);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Graphics_DrawRect(0, 0, TIER2_CANVAS_SIZE, TIER2_CANVAS_SIZE);
}

// CORNER: the clip spans [50,150] x [50,150]
static void Scn_ClipCorner(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Clip(50, 50, 100, 100);
	FillCanvasRed();
}

// CENTER (the baseline): the clip spans [70,130] x [70,130]
static void Scn_ClipCenter(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_Clip(100, 100, 60, 60);
	FillCanvasRed();
}

// A negative width and height go left and up from (150, 150), as a
// rectangle does: [50,150] x [50,150] again
static void Scn_ClipNegativeSize(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Clip(150, 150, -100, -100);
	FillCanvasRed();
}

// The clip, [50,150] x [50,150], stays where it was set while the drawing
// moves 100 to the right under it: red only in [100,150]
static void Scn_ClipStaysPut(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Clip(50, 50, 100, 100);
	CP_Settings_Translate(100, 0);
	FillCanvasRed();
}

// A second clip, the bottom right quarter, replaces the first, the top left
// quarter (an intersection would leave nothing). Then NoClip, and a blue
// square in the top right, outside both.
static void Scn_ClipReplaceAndNoClip(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Clip(0, 0, 100, 100);
	CP_Settings_Clip(100, 100, 100, 100);
	FillCanvasRed();
	CP_Settings_NoClip();
	CP_Settings_Fill(BLUE);
	CP_Graphics_DrawRect(150, 0, 50, 50);
}

// Clip to the left half, Save, clip to the right half, Restore: the left
// half again
static void Scn_ClipSaveRestore(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Clip(0, 0, 100, 200);
	CP_Settings_Save();
	CP_Settings_Clip(100, 0, 100, 200);
	CP_Settings_Restore();
	FillCanvasRed();
}

// A 100x100 clip set after Translate(100, 100) and Rotate(45): a diamond
// around the middle, its corners 70.7 from it, straight up, down, left and
// right. (145, 145) is inside the square unturned, outside the diamond.
static void Scn_ClipTurned(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_Translate(100, 100);
	CP_Settings_Rotate(45);
	CP_Settings_Clip(0, 0, 100, 100);
	CP_Settings_ResetMatrix();
	FillCanvasRed();
}

// The clip, the left half, cuts images and text too: the four color image
// across the top, and black "HH" across the bottom
static void Scn_ClipTextAndImage(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Clip(0, 0, 100, 200);
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_Image image = CreateFourColorImage();
	CP_Image_Draw(image, 0, 0, 200, 100, 255);
	CP_Image_Free(&image);
	CP_Font_Set(CP_Font_GetDefault());
	CP_Settings_Fill(BLACK);
	CP_Settings_TextSize(100.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Font_DrawText("HH", 100, 150);
}

// ClearBackground inside a clip still clears the whole window, and the clip
// is still set after it
static void Scn_ClipClearBackground(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Clip(0, 0, 100, 200);
	CP_Graphics_ClearBackground(BLUE);
	FillCanvasRed();
}

// With the right half clipped, a 50x50 image is drawn into: there's no clip
// inside, so the image is all red. After EndDrawing the clip is back: the
// image shows on the right and not on the left.
static void Scn_ClipImageDrawing(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Clip(100, 0, 100, 200);
	CP_Image image = CP_Image_Create(50, 50);
	CP_Image_BeginDrawing(image);
	FillCanvasRed();
	CP_Image_EndDrawing();
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_Image_Draw(image, 120, 20, 50, 50, 255);
	CP_Image_Draw(image, 20, 20, 50, 50, 255);
	CP_Image_Free(&image);
}

// Leaves a clip, the top left 20x20, set at the end of the frame...
static void Scn_ClipLeftSet(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Clip(0, 0, 20, 20);
	FillCanvasRed();
}

// ...and the next frame draws everywhere
static void Scn_ClipGoneNextFrame(void)
{
	CP_Graphics_ClearBackground(WHITE);
	FillCanvasRed();
}


// The image drawn 100x100, nearest on the left half of the canvas and
// linear on the right. Nearest keeps four solid colors; linear blends them
// in the middle.
static void Scn_ImageFilterModes(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_Image image = CreateFourColorImage();
	CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_NEAREST);
	CP_Image_Draw(image, 0, 50, 100, 100, 255);
	CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_LINEAR);
	CP_Image_Draw(image, 100, 50, 100, 100, 255);
	CP_Image_Free(&image);
}

// The image drawn once per wrap mode as an 80x80 tile, with a source
// rectangle twice the image's size: the image fills the tile's top-left
// 40x40, and the rest shows what the wrap mode does past the image's edge.
// Tiles: clamp (10, 10), clamp to edge (110, 10), repeat (10, 110),
// mirror (110, 110).
static void Scn_ImageWrapModes(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_NEAREST);
	CP_Image image = CreateFourColorImage();
	const CP_IMAGE_WRAP_MODE modes[4] = {
		CP_IMAGE_WRAP_CLAMP, CP_IMAGE_WRAP_CLAMP_EDGE, CP_IMAGE_WRAP_REPEAT, CP_IMAGE_WRAP_MIRROR
	};
	for (int i = 0; i < 4; ++i)
	{
		CP_Settings_ImageWrapMode(modes[i]);
		CP_Image_DrawSubImage(image, 10.0f + 100.0f * (i % 2), 10.0f + 100.0f * (i / 2), 80, 80, 0, 0, 4, 4, 255);
	}
	CP_Image_Free(&image);
	CP_Settings_ImageWrapMode(CP_IMAGE_WRAP_CLAMP_EDGE);
}

// Settings carry over to the next frame, as in Processing (NanoVG resets
// them every frame unless patched). One frame sets the text size; the next
// draws a capital H without setting it. At size 150, Exo 2's H is 103.5
// pixels tall; at NanoVG's reset size of 16 it would be 11.
static void Scn_SettingsLastFrameSetsTextSize(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_TextSize(150.0f);
}

static void Scn_SettingsCarryToNextFrame(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_Fill(BLACK);
	CP_Font_Set(CP_Font_GetDefault());
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);
	CP_Font_DrawText("H", 20, 150);
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
// Test fixture: assets/quadrants.png, a 4x4 image with four distinct,
// solid-colored 2x2 quadrants (TL=red, TR=green, BL=blue, BR=yellow) --
// see the Phase E commit message for how it was generated. A tiny
// synthetic fixture with exact, known pixel values is more testable than
// a photo, and keeps this project's test assets independent of the
// template's assets folder.

// The quadrants image (top left red, top right green, bottom left blue,
// bottom right yellow) turned 90 degrees clockwise about its center, which
// moves each quadrant one corner clockwise
static void Scn_ImageDrawRotated(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_NEAREST);
	CP_Settings_ImageMode(CP_POSITION_CENTER);

	CP_Image img = CP_Image_Load("assets/quadrants.png");
	CP_Settings_Save();
	CP_Settings_Translate(100, 100);
	CP_Settings_Rotate(90);
	CP_Image_Draw(img, 0, 0, 80, 80, 255);
	CP_Settings_Restore();
	CP_Image_Free(&img);
}

static void Scn_ImageLoadAndDraw(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_NEAREST); // crisp quadrant edges, no bilinear bleed
	CP_Settings_ImageMode(CP_POSITION_CENTER);

	CP_Image img = CP_Image_Load("assets/quadrants.png");
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
	CP_Image synthetic = CP_Image_CreateFromData(2, 2, source);
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

	CP_Image img = CP_Image_Load("assets/quadrants.png");
	// s0/t0/s1/t1 are *pixel* coordinates into the source image, not
	// normalized [0,1] UVs (CP_Image.c:102-145, confirmed against the
	// wiki's Image page). (2,0)-(4,2) selects just the top-right (green)
	// quadrant of the 4x4 fixture; drawn as a 60x60 square centered on the
	// canvas.
	CP_Image_DrawSubImage(img, 100, 100, 60, 60, 2, 0, 4, 2, 255);
	CP_Image_Free(&img);
}

// A PNG with 16 bits per channel, as paint programs often save: the same
// 4x4 quadrants as quadrants.png. stb_image before 2.11 couldn't load
// these at all (CProcessing had 2.10 until 3.0).
static void Scn_Image16Bit(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_NEAREST);
	CP_Settings_ImageMode(CP_POSITION_CENTER);
	CP_Image img = CP_Image_Load("assets/quadrants16.png");
	tier2_scalars.image16BitWidth = CP_Image_GetWidth(img);
	CP_Image_Draw(img, 100, 100, 80, 80, 255);
	CP_Image_Free(&img);
}

// More images than CProcessing's lists start out holding (12), so they have
// to grow. GCC and Clang builds used to write past the old list when one
// grew, which the ASan CI job reports. Then load a file, which looks through
// every image already in the list, and free them all in the same frame.
#define MANY_IMAGES 40

static void Scn_ImageMany(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_NEAREST);
	CP_Settings_ImageMode(CP_POSITION_CORNER);

	CP_Color pixels[4] = { BLUE, BLUE, BLUE, BLUE };
	CP_Image images[MANY_IMAGES];
	for (int i = 0; i < MANY_IMAGES; ++i)
	{
		images[i] = CP_Image_CreateFromData(2, 2, pixels);
		tier2_scalars.manyImagesCreated += images[i] != NULL;
		CP_Image_Draw(images[i], (float)(i % 8) * 20, (float)(i / 8) * 20, 10, 10, 255);
	}

	CP_Image fromFile = CP_Image_Load("assets/quadrants.png");
	tier2_scalars.manyImagesFileWidth = CP_Image_GetWidth(fromFile);
	CP_Image_Free(&fromFile);

	for (int i = 0; i < MANY_IMAGES; ++i)
	{
		CP_Image_Free(&images[i]);
		tier2_scalars.manyImagesFreed += images[i] == NULL;
	}
}

// Loading a file that's already loaded returns the same handle. Freeing one
// of those loads must leave the file loaded for the other: before, the
// first CP_Image_Free freed it for both. Loading the file again after that
// free must therefore give back the handle still in use, not a new copy.
static void Scn_SharedAssets(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_Settings_ImageFilterMode(CP_IMAGE_FILTER_NEAREST);

	CP_Image first = CP_Image_Load("assets/quadrants.png");
	CP_Image second = CP_Image_Load("assets/quadrants.png");
	tier2_scalars.sharedImageSame = first != NULL && first == second;
	CP_Image_Free(&first);
	tier2_scalars.sharedImageFirstCleared = first == NULL;
	CP_Image third = CP_Image_Load("assets/quadrants.png");
	tier2_scalars.sharedImageReloadSame = third != NULL && third == second;
	CP_Image_Draw(second, 0, 0, 200, 200, 255);

	// Images made from data have no path, so an empty path can't find them
	CP_Color pixels[4] = { RED, RED, RED, RED };
	CP_Image fromData = CP_Image_CreateFromData(2, 2, pixels);
	BeginMessages();
	tier2_scalars.emptyPathLoadIsNull = CP_Image_Load("") == NULL;
	EndMessages(tier2_scalars.messageEmptyPath, sizeof(tier2_scalars.messageEmptyPath));
	CP_Image_Free(&fromData);
	CP_Image_Free(&second);
	CP_Image_Free(&third);

	CP_Font font1 = CP_Font_Load("assets/Exo2-Regular.otf");
	CP_Font font2 = CP_Font_Load("assets/Exo2-Regular.otf");
	tier2_scalars.sharedFontSame = font1 != NULL && font1 == font2;
	CP_Font_Free(&font1);
	CP_Font font3 = CP_Font_Load("assets/Exo2-Regular.otf");
	tier2_scalars.sharedFontReloadSame = font3 != NULL && font3 == font2;
	CP_Font_Free(&font2);
	CP_Font_Free(&font3);

	// Without an audio device sounds don't load, and the test is skipped
	CP_Sound sound1 = CP_Sound_Load("assets/beep.wav");
	tier2_scalars.soundAvailable = sound1 != NULL;
	CP_Sound stream = CP_Sound_LoadStream("assets/beep.wav");
	tier2_scalars.soundAndStreamDiffer = sound1 != NULL && stream != NULL && stream != sound1;
	CP_Sound sound2 = CP_Sound_Load("assets/beep.wav");
	CP_Sound_Free(&sound1);
	CP_Sound sound3 = CP_Sound_Load("assets/beep.wav");
	tier2_scalars.sharedSoundReloadSame = sound3 != NULL && sound3 == sound2;
	CP_Sound_Free(&sound2);
	CP_Sound_Free(&sound3);
	CP_Sound_Free(&stream);
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
	CP_Font customFont = CP_Font_Load("assets/Exo2-Regular.ttf");
	CP_Font_Set(customFont);
	CP_Settings_TextSize(60.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
	CP_Font_DrawText("I", 40, 40);
	CP_Font_Free(&customFont);
}

// ---- OpenType fonts with CFF outlines (v3) ----
// A .otf font whose glyphs are CFF outlines: the official OpenType build
// of Exo 2, the default font. stb_truetype before 1.13 couldn't load these
// (CProcessing had 1.09 until 3.0). Same metrics as the .ttf, so its
// capital H at size 100 is 69 pixels tall too.
static void Scn_FontOpenType(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_Fill(BLACK);
	CP_Font font = CP_Font_Load("assets/Exo2-Regular.otf");
	tier2_scalars.openTypeFontLoaded = font != NULL;
	CP_Font_Set(font);
	CP_Settings_TextSize(100.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);
	CP_Font_DrawText("H", 20, 150);
	CP_Font_Free(&font);
	CP_Font_Set(CP_Font_GetDefault());
}

// ---- Freeing a font keeps the others working (v3) ----
// A font's handle is its index in NanoVG's font list, and CP_Font_Free used
// to remove a font by moving the ones after it down. Text in a font loaded
// after the freed one then drew in the wrong font, or not at all. Three
// fonts (the same file under three names): draw with the second and third,
// free the first, draw the same again, and compare. Then a font loaded
// after the free must work too; it is what the scenario captures.

static CP_Color fontFreeBefore[TIER2_CANVAS_SIZE * TIER2_CANVAS_SIZE];
static CP_Color fontFreeAfter[TIER2_CANVAS_SIZE * TIER2_CANVAS_SIZE];

static void DrawWithTwoFonts(CP_Font upper, CP_Font lower)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Font_Set(upper);
	CP_Font_DrawText("Hello", 100, 50);
	CP_Font_Set(lower);
	CP_Font_DrawText("Hello", 100, 150);
}

static void Scn_FontFreeKeepsOthers(void)
{
	CP_Settings_Fill(BLACK);
	CP_Settings_TextSize(40.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Font first = CP_Font_Load("assets/Exo2-Regular.ttf");
	CP_Font second = CP_Font_Load("assets/./Exo2-Regular.ttf");
	CP_Font third = CP_Font_Load("assets/../assets/Exo2-Regular.ttf");
	tier2_scalars.fontFreeLoaded = (first != NULL) + (second != NULL) + (third != NULL);

	DrawWithTwoFonts(second, third);
	CaptureInto(fontFreeBefore);
	CP_Font_Free(&first);
	DrawWithTwoFonts(second, third);
	CaptureInto(fontFreeAfter);

	for (int i = 0; i < TIER2_CANVAS_SIZE * TIER2_CANVAS_SIZE; ++i)
	{
		tier2_scalars.fontFreeInkBefore += fontFreeBefore[i].r < 128;
		tier2_scalars.fontFreeMismatched += (fontFreeBefore[i].r < 128) != (fontFreeAfter[i].r < 128);
	}

	CP_Font later = CP_Font_Load("assets/./../assets/Exo2-Regular.ttf");
	CP_Graphics_ClearBackground(WHITE);
	CP_Font_Set(later);
	CP_Font_DrawText("Hello", 100, 100);

	CP_Font_Free(&second);
	CP_Font_Free(&third);
	CP_Font_Free(&later);
	CP_Font_Set(CP_Font_GetDefault());
}

// ---- Text animated by size or by scale (v3) ----
// Students often change the text size, or the scale the text is drawn at,
// every frame: a sine wave from 10 to 100, say. Every new size needs new
// glyph bitmaps in NanoVG's glyph atlas, so the atlas fills up every few
// seconds and NanoVG moves on to a fresh atlas texture, usually partway
// through a string. That frame's text used to vanish, be cut off, or show
// garbage.
//
// Each step below stands in for one such frame: draw the text at the next
// size and capture it, then draw exactly the same thing again and capture
// that. A screenshot ends the NanoVG frame, as the end of a real frame does.
// The redraw is always right, since every glyph it needs is already in the
// current atlas, so the first draw must match it. Comparing two draws from
// the same run keeps this independent of how a given GPU renders glyphs.

#define SWEEP_FRAMES            240
// Many different glyphs, so the atlas fills quickly
#define SWEEP_TEXT              "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
// A channel that differs by more than this is a real difference, not
// filtering noise.
#define SWEEP_CHANNEL_TOLERANCE 64
// Frames almost always match exactly, but on a real GPU about one run in
// twenty had a single pixel off, on a frame with no atlas switch. A broken
// frame loses whole glyphs: typically thousands of pixels, and at least 16
// (the "A" alone at size 10).
#define SWEEP_STRAY_PIXELS      4

static CP_Color sweepDraw[TIER2_CANVAS_SIZE * TIER2_CANVAS_SIZE];
static CP_Color sweepRedraw[TIER2_CANVAS_SIZE * TIER2_CANVAS_SIZE];

static void DrawSweepText(float textSize, float scale)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_ResetMatrix();
	CP_Settings_Translate(4, 4);
	CP_Settings_Scale(scale, scale);
	CP_Settings_TextSize(textSize);
	CP_Font_DrawText(SWEEP_TEXT, 0, 0);
}

static void RunTextSweep(Tier2TextSweep* result, bool animateScale)
{
	CP_Settings_Fill(BLACK);
	CP_Font_Set(CP_Font_GetDefault());
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);

	result->frames = SWEEP_FRAMES;
	result->firstBadFrame = -1;
	for (int frame = 0; frame < SWEEP_FRAMES; ++frame)
	{
		// 0 to 1 and back, about three times over the sweep. Either way the
		// text ends up 10 to 100 pixels tall.
		float wave = 0.5f - 0.5f * cosf(frame * 0.08f);
		float textSize = animateScale ? 25.0f : 10.0f + 90.0f * wave;
		float scale = animateScale ? 0.4f + 3.6f * wave : 1.0f;

		DrawSweepText(textSize, scale);
		CaptureInto(sweepDraw);
		DrawSweepText(textSize, scale);
		CaptureInto(sweepRedraw);

		int differing = 0;
		bool inked = false;
		for (int i = 0; i < TIER2_CANVAS_SIZE * TIER2_CANVAS_SIZE; ++i)
		{
			if (abs(sweepDraw[i].r - sweepRedraw[i].r) > SWEEP_CHANNEL_TOLERANCE || abs(sweepDraw[i].g - sweepRedraw[i].g) > SWEEP_CHANNEL_TOLERANCE || abs(sweepDraw[i].b - sweepRedraw[i].b) > SWEEP_CHANNEL_TOLERANCE)
			{
				++differing;
			}
			inked = inked || sweepRedraw[i].r < 128;
		}

		if (differing > result->worstPixels)
		{
			result->worstPixels = differing;
		}
		if (differing > SWEEP_STRAY_PIXELS)
		{
			++result->badFrames;
			if (result->firstBadFrame < 0)
			{
				result->firstBadFrame = frame;
			}
		}
		if (!inked)
		{
			++result->blankFrames;
		}
	}
}

static void Scn_FontSizeSweep(void)
{
	RunTextSweep(&tier2_scalars.textSizeSweep, false);
}

static void Scn_FontScaleSweep(void)
{
	RunTextSweep(&tier2_scalars.textScaleSweep, true);
}

// ---- Text size is the em size (v3) ----
// CP_Settings_TextSize sets the font's em size in pixels, as Processing and
// CSS do (NanoVG upstream 69e1a47). Exo 2's capital H is 690 units tall in
// a 1000-unit em, so at size 100 it is 69 pixels tall. Before v3 the size
// was the font's whole height, ascender to descender (1200 units), which
// made the H 57.5 pixels tall.
static void Scn_FontEmSize(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_Fill(BLACK);
	CP_Font_Set(CP_Font_GetDefault());
	CP_Settings_TextSize(100.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);
	CP_Font_DrawText("H", 20, 150);

	// Measuring text, at the same size
	tier2_scalars.textWidthH = CP_Font_GetTextWidth("H");
	tier2_scalars.textWidthHH = CP_Font_GetTextWidth("HH");
	tier2_scalars.textWidthHSpace = CP_Font_GetTextWidth("H ");
	tier2_scalars.textSizeH = CP_Font_GetTextSize("H");
	tier2_scalars.textSizeLong = CP_Font_GetTextSize("Hello, world");
	tier2_scalars.textBoxOneLine = CP_Font_GetTextBoxSize("Hi", 1000.0f);
	tier2_scalars.textBoxWrapped = CP_Font_GetTextBoxSize("one two three four five six", 300.0f);
	tier2_scalars.textWidthNull = CP_Font_GetTextWidth(NULL);
	CP_Settings_TextSize(200.0f);
	tier2_scalars.textWidthH200 = CP_Font_GetTextWidth("H");
	CP_Settings_TextSize(100.0f);
}

// ---- Large text (v3) ----
// Fontstash rasterizes each glyph in a fixed scratch buffer. On 32-bit
// builds the 2018 NanoVG's buffer ran out for glyphs of a few hundred
// pixels: text at size 600 tripped an assert in stb_truetype in Debug
// builds and drew nothing in Release (fixed by the NanoVG update). An @ at
// size 600, centered, so its middle strokes fall on the canvas. This only
// fails on 32-bit (x86) builds; 64-bit ones always had room.
static void Scn_FontLarge(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_Fill(BLACK);
	CP_Font_Set(CP_Font_GetDefault());
	CP_Settings_TextSize(600.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Font_DrawText("@", 100, 100);
}

// ---- Mirrored and flipped text (v3) ----
// NanoVG culls back faces, and a mirroring transform turns each glyph's
// quad around, so text drawn with CP_Settings_Scale(-1, 1), the usual way
// to flip a sprite, or (1, -1) vanished (fixed in NanoVG upstream 621e0b8).
// Three bands, each centered on its own line: the text as is (y = 33),
// mirrored about x = 100 (y = 100), and flipped about y = 167.
static void DrawTextTransformed(float y, float scaleX, float scaleY)
{
	CP_Settings_ResetMatrix();
	CP_Settings_Translate(100, y);
	CP_Settings_Scale(scaleX, scaleY);
	CP_Font_DrawText("Hello", 0, 0);
}

static void Scn_FontMirrored(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_Fill(BLACK);
	CP_Font_Set(CP_Font_GetDefault());
	CP_Settings_TextSize(40.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	DrawTextTransformed(33, 1, 1);
	DrawTextTransformed(100, -1, 1);
	DrawTextTransformed(167, 1, -1);
	CP_Settings_ResetMatrix();
}

// ---- Text effects ----
// The letter I at size 60, near the top left of each 100x100 quarter of the
// canvas, so the quarters can be compared pixel for pixel.
static void SetUpTextQuarters(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_Fill(BLACK);
	CP_Font_Set(CP_Font_GetDefault());
	CP_Settings_TextSize(60.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
}

static void DrawIInQuarter(int quarter, float blur)
{
	CP_Settings_TextBlur(blur);
	CP_Font_DrawText("I", 30.0f + (float)(quarter % 2) * 100.0f, 20.0f + (float)(quarter / 2) * 100.0f);
}

// Top left sharp; top right blurred by 8, which Restore brought back; bottom
// left a blur below 0, which is sharp
static void Scn_TextBlur(void)
{
	SetUpTextQuarters();
	DrawIInQuarter(0, 0.0f);
	CP_Settings_TextBlur(8.0f);
	CP_Settings_Save();
	CP_Settings_TextBlur(0.0f);
	CP_Settings_Restore();
	CP_Font_DrawText("I", 130.0f, 20.0f);
	DrawIInQuarter(2, -5.0f);
}

// The blur rounds to whole steps (a CProcessing patch to fontstash, which
// cuts off the fraction): 2.8 draws like 3, and 2.2 like 2. Chosen so the
// pairs also match at a pixel ratio of 2 (5.6 and 6, 4.4 and 4).
static void Scn_TextBlurRounds(void)
{
	SetUpTextQuarters();
	DrawIInQuarter(0, 2.8f);
	DrawIInQuarter(1, 3.0f);
	DrawIInQuarter(2, 2.2f);
	DrawIInQuarter(3, 2.0f);
}

// "HHHH" plain at the top and with a letter spacing of 10 (brought back by
// Restore) in the middle; "HH" centered on x = 100 with a spacing of 30 at
// the bottom
static void Scn_TextLetterSpacing(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_Fill(BLACK);
	CP_Font_Set(CP_Font_GetDefault());
	CP_Settings_TextSize(40.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
	tier2_scalars.letterWidthPlain = CP_Font_GetTextWidth("HHHH");
	CP_Font_DrawText("HHHH", 10.0f, 10.0f);

	CP_Settings_TextLetterSpacing(10.0f);
	CP_Settings_Save();
	CP_Settings_TextLetterSpacing(0.0f);
	CP_Settings_Restore();
	tier2_scalars.letterWidthSpaced = CP_Font_GetTextWidth("HHHH");
	tier2_scalars.letterSizeSpaced = CP_Font_GetTextSize("HHHH").x;
	CP_Font_DrawText("HHHH", 10.0f, 70.0f);

	CP_Settings_TextLetterSpacing(-5.0f);
	tier2_scalars.letterWidthTight = CP_Font_GetTextWidth("HHHH");

	CP_Settings_TextLetterSpacing(30.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_TOP);
	CP_Font_DrawText("HH", 100.0f, 130.0f);
}

// Three lines (a box too narrow for more than one I) plain on the left, and
// with a line spacing of 20 (brought back by Restore) on the right. NanoVG
// keeps the line height it was last given, so each box is measured or drawn
// right after one with the other spacing (set directly: Restore would bring
// NanoVG's back too), so each has to give NanoVG its own.
static void Scn_TextLineSpacing(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Settings_Fill(BLACK);
	CP_Font_Set(CP_Font_GetDefault());
	CP_Settings_TextSize(30.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
	tier2_scalars.lineBoxPlain = CP_Font_GetTextBoxSize("I I I", 1.0f);
	tier2_scalars.lineBoxOneLinePlain = CP_Font_GetTextBoxSize("I", 1000.0f);
	tier2_scalars.lineTextHeightPlain = CP_Font_GetTextSize("I").y;
	CP_Font_DrawTextBox("I I I", 30.0f, 10.0f, 1.0f);

	CP_Settings_TextLineSpacing(20.0f);
	CP_Settings_Save();
	CP_Settings_TextLineSpacing(0.0f);
	CP_Settings_Restore();
	tier2_scalars.lineBoxSpaced = CP_Font_GetTextBoxSize("I I I", 1.0f);
	tier2_scalars.lineBoxOneLineSpaced = CP_Font_GetTextBoxSize("I", 1000.0f);
	tier2_scalars.lineTextHeightSpaced = CP_Font_GetTextSize("I").y;
	CP_Settings_TextLineSpacing(0.0f);
	CP_Font_GetTextBoxSize("I I I", 1.0f);
	CP_Settings_TextLineSpacing(20.0f);
	CP_Font_DrawTextBox("I I I", 130.0f, 10.0f, 1.0f);
}

// ---- CP_System / CP_Engine (Phase E) ----
// These read state via the public getters rather than touching _CORE
// directly, and must run *during* the single CP_Engine_Run() call: once it
// returns, CP_Shutdown() has already torn down the GLFW window/context, and
// the window queries (GetWindowFocus, GetDisplayRefreshRate, ...) only
// report "no window".

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
}

// A frame rate of 0 or less means no limit. 1 / 0 used to make the frame
// limiter wait forever, so the scenarios after this one would never be
// captured. ResetToBaseline puts 60 back for the next one.
static void Scn_SystemFrameRateNoLimit(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_System_SetFrameRate(0.0f);
	CP_System_SetFrameRate(-30.0f);
	tier2_scalars.frameRateNoLimitSet = true;
}

// A frame that takes 0.3 s, like one after a breakpoint or a window drag.
// The next frame's dt is capped at 0.1 s, so things moved by speed * dt don't
// jump; CP_System_GetSeconds still tells the real time.
static void Scn_SystemStall(void)
{
	CP_Graphics_ClearBackground(WHITE);
	tier2_scalars.secondsBeforeStall = CP_System_GetSeconds();
	while (CP_System_GetSeconds() - tier2_scalars.secondsBeforeStall < 0.3f)
	{
		// wait without any operating system header
	}
}

static void Scn_SystemDtAfterStall(void)
{
	CP_Graphics_ClearBackground(WHITE);
	tier2_scalars.dtAfterStall = CP_System_GetDt();
	tier2_scalars.secondsAfterStall = CP_System_GetSeconds();
}

// ---- CP_Sound (Phase E) ----
// Per 06-test-suite-plan.md: no reliable way to assert "it sounds
// correct" automatically, so the realistic goal is load/free/play/pause/
// stop not crashing or leaking, plus group volume/pitch getters
// round-tripping what was set.

static void Scn_SoundRoundTrip(void)
{
	CP_Graphics_ClearBackground(WHITE);

	CP_Sound sound = CP_Sound_Load("assets/beep.wav");

	CP_Sound_SetGroupVolume(CP_SOUND_GROUP_SFX, 0.3f);
	tier2_scalars.volumeAfterSet = CP_Sound_GetGroupVolume(CP_SOUND_GROUP_SFX);
	CP_Sound_SetGroupPitch(CP_SOUND_GROUP_SFX, 1.5f);
	tier2_scalars.pitchAfterSet = CP_Sound_GetGroupPitch(CP_SOUND_GROUP_SFX);

	// One playing sound, by its instance: two looping copies, stop one
	tier2_scalars.soundLoaded = sound != NULL;
	CP_SoundInstance first = CP_Sound_PlayAdvanced(sound, 0.0f, 1.0f, true, CP_SOUND_GROUP_SFX);
	CP_SoundInstance second = CP_Sound_PlayAdvanced(sound, 0.0f, 1.0f, true, CP_SOUND_GROUP_SFX);
	tier2_scalars.instanceGiven = first != 0 && second != 0 && first != second;
	tier2_scalars.playingAfterPlay = CP_Sound_IsPlaying(first);
	CP_Sound_Pause(first);
	CP_Sound_Resume(first);
	CP_Sound_SetVolume(first, 0.0f);
	CP_Sound_SetPitch(first, 1.0f);
	tier2_scalars.playingAfterPauseResume = CP_Sound_IsPlaying(first);
	CP_Sound_Stop(first);
	tier2_scalars.playingAfterStop = CP_Sound_IsPlaying(first);
	tier2_scalars.otherPlayingAfterStop = CP_Sound_IsPlaying(second);
	CP_Sound_Stop(second);
	CP_Sound_Stop(first); // stopping a sound that has stopped is fine
	CP_Sound_Pause(0);
	CP_Sound_Resume(0);
	CP_Sound_Stop(0);
	CP_Sound_SetVolume(0, 1.0f);
	CP_Sound_SetPitch(0, 1.0f);
	tier2_scalars.noInstanceIsNotPlaying = !CP_Sound_IsPlaying(0);

	CP_Sound_PlayAdvanced(sound, 0.1f, 1.0f, false, CP_SOUND_GROUP_SFX);
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
// out into src/Internal_InputLogic.h together with the XInput -> GLFW
// gamepad migration, and is unit-tested directly in the Tier 1 project
// (tests/tier1/test_cp_input_logic.c).
//-------------------------
// The game state stack. The harness (the bottom state) draws red and pushes
// state A, which runs for a few frames while the harness is frozen (the
// default pass-through):
//   frame 0  A draws a half-black veil and a green square over the frozen
//            red harness (captured as this scenario), and pushes B, which
//            lets CP_PASSTHROUGH_ALL through
//   frame 1  A runs under B, without input; B pops
//   frame 2  A has its own settings back, and pushes C, which lets
//            CP_PASSTHROUGH_NONE through
//   frame 3  C draws nothing: everything under it is hidden (captured), pops
//   frame 4  A pushes F, which lets CP_PASSTHROUGH_ALL through
//   frame 5  A, running under F, pops: F and A both leave, and the harness
//            carries on in the next frame
static bool stackAwaitingResume = false;
static int stackAFrame = 0;
static const CP_Color kStackHarnessFill = { 10, 20, 30, 255 };
static const CP_Color kStackAFill = { 0, 0, 255, 255 };
CP_Color tier2_stackHiddenPixels[TIER2_CANVAS_SIZE * TIER2_CANVAS_SIZE];

static bool SameColor(CP_Color a, CP_Color b)
{
	return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

static void StackLog(const char* what)
{
	size_t used = strlen(tier2_scalars.stackLog);
	snprintf(tier2_scalars.stackLog + used, sizeof(tier2_scalars.stackLog) - used, "%s%s", used ? " " : "", what);
}

static void StackB_Init(void)
{
	StackLog("B.init");
	CP_Engine_SetGameStatePassThrough(CP_PASSTHROUGH_ALL);
	tier2_scalars.stackPushInheritsSettings = SameColor(CP_Settings_GetFill(), kStackAFill);
}
static void StackB_Update(void)
{
	CP_Settings_Fill(CP_Color_Create(255, 255, 0, 255)); // must not leak back into A
	tier2_scalars.stackRunningStateIsTop = CP_Engine_IsTopGameState();
	CP_Engine_PopGameState();
}
static void StackB_Exit(void) { StackLog("B.exit"); }

static void StackC_Init(void)
{
	StackLog("C.init");
	CP_Engine_SetGameStatePassThrough(CP_PASSTHROUGH_NONE);
}
static void StackC_Update(void)
{
	CaptureInto(tier2_stackHiddenPixels);
	CP_Engine_PopGameState();
}
static void StackC_Exit(void) { StackLog("C.exit"); }

static void StackF_Init(void)
{
	StackLog("F.init");
	CP_Engine_SetGameStatePassThrough(CP_PASSTHROUGH_ALL);
}
static void StackF_Update(void) {}
static void StackF_Exit(void) { StackLog("F.exit"); }

static void StackA_Init(void)
{
	StackLog("A.init");
	tier2_scalars.stackDepthInPushedInit = CP_Engine_GetGameStateDepth();
	stackAFrame = 0;
}
static void StackA_Update(void)
{
	++tier2_scalars.stackUpdatesOfFirstPushed;
	if (!CP_Engine_IsTopGameState())
	{
		// running under B
		++tier2_scalars.stackUpdatesWhileRunningBelow;
		tier2_scalars.stackRunningBelowIsNotTop = true;
		tier2_scalars.stackRunningBelowKeyMuted = !CP_Input_KeyDown(CP_KEY_ANY) && !CP_Input_MouseDown(CP_MOUSE_LEFT);
		tier2_scalars.stackRunningBelowMouseX = CP_Input_GetMouseX();
		if (tier2_scalars.stackRunningBelowMouseX != tier2_scalars.stackMouseXWhenCovered)
		{
			tier2_scalars.stackRunningBelowMouseMoved = true;
		}
		if (stackAFrame == 1)
		{
			tier2_scalars.stackRunningBelowWorldMouseX = CP_Input_GetMouseWorldX();
		}
		if (stackAFrame == 3)
		{
			// under F: takes F off too
			CP_Engine_PopGameState();
		}
		return;
	}
	switch (stackAFrame++)
	{
	case 0:
		// The pops before the push were refused as this frame began
		EndMessages(tier2_scalars.messagePopLast, sizeof(tier2_scalars.messagePopLast));
		CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 128));
		CP_Settings_NoStroke();
		CP_Settings_Fill(CP_Color_Create(0, 255, 0, 255));
		CP_Graphics_DrawRect(100.0f, 100.0f, 50.0f, 50.0f);
		CaptureCurrentFrame(SCN_GAMESTATE_STACK);
		tier2_scalars.stackPushedIsTop = CP_Engine_IsTopGameState();
		tier2_scalars.stackPushedTime = CP_Engine_GetGameStateTime();
		CP_Settings_Fill(kStackAFill);
		tier2_scalars.stackMouseXWhenCovered = CP_Input_GetMouseX();
		// A camera moved when the push is asked for: the world mouse A sees
		// under B is the one it saw here (the push happens next frame, when
		// nothing is moved yet)
		CP_Settings_Translate(37.0f, 0.0f);
		tier2_scalars.stackMouseXWhenBCovered = CP_Input_GetMouseX();
		tier2_scalars.stackWorldMouseXWhenCovered = CP_Input_GetMouseWorldX();
		CP_Engine_PushGameState(StackB_Init, StackB_Update, StackB_Exit);
		CP_Settings_ResetMatrix();
		break;
	case 1:
		tier2_scalars.stackSettingsBackAfterPop = SameColor(CP_Settings_GetFill(), kStackAFill);
		CP_Engine_PushGameState(StackC_Init, StackC_Update, StackC_Exit);
		break;
	case 2:
		// The real mouse may have moved since B covered A: A now keeps this one
		tier2_scalars.stackMouseXWhenCovered = CP_Input_GetMouseX();
		CP_Engine_PushGameState(StackF_Init, StackF_Update, StackF_Exit);
		break;
	default:
		break;
	}
}
static void StackA_Exit(void) { StackLog("A.exit"); }

static void Scn_GameStateStack(void)
{
	CP_Graphics_ClearBackground(RED);
	CP_Settings_Fill(kStackHarnessFill);
	// Only one state: both pops are refused when they're applied, as the next
	// frame begins, with one message (StackA_Update's first frame reads it)
	BeginMessages();
	CP_Engine_PopGameState();
	CP_Engine_PopGameState();
	CP_Engine_PushGameState(StackA_Init, StackA_Update, StackA_Exit);
	stackAwaitingResume = true;
}

//-------------------------
// A picture of one state drawn in the next, as examples/transitions.c does
// for its crossfades and slides. The harness draws red and pushes D, which
// clears its own layer to green and takes a screenshot: the whole picture,
// which is D's green. D pushes E, which clears to blue and draws the
// screenshot over it at half alpha (captured as this scenario), then pops,
// and D pops in the frame after.
//
// E's init also takes a screenshot. A pushed state's layer starts clear, so
// that one is the picture below it (D's green), which is how a pause blurs
// the game under it (examples/state_effects.c).
static bool snapshotAwaitingResume = false;
static int snapshotDFrame = 0;
static CP_Image snapshotImage = NULL;
static CP_Color snapshotBelowPixels[TIER2_CANVAS_SIZE * TIER2_CANVAS_SIZE];

static void SnapshotE_Init(void)
{
	CaptureInto(snapshotBelowPixels);
	tier2_scalars.snapshotInPushedInitCenter = snapshotBelowPixels[100 * TIER2_CANVAS_SIZE + 100];
	tier2_scalars.snapshotInPushedInitCorner = snapshotBelowPixels[190 * TIER2_CANVAS_SIZE + 10];
}

static void SnapshotE_Update(void)
{
	CP_Graphics_ClearBackground(BLUE);
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_Image_Draw(snapshotImage, 0.0f, 0.0f, (float)TIER2_CANVAS_SIZE, (float)TIER2_CANVAS_SIZE, 128);
	CaptureCurrentFrame(SCN_GAMESTATE_SNAPSHOT);
	CP_Image_Free(&snapshotImage);
	CP_Engine_PopGameState();
}

static void SnapshotD_Init(void) { snapshotDFrame = 0; }
static void SnapshotD_Update(void)
{
	switch (snapshotDFrame++)
	{
	case 0:
		CP_Graphics_ClearBackground(CP_Color_Create(0, 255, 0, 255));
		snapshotImage = CP_Image_Screenshot(0, 0, TIER2_CANVAS_SIZE, TIER2_CANVAS_SIZE);
		tier2_scalars.snapshotTaken = snapshotImage != NULL;
		CP_Engine_PushGameState(SnapshotE_Init, SnapshotE_Update, NULL);
		break;
	case 1:
		CP_Engine_PopGameState();
		break;
	default:
		break;
	}
}

static void Scn_GameStateSnapshot(void)
{
	CP_Graphics_ClearBackground(RED);
	CP_Engine_PushGameState(SnapshotD_Init, SnapshotD_Update, NULL);
	snapshotAwaitingResume = true;
}

// A clip and game states. The harness pushes ClipBelow, which pushes
// ClipAbove; ClipAbove lets ClipBelow keep running under it. In one frame
// ClipBelow updates first and leaves a clip set, the left 20 pixels, and then
// ClipAbove draws a red canvas with no clip of its own (captured): it isn't
// cut, because each state starts drawing unclipped. Then both pop.
static int clipBelowFrame = 0;
static unsigned clipBelowFrameCount = 0;

static void ClipAbove_Init(void)
{
	CP_Engine_SetGameStatePassThrough(CP_PASSTHROUGH_ALL);
}

static void ClipAbove_Update(void)
{
	tier2_scalars.clipStateBelowRanFirst = clipBelowFrameCount == CP_System_GetFrameCount();
	CP_Graphics_ClearBackground(WHITE);
	FillCanvasRed();
	CaptureCurrentFrame(SCN_CLIP_NEW_STATE);
	CP_Engine_PopGameState();
}

static void ClipBelow_Init(void) { clipBelowFrame = 0; }
static void ClipBelow_Update(void)
{
	switch (clipBelowFrame++)
	{
	case 0:
		CP_Graphics_ClearBackground(WHITE);
		CP_Engine_PushGameState(ClipAbove_Init, ClipAbove_Update, NULL);
		break;
	case 1:
		CP_Settings_RectMode(CP_POSITION_CORNER);
		CP_Settings_Clip(0, 0, 20, TIER2_CANVAS_SIZE);
		clipBelowFrameCount = CP_System_GetFrameCount();
		break;
	case 2:
		CP_Engine_PopGameState();
		break;
	default:
		break;
	}
}

static void Scn_ClipNewState(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Engine_PushGameState(ClipBelow_Init, ClipBelow_Update, NULL);
}

// Drawing into an image: a 50x50 image gets a red square in its top left
// corner and a half-transparent blue one in its bottom right, then is drawn
// over white at 4 times its size (captured)
static void Scn_ImageDrawInto(void)
{
	CP_Graphics_ClearBackground(WHITE);
	CP_Image image = CP_Image_Create(50, 50);
	tier2_scalars.drawIntoCreated = image != NULL && CP_Image_GetWidth(image) == 50;

	CP_Settings_Fill(kStackHarnessFill);
	CP_Image_BeginDrawing(image);
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(RED);
	CP_Graphics_DrawRect(0.0f, 0.0f, 25.0f, 25.0f);
	CP_Settings_Fill(CP_Color_Create(0, 0, 255, 128));
	CP_Graphics_DrawRect(25.0f, 25.0f, 25.0f, 25.0f);
	CP_Image_EndDrawing();
	tier2_scalars.drawIntoSettingsBack = SameColor(CP_Settings_GetFill(), kStackHarnessFill);

	CP_Settings_ImageMode(CP_POSITION_CENTER); // (not part of the baseline)
	CP_Image_Draw(image, 100.0f, 100.0f, 200.0f, 200.0f, 255);

	CP_Color pixels[50 * 50];
	CP_Image_GetPixelData(image, pixels);
	tier2_scalars.drawIntoTopLeft = pixels[5 * 50 + 5];
	tier2_scalars.drawIntoBottomRight = pixels[45 * 50 + 45];

	pixels[0] = CP_Color_Create(10, 200, 30, 255);
	CP_Image_UpdatePixelData(image, pixels);
	CP_Color again[50 * 50];
	CP_Image_GetPixelData(image, again);
	tier2_scalars.drawIntoAfterUpdate = again[0];
	CP_Image_Free(&image);

	CP_Image loaded = CP_Image_Load("assets/quadrants16.png");
	BeginMessages();
	CP_Image_BeginDrawing(loaded); // refused, with a message
	EndMessages(tier2_scalars.messageDrawIntoLoaded, sizeof(tier2_scalars.messageDrawIntoLoaded));
	tier2_scalars.drawIntoLoadedRefused = true;
	CP_Image_EndDrawing(); // nothing to end: harmless
	CP_Image_Free(&loaded);
}

// CP_Image_Save, into the folder the tests run in, and each file loaded back
// and compared pixel by pixel. The files are deleted at the end.
static bool SameColor4(CP_Color a, CP_Color b)
{
	return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

static bool FileExists(const char* path)
{
	FILE* file = fopen(path, "rb");
	if (file)
	{
		fclose(file);
	}
	return file != NULL;
}

// Saves the image, loads the file back, and compares every pixel
static bool SavesAndLoadsBack(CP_Image image, const char* path)
{
	const int w = CP_Image_GetWidth(image);
	const int h = CP_Image_GetHeight(image);
	if (!CP_Image_Save(image, path))
	{
		return false;
	}
	CP_Image loaded = CP_Image_Load(path);
	if (!loaded || CP_Image_GetWidth(loaded) != w || CP_Image_GetHeight(loaded) != h)
	{
		CP_Image_Free(&loaded);
		return false;
	}
	CP_Color* before = (CP_Color*)malloc(sizeof(CP_Color) * (size_t)w * (size_t)h);
	CP_Color* after = (CP_Color*)malloc(sizeof(CP_Color) * (size_t)w * (size_t)h);
	bool same = before != NULL && after != NULL;
	if (same)
	{
		CP_Image_GetPixelData(image, before);
		CP_Image_GetPixelData(loaded, after);
		for (int i = 0; i < w * h && same; ++i)
		{
			same = SameColor4(before[i], after[i]);
		}
	}
	free(before);
	free(after);
	CP_Image_Free(&loaded);
	return same;
}

static void Scn_ImageSave(void)
{
	CP_Graphics_ClearBackground(WHITE);

	// An image made from data, with see-through pixels, including one that
	// is invisible but still has a color (alpha 0 isn't turned into black)
	CP_Color data[4 * 3] = {
		CP_Color_Create(255, 0, 0, 255),
		CP_Color_Create(0, 255, 0, 255),
		CP_Color_Create(0, 0, 255, 255),
		CP_Color_Create(255, 255, 255, 255),
		CP_Color_Create(10, 20, 30, 0),
		CP_Color_Create(200, 100, 50, 128),
		CP_Color_Create(1, 2, 3, 4),
		CP_Color_Create(0, 0, 0, 255),
		CP_Color_Create(128, 128, 128, 77),
		CP_Color_Create(250, 240, 230, 220),
		CP_Color_Create(5, 255, 5, 1),
		CP_Color_Create(60, 70, 80, 90),
	};
	CP_Image image = CP_Image_CreateFromData(4, 3, data);
	tier2_scalars.saveReturnedTrue = CP_Image_Save(image, "tier2_saved.png");
	FILE* file = fopen("tier2_saved.png", "rb");
	if (file)
	{
		unsigned char signature[8] = { 0 };
		const unsigned char png[8] = { 0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n' };
		tier2_scalars.saveFileIsPNG = fread(signature, 1, 8, file) == 8 && memcmp(signature, png, 8) == 0;
		fclose(file);
	}
	tier2_scalars.saveRoundTrip = SavesAndLoadsBack(image, "tier2_saved_again.png");
	tier2_scalars.saveCapitalPNG = CP_Image_Save(image, "tier2_saved_capital.PNG");

	// Saves that must fail, each with a message, and without making a file
	int refusals = 0;
	BeginMessages();
	refusals += !CP_Image_Save(NULL, "tier2_saved_null.png");
	refusals += !CP_Image_Save(image, NULL);
	refusals += !CP_Image_Save(image, "");
	refusals += !CP_Image_Save(image, "tier2_saved.jpg");
	refusals += !CP_Image_Save(image, "tier2_no_such_folder/saved.png");
	EndMessages(tier2_scalars.messageSaveRefusals, sizeof(tier2_scalars.messageSaveRefusals));
	tier2_scalars.saveRefusals = refusals;
	tier2_scalars.saveRefusedLeftNoFile = !FileExists("tier2_saved.jpg") && !FileExists("tier2_saved_null.png");
	CP_Image_Free(&image);

	// An image drawn into (kept upside down, with premultiplied alpha): red
	// along the top, half-transparent blue along the bottom
	CP_Image drawn = CP_Image_Create(20, 20);
	CP_Image_BeginDrawing(drawn);
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(RED);
	CP_Graphics_DrawRect(0.0f, 0.0f, 20.0f, 8.0f);
	CP_Settings_Fill(CP_Color_Create(0, 0, 255, 128));
	CP_Graphics_DrawRect(0.0f, 12.0f, 20.0f, 8.0f);
	CP_Image_EndDrawing();
	tier2_scalars.saveDrawnRoundTrip = SavesAndLoadsBack(drawn, "tier2_saved_drawn.png");
	CP_Image_Free(&drawn);
	CP_Image reloaded = CP_Image_Load("tier2_saved_drawn.png");
	if (reloaded)
	{
		CP_Color pixels[20 * 20];
		CP_Image_GetPixelData(reloaded, pixels);
		tier2_scalars.saveDrawnTopRowFirst = SameColor4(pixels[2 * 20 + 10], RED) && pixels[17 * 20 + 10].b > 200 && pixels[17 * 20 + 10].r == 0;
		CP_Image_Free(&reloaded);
	}

	// A screenshot of what this frame has drawn so far
	CP_Settings_Fill(GREEN);
	CP_Graphics_DrawRect(0.0f, 0.0f, 100.0f, 100.0f);
	CP_Image shot = CP_Image_Screenshot(0, 0, 100, 100);
	tier2_scalars.saveScreenshotRoundTrip = SavesAndLoadsBack(shot, "tier2_saved_screenshot.png");
	CP_Image_Free(&shot);

	const char* files[] = { "tier2_saved.png", "tier2_saved_again.png", "tier2_saved_capital.PNG",
		"tier2_saved_drawn.png", "tier2_saved_screenshot.png", "tier2_saved.jpg",
		"tier2_saved_null.png" };
	for (size_t i = 0; i < sizeof(files) / sizeof(files[0]); ++i)
	{
		remove(files[i]);
	}
}

// Pushed on the next-to-last frame, running, so the harness keeps going:
// still on the stack when the program ends, its exit must run
// pushed with no init, so it says what it lets through in its update: the
// harness under it misses one frame, then keeps running
static void ShutdownProbe_Update(void) { CP_Engine_SetGameStatePassThrough(CP_PASSTHROUGH_ALL); }
static void ShutdownProbe_Exit(void) { tier2_scalars.stackShutdownExitRan = true; }

static void Scn_InputQuiescentDefaults(void)
{
	CP_Graphics_ClearBackground(WHITE);

	// Gamepads may or may not be plugged in, so capture what holds either
	// way: the default is index 0, indexes are filled from 0 with no gaps,
	// and an index past the last is never connected
	tier2_scalars.gamepadConnected = CP_Input_GamepadConnected();
	tier2_scalars.gamepad0ConnectedAdvanced = CP_Input_GamepadConnectedAdvanced(0);
	tier2_scalars.gamepadIndexesHaveNoGaps = true;
	for (unsigned i = 1; i < CP_MAX_GAMEPADS; ++i)
	{
		if (CP_Input_GamepadConnectedAdvanced(i) && !CP_Input_GamepadConnectedAdvanced(i - 1))
		{
			tier2_scalars.gamepadIndexesHaveNoGaps = false;
		}
	}
	BeginMessages(); // an off-by-one gamepad number says so
	tier2_scalars.gamepadPastLastConnected = CP_Input_GamepadConnectedAdvanced(CP_MAX_GAMEPADS);
	EndMessages(tier2_scalars.messageGamepadPastLast, sizeof(tier2_scalars.messageGamepadPastLast));
	tier2_scalars.mouseWheel = CP_Input_MouseWheel();
	tier2_scalars.mouseDoubleClicked = CP_Input_MouseDoubleClicked();
	tier2_scalars.keyADown = CP_Input_KeyDown(CP_KEY_A);
	tier2_scalars.keyTriggered = CP_Input_GetKeyTriggered();
	tier2_scalars.keyNoneEverOn = CP_Input_KeyDown(CP_KEY_NONE) || CP_Input_KeyTriggered(CP_KEY_NONE) || CP_Input_KeyReleased(CP_KEY_NONE);
	tier2_scalars.mouseLeftDown = CP_Input_MouseDown(CP_MOUSE_LEFT);
	tier2_scalars.typedTextIsEmpty = CP_Input_GetTypedText() != NULL && CP_Input_GetTypedText()[0] == '\0';
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
// Missing files and NULL handles must give NULL or do nothing rather than
// crash -- including when there is no audio device at all, which is the
// normal case on CI runners -- and each says why, once (DESIGN.md,
// "Messages"; the image load's empty path was said in Scn_SharedAssets).

static void Scn_ErrorPaths(void)
{
	CP_Graphics_ClearBackground(WHITE);

	// CI runners may have no audio device, and then no sound loads at all,
	// and no sound message is printed here (one was, as the program started)
	CP_Sound probe = CP_Sound_Load("assets/beep.wav");
	tier2_scalars.errorPathsSoundAvailable = probe != NULL;
	CP_Sound_Free(&probe);

	BeginMessages();
	CP_Image missingImage = CP_Image_Load("assets/this_file_does_not_exist.png");
	tier2_scalars.missingImageIsNull = missingImage == NULL;
	CP_Font missingFont = CP_Font_Load("assets/this_file_does_not_exist.ttf");
	tier2_scalars.missingFontIsNull = missingFont == NULL;
	CP_Sound missingSound = CP_Sound_Load("assets/this_file_does_not_exist.wav");
	tier2_scalars.missingSoundIsNull = missingSound == NULL;
	// a NULL path is treated like a missing file
	tier2_scalars.missingImageIsNull = tier2_scalars.missingImageIsNull && CP_Image_Load(NULL) == NULL;
	tier2_scalars.missingFontIsNull = tier2_scalars.missingFontIsNull && CP_Font_Load(NULL) == NULL;
	tier2_scalars.missingSoundIsNull = tier2_scalars.missingSoundIsNull && CP_Sound_Load(NULL) == NULL && CP_Sound_LoadStream(NULL) == NULL;

	// None of these may crash
	CP_Image nullImage = NULL;
	CP_Sound nullSound = NULL;
	CP_Font nullFont = NULL;
	CP_Image_Draw(NULL, 10, 10, 10, 10, 255);
	CP_Image_Free(NULL);
	CP_Image_Free(&nullImage);
	CP_Sound_Play(NULL);
	CP_Sound_PlayAdvanced(NULL, 1.0f, 1.0f, false, CP_SOUND_GROUP_SFX);
	CP_Sound_Free(NULL);
	CP_Sound_Free(&nullSound);
	CP_Font_Set(NULL);
	CP_Font_Free(NULL);
	CP_Font_Free(&nullFont);
	tier2_scalars.survivedNullCalls = true;

	// Said once: the same missing file, and the same NULL call, again
	CP_Image missingAgain = CP_Image_Load("assets/this_file_does_not_exist.png");
	CP_Image_Draw(NULL, 10, 10, 10, 10, 255);
	CP_Font_Set(NULL);
	CP_Sound_Play(NULL);
	tier2_scalars.missingImageIsNull = tier2_scalars.missingImageIsNull && missingAgain == NULL;

	// Files that are there, but aren't the kind asked for
	CP_Image notAnImage = CP_Image_Load("assets/beep.wav");
	CP_Font notAFont = CP_Font_Load("assets/quadrants.png");
	CP_Sound notASound = CP_Sound_Load("assets/quadrants.png");
	tier2_scalars.unreadableFilesAreNull = notAnImage == NULL && notAFont == NULL && notASound == NULL;
	EndMessages(tier2_scalars.messageErrorPaths, sizeof(tier2_scalars.messageErrorPaths));

	// A file loaded 1000 times without a free is probably loaded on every
	// frame: said once, on the 1000th load
	BeginMessages();
	static CP_Image loads[1001];
	for (int i = 0; i < 1001; ++i)
	{
		loads[i] = CP_Image_Load("assets/quadrants.png");
	}
	for (int i = 0; i < 1001; ++i)
	{
		CP_Image_Free(&loads[i]);
	}
	EndMessages(tier2_scalars.messageLoadedOften, sizeof(tier2_scalars.messageLoadedOften));
}

// ---- Cursor and console (v3) ----
// The cursor was hidden before CP_Engine_Run (tier2_RunCaptureOnce), and
// HarnessInit checked that it still was once the window existed. Here the
// getters must follow each ShowCursor and SetCursorMode call. (What each mode
// does to the real cursor is checked by hand.)
//
// The console can only be checked loosely: whether there is one to show
// depends on how the tests were started (from a terminal, an IDE, Explorer
// or a CI runner), and a console shared with a terminal is never hidden.
// So: hiding then showing again must end where showing started, and printf
// must keep working throughout. The visible behavior is checked by hand
// (C in examples/window_options.c).
static void Scn_SystemWindowSettings(void)
{
	CP_Graphics_ClearBackground(WHITE);

	CP_System_ShowCursor(true);
	tier2_scalars.cursorVisibleAfterShow = CP_System_GetCursorVisible();
	CP_System_ShowCursor(false);
	CP_System_ShowCursor(false); // repeating a call is harmless
	tier2_scalars.cursorVisibleAfterHide = CP_System_GetCursorVisible();

	CP_System_SetCursorMode(CP_CURSOR_DISABLED);
	tier2_scalars.cursorModeAfterDisable = CP_System_GetCursorMode();
	tier2_scalars.cursorVisibleWhileDisabled = CP_System_GetCursorVisible();
	CP_System_SetCursorMode(CP_CURSOR_CAPTURED);
	tier2_scalars.cursorModeAfterCapture = CP_System_GetCursorMode();
	tier2_scalars.cursorVisibleWhileCaptured = CP_System_GetCursorVisible();
	CP_System_SetCursorMode((CP_CURSOR_MODE)99); // not a mode: ignored
	tier2_scalars.cursorModeAfterInvalid = CP_System_GetCursorMode();
	CP_System_ShowCursor(true);
	tier2_scalars.cursorModeAfterShow = CP_System_GetCursorMode();

	CP_System_ShowConsole(false);
	CP_System_ShowConsole(false);
	tier2_scalars.consoleVisibleAfterHide = CP_System_GetConsoleVisible();
	tier2_scalars.printfAfterHideConsole = printf("Tier 2: printf while the console is hidden\n");
	CP_System_ShowConsole(true); // leave it visible for the test results
	tier2_scalars.consoleVisibleAfterReshow = CP_System_GetConsoleVisible();
}

// ---- assets next to the program (3.0.2) ----
// A program double-clicked in macOS Finder starts in the home folder, not in
// its own folder, so relative asset paths are also looked for next to the
// program. This moves to an empty folder, where "assets/..." doesn't exist,
// loads one file of each kind, and moves back. The paths start with "./"
// because the other scenarios loaded these files as "assets/...", and loading
// a path that's already loaded returns that file without opening anything.
#define TIER2_EMPTY_FOLDER "tier2_empty_folder"

static void Scn_AssetsNextToProgram(void)
{
	CP_Graphics_ClearBackground(WHITE);

	// CI runners may have no audio device, and then no sound loads at all
	CP_Sound probe = CP_Sound_Load("assets/beep.wav");
	tier2_scalars.assetSoundAvailable = probe != NULL;
	CP_Sound_Free(&probe);

	char previousFolder[4096];
	if (tier2_getcwd(previousFolder, sizeof previousFolder) == NULL)
	{
		return;
	}
	tier2_mkdir(TIER2_EMPTY_FOLDER); // it may be left over from an interrupted run
	tier2_scalars.assetFolderEntered = tier2_chdir(TIER2_EMPTY_FOLDER) == 0;

	CP_Image image = CP_Image_Load("./assets/quadrants.png");
	CP_Font font = CP_Font_Load("./assets/Exo2-Regular.ttf");
	CP_Sound sound = CP_Sound_Load("./assets/beep.wav");
	tier2_scalars.assetImageLoaded = image != NULL;
	tier2_scalars.assetFontLoaded = font != NULL;
	tier2_scalars.assetSoundLoaded = sound != NULL;

	tier2_chdir(previousFolder);
	tier2_rmdir(TIER2_EMPTY_FOLDER);
	CP_Image_Free(&image);
	CP_Font_Free(&font);
	CP_Sound_Free(&sound);
}

// ---- Messages about drawing and settings (DESIGN.md, "Messages") ----
// A NaN or infinite number is ignored and said once per function (a NaN
// translate would otherwise stop everything after it from drawing), as are
// a mode that isn't a mode, an image of size 0, a Restore without a Save and
// text that is NULL. The square drawn last, after the NaN translate, must
// still be in the middle. Last, a Save is left open, so the end of the frame
// says so; the next scenario reads that and closes it.
static void Scn_DrawingMessages(void)
{
	CP_Graphics_ClearBackground(WHITE);
	volatile float zero = 0.0f; // volatile: worked out when the test runs
	const float notANumber = zero / zero;
	const float infinite = 1.0f / zero;

	BeginMessages();
	CP_Graphics_DrawRect(100, notANumber, 50, 50);
	CP_Graphics_DrawRect(100, notANumber, 50, 50); // said once
	CP_Graphics_DrawCircle(infinite, 100, 20);
	CP_Settings_Translate(notANumber, 0.0f);
	CP_Settings_StrokeWeight(notANumber);
	CP_Settings_RectMode((CP_POSITION_MODE)7);
	CP_Settings_LineCapMode((CP_LINE_CAP_MODE)99);
	CP_Settings_BlendMode((CP_BLEND_MODE)99);
	CP_Settings_BlendMode(CP_BLEND_ALPHA); // a real mode: nothing to say
	CP_Image zeroSize = CP_Image_Create(0, 10);
	CP_Settings_Restore(); // no Save before it
	CP_Font_DrawText(NULL, 10, 10);
	EndMessages(tier2_scalars.messageDrawing, sizeof(tier2_scalars.messageDrawing));

	tier2_scalars.drawingZeroSizeImageIsNull = zeroSize == NULL;
	tier2_scalars.drawingStrokeWeightKept = CP_Settings_GetStrokeWeight();
	CP_Settings_NoStroke();
	CP_Settings_Fill(BLUE);
	CP_Graphics_DrawRect(100, 100, 40, 40); // CENTER, from ResetToBaseline

	CP_Settings_Save(); // no Restore this frame
	BeginMessages();
}

static void Scn_DrawingMessagesNextFrame(void)
{
	EndMessages(tier2_scalars.messageSaveOpen, sizeof(tier2_scalars.messageSaveOpen));
	CP_Settings_Restore();
	CP_Graphics_ClearBackground(WHITE);
}

// ---- Messages about input and game states (DESIGN.md, "Messages") ----
// A key, button or gamepad number that isn't one gives false, said once per
// function; lowercase 'a' gets told to use CP_KEY_A. CP_KEY_NONE (a key
// setting not chosen yet) stays quiet. Then the harness asks for the state it
// is already in: nothing happens, and the message comes as the next frame
// applies the request, so the next scenario reads it.
static void HarnessInit(void);
static void HarnessUpdate(void);

static void Scn_InputMessages(void)
{
	CP_Graphics_ClearBackground(WHITE);

	BeginMessages();
	tier2_scalars.inputMistakesAllFalse = !CP_Input_KeyDown((CP_KEY)'a') && !CP_Input_KeyDown((CP_KEY)'a') && !CP_Input_KeyTriggered((CP_KEY)500) && !CP_Input_KeyDown(CP_KEY_NONE) && !CP_Input_MouseDown((CP_MOUSE)99) && !CP_Input_GamepadDown((CP_GAMEPAD)99) && !CP_Input_GamepadDownAdvanced(CP_GAMEPAD_A, 7);
	// Not mistakes: the functions without "Advanced" read the first connected
	// gamepad, and with none connected (as in CI) they quietly give nothing
	(void)CP_Input_GamepadDown(CP_GAMEPAD_A);
	(void)CP_Input_GamepadTriggered(CP_GAMEPAD_A);
	(void)CP_Input_GamepadReleased(CP_GAMEPAD_A);
	(void)CP_Input_GamepadLeftStick();
	(void)CP_Input_GamepadRightStick();
	(void)CP_Input_GamepadLeftTrigger();
	(void)CP_Input_GamepadRightTrigger();
	(void)CP_Input_GamepadConnected();
	EndMessages(tier2_scalars.messageInput, sizeof(tier2_scalars.messageInput));

	tier2_scalars.sameStateAsked = CP_Engine_GetGameStateDepth() == 1;
	if (tier2_scalars.sameStateAsked)
	{
		CP_Engine_SetNextGameState(HarnessInit, HarnessUpdate, NULL);
		BeginMessages();
	}
}

static void Scn_InputMessagesNextFrame(void)
{
	if (tier2_scalars.sameStateAsked)
	{
		EndMessages(tier2_scalars.messageSameState, sizeof(tier2_scalars.messageSameState));
	}
	CP_Graphics_ClearBackground(WHITE);
}

typedef void (*ScenarioFunc)(void);

static const ScenarioFunc kScenarios[SCN_COUNT] = {
	[SCN_SETTINGS_BEFORE_RUN] = Scn_SettingsBeforeRun,
	[SCN_SYSTEM_GETTERS] = Scn_SystemGetters,
	[SCN_SETTINGS_GETTERS] = Scn_SettingsGetters,
	[SCN_CLEAR_BACKGROUND] = Scn_ClearBackground,
	[SCN_DRAW_POINT] = Scn_DrawPoint,
	[SCN_DRAW_LINE] = Scn_DrawLine,
	[SCN_DRAW_LINE_ROTATED] = Scn_DrawLineRotated,
	[SCN_DRAW_RECT] = Scn_DrawRect,
	[SCN_DRAW_RECT_ROTATED] = Scn_DrawRectRotated,
	[SCN_DRAW_ROUNDED_RECT] = Scn_DrawRoundedRect,
	[SCN_DRAW_CIRCLE] = Scn_DrawCircle,
	[SCN_DRAW_ELLIPSE] = Scn_DrawEllipse,
	[SCN_DRAW_ELLIPSE_ROTATED] = Scn_DrawEllipseRotated,
	[SCN_DRAW_ARC] = Scn_DrawArc,
	[SCN_DRAW_ARC_COUNTERCLOCKWISE] = Scn_DrawArcCounterclockwise,
	[SCN_DRAW_PIE] = Scn_DrawPie,
	[SCN_DRAW_PIE_CORNER_MODE] = Scn_DrawPieCornerMode,
	[SCN_DRAW_BEZIER] = Scn_DrawBezier,
	[SCN_DRAW_TRIANGLE] = Scn_DrawTriangle,
	[SCN_DRAW_TRIANGLE_ROTATED] = Scn_DrawTriangleRotated,
	[SCN_DRAW_QUAD] = Scn_DrawQuad,
	[SCN_DRAW_QUAD_ROTATED] = Scn_DrawQuadRotated,
	[SCN_CUSTOM_SHAPE] = Scn_CustomShape,
	[SCN_SHAPE_OUTLINE_CLOSED] = Scn_ShapeOutlineClosed,
	[SCN_SHAPE_OUTLINE_OPEN] = Scn_ShapeOutlineOpen,
	[SCN_SHAPE_QUADRATIC] = Scn_ShapeQuadratic,
	[SCN_SHAPE_BEZIER] = Scn_ShapeBezier,
	[SCN_SHAPE_CURVE_FIRST] = Scn_ShapeCurveFirst,
	[SCN_SHAPE_CURVE_OPEN] = Scn_ShapeCurveOpen,
	[SCN_SHAPE_HOLE_EITHER_WAY] = Scn_ShapeHoleEitherWay,
	[SCN_SHAPE_HOLE_FIRST] = Scn_ShapeHoleFirst,
	[SCN_SHAPE_HOLES_OUTLINED] = Scn_ShapeHolesOutlined,
	[SCN_SHAPE_OUTLINE_AFTER_HOLE] = Scn_ShapeOutlineAfterHole,
	[SCN_SHAPE_HOLE_OUTSIDE] = Scn_ShapeHoleOutside,
	[SCN_SHAPE_HOLES_OVERLAP] = Scn_ShapeHolesOverlap,
	[SCN_GRADIENT_LINEAR] = Scn_GradientLinear,
	[SCN_GRADIENT_RADIAL] = Scn_GradientRadial,
	[SCN_GRADIENT_TURNED] = Scn_GradientTurned,
	[SCN_GRADIENT_FILL_SAVE_RESTORE] = Scn_GradientFillSaveRestore,
	[SCN_GRADIENT_TEXT_AND_STROKE] = Scn_GradientTextAndStroke,
	[SCN_GRADIENT_FADE_TO_TRANSPARENT] = Scn_GradientFadeToTransparent,
	[SCN_IMAGE_FILL_TILES] = Scn_ImageFillTiles,
	[SCN_IMAGE_FILL_SHAPES] = Scn_ImageFillShapes,
	[SCN_IMAGE_FILL_TRANSFORMED] = Scn_ImageFillTransformed,
	[SCN_IMAGE_FILL_SETTINGS] = Scn_ImageFillSettings,
	[SCN_IMAGE_FILL_FREED] = Scn_ImageFillFreed,
	[SCN_IMAGE_FILL_FREED_NEXT_FRAME] = Scn_ImageFillFreedNextFrame,
	[SCN_FILL_AFTER_IMAGE] = Scn_FillAfterImage,
	[SCN_CLIP_CORNER] = Scn_ClipCorner,
	[SCN_CLIP_CENTER] = Scn_ClipCenter,
	[SCN_CLIP_NEGATIVE_SIZE] = Scn_ClipNegativeSize,
	[SCN_CLIP_STAYS_PUT] = Scn_ClipStaysPut,
	[SCN_CLIP_REPLACE_AND_NOCLIP] = Scn_ClipReplaceAndNoClip,
	[SCN_CLIP_SAVE_RESTORE] = Scn_ClipSaveRestore,
	[SCN_CLIP_TURNED] = Scn_ClipTurned,
	[SCN_CLIP_TEXT_AND_IMAGE] = Scn_ClipTextAndImage,
	[SCN_CLIP_CLEAR_BACKGROUND] = Scn_ClipClearBackground,
	[SCN_CLIP_IMAGE_DRAWING] = Scn_ClipImageDrawing,
	[SCN_CLIP_LEFT_SET] = Scn_ClipLeftSet,
	[SCN_CLIP_GONE_NEXT_FRAME] = Scn_ClipGoneNextFrame,
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
	[SCN_SETTINGS_BLEND_MODES] = Scn_SettingsBlendModes,
	[SCN_SETTINGS_TINT] = Scn_SettingsTint,
	[SCN_SETTINGS_OVERLAY] = Scn_SettingsOverlay,
	[SCN_IMAGE_FILTER_MODES] = Scn_ImageFilterModes,
	[SCN_IMAGE_WRAP_MODES] = Scn_ImageWrapModes,
	[SCN_SETTINGS_LAST_FRAME_SETS_TEXT_SIZE] = Scn_SettingsLastFrameSetsTextSize,
	[SCN_SETTINGS_CARRY_TO_NEXT_FRAME] = Scn_SettingsCarryToNextFrame,
	[SCN_SETTINGS_SAVE_RESTORE] = Scn_SettingsSaveRestore,
	[SCN_IMAGE_LOAD_AND_DRAW] = Scn_ImageLoadAndDraw,
	[SCN_IMAGE_SUBIMAGE] = Scn_ImageSubImage,
	[SCN_IMAGE_DRAW_ROTATED] = Scn_ImageDrawRotated,
	[SCN_IMAGE_16_BIT] = Scn_Image16Bit,
	[SCN_IMAGE_MANY] = Scn_ImageMany,
	[SCN_SHARED_ASSETS] = Scn_SharedAssets,
	[SCN_FONT_DRAWTEXT] = Scn_FontDrawText,
	[SCN_FONT_LOAD_FREE] = Scn_FontLoadFree,
	[SCN_FONT_GET_CURRENT] = Scn_FontGetCurrent,
	[SCN_FONT_FREE_KEEPS_OTHERS] = Scn_FontFreeKeepsOthers,
	[SCN_FONT_OPENTYPE] = Scn_FontOpenType,
	[SCN_FONT_SIZE_SWEEP] = Scn_FontSizeSweep,
	[SCN_FONT_SCALE_SWEEP] = Scn_FontScaleSweep,
	[SCN_FONT_EM_SIZE] = Scn_FontEmSize,
	[SCN_FONT_LARGE] = Scn_FontLarge,
	[SCN_FONT_MIRRORED] = Scn_FontMirrored,
	[SCN_TEXT_BLUR] = Scn_TextBlur,
	[SCN_TEXT_BLUR_ROUNDS] = Scn_TextBlurRounds,
	[SCN_TEXT_LETTER_SPACING] = Scn_TextLetterSpacing,
	[SCN_TEXT_LINE_SPACING] = Scn_TextLineSpacing,
	[SCN_SYSTEM_ENGINE_STATE] = Scn_SystemEngineState,
	[SCN_SYSTEM_FRAME_RATE_NO_LIMIT] = Scn_SystemFrameRateNoLimit,
	[SCN_SYSTEM_STALL] = Scn_SystemStall,
	[SCN_SYSTEM_DT_AFTER_STALL] = Scn_SystemDtAfterStall,
	[SCN_SOUND_ROUNDTRIP] = Scn_SoundRoundTrip,
	[SCN_INPUT_QUIESCENT_DEFAULTS] = Scn_InputQuiescentDefaults,
	[SCN_GAMESTATE_STACK] = Scn_GameStateStack,
	[SCN_GAMESTATE_SNAPSHOT] = Scn_GameStateSnapshot,
	[SCN_CLIP_NEW_STATE] = Scn_ClipNewState,
	[SCN_IMAGE_DRAW_INTO] = Scn_ImageDrawInto,
	[SCN_IMAGE_SAVE] = Scn_ImageSave,
	[SCN_PERSIST_FIRST_FRAME] = Scn_PersistFirstFrame,
	[SCN_PERSIST_NEXT_FRAME] = Scn_PersistNextFrame,
	[SCN_SCREENSHOT_SUBREGION] = Scn_ScreenshotSubregion,
	[SCN_ERROR_PATHS] = Scn_ErrorPaths,
	[SCN_DRAWING_MESSAGES] = Scn_DrawingMessages,
	[SCN_DRAWING_MESSAGES_NEXT_FRAME] = Scn_DrawingMessagesNextFrame,
	[SCN_INPUT_MESSAGES] = Scn_InputMessages,
	[SCN_INPUT_MESSAGES_NEXT_FRAME] = Scn_InputMessagesNextFrame,
	[SCN_SYSTEM_WINDOW_SETTINGS] = Scn_SystemWindowSettings,
	[SCN_ASSETS_NEXT_TO_PROGRAM] = Scn_AssetsNextToProgram,
};

static void HarnessInit(void)
{
	// the window now exists; the cursor was hidden before it did
	tier2_scalars.cursorVisibleAtStart = CP_System_GetCursorVisible();

	CP_System_SetWindowSize(TIER2_CANVAS_SIZE, TIER2_CANVAS_SIZE);
	CP_System_SetWindowTitle("CProcessing Tier 2 Test Capture");
	CP_Engine_SetPreUpdateFunction(PreUpdateHook);
	CP_Engine_SetPostUpdateFunction(PostUpdateHook);
	frameCount = 0;
	firstScenarioFrame = -1;
}

static void LeaveAssetsLoadedForTheExitReport(void);

static void HarnessUpdate(void)
{
	if (stackAwaitingResume)
	{
		// the states pushed in Scn_GameStateStack have all popped
		stackAwaitingResume = false;
		tier2_scalars.stackHarnessSettingsBack = SameColor(CP_Settings_GetFill(), kStackHarnessFill);
		tier2_scalars.stackHarnessDepthAfter = CP_Engine_GetGameStateDepth();
	}
	if (snapshotAwaitingResume)
	{
		snapshotAwaitingResume = false;
		tier2_scalars.snapshotHarnessDepthAfter = CP_Engine_GetGameStateDepth();
	}

	if (frameCount == 0)
	{
		// HarnessInit chose the window size in the same frame
		tier2_scalars.windowWidthInFirstUpdate = CP_System_GetWindowWidth();
	}

	// Let the deferred window resize (requested in HarnessInit) take effect
	// before drawing/capturing anything.
	if (firstScenarioFrame < 0)
	{
		bool sizeSettled = CP_System_GetWindowWidth() == TIER2_CANVAS_SIZE && CP_System_GetWindowHeight() == TIER2_CANVAS_SIZE;
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
		if (scenarioIndex != SCN_SETTINGS_BEFORE_RUN && scenarioIndex != SCN_IMAGE_FILL_FREED_NEXT_FRAME)
		{
			ResetToBaseline();
		}
		kScenarios[scenarioIndex]();
		CaptureCurrentFrame((Tier2Scenario)scenarioIndex);
		++tier2_scalars.scenariosCaptured;
	}

	if (scenarioIndex == SCN_COUNT - 2)
	{
		CP_Engine_PushGameState(NULL, ShutdownProbe_Update, ShutdownProbe_Exit);
	}

	if (scenarioIndex >= SCN_COUNT - 1)
	{
		LeaveAssetsLoadedForTheExitReport();
		CP_Engine_Terminate();
	}
}

// When it shuts down, CProcessing reports the assets that were never freed,
// on stderr. On the last frame, leave two loads of an image and an image made
// from data unfreed, and send stderr to a file until CP_Engine_Run returns
// (test_tier2_assets_still_loaded_are_reported_at_exit reads it).
static FILE* exitReportFile = NULL;
static int savedStderr = -1;

static void LeaveAssetsLoadedForTheExitReport(void)
{
	// (kept in statics only because the loads' results must be kept: they
	// are never freed, on purpose)
	static CP_Image unfreed[3];
	unfreed[0] = CP_Image_Load("assets/quadrants16.png");
	unfreed[1] = CP_Image_Load("assets/quadrants16.png");
	CP_Color pixels[4] = { RED, RED, RED, RED };
	unfreed[2] = CP_Image_CreateFromData(2, 2, pixels);

	fflush(stderr);
	exitReportFile = fopen("tier2_exit_report.txt", "w+");
	if (exitReportFile)
	{
		savedStderr = tier2_dup(tier2_fileno(stderr));
		tier2_dup2(tier2_fileno(exitReportFile), tier2_fileno(stderr));
	}
}

static void ReadExitReport(void)
{
	if (!exitReportFile)
	{
		return;
	}
	fflush(stderr);
	tier2_dup2(savedStderr, tier2_fileno(stderr));
	tier2_close(savedStderr);

	rewind(exitReportFile);
	size_t length = fread(tier2_scalars.exitReport, 1, sizeof(tier2_scalars.exitReport) - 1, exitReportFile);
	tier2_scalars.exitReport[length] = '\0';
	fclose(exitReportFile);
	remove("tier2_exit_report.txt");

	// Still show it in the log
	fputs(tier2_scalars.exitReport, stderr);
}

void tier2_RunCaptureOnce(void)
{
	// Before the window exists: settings are stored for later, queries
	// return "nothing yet", and none of it may crash
	// Vsync and a resizable window, chosen before the window exists
	CP_System_SetVSync(true);
	CP_System_SetWindowResizable(true);
	tier2_scalars.vsyncBeforeRun = CP_System_GetVSync();
	tier2_scalars.resizableBeforeRun = CP_System_GetWindowResizable();
	tier2_scalars.fullscreenBeforeRun = CP_System_IsFullscreen();
	CP_System_ShowCursor(false);
	tier2_scalars.cursorVisibleBeforeRun = CP_System_GetCursorVisible();
	tier2_scalars.cursorModeBeforeRun = CP_System_GetCursorMode();
	CP_System_SetWindowTitle("CProcessing Tier 2 Test Capture");
	tier2_scalars.windowFocusBeforeRun = CP_System_GetWindowFocus();
	tier2_scalars.displayRefreshRateBeforeRun = CP_System_GetDisplayRefreshRate();
	CP_Engine_Terminate(); // nothing to terminate yet: must not stop the run below
	CP_System_ShowConsole(true);
	CP_System_ShowConsole(false);
	CP_System_ShowConsole(true);
	tier2_scalars.consoleVisibleBeforeRun = CP_System_GetConsoleVisible();
	tier2_scalars.printfAfterShowConsole = printf("Tier 2: capturing scenarios\n");

	// Drawing settings made before the window exists are kept and applied
	// when it opens (Scn_SettingsBeforeRun checks them). Transforms,
	// Save/Restore and drawing do nothing yet, and none of it may crash.
	CP_Settings_Fill(CP_Color_Create(12, 200, 34, 255));
	tier2_scalars.fillBeforeRun = CP_Settings_GetFill();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	// (drawing and transforms say they did nothing, once each)
	BeginMessages();
	CP_Settings_Translate(30.0f, 30.0f);
	CP_Settings_Rotate(45.0f);
	CP_Settings_Save();
	CP_Settings_Restore();
	CP_Graphics_ClearBackground(RED);
	CP_Graphics_DrawRect(10.0f, 10.0f, 20.0f, 20.0f);
	CP_Graphics_DrawCircle(10.0f, 10.0f, 20.0f);
	CP_Graphics_BeginShape();
	CP_Graphics_AddVertex(0.0f, 0.0f);
	CP_Graphics_AddVertex(10.0f, 0.0f);
	CP_Graphics_EndShape();
	EndMessages(tier2_scalars.messageDrawBeforeRun, sizeof(tier2_scalars.messageDrawBeforeRun));

	// Loading before the window exists loads nothing, and says so once for
	// each function
	BeginMessages();
	CP_Image image = CP_Image_Load("assets/quadrants.png");
	CP_Image imageAgain = CP_Image_Load("assets/quadrants.png");
	CP_Sound sound = CP_Sound_Load("assets/beep.wav");
	CP_Sound stream = CP_Sound_LoadStream("assets/beep.wav");
	CP_Font font = CP_Font_Load("assets/Exo2-Regular.ttf");
	EndMessages(tier2_scalars.messageLoadBeforeRun, sizeof(tier2_scalars.messageLoadBeforeRun));
	tier2_scalars.loadsBeforeRunAreNull = image == NULL && imageAgain == NULL && sound == NULL && stream == NULL && font == NULL;

	CP_Engine_SetNextGameState(HarnessInit, HarnessUpdate, NULL);
	CP_Engine_Run();
	ReadExitReport();
}
