// CP_Sound: no reliable way to assert "it sounds correct" automatically
// (06-test-suite-plan.md), so this covers load/free/play/pause/stop/resume
// not crashing (if Scn_SoundRoundTrip had crashed, this whole test binary
// would never have reached UNITY_BEGIN()) plus group volume/pitch
// round-tripping what was set.
#include "unity.h"
#include "tier2_capture.h"

void test_tier2_sound_group_volume_roundtrips(void)
{
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.3f, tier2_scalars.volumeAfterSet);
}

void test_tier2_sound_group_pitch_roundtrips(void)
{
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.5f, tier2_scalars.pitchAfterSet);
}

void test_tier2_sound_play_returns_an_instance_that_stops_one_sound(void)
{
    // See Scn_SoundRoundTrip (needs an audio device, or SoLoud's silent
    // fallback, to have loaded the sound at all)
    if (!tier2_scalars.soundLoaded)
    {
        TEST_IGNORE_MESSAGE("no sound loaded on this machine");
    }
    TEST_ASSERT_TRUE(tier2_scalars.instanceGiven);
    TEST_ASSERT_TRUE(tier2_scalars.playingAfterPlay);
    TEST_ASSERT_TRUE(tier2_scalars.playingAfterPauseResume);
    TEST_ASSERT_FALSE(tier2_scalars.playingAfterStop);
    TEST_ASSERT_TRUE(tier2_scalars.otherPlayingAfterStop);
}

void test_tier2_sound_no_instance_is_not_playing_and_is_safe(void)
{
    TEST_ASSERT_TRUE(tier2_scalars.noInstanceIsNotPlaying);
}
