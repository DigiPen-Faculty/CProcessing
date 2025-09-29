//------------------------------------------------------------------------------
// file:	CP_Sound.c
// author:	Daniel Hamilton, Andrea Ellinger, Justin Chambers
// brief:	Load, play and manipulate sound files 
//
// Copyright © 2025 DigiPen, All rights reserved.
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

#define CP_INITIAL_SOUND_CAPACITY   12

VECT_GENERATE_TYPE(CP_Sound)

static Soloud* _soloud_system = NULL;

static int result = 0;

static vect_CP_Sound* sound_vector = NULL;

static CP_VoiceGroup_Struct voice_groups[CP_SOUND_GROUP_MAX] = { 0 };

//------------------------------------------------------------------------------
// SoLoud Helper Functions:
//------------------------------------------------------------------------------

static void SL_Sound_Release(CP_Sound sound)
{
	if (sound->type == SL_AUDIOSOURCE_STREAM)
	{
		WavStream_destroy((WavStream*)sound->sound);
	}
	else
	{
		Wav_destroy((Wav*)sound->sound);
	}
}

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

	// Create the SoLoud system
	_soloud_system = Soloud_create();
	result = Soloud_init(_soloud_system);

	if (result != 0)
	{
		// TODO: handle error - FMOD_ErrorString(result)
		CP_Sound_Shutdown();
		return;
	}
	// Create the channel groups (for stopping/pausing and controlling pitch and volume on a per group basis)
	for (unsigned index = 0; index < CP_SOUND_GROUP_MAX; ++index)
	{
		unsigned int voicegroup_handle = Soloud_createVoiceGroup(_soloud_system);
		voice_groups[index].handle = voicegroup_handle;
		voice_groups[index].volume = 1.0f;
		voice_groups[index].pitch = 1.0f;
	}
}

void CP_Sound_Update(void)
{

}

void CP_Sound_Shutdown(void)
{
	if (_soloud_system != NULL)
	{
		// Stop all current sounds
		CP_Sound_StopAll();

		// Free sounds 
		for (unsigned i = 0; i < sound_vector->size; ++i)
		{
			CP_Sound sound = vect_at_CP_Sound(sound_vector, i);
			// Release the sound from SoLoud
			SL_Sound_Release(sound);
			// Free the struct's memory
			free(sound);
		}

		// Free lists
		vect_free(sound_vector);

		// Release system
		Soloud_deinit(_soloud_system);
		Soloud_destroy(_soloud_system);
		_soloud_system = NULL;
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

	// Create the SoLoud sound
	if (streamFromDisc)
	{
		// TODO: move error checking up here so we can release SL memory
		WavStream* wavstream = WavStream_create();
		result = WavStream_load(wavstream, filepath);
		sound->sound = (AudioSource*)wavstream;
		sound->type = SL_AUDIOSOURCE_STREAM;
	}
	else
	{
		// TODO: move error checking up here so we can release SL memory
		Wav* wav = Wav_create();
		result = Wav_load(wav, filepath);
		sound->sound = (AudioSource*)wav;
		sound->type = SL_AUDIOSOURCE_WAV;
	}
	if (result != 0)
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

CP_API CP_Sound CP_Sound_LoadStream(const char* filepath)
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
			// Release the sound from SoLoud
			SL_Sound_Release((*sound));
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
	if (sound->type == SL_AUDIOSOURCE_STREAM)
	{
		CP_Sound_PlayAdvanced(sound, 1.0f, 1.0f, TRUE, CP_SOUND_GROUP_MUSIC);
	}
	else
	{
		CP_Sound_PlayAdvanced(sound, 1.0f, 1.0f, FALSE, CP_SOUND_GROUP_SFX);
	}
}

CP_API void CP_Sound_PlayAdvanced(CP_Sound sound, float volume, float pitch, CP_BOOL looping, CP_SOUND_GROUP group)
{
	// TODO: handle voice group
	if (!CP_IsValidSoundGroup(group) || sound == NULL)
	{
		return;
	}

	// Tell the sound to loop
	if (sound->type == SL_AUDIOSOURCE_STREAM)
	{
		WavStream_setLooping(sound->sound, looping);
	}
	else
	{
		Wav_setLooping(sound->sound, looping);
	}

	// Start the sound paused so we can set parameters on it
	unsigned int voice = Soloud_playEx(_soloud_system, sound->sound, volume * voice_groups[group].volume, 0, TRUE, 0);
	Soloud_addVoiceToGroup(_soloud_system, voice_groups[group].handle, voice);

	// Set the pitch if it is not 1.0
	// (0.5 is half pitch, 2.0 is double pitch)
	if (pitch != 1.0f)
	{
		if (pitch < 0.0f)
			pitch = 0.0f;

		result = Soloud_setRelativePlaySpeed(_soloud_system, voice, pitch * voice_groups[group].pitch);
	}

	// Resume playing the sound
	Soloud_setPause(_soloud_system, voice, FALSE);
	// TODO: handle error
}

CP_API void CP_Sound_PauseAll(void)
{
	Soloud_setPauseAll(_soloud_system, TRUE);
}

CP_API void CP_Sound_PauseGroup(CP_SOUND_GROUP group)
{
	if(CP_IsValidSoundGroup(group))
	{
		Soloud_setPause(_soloud_system, voice_groups[group].handle, TRUE);
	}
}

CP_API void CP_Sound_ResumeAll(void)
{
	Soloud_setPauseAll(_soloud_system, FALSE);
}

CP_API void CP_Sound_ResumeGroup(CP_SOUND_GROUP group)
{
	if (CP_IsValidSoundGroup(group))
	{
		Soloud_setPause(_soloud_system, voice_groups[group].handle, FALSE);
	}
}

CP_API void CP_Sound_StopAll(void)
{
	Soloud_stopAll(_soloud_system);
}

CP_API void CP_Sound_StopGroup(CP_SOUND_GROUP group)
{
	if (CP_IsValidSoundGroup(group))
	{
		Soloud_stop(_soloud_system, voice_groups[group].handle);
	}
}

CP_API void CP_Sound_SetGroupVolume(CP_SOUND_GROUP group, float volume)
{
	if (volume < 0)
	{
		volume = 0;
	}
	if (CP_IsValidSoundGroup(group))
	{
		Soloud_setVolume(_soloud_system, voice_groups[group].handle, volume);
		voice_groups[group].volume = volume;
	}
}

CP_API float CP_Sound_GetGroupVolume(CP_SOUND_GROUP group)
{
	if (CP_IsValidSoundGroup(group))
	{
		return voice_groups[group].volume;
	}
	return 1.0f;
}

CP_API void CP_Sound_SetGroupPitch(CP_SOUND_GROUP group, float pitch)
{
	if (pitch < 0)
	{
		pitch = 0;
	}
	if (CP_IsValidSoundGroup(group))
	{
		Soloud_setRelativePlaySpeed(_soloud_system, voice_groups[group].handle, pitch);
		voice_groups[group].pitch = pitch;
	}
}

CP_API float CP_Sound_GetGroupPitch(CP_SOUND_GROUP group)
{
	if (CP_IsValidSoundGroup(group))
	{
		return voice_groups[group].pitch;
	}
	return 1.0f;
}
