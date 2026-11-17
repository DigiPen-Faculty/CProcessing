// Tier 1 coverage for Source/Internal_SoundLogic.h: how a sound's own volume
// and pitch combine with its group's. A small stand-in for SoLoud holds what
// SoLoud was last given for each voice, and the tests do what CP_Sound.c
// does with the table: play, change a group, change one sound.
#include <stdbool.h>
#include <stdio.h>
#include "unity.h"
#include "Internal_SoundLogic.h"

#define GROUPS 3

typedef struct FakeSoloud
{
	bool alive[64];
	float volume[64];
	float speed[64];
} FakeSoloud;

static CP_SVL_Table table;
static FakeSoloud soloud;
static float groupVolume[GROUPS];
static float groupPitch[GROUPS];

static bool IsAlive(void* user, unsigned int handle)
{
	return ((FakeSoloud*)user)->alive[handle];
}

static void Reset(void)
{
	table.count = 0;
	for (int i = 0; i < 64; ++i)
	{
		soloud.alive[i] = false;
		soloud.volume[i] = soloud.speed[i] = 0.0f;
	}
	for (int g = 0; g < GROUPS; ++g)
	{
		groupVolume[g] = groupPitch[g] = 1.0f;
	}
}

// CP_Sound_PlayAdvanced
static void Play(unsigned int handle, int group, float volume, float pitch)
{
	soloud.alive[handle] = true;
	soloud.volume[handle] = CP_SVL_Combine(volume, groupVolume[group]);
	soloud.speed[handle] = CP_SVL_Combine(pitch, groupPitch[group]);
	CP_SVL_Add(&table, handle, group, volume, pitch, IsAlive, &soloud);
}

static void Apply(const CP_SVL_Voice* v)
{
	soloud.volume[v->handle] = CP_SVL_Combine(v->volume, groupVolume[v->group]);
	soloud.speed[v->handle] = CP_SVL_Combine(v->pitch, groupPitch[v->group]);
}

static void ApplyGroup(int group)
{
	for (unsigned int i = 0; i < table.count; ++i)
	{
		if (table.voices[i].group == group)
		{
			Apply(&table.voices[i]);
		}
	}
}

// CP_Sound_SetGroupVolume and CP_Sound_SetGroupPitch
static void SetGroupVolume(int group, float volume)
{
	groupVolume[group] = volume;
	ApplyGroup(group);
}

static void SetGroupPitch(int group, float pitch)
{
	groupPitch[group] = pitch;
	ApplyGroup(group);
}

// CP_Sound_SetVolume and CP_Sound_SetPitch
static void SetVolume(unsigned int handle, float volume)
{
	CP_SVL_Voice* v = CP_SVL_Find(&table, handle);
	if (v)
	{
		v->volume = volume;
		Apply(v);
	}
}

static void SetPitch(unsigned int handle, float pitch)
{
	CP_SVL_Voice* v = CP_SVL_Find(&table, handle);
	if (v)
	{
		v->pitch = pitch;
		Apply(v);
	}
}

void test_soundlogic_combine_multiplies(void)
{
	TEST_ASSERT_EQUAL_FLOAT(0.25f, CP_SVL_Combine(0.5f, 0.5f));
	TEST_ASSERT_EQUAL_FLOAT(0.0f, CP_SVL_Combine(0.5f, 0.0f));
	TEST_ASSERT_EQUAL_FLOAT(0.5f, CP_SVL_Combine(0.5f, 1.0f));
}

void test_soundlogic_a_sound_played_in_a_group_has_its_volume_times_the_groups(void)
{
	Reset();
	groupVolume[1] = 0.5f;
	Play(1, 1, 0.5f, 1.0f);
	TEST_ASSERT_EQUAL_FLOAT(0.25f, soloud.volume[1]);
}

void test_soundlogic_changing_a_group_keeps_each_sounds_own_volume(void)
{
	Reset();
	Play(1, 0, 1.0f, 1.0f);
	Play(2, 0, 0.5f, 1.0f);
	Play(3, 0, 0.2f, 1.0f);
	SetGroupVolume(0, 0.5f);
	TEST_ASSERT_EQUAL_FLOAT(0.5f, soloud.volume[1]);
	TEST_ASSERT_EQUAL_FLOAT(0.25f, soloud.volume[2]);
	TEST_ASSERT_EQUAL_FLOAT(0.1f, soloud.volume[3]);
}

void test_soundlogic_a_group_at_zero_and_back_loses_nothing(void)
{
	Reset();
	Play(1, 0, 0.5f, 1.0f);
	Play(2, 0, 0.8f, 1.0f);
	SetGroupVolume(0, 0.0f);
	TEST_ASSERT_EQUAL_FLOAT(0.0f, soloud.volume[1]);
	TEST_ASSERT_EQUAL_FLOAT(0.0f, soloud.volume[2]);
	SetGroupVolume(0, 1.0f);
	TEST_ASSERT_EQUAL_FLOAT(0.5f, soloud.volume[1]);
	TEST_ASSERT_EQUAL_FLOAT(0.8f, soloud.volume[2]);
}

