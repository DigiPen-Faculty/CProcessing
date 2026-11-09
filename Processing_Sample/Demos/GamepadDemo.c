//---------------------------------------------------------
// file:	GamepadDemo.c
//
// CProcessing features in this demo:
// - Check for connected gamepads
// - Find the default gamepad index (this changes as devices are added removed)
// - Check and visualize all analog and digital inputs from each gamepad
// - Play sounds with a volume and pitch (CP_Sound_PlayAdvanced) in a sound
//   group with its own volume (CP_Sound_SetGroupVolume)
// - Take screenshots every frame, of the whole window and of a small region
//   (CP_Image_Screenshot), and draw them scaled (CP_Image_Draw)
//
// Controls (also listed on screen, top right):
// - D-pad up, right, down, left and the A, B, X, Y buttons play the notes
//   C D E F G A B C, and so do the keys Q W E R T Y U I
// - The left trigger raises the volume of new notes (1x to 2x), the right
//   trigger raises their pitch (1x to 3x)
// - A screenshot of the window, at 1/8 size, follows the mouse
// - Hold the left mouse button for a 2x magnifier
// - Space switches tunnel mode on and off: each screenshot is taken after the
//   previous one was drawn, so it contains it, and so on
//---------------------------------------------------------

#include "cprocessing.h"
#include "GamepadDemo.h"
#include "DemoManager.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

//-------------------------
// NOTES

// The notes play in their own sound group, at half volume, so that the
// left trigger's 2x volume reaches full volume
#define NOTE_GROUP CP_SOUND_GROUP_2
#define NOTE_GROUP_VOLUME 0.5f

typedef struct Note
{
	const char* name;		// shown in the list of notes played
	const char* file;
	CP_GAMEPAD button;
	CP_KEY key;
	CP_Sound sound;
} Note;

// The C major scale, C to C
static Note notes[] =
{
	{ "C",         "Assets/Notes/c.wav", GAMEPAD_DPAD_UP,    KEY_Q, NULL },
	{ "D",         "Assets/Notes/d.wav", GAMEPAD_DPAD_RIGHT, KEY_W, NULL },
	{ "E",         "Assets/Notes/e.wav", GAMEPAD_DPAD_DOWN,  KEY_E, NULL },
	{ "F",         "Assets/Notes/f.wav", GAMEPAD_DPAD_LEFT,  KEY_R, NULL },
	{ "G",         "Assets/Notes/g.wav", GAMEPAD_A,          KEY_T, NULL },
	{ "A",         "Assets/Notes/a.wav", GAMEPAD_B,          KEY_Y, NULL },
	{ "B",         "Assets/Notes/b.wav", GAMEPAD_X,          KEY_U, NULL },
	{ "C (piano)", "Assets/Piano/c.wav", GAMEPAD_Y,          KEY_I, NULL },
};
#define NOTE_COUNT (int)(sizeof(notes) / sizeof(notes[0]))

// The most recent notes played, newest first
#define NOTE_LOG_SIZE 5
typedef struct NotePlayed
{
	const char* name;
	float volume;
	float pitch;
} NotePlayed;
static NotePlayed notesPlayed[NOTE_LOG_SIZE];
static int notesPlayedCount = 0;
static CP_BOOL soundFilesMissing = FALSE;

//-------------------------
// SCREENSHOTS

#define THUMBNAIL_SCALE 8		// the window screenshot is drawn at 1/8 size
#define LENS_SIZE 80			// the magnifier shows an 80 x 80 region...
#define LENS_ZOOM 2				// ...at 2x

static CP_BOOL tunnelMode = FALSE;
static CP_Image tunnelScreenshot = NULL;	// in tunnel mode, the previous frame's screenshot
static unsigned screenshotsTaken = 0;

static CP_Image TakeScreenshot(int x, int y, int width, int height)
{
	++screenshotsTaken;
	return CP_Image_Screenshot(x, y, width, height);
}

static int ClampInt(int value, int low, int high)
{
	return value < low ? low : (value > high ? high : value);
}

