//---------------------------------------------------------------------------------------------------------------------
// file:    AudioEngine.cpp
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#include "AudioEngine.h"
#include "Mixer.h"
#include "SoundAsset.h"
#include "SoundInstance.h"

using namespace AudioEngine;

typedef struct AE_System
{
    Mixer Mixer;
} AE_System;

typedef struct AE_SoundAsset
{
    SoundAsset Asset;
} AE_SoundAsset;

typedef struct AE_SoundInstance
{
    AE_SoundInstance(InstanceIDType id) : InstanceID(id) {}

    InstanceIDType InstanceID;
} AE_SoundInstance;

typedef struct AE_SoundGroup
{
    GroupIDType GroupID;
} AE_SoundGroup;


//*********************************************************************************************************************
AE_RESULT AE_System_Initialize(AE_System** system)
{
    if (!system)
        return AE_POINTER_NOT_VALID;

    AE_System* newSystem = new AE_System();
    AE_RESULT result = newSystem->Mixer.Initialize();

    *system = newSystem;

    return result;
}

//*********************************************************************************************************************
void AE_System_Update(AE_System* system)
{
    if (!system)
        return;

    system->Mixer.Update();
}

//*********************************************************************************************************************
void AE_System_ShutDown(AE_System* system)
{
    if (!system)
        return;

    system->Mixer.ShutDown();
}

//*********************************************************************************************************************
void AE_System_Release(AE_System** system)
{
    if (!system || !*system)
        return;

    delete* system;

    *system = nullptr;
}

//*********************************************************************************************************************
AE_RESULT AE_Sounds_LoadSound(AE_System* system, AE_SoundAsset** asset, const char* file, int streaming)
{
    if (!system || !asset)
        return AE_POINTER_NOT_VALID;

    AE_SoundAsset* newAsset = new AE_SoundAsset;
    AE_RESULT result = newAsset->Asset.Load(file, streaming);
    if (result != AE_OK)
    {
        *asset = nullptr;
        delete newAsset;
        return result;
    }

    *asset = newAsset;

    return AE_OK;
}

//*********************************************************************************************************************
AE_RESULT AE_Sounds_ReleaseSound(AE_System* system, AE_SoundAsset* asset)
{
    if (!system || !asset)
        return AE_POINTER_NOT_VALID;

    if (asset->Asset.HasInstances())
        return AE_SOUND_STILL_PLAYING;

    delete asset;

    return AE_OK;
}

//*********************************************************************************************************************
AE_RESULT AE_Sounds_PlaySound(AE_System* system, AE_SoundAsset* asset, AE_SoundInstance** instance, int loop, 
    int startPaused, AE_SoundGroup* group)
{
    if (!system || !asset)
        return AE_POINTER_NOT_VALID;

    InstanceIDType id;
    GroupIDType groupID = 0;
    if (group)
        groupID = group->GroupID;
    AE_RESULT result = asset->Asset.Play(&id, loop, startPaused, groupID);
    if (result != AE_OK)
    {
        *instance = nullptr;
        return result;
    }

    *instance = new AE_SoundInstance(id);

    return AE_OK;
}

//*********************************************************************************************************************
AE_RESULT AE_Instance_GetVolume(AE_System* system, AE_SoundInstance* instance, float* volume)
{
    if (!system || !instance)
        return AE_POINTER_NOT_VALID;

    SoundInstance* inst = system->Mixer.FindInstance(instance->InstanceID);
    if (!inst)
    {
        *volume = 0.0f;
        return AE_INSTANCE_DOES_NOT_EXIST;
    }

    *volume = inst->GetVolume();

    return AE_OK;
}

//*********************************************************************************************************************
AE_RESULT AE_Instance_SetVolume(AE_System* system, AE_SoundInstance* instance, float volume)
{
    if (!system || !instance)
        return AE_POINTER_NOT_VALID;

    SoundInstance* inst = system->Mixer.FindInstance(instance->InstanceID);
    if (!inst)
        return AE_INSTANCE_DOES_NOT_EXIST;

    inst->SetVolume(volume);

    return AE_OK;
}

