//---------------------------------------------------------
// file:	GamepadSoundDemo.c
//
// CProcessing features in this demo:
// - Check for connected gamepads
// - Find the default gamepad index (this changes as devices are added removed)
// - Check and visualize all analog and digital inputs from each gamepad
// - Play sounds with a volume and pitch (CP_Sound_PlayAdvanced) in a sound
//   group with its own volume (CP_Sound_SetGroupVolume)
//
// Controls (also listed on screen, top right):
// - D-pad up, right, down, left and the A, B, X, Y buttons play the notes
//   C D E F G A B C, and so do the keys Q W E R T Y U I
// - The left trigger raises the volume of new notes (1x to 2x), the right
//   trigger raises their pitch (1x to 3x)
//---------------------------------------------------------

#include "cprocessing.h"
#include "GamepadSoundDemo.h"
#include "DemoManager.h"
#include <stdio.h>

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

static void DrawGamepadData(int posX, int posY, int i)
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
	int lightness = 50;

	// left stick
	stick = CP_Input_GamepadLeftStickAdvanced(i);
	lightness = CP_Input_GamepadDownAdvanced(GAMEPAD_LEFT_THUMB, i) ? 80 : 50;
	CP_Settings_Fill(CP_Color_FromColorHSL(CP_ColorHSL_Create(stick.y < 0 ? (int)CP_Vector_Angle(stick, rightVec) : 360 - (int)CP_Vector_Angle(stick, rightVec), (int)(CP_Vector_Length(stick) * 100), lightness, (int)(CP_Vector_Length(stick) * 255) + (CP_Input_GamepadDownAdvanced(GAMEPAD_LEFT_THUMB, i) ? 255 : 0))));
	CP_Graphics_DrawCircle(100.0f + (stick.x * movementOffset), 160.0f + (-stick.y * movementOffset), circleDiameter);
	// right stick
	stick = CP_Input_GamepadRightStickAdvanced(i);
	lightness = CP_Input_GamepadDownAdvanced(GAMEPAD_RIGHT_THUMB, i) ? 80 : 50;
	CP_Settings_Fill(CP_Color_FromColorHSL(CP_ColorHSL_Create(stick.y < 0 ? (int)CP_Vector_Angle(stick, rightVec) : 360 - (int)CP_Vector_Angle(stick, rightVec), (int)(CP_Vector_Length(stick) * 100), lightness, (int)(CP_Vector_Length(stick) * 255) + (CP_Input_GamepadDownAdvanced(GAMEPAD_RIGHT_THUMB, i) ? 255 : 0))));
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

	CP_Settings_Restore();
}

void GamepadSoundDemoInit(void)
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
}

void GamepadSoundDemoUpdate(void)
{
	// clear background and draw separation lines
	CP_Graphics_ClearBackground(GetCommonBackgroundColor());
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
}

void GamepadSoundDemoExit(void)
{
	CP_Sound_StopGroup(NOTE_GROUP);
	for (int i = 0; i < NOTE_COUNT; ++i)
	{
		CP_Sound_Free(&notes[i].sound);
	}
	CP_Sound_SetGroupVolume(NOTE_GROUP, 1.0f);
	CP_Settings_Restore();
}
