//------------------------------------------------------------------------------
// file:	Internal_SoundLogic.h
// brief:	How a sound's own volume and pitch combine with its group's,
//			used by CP_Sound.c.
//
//			A sound in a group is heard at its own volume times the group's
//			volume, and plays at its own pitch times the group's pitch. To
//			keep that true when either one changes, each playing sound's
//			own volume and pitch are remembered here; what is given to
//			SoLoud is always recomputed from them, never from what SoLoud
//			holds now. (Scaling SoLoud's value by the change in the group
//			would lose a sound for good when a group goes to 0 and back.)
//
//			This file is only the bookkeeping, with no SoLoud and no globals,
//			so the Tier 1 test project tests it directly.
//
// Internal header: not part of the public API.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology
//------------------------------------------------------------------------------

#pragma once

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// SoLoud plays at most 1024 voices at once, and finished ones are dropped
// from this table only when it is full
#define CP_MAX_SOUND_VOICES 1024

typedef struct CP_SVL_Voice
{
	unsigned int handle; // SoLoud's handle: it is what CP_SoundInstance holds
	int group;
	float volume; // its own, as given to CP_Sound_PlayAdvanced or CP_Sound_SetVolume
	float pitch;  // its own, as given to CP_Sound_PlayAdvanced or CP_Sound_SetPitch
} CP_SVL_Voice;

typedef struct CP_SVL_Table
{
	CP_SVL_Voice voices[CP_MAX_SOUND_VOICES];
	unsigned int count;
} CP_SVL_Table;

// Does SoLoud still know this voice? (false once it has finished or been stopped)
typedef bool (*CP_SVL_IsAlive)(void* user, unsigned int handle);

// What is heard (or given to SoLoud): the sound's own value times its group's
static inline float CP_SVL_Combine(float own, float group)
{
	return own * group;
}

static inline CP_SVL_Voice* CP_SVL_Find(CP_SVL_Table* table, unsigned int handle)
{
	if (handle == 0)
	{
		return NULL;
	}
	for (unsigned int i = table->count; i > 0; --i) // newest first: the likely one
	{
		if (table->voices[i - 1].handle == handle)
		{
			return &table->voices[i - 1];
		}
	}
	return NULL;
}

// Forget the voices that have finished
static inline void CP_SVL_Prune(CP_SVL_Table* table, CP_SVL_IsAlive isAlive, void* user)
{
	unsigned int kept = 0;
	for (unsigned int i = 0; i < table->count; ++i)
	{
		if (isAlive(user, table->voices[i].handle))
		{
			table->voices[kept++] = table->voices[i];
		}
	}
	table->count = kept;
}

// Remember a voice that has just started. Returns NULL when there is no room
// even after dropping the finished ones (then it simply doesn't follow its
// group's changes, as a sound did before).
static inline CP_SVL_Voice* CP_SVL_Add(CP_SVL_Table* table, unsigned int handle, int group, float volume, float pitch, CP_SVL_IsAlive isAlive, void* user)
{
	if (handle == 0)
	{
		return NULL;
	}
	if (table->count >= CP_MAX_SOUND_VOICES)
	{
		CP_SVL_Prune(table, isAlive, user);
		if (table->count >= CP_MAX_SOUND_VOICES)
		{
			return NULL;
		}
	}
	CP_SVL_Voice* voice = &table->voices[table->count++];
	voice->handle = handle;
	voice->group = group;
	voice->volume = volume;
	voice->pitch = pitch;
	return voice;
}

#ifdef __cplusplus
}
#endif
