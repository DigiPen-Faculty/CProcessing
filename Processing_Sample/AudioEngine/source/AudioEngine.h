//---------------------------------------------------------------------------------------------------------------------
// file:    AudioEngine.h
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#pragma once

typedef struct AE_System AE_System;
typedef struct AE_SoundAsset AE_SoundAsset;
typedef struct AE_SoundInstance AE_SoundInstance;
typedef struct AE_SoundGroup AE_SoundGroup;

typedef enum AE_RESULT
{
    AE_OK,
    AE_POINTER_NOT_VALID,
    AE_SOUND_STILL_PLAYING,
    AE_STREAMING_SOUND_ALREADY_PLAYING,
    AE_ERROR_OPENING_FILE,
    AE_ERROR_UNSUPPORTED_FILE_TYPE,
    AE_ERROR_READING_AUDIO_FILE,
    AE_SOUND_NOT_LOADED,
    AE_ERROR_STARTING_AUDIO_OUTPUT,
    AE_INSTANCE_DOES_NOT_EXIST,
    AE_MAXIMUM_GROUPS_REACHED,
    AE_INVALID_GROUP_ID,
} AE_RESULT;

#ifdef __cplusplus
extern "C"
{
#endif

AE_RESULT AE_System_Initialize(AE_System** system);

void AE_System_Update(AE_System* system);

void AE_System_ShutDown(AE_System* system);

void AE_System_Release(AE_System** system);

AE_RESULT AE_Sounds_LoadSound(AE_System* system, AE_SoundAsset** asset, const char* file, int streaming);

AE_RESULT AE_Sounds_ReleaseSound(AE_System* system, AE_SoundAsset* asset);

AE_RESULT AE_Sounds_PlaySound(AE_System* system, AE_SoundAsset* asset, AE_SoundInstance** instance, int loop, 
    int startPaused, AE_SoundGroup* group);

AE_RESULT AE_Instance_GetVolume(AE_System* system, AE_SoundInstance* instance, float* volume);

AE_RESULT AE_Instance_SetVolume(AE_System* system, AE_SoundInstance* instance, float volume);

AE_RESULT AE_Instance_GetPitch(AE_System* system, AE_SoundInstance* instance, float* pitch);

AE_RESULT AE_Instance_SetPitch(AE_System* system, AE_SoundInstance* instance, float pitch);

AE_RESULT AE_Instance_SetPaused(AE_System* system, AE_SoundInstance* instance, int paused);

AE_RESULT AE_Instance_Stop(AE_System* system, AE_SoundInstance* instance);

AE_RESULT AE_Group_CreateGroup(AE_System* system, AE_SoundGroup** group);

AE_RESULT AE_Group_ReleaseGroup(AE_System* system, AE_SoundGroup* group);

AE_RESULT AE_Group_SetPaused(AE_System* system, AE_SoundGroup* group, int paused);

AE_RESULT AE_Group_Stop(AE_System* system, AE_SoundGroup* group);

AE_RESULT AE_Group_GetVolume(AE_System* system, AE_SoundGroup* group, float* volume);

AE_RESULT AE_Group_SetVolume(AE_System* system, AE_SoundGroup* group, float volume);

AE_RESULT AE_Group_GetPitch(AE_System* system, AE_SoundGroup* group, float* pitch);

AE_RESULT AE_Group_SetPitch(AE_System* system, AE_SoundGroup* group, float pitch);

const char* AE_GetErrorText(AE_RESULT result);

#ifdef __cplusplus
}
#endif