void DrawGamepadData(int posX, int posY, int i)
{
	// if the gamepad isn't connected then don't render the input state
	if (!CP_Input_GamepadConnectedAdvanced(i))
	{
		return;
	}

	// find default gamepad
	int defaultId = -1;
	for (int id = 0; id < CP_MAX_GAMEPADS; ++id)
	{
		if (CP_Input_GamepadConnectedAdvanced(id))
		{
			defaultId = id;
			break;
		}
	}

	CP_Settings_Save();
	CP_Settings_Translate((float)posX, (float)posY);	// move to the specified quadrant

	//-------------------------
	// INDEX and DEFAULT
	char buffer[32] = { 0 };
	snprintf(buffer, sizeof(buffer), "Gamepad: %d", i + 1);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Font_DrawText(buffer, 10, 30);
	if (defaultId == i)
	{
		CP_Font_DrawText("Default", 10, 60);
	}

	//-------------------------
	// STICKS
	float circleDiameter = 60.0f;
	float movementOffset = 30.0f;
	CP_Vector rightVec = CP_Vector_Set(1.0f, 0);
	CP_Vector stick = CP_Vector_Zero();
	CP_BOOL stickClick = FALSE;

	// left stick
	stick = CP_Input_GamepadLeftStickAdvanced(i);
	stickClick = CP_Input_GamepadDownAdvanced(GAMEPAD_LEFT_THUMB, i) ? 80 : 50;
	CP_Settings_Fill(CP_Color_FromColorHSL(CP_ColorHSL_Create(stick.y < 0 ? (int)CP_Vector_Angle(stick, rightVec) : 360 - (int)CP_Vector_Angle(stick, rightVec), (int)(CP_Vector_Length(stick) * 100), stickClick, (int)(CP_Vector_Length(stick) * 255) + (CP_Input_GamepadDownAdvanced(GAMEPAD_LEFT_THUMB, i) ? 255 : 0))));
	CP_Graphics_DrawCircle(100.0f + (stick.x * movementOffset), 160.0f + (-stick.y * movementOffset), circleDiameter);
	// right stick
	stick = CP_Input_GamepadRightStickAdvanced(i);
	stickClick = CP_Input_GamepadDownAdvanced(GAMEPAD_RIGHT_THUMB, i) ? 80 : 50;
	CP_Settings_Fill(CP_Color_FromColorHSL(CP_ColorHSL_Create(stick.y < 0 ? (int)CP_Vector_Angle(stick, rightVec) : 360 - (int)CP_Vector_Angle(stick, rightVec), (int)(CP_Vector_Length(stick) * 100), stickClick, (int)(CP_Vector_Length(stick) * 255) + (CP_Input_GamepadDownAdvanced(GAMEPAD_RIGHT_THUMB, i) ? 255 : 0))));
	CP_Graphics_DrawCircle(300.0f + (stick.x * movementOffset), 160.0f + (-stick.y * movementOffset), circleDiameter);

	//-------------------------
	// TRIGGERS
	CP_Settings_RectMode(CP_POSITION_CORNER);
	float rectWidth = 80.0f;
	CP_Input_GamepadDownAdvanced(GAMEPAD_LEFT_SHOULDER, i) ? CP_Settings_Fill(CP_Color_Create(200, 200, 200, 255)) : CP_Settings_NoFill();
	CP_Graphics_DrawRect(60.0f, 80.0f, rectWidth, 20.0f);
	CP_Input_GamepadDownAdvanced(GAMEPAD_RIGHT_SHOULDER, i) ? CP_Settings_Fill(CP_Color_Create(200, 200, 200, 255)) : CP_Settings_NoFill();
	CP_Graphics_DrawRect(260.0f, 80.0f, rectWidth, 20.0f);
	float trigger = CP_Input_GamepadLeftTriggerAdvanced(i);
	CP_Settings_Fill(CP_Color_Create(255 - (int)(trigger * 255), 0, (int)(trigger * 255), 255));
	CP_Graphics_DrawRect(60.0f, 80.0f, trigger * rectWidth, 20.0f);
	trigger = CP_Input_GamepadRightTriggerAdvanced(i);
	CP_Settings_Fill(CP_Color_Create((int)(trigger * 255), 255, 0, 255));
	CP_Graphics_DrawRect(260.0f, 80.0f, trigger * rectWidth, 20.0f);

	//-------------------------
	// DPAD
	CP_Settings_RectMode(CP_POSITION_CENTER);
	float buttonSize = 30.0f;
	CP_Color fillcolor = CP_Color_FromColorHSL(CP_ColorHSL_Create((int)(CP_System_GetSeconds() * 60.0f) % 360, 100, 50, 255));
	CP_Input_GamepadDownAdvanced(GAMEPAD_DPAD_UP, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawRect(100.0f, 240.0f, buttonSize, buttonSize);
	CP_Input_GamepadDownAdvanced(GAMEPAD_DPAD_LEFT, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawRect(60.0f, 280.0f, buttonSize, buttonSize);
	CP_Input_GamepadDownAdvanced(GAMEPAD_DPAD_RIGHT, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawRect(140.0f, 280.0f, buttonSize, buttonSize);
	CP_Input_GamepadDownAdvanced(GAMEPAD_DPAD_DOWN, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawRect(100.0f, 320.0f, buttonSize, buttonSize);

	//-------------------------
	// ABXY
	CP_Input_GamepadDownAdvanced(GAMEPAD_Y, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawCircle(300.0f, 240.0f, buttonSize);
	CP_Input_GamepadDownAdvanced(GAMEPAD_X, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawCircle(260.0f, 280.0f, buttonSize);
	CP_Input_GamepadDownAdvanced(GAMEPAD_B, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawCircle(340.0f, 280.0f, buttonSize);
	CP_Input_GamepadDownAdvanced(GAMEPAD_A, i) ? CP_Settings_Fill(fillcolor) : CP_Settings_NoFill();
	CP_Graphics_DrawCircle(300.0f, 320.0f, buttonSize);

	CP_Settings_Restore();
}

// The volume and pitch new notes play at, from the default gamepad's
// triggers (1x with no gamepad)
static float NoteVolume(void)
{
	return 1.0f + CP_Input_GamepadLeftTrigger();		// 1x to 2x
}

static float NotePitch(void)
{
	return 1.0f + 2.0f * CP_Input_GamepadRightTrigger();	// 1x to 3x
}

// Play a note for each button or key pressed this frame
static void PlayNotes(void)
{
	for (int i = 0; i < NOTE_COUNT; ++i)
	{
		if (CP_Input_GamepadTriggered(notes[i].button) || CP_Input_KeyTriggered(notes[i].key))
		{
			float volume = NoteVolume();
			float pitch = NotePitch();
			CP_Sound_PlayAdvanced(notes[i].sound, volume, pitch, FALSE, NOTE_GROUP);

			// add it to the top of the list of notes played
			for (int n = NOTE_LOG_SIZE - 1; n > 0; --n)
			{
				notesPlayed[n] = notesPlayed[n - 1];
			}
			notesPlayed[0].name = notes[i].name;
			notesPlayed[0].volume = volume;
			notesPlayed[0].pitch = pitch;
			++notesPlayedCount;
		}
	}
}

// The controls and what's happening, in the top right corner
static void DrawInfo(void)
{
	CP_Settings_Save();
	CP_Settings_TextSize(16.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_RIGHT, CP_TEXT_ALIGN_V_TOP);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));

	const float x = CP_System_GetWindowWidth() - 15.0f;
	const float lineHeight = 19.0f;
	float y = 10.0f;
	char line[96];

	snprintf(line, sizeof(line), "SOUND (notes play at group volume %.1f)", NOTE_GROUP_VOLUME);
	CP_Font_DrawText(line, x, y); y += lineHeight;
	CP_Font_DrawText("D-pad up, right, down, left: C D E F", x, y); y += lineHeight;
	CP_Font_DrawText("A, B, X, Y: G A B C (piano)", x, y); y += lineHeight;
	CP_Font_DrawText("Keys Q W E R T Y U I: the same 8 notes", x, y); y += lineHeight;
	snprintf(line, sizeof(line), "Left trigger, volume: %.2fx (1x to 2x)", NoteVolume());
	CP_Font_DrawText(line, x, y); y += lineHeight;
	snprintf(line, sizeof(line), "Right trigger, pitch: %.2fx (1x to 3x)", NotePitch());
	CP_Font_DrawText(line, x, y); y += lineHeight;
	if (soundFilesMissing)
	{
		CP_Font_DrawText("Sound files not found: run from the folder with Assets", x, y); y += lineHeight;
	}

	snprintf(line, sizeof(line), "Notes played: %d", notesPlayedCount);
	CP_Font_DrawText(line, x, y); y += lineHeight;
	for (int n = 0; n < NOTE_LOG_SIZE && n < notesPlayedCount; ++n)
	{
		snprintf(line, sizeof(line), "%s   volume %.2fx   pitch %.2fx", notesPlayed[n].name, notesPlayed[n].volume, notesPlayed[n].pitch);
		CP_Font_DrawText(line, x, y); y += lineHeight;
	}

	y += lineHeight;
	CP_Font_DrawText("SCREENSHOTS: the window at 1/8 size follows the mouse", x, y); y += lineHeight;
	CP_Font_DrawText("Hold the left mouse button: 2x magnifier", x, y); y += lineHeight;
	snprintf(line, sizeof(line), "Space: tunnel mode (%s)", tunnelMode ? "ON" : "off");
	CP_Font_DrawText(line, x, y); y += lineHeight;
	snprintf(line, sizeof(line), "%.0f FPS, %u screenshots taken", CP_System_GetFrameRate(), screenshotsTaken);
	CP_Font_DrawText(line, x, y);

	CP_Settings_Restore();
}

// Draw an image with its top left corner at (x, y), framed in white
static void DrawFramed(CP_Image image, float x, float y, float width, float height)
{
	CP_Settings_Save();
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_Image_Draw(image, x, y, width, height, 255);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_NoFill();
	CP_Settings_Stroke(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_StrokeWeight(2.0f);
	CP_Graphics_DrawRect(x, y, width, height);
	CP_Settings_Restore();
}

static void DrawScreenshots(void)
{
	const int width = CP_System_GetWindowWidth();
	const int height = CP_System_GetWindowHeight();
	const float mouseX = CP_Input_GetMouseX();
	const float mouseY = CP_Input_GetMouseY();
	const float thumbnailWidth = (float)width / THUMBNAIL_SCALE;
	const float thumbnailHeight = (float)height / THUMBNAIL_SCALE;

	if (CP_Input_KeyTriggered(KEY_SPACE))
	{
		tunnelMode = !tunnelMode;
		if (!tunnelMode)
		{
			CP_Image_Free(&tunnelScreenshot);
		}
	}

	// The magnifier's region: centered on the mouse, but kept inside the
	// window. It's captured before anything is drawn over it.
	CP_Image lens = NULL;
	int lensX = ClampInt((int)mouseX - LENS_SIZE / 2, 0, width - LENS_SIZE);
	int lensY = ClampInt((int)mouseY - LENS_SIZE / 2, 0, height - LENS_SIZE);
	if (CP_Input_MouseDown(MOUSE_BUTTON_LEFT))
	{
		lens = TakeScreenshot(lensX, lensY, LENS_SIZE, LENS_SIZE);
	}

	if (tunnelMode)
	{
		// last frame's screenshot, which shows last frame's thumbnail, which
		// shows the one before it, and so on
		if (tunnelScreenshot)
		{
			DrawFramed(tunnelScreenshot, mouseX, mouseY, thumbnailWidth, thumbnailHeight);
		}
	}
	else
	{
		// the window as it is now
		CP_Image screenshot = TakeScreenshot(0, 0, width, height);
		DrawFramed(screenshot, mouseX, mouseY, thumbnailWidth, thumbnailHeight);
		CP_Image_Free(&screenshot);
	}

	if (lens)
	{
		// drawn centered over the region it shows, with a cross at its center
		const float zoomedSize = (float)(LENS_SIZE * LENS_ZOOM);
		const float centerX = lensX + LENS_SIZE * 0.5f;
		const float centerY = lensY + LENS_SIZE * 0.5f;
		DrawFramed(lens, centerX - zoomedSize * 0.5f, centerY - zoomedSize * 0.5f, zoomedSize, zoomedSize);
		CP_Image_Free(&lens);

		CP_Settings_Save();
		CP_Settings_Stroke(CP_Color_Create(255, 0, 0, 255));
		CP_Settings_StrokeWeight(1.0f);
		CP_Graphics_DrawLine(centerX - 8.0f, centerY, centerX + 8.0f, centerY);
		CP_Graphics_DrawLine(centerX, centerY - 8.0f, centerX, centerY + 8.0f);
		CP_Settings_Restore();
	}

	if (tunnelMode)
	{
		// taken after everything is drawn, so the next thumbnail contains this
		// frame's thumbnail
		CP_Image_Free(&tunnelScreenshot);
		tunnelScreenshot = TakeScreenshot(0, 0, width, height);
	}
}

void GamepadDemoInit(void)
{
	CP_Settings_Save();
	CP_Settings_TextSize(25);
	CP_Settings_Stroke(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_StrokeWeight(3);

	soundFilesMissing = FALSE;
	for (int i = 0; i < NOTE_COUNT; ++i)
	{
		notes[i].sound = CP_Sound_Load(notes[i].file);
		if (!notes[i].sound)
		{
			soundFilesMissing = TRUE;
		}
	}
	CP_Sound_SetGroupVolume(NOTE_GROUP, NOTE_GROUP_VOLUME);
	notesPlayedCount = 0;
	tunnelMode = FALSE;
	screenshotsTaken = 0;
}

void GamepadDemoUpdate(void)
{
	// clear background and draw separation lines
	CP_Graphics_ClearBackground(GetCommonBackgoundColor());
	CP_Graphics_DrawLine(CP_System_GetWindowWidth() * 0.5f, 0, CP_System_GetWindowWidth() * 0.5f, (float)CP_System_GetWindowHeight());
	CP_Graphics_DrawLine(0, CP_System_GetWindowHeight() * 0.5f, (float)CP_System_GetWindowWidth(), CP_System_GetWindowHeight() * 0.5f);

	// if no gamepad is connected let the user know to add one
	if (!CP_Input_GamepadConnected())
	{
		CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
		CP_Font_DrawText("Connect a gamepad to see visualization data.", 50, 50);
	}

	// calculate positioning information
	int gamepadVisualizationWidth = 400;
	int windowWidth = CP_System_GetWindowWidth();
	int halfWidth = windowWidth / 2;
	int halfHeight = CP_System_GetWindowHeight() / 2;
	int leftSideX = (halfWidth - gamepadVisualizationWidth) / 2;

	// render all the gamepad data
	DrawGamepadData(leftSideX, 0, 0);							// top left
	DrawGamepadData(halfWidth + leftSideX, 0, 1);				// top right
	DrawGamepadData(leftSideX, halfHeight, 2);					// bottom left
	DrawGamepadData(halfWidth + leftSideX, halfHeight, 3);		// bottom right

	PlayNotes();
	DrawInfo();

	// last, so the screenshots show everything above
	DrawScreenshots();
}

void GamepadDemoExit(void)
{
	CP_Sound_StopGroup(NOTE_GROUP);
	for (int i = 0; i < NOTE_COUNT; ++i)
	{
		CP_Sound_Free(&notes[i].sound);
	}
	CP_Sound_SetGroupVolume(NOTE_GROUP, 1.0f);
	CP_Image_Free(&tunnelScreenshot);
	CP_Settings_Restore();
}
