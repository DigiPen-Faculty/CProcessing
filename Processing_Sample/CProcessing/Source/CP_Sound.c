//------------------------------------------------------------------------------
// file:	CP_Sound.c
// author:	Daniel Hamilton, Andrea Ellinger
// brief:	Load, play and manipulate sound files 
//
// Copyright © 2019 DigiPen, All rights reserved.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include Files:
//------------------------------------------------------------------------------

#include "cprocessing.h"
#include "Internal_Sound.h"
#include "vect.h"

#include "AudioEngine.h"

//------------------------------------------------------------------------------
// Defines and Internal Variables:
//------------------------------------------------------------------------------

#define CP_INITIAL_SOUND_CAPACITY   12

VECT_GENERATE_TYPE(CP_Sound)

static AE_RESULT result = 0;
static AE_System* _audio_system = NULL;

static vect_CP_Sound* sound_vector = NULL;

static AE_SoundGroup* sound_groups[CP_SOUND_GROUP_MAX] = { NULL };

//------------------------------------------------------------------------------
// Internal Functions:
//------------------------------------------------------------------------------

static void CP_PrintAudioError(AE_RESULT error)
{
	printf("Audio error: %s\n", AE_GetErrorText(error));
}

static BOOL CP_IsValidSoundGroup(CP_SOUND_GROUP group)
{
	return group >= 0 && group < CP_SOUND_GROUP_MAX;
}

static CP_Sound CP_CheckIfSoundIsLoaded(const char* filepath)
{
	for (unsigned i = 0; i < sound_vector->size; ++i)
	{
		CP_Sound snd = vect_at_CP_Sound(sound_vector, i);
		if (snd && !strcmp(filepath, snd->filepath))
		{
			return snd;
		}
	}

	return NULL;
}

void CP_Sound_Init(void)
{
	// Allocate the initial vector size for loaded sounds
	sound_vector = vect_init_CP_Sound(CP_INITIAL_SOUND_CAPACITY); 

	// Create the audio system
	result = AE_System_Initialize(&_audio_system);
	if (result != AE_OK)
	{
		CP_PrintAudioError(result);
		_audio_system = NULL;
		return;
	}

	// Create the groups (for stopping/pausing and controlling pitch and volume on a per group basis)
	for (unsigned index = 0; index < CP_SOUND_GROUP_MAX && result == AE_OK; ++index)
	{
		result = AE_Group_CreateGroup(_audio_system, &sound_groups[index]);
	}
	if (result != AE_OK)
	{
		CP_PrintAudioError(result);
		CP_Sound_Shutdown();
		_audio_system = NULL;
		return;
	}
}

void CP_Sound_Update(void)
{
	if (_audio_system != NULL)
	{
		AE_System_Update(_audio_system);
	}
}

void CP_Sound_Shutdown(void)
{
	if (_audio_system == NULL)
		return;
	
	// Stop all current sounds
	CP_Sound_StopAll();

	// Free sounds 
	for (unsigned i = 0; i < sound_vector->size; ++i)
	{
		CP_Sound sound = vect_at_CP_Sound(sound_vector, i);
		// Release the sound 
		AE_Sounds_ReleaseSound(_audio_system, sound->sound);
		// Free the struct's memory
		free(sound);
	}

	// Free lists
	vect_free(sound_vector);

	// Release system
	AE_System_ShutDown(_audio_system);
	AE_System_Release(&_audio_system);
	_audio_system = NULL;
}

CP_Sound CP_Sound_LoadInternal(const char* filepath, CP_BOOL streamFromDisc)
{
	if (!filepath || _audio_system == NULL)
		return NULL;

	CP_Sound sound = NULL;

	// Check if the sound is already loaded
	sound = CP_CheckIfSoundIsLoaded(filepath);
	if (sound)
	{
		return sound;
	}

	// Allocate memory for the struct
	sound = (CP_Sound)malloc(sizeof(CP_Sound_Struct));
	if (!sound)
	{
		return NULL;
	}

	// Create the sound
	if (streamFromDisc)
	{
		result = AE_Sounds_LoadSound(_audio_system, &(sound->sound), filepath, 1);
	}
	else
	{
		result = AE_Sounds_LoadSound(_audio_system, &(sound->sound), filepath, 0);
	}
	if (result != AE_OK)
	{
		CP_PrintAudioError(result);
		free(sound);
		return NULL;
	}

	// Set filepath string for cache checking
	strcpy_s(sound->filepath, MAX_PATH, filepath);

	// Add it to the list
	vect_push_CP_Sound(sound_vector, sound);

	return sound;
}

//------------------------------------------------------------------------------
// Library Functions:
//------------------------------------------------------------------------------

CP_API CP_Sound CP_Sound_Load(const char* filepath)
{
	return CP_Sound_LoadInternal(filepath, FALSE);
}

CP_API CP_Sound CP_Sound_LoadMusic(const char* filepath)
{
	return CP_Sound_LoadInternal(filepath, TRUE);
}

CP_API void CP_Sound_Free(CP_Sound* sound)
{
	if (_audio_system == NULL || sound == NULL || *sound == NULL)
	{
		return;
	}

	// Find the sound in the list
	for (unsigned i = 0; i < sound_vector->size; ++i)
	{
		// Check if this pointer is the same as the one we're looking for
		if (vect_at_CP_Sound(sound_vector, i) == *sound)
		{
			// Remove the sound from the list
			vect_rem_CP_Sound(sound_vector, i);
			// Release the sound 
			result = AE_Sounds_ReleaseSound(_audio_system, (*sound)->sound);
			if (result != AE_OK)
				CP_PrintAudioError(result);
			// Free the struct's memory
			free(*sound);
			*sound = NULL;
			return;
		}
	}

	// TODO: handle error - we reached the end of the list without finding the sound
}