//*********************************************************************************************************************
AE_RESULT AE_Instance_GetPitch(AE_System* system, AE_SoundInstance* instance, float* pitch)
{
    if (!system || !instance)
        return AE_POINTER_NOT_VALID;

    SoundInstance* inst = system->Mixer.FindInstance(instance->InstanceID);
    if (!inst)
    {
        *pitch = 0.0f;
        return AE_INSTANCE_DOES_NOT_EXIST;
    }

    *pitch = inst->GetPitch();

    return AE_OK;
}

//*********************************************************************************************************************
AE_RESULT AE_Instance_SetPitch(AE_System* system, AE_SoundInstance* instance, float pitch)
{
    if (!system || !instance)
        return AE_POINTER_NOT_VALID;

    SoundInstance* inst = system->Mixer.FindInstance(instance->InstanceID);
    if (!inst)
        return AE_INSTANCE_DOES_NOT_EXIST;

    inst->SetPitch(pitch);

    return AE_OK;
}

//*********************************************************************************************************************
AE_RESULT AE_Instance_SetPaused(AE_System* system, AE_SoundInstance* instance, int paused)
{
    if (!system || !instance)
        return AE_POINTER_NOT_VALID;

    SoundInstance* inst = system->Mixer.FindInstance(instance->InstanceID);
    if (!inst)
        return AE_INSTANCE_DOES_NOT_EXIST;

    inst->SetPaused(paused);

    return AE_OK;
}

//*********************************************************************************************************************
AE_RESULT AE_Instance_Stop(AE_System* system, AE_SoundInstance* instance)
{
    if (!system || !instance)
        return AE_POINTER_NOT_VALID;

    SoundInstance* inst = system->Mixer.FindInstance(instance->InstanceID);
    if (!inst)
        return AE_INSTANCE_DOES_NOT_EXIST;

    inst->Stop();

    return AE_OK;
}

//*********************************************************************************************************************
AE_RESULT AE_Group_CreateGroup(AE_System* system, AE_SoundGroup** group)
{
    if (!system || !group)
        return AE_POINTER_NOT_VALID;

    GroupIDType id;
    AE_RESULT result = system->Mixer.GetNewInstanceGroup(&id);
    if (result == AE_OK)
    {
        *group = new AE_SoundGroup();
        (*group)->GroupID = id;
    }
    
    return result;
}

//*********************************************************************************************************************
AE_RESULT AE_Group_ReleaseGroup(AE_System* system, AE_SoundGroup* group)
{
    if (!system || !group)
        return AE_POINTER_NOT_VALID;

    AE_RESULT result = system->Mixer.ReleaseInstanceGroup(group->GroupID);

    delete group;

    return result;
}

//*********************************************************************************************************************
AE_RESULT AE_Group_SetPaused(AE_System* system, AE_SoundGroup* group, int paused)
{
    if (!system || !group)
        return AE_POINTER_NOT_VALID;

    system->Mixer.SetGroupPaused(group->GroupID, paused);

    return AE_OK;
}

//*********************************************************************************************************************
AE_RESULT AE_Group_Stop(AE_System* system, AE_SoundGroup* group)
{
    if (!system || !group)
        return AE_POINTER_NOT_VALID;

    system->Mixer.StopGroup(group->GroupID);

    return AE_OK;
}

//*********************************************************************************************************************
AE_RESULT AE_Group_GetVolume(AE_System* system, AE_SoundGroup* group, float* volume)
{
    if (!system || !group || !volume)
        return AE_POINTER_NOT_VALID;

    return system->Mixer.GetGroupVolume(group->GroupID, volume);
}

//*********************************************************************************************************************
AE_RESULT AE_Group_SetVolume(AE_System* system, AE_SoundGroup* group, float volume)
{
    if (!system || !group)
        return AE_POINTER_NOT_VALID;

    return system->Mixer.SetGroupVolume(group->GroupID, volume);
}

//*********************************************************************************************************************
AE_RESULT AE_Group_GetPitch(AE_System* system, AE_SoundGroup* group, float* pitch)
{
    if (!system || !group || !pitch)
        return AE_POINTER_NOT_VALID;

    return system->Mixer.GetGroupPitch(group->GroupID, pitch);
}

//*********************************************************************************************************************
AE_RESULT AE_Group_SetPitch(AE_System* system, AE_SoundGroup* group, float pitch)
{
    if (!system || !group)
        return AE_POINTER_NOT_VALID;

    return system->Mixer.SetGroupPitch(group->GroupID, pitch);
}
