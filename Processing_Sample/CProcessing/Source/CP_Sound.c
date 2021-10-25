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

//------------------------------------------------------------------------------
// Defines and Internal Variables:
//------------------------------------------------------------------------------

#define MAX_FMOD_CHANNELS 128
#define CP_INITIAL_SOUND_CAPACITY   12
#define FMOD_TRUE 1
#define FMOD_FALSE 0

VECT_GENERATE_TYPE(CP_Sound)

static FMOD_RESULT result = 0;
static FMOD_SYSTEM* _fmod_system = NULL;

static vect_CP_Sound* sound_vector = NULL;

static FMOD_CHANNELGROUP* channel_groups[CP_SOUND_GROUP_MAX] = { NULL };

//------------------------------------------------------------------------------
// Internal Functions:
//------------------------------------------------------------------------------

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

	// Create the FMOD system
	result = FMOD_System_Create(&_fmod_system);
	if (result != FMOD_OK)
	{
		// TODO: handle error - FMOD_ErrorString(result)
		printf("audio error");
		_fmod_system = NULL;
		return;
	}

	// Initialize the system
	result = FMOD_System_Init(_fmod_system, MAX_FMOD_CHANNELS, FMOD_INIT_NORMAL, NULL);
	if (result != FMOD_OK)
	{
		// TODO: handle error - FMOD_ErrorString(result)
		_fmod_system = NULL;
		return;
	}

	// Create the channel groups (for stopping/pausing and controlling pitch and volume on a per group basis)
	for (unsigned index = 0; index < CP_SOUND_GROUP_MAX && result == FMOD_OK; ++index)
	{
		result = FMOD_System_CreateChannelGroup(_fmod_system, NULL, &channel_groups[index]);
	}
	if (result != FMOD_OK)
	{
		// TODO: handle error - FMOD_ErrorString(result)
		CP_Sound_Shutdown();
		return;
	}
}

void CP_Sound_Update(void)
{
	if (_fmod_system != NULL)
	{
		result = FMOD_System_Update(_fmod_system);
		if (result != FMOD_OK)
		{
			// TODO: handle error - FMOD_ErrorString(result)

			// Assume this is a fatal problem and shut down FMOD
			CP_Sound_Shutdown();
			return;
		}
	}
}

void CP_Sound_Shutdown(void)
{
	if (_fmod_system != NULL)
	{
		// Stop all current sounds
		CP_Sound_StopAll();

		// Free sounds 
		for (unsigned i = 0; i < sound_vector->size; ++i)
		{
			CP_Sound sound = vect_at_CP_Sound(sound_vector, i);
			// Release the sound from FMOD
			FMOD_Sound_Release(sound->sound);
			// Free the struct's memory
			free(sound);
		}

		// Free lists
		vect_free(sound_vector);

		// Release system
		FMOD_System_Release(_fmod_system);
		_fmod_system = NULL;
	}
}

CP_Sound CP_Sound_LoadInternal(const char* filepath, CP_BOOL streamFromDisc)
{
	if (!filepath)
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
		// TODO handle error 
		return NULL;
	}

	// Create the FMOD sound
	if (streamFromDisc)
	{
		result = FMOD_System_CreateStream(_fmod_system, filepath, FMOD_DEFAULT, NULL, &(sound->sound));

	}
	else
	{
		result = FMOD_System_CreateSound(_fmod_system, filepath, FMOD_DEFAULT, NULL, &(sound->sound));
	}
	if (result != FMOD_OK)
	{
		// TODO: handle error - FMOD_ErrorString(result)
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
	if (sound == NULL || *sound == NULL)
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
			// Release the sound from FMOD
			FMOD_Sound_Release((*sound)->sound);
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
	if (!CP_IsValidSoundGroup(group) || sound == NULL)
	{
		return;
	}

	// Only need to save the channel value temporarily since there is no channel-specific functionality
	FMOD_CHANNEL* channel;

	// Start the sound paused so we can set parameters on it
	result = FMOD_System_PlaySound(_fmod_system, sound->sound, channel_groups[group], FMOD_TRUE, &channel);
	if (result != FMOD_OK)
	{
		// TODO: handle error - FMOD_ErrorString(result)
		return;
	}

	// Set the volume if it is not 1.0
	// (2.0 is double volume, 0.0 is silent)
	if (volume != 1.0f)
	{
		if (volume < 0.0f)
			volume = 0.0f;

		result = FMOD_Channel_SetVolume(channel, volume);
		// TODO: handle error - FMOD_ErrorString(result)
	}

	// Set the pitch if it is not 1.0
	// (0.5 is half pitch, 2.0 is double pitch)
	if (pitch != 1.0f)
	{
		if (pitch < 0.0f)
			pitch = 0.0f;

		result = FMOD_Channel_SetPitch(channel, pitch);
		// TODO: handle error - FMOD_ErrorString(result)
	}

	// Tell the sound to loop if the loop count is not zero
	// (-1 means loop infinitely, >0 makes it loop that many times then stop)
	if (looping)
	{
		result = FMOD_Channel_SetMode(channel, FMOD_LOOP_NORMAL);
		// TODO: handle error - FMOD_ErrorString(result)
		result = FMOD_Channel_SetLoopCount(channel, -1);
		// TODO: handle error - FMOD_ErrorString(result)
	}

	// Resume playing the sound
	result = FMOD_Channel_SetPaused(channel, FMOD_FALSE);
	// TODO: handle error - FMOD_ErrorString(result)
}