CP_API void CP_Sound_Play(CP_Sound sound)
{
	CP_Sound_PlayAdvanced(sound, 1.0f, 1.0f, FALSE, CP_SOUND_GROUP_SFX);
}

CP_API void CP_Sound_PlayMusic(CP_Sound sound)
{
	CP_Sound_PlayAdvanced(sound, 1.0f, 1.0f, TRUE, CP_SOUND_GROUP_MUSIC);
}

CP_API void CP_Sound_PlayAdvanced(CP_Sound sound, float volume, float pitch, CP_BOOL looping, CP_SOUND_GROUP group)
{
	if (_audio_system == NULL || !CP_IsValidSoundGroup(group) || sound == NULL)
	{
		return;
	}

	// Only need to save the instance temporarily since there is no instance-specific functionality
	AE_SoundInstance* instance;

	// Start the sound paused so we can set parameters on it
	result = AE_Sounds_PlaySound(_audio_system, sound->sound, &instance, looping, 1, sound_groups[group]);
	if (result != AE_OK)
	{
		CP_PrintAudioError(result);
		return;
	}

	// Set the volume if it is not 1.0
	// (2.0 is double volume, 0.0 is silent)
	if (volume != 1.0f)
	{
		if (volume < 0.0f)
			volume = 0.0f;

		result = AE_Instance_SetVolume(_audio_system, instance, volume);
		if (result != AE_OK)
			CP_PrintAudioError(result);
	}

	// Set the pitch if it is not 1.0
	// (0.5 is half pitch, 2.0 is double pitch)
	if (pitch != 1.0f)
	{
		if (pitch < 0.0f)
			pitch = 0.0f;

		result = AE_Instance_SetPitch(_audio_system, instance, pitch);
		if (result != AE_OK)
			CP_PrintAudioError(result);
	}

	// Resume playing the sound
	result = AE_Instance_SetPaused(_audio_system, instance, 0);
	if (result != AE_OK)
		CP_PrintAudioError(result);
}

CP_API void CP_Sound_PauseAll(void)
{
	if (_audio_system == NULL)
		return;

	for (unsigned index = 0; index < CP_SOUND_GROUP_MAX; ++index)
	{
		result = AE_Group_SetPaused(_audio_system, sound_groups[index], 1);
		if (result != AE_OK)
			CP_PrintAudioError(result);
	}
}

CP_API void CP_Sound_PauseGroup(CP_SOUND_GROUP group)
{
	if (_audio_system == NULL)
		return;

	if(CP_IsValidSoundGroup(group))
	{
		result = AE_Group_SetPaused(_audio_system, sound_groups[group], 1);
		if (result != AE_OK)
			CP_PrintAudioError(result);
	}
}

CP_API void CP_Sound_ResumeAll(void)
{
	if (_audio_system == NULL)
		return;

	for (unsigned index = 0; index < CP_SOUND_GROUP_MAX; ++index)
	{
		result = AE_Group_SetPaused(_audio_system, sound_groups[index], 0);
		if (result != AE_OK)
			CP_PrintAudioError(result);
	}
}

CP_API void CP_Sound_ResumeGroup(CP_SOUND_GROUP group)
{
	if (_audio_system == NULL)
		return;

	if (CP_IsValidSoundGroup(group))
	{
		result = AE_Group_SetPaused(_audio_system, sound_groups[group], 0);
		if (result != AE_OK)
			CP_PrintAudioError(result);
	}
}

CP_API void CP_Sound_StopAll(void)
{
	if (_audio_system == NULL)
		return;

	for (unsigned index = 0; index < CP_SOUND_GROUP_MAX; ++index)
	{
		result = AE_Group_Stop(_audio_system, sound_groups[index]);
		if (result != AE_OK)
			CP_PrintAudioError(result);
	}
}

CP_API void CP_Sound_StopGroup(CP_SOUND_GROUP group)
{
	if (_audio_system == NULL)
		return;

	if (CP_IsValidSoundGroup(group))
	{
		result = AE_Group_Stop(_audio_system, sound_groups[group]);
		if (result != AE_OK)
			CP_PrintAudioError(result);
	}
}

CP_API void CP_Sound_SetGroupVolume(CP_SOUND_GROUP group, float volume)
{
	if (_audio_system == NULL)
		return;

	if (CP_IsValidSoundGroup(group))
	{
		result = AE_Group_SetVolume(_audio_system, sound_groups[group], volume);
		if (result != AE_OK)
			CP_PrintAudioError(result);
	}
}

CP_API float CP_Sound_GetGroupVolume(CP_SOUND_GROUP group)
{
	if (_audio_system == NULL)
		return 0.0f;

	float volume = 0;
	if (CP_IsValidSoundGroup(group))
	{
		result = AE_Group_GetVolume(_audio_system, sound_groups[group], &volume);
		if (result != AE_OK)
			CP_PrintAudioError(result);
	}
	return volume;
}

CP_API void CP_Sound_SetGroupPitch(CP_SOUND_GROUP group, float pitch)
{
	if (_audio_system == NULL)
		return;

	if (CP_IsValidSoundGroup(group))
	{
		result = AE_Group_SetPitch(_audio_system, sound_groups[group], pitch);
		if (result != AE_OK)
			CP_PrintAudioError(result);
	}
}

CP_API float CP_Sound_GetGroupPitch(CP_SOUND_GROUP group)
{
	if (_audio_system == NULL)
		return 0.0f;

	float pitch = 0;
	if (CP_IsValidSoundGroup(group))
	{
		result = AE_Group_GetPitch(_audio_system, sound_groups[group], &pitch);
		if (result != AE_OK)
			CP_PrintAudioError(result);
	}
	return pitch;
}
