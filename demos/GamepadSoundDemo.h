//---------------------------------------------------------
// file:	GamepadSoundDemo.h
//
// CProcessing features in this demo:
// - Check for connected gamepads
// - Find the default gamepad index (this changes as devices are added removed)
// - Check and visualize all analog and digital inputs from each gamepad
// - Play notes with a volume and pitch, in a sound group with its own volume
//---------------------------------------------------------

#pragma once

void GamepadSoundDemoInit(void);
void GamepadSoundDemoUpdate(void);
void GamepadSoundDemoExit(void);