CP_API void CP_Sound_PauseAll(void)
{
	for (unsigned index = 0; index < CP_SOUND_GROUP_MAX; ++index)
	{
		result = FMOD_ChannelGroup_SetPaused(channel_groups[index], FMOD_TRUE);
		if (result != FMOD_OK)
		{
			// TODO: handle error - FMOD_ErrorString(result)
		}
	}
}

CP_API void CP_Sound_PauseGroup(CP_SOUND_GROUP group)
{
	if(CP_IsValidSoundGroup(group))
	{
		result = FMOD_ChannelGroup_SetPaused(channel_groups[group], FMOD_TRUE);
		if (result != FMOD_OK)
		{
			// TODO: handle error - FMOD_ErrorString(result)
		}
	}
}

CP_API void CP_Sound_ResumeAll(void)
{
	for (unsigned index = 0; index < CP_SOUND_GROUP_MAX; ++index)
	{
		result = FMOD_ChannelGroup_SetPaused(channel_groups[index], FMOD_FALSE);
		if (result != FMOD_OK)
		{
			// TODO: handle error - FMOD_ErrorString(result)
		}
	}
}

CP_API void CP_Sound_ResumeGroup(CP_SOUND_GROUP group)
{
	if (CP_IsValidSoundGroup(group))
	{
		result = FMOD_ChannelGroup_SetPaused(channel_groups[group], FMOD_FALSE);
		if (result != FMOD_OK)
		{
			// TODO: handle error - FMOD_ErrorString(result)
		}
	}
}

CP_API void CP_Sound_StopAll(void)
{
	for (unsigned index = 0; index < CP_SOUND_GROUP_MAX; ++index)
	{
		result = FMOD_ChannelGroup_Stop(channel_groups[index]);
		if (result != FMOD_OK)
		{
			// TODO: handle error - FMOD_ErrorString(result)
		}
	}
}

CP_API void CP_Sound_StopGroup(CP_SOUND_GROUP group)
{
	if (CP_IsValidSoundGroup(group))
	{
		result = FMOD_ChannelGroup_Stop(channel_groups[group]);
		if (result != FMOD_OK)
		{
			// TODO: handle error - FMOD_ErrorString(result)
		}
	}
}

CP_API void CP_Sound_SetGroupVolume(CP_SOUND_GROUP group, float volume)
{
	if (CP_IsValidSoundGroup(group))
	{
		result = FMOD_ChannelGroup_SetVolume(channel_groups[group], volume);
		if (result != FMOD_OK)
		{
			// TODO: handle error - FMOD_ErrorString(result)
		}
	}
}

CP_API float CP_Sound_GetGroupVolume(CP_SOUND_GROUP group)
{
	float volume = 0;
	if (CP_IsValidSoundGroup(group))
	{
		result = FMOD_ChannelGroup_GetVolume(channel_groups[group], &volume);
		if (result != FMOD_OK)
		{
			// TODO: handle error - FMOD_ErrorString(result)
		}
	}
	return volume;
}

CP_API void CP_Sound_SetGroupPitch(CP_SOUND_GROUP group, float pitch)
{
	if (CP_IsValidSoundGroup(group))
	{
		result = FMOD_ChannelGroup_SetPitch(channel_groups[group], pitch);
		if (result != FMOD_OK)
		{
			// TODO: handle error - FMOD_ErrorString(result)
		}
	}
}

CP_API float CP_Sound_GetGroupPitch(CP_SOUND_GROUP group)
{
	float pitch = 0;
	if (CP_IsValidSoundGroup(group))
	{
		result = FMOD_ChannelGroup_GetPitch(channel_groups[group], &pitch);
		if (result != FMOD_OK)
		{
			// TODO: handle error - FMOD_ErrorString(result)
		}
	}
	return pitch;
}