void test_soundlogic_many_group_changes_do_not_drift(void)
{
	Reset();
	Play(1, 0, 0.7f, 1.0f);
	for (int i = 0; i < 1000; ++i)
	{
		SetGroupVolume(0, 0.3f);
		SetGroupVolume(0, 0.9f);
	}
	SetGroupVolume(0, 0.5f);
	TEST_ASSERT_EQUAL_FLOAT(0.35f, soloud.volume[1]);
}

void test_soundlogic_changing_one_sound_is_still_times_its_group(void)
{
	Reset();
	Play(1, 0, 1.0f, 1.0f);
	SetGroupVolume(0, 0.5f);
	SetVolume(1, 0.8f);
	TEST_ASSERT_EQUAL_FLOAT(0.4f, soloud.volume[1]);
	SetGroupVolume(0, 1.0f);
	TEST_ASSERT_EQUAL_FLOAT(0.8f, soloud.volume[1]);
}

void test_soundlogic_a_group_change_leaves_other_groups_alone(void)
{
	Reset();
	Play(1, 0, 0.5f, 1.0f);
	Play(2, 1, 0.5f, 1.0f);
	SetGroupVolume(0, 0.2f);
	SetGroupPitch(0, 2.0f);
	TEST_ASSERT_EQUAL_FLOAT(0.1f, soloud.volume[1]);
	TEST_ASSERT_EQUAL_FLOAT(0.5f, soloud.volume[2]);
	TEST_ASSERT_EQUAL_FLOAT(2.0f, soloud.speed[1]);
	TEST_ASSERT_EQUAL_FLOAT(1.0f, soloud.speed[2]);
}

void test_soundlogic_pitch_is_the_sounds_own_times_the_groups(void)
{
	Reset();
	Play(1, 0, 1.0f, 2.0f);
	SetGroupPitch(0, 0.5f);
	TEST_ASSERT_EQUAL_FLOAT(1.0f, soloud.speed[1]);
	SetPitch(1, 1.5f);
	TEST_ASSERT_EQUAL_FLOAT(0.75f, soloud.speed[1]);
	SetGroupPitch(0, 1.0f);
	TEST_ASSERT_EQUAL_FLOAT(1.5f, soloud.speed[1]);
}

void test_soundlogic_a_sound_that_finished_is_not_found_after_pruning(void)
{
	Reset();
	Play(1, 0, 0.5f, 1.0f);
	Play(2, 0, 0.5f, 1.0f);
	soloud.alive[1] = false;
	CP_SVL_Prune(&table, IsAlive, &soloud);
	TEST_ASSERT_EQUAL_UINT(1, table.count);
	TEST_ASSERT_NULL(CP_SVL_Find(&table, 1));
	TEST_ASSERT_NOT_NULL(CP_SVL_Find(&table, 2));
}

void test_soundlogic_no_instance_is_never_found_or_added(void)
{
	Reset();
	Play(1, 0, 0.5f, 1.0f);
	TEST_ASSERT_NULL(CP_SVL_Find(&table, 0));
	TEST_ASSERT_NULL(CP_SVL_Add(&table, 0, 0, 1.0f, 1.0f, IsAlive, &soloud));
	TEST_ASSERT_EQUAL_UINT(1, table.count);
}

// A full table: sounds with an odd handle are still playing, the rest have finished
static bool AliveIfOdd(void* user, unsigned int handle)
{
	(void)user;
	return (handle & 1u) != 0;
}

void test_soundlogic_a_full_table_drops_finished_sounds_to_make_room(void)
{
	Reset();
	for (unsigned int h = 1; h <= CP_MAX_SOUND_VOICES; ++h)
	{
		TEST_ASSERT_NOT_NULL(CP_SVL_Add(&table, h, 0, 0.5f, 1.0f, AliveIfOdd, NULL));
	}
	TEST_ASSERT_EQUAL_UINT(CP_MAX_SOUND_VOICES, table.count);
	CP_SVL_Voice* added = CP_SVL_Add(&table, CP_MAX_SOUND_VOICES + 1, 0, 0.25f, 1.0f, AliveIfOdd, NULL);
	TEST_ASSERT_NOT_NULL(added);
	TEST_ASSERT_EQUAL_UINT(CP_MAX_SOUND_VOICES / 2 + 1, table.count);
	TEST_ASSERT_NOT_NULL(CP_SVL_Find(&table, 1));
	TEST_ASSERT_NULL(CP_SVL_Find(&table, 2));
	TEST_ASSERT_EQUAL_FLOAT(0.25f, CP_SVL_Find(&table, CP_MAX_SOUND_VOICES + 1)->volume);
}

void test_soundlogic_a_table_full_of_playing_sounds_takes_no_more(void)
{
	Reset();
	for (unsigned int h = 1; h <= CP_MAX_SOUND_VOICES; ++h)
	{
		CP_SVL_Add(&table, h * 2 - 1, 0, 0.5f, 1.0f, AliveIfOdd, NULL);
	}
	TEST_ASSERT_NULL(CP_SVL_Add(&table, 4001, 0, 1.0f, 1.0f, AliveIfOdd, NULL));
	TEST_ASSERT_EQUAL_UINT(CP_MAX_SOUND_VOICES, table.count);
}
