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
