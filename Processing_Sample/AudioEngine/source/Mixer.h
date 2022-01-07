//---------------------------------------------------------------------------------------------------------------------
// file:    Mixer.h
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#pragma once

#include "AEMath.h"
#include "RingBuffer.h"
#include <unordered_map>
#include <vector>
#include "InList.hpp"
#include "WASAPI.h"
#include "AudioEngine.h"
#include "VolumeModifier.h"
#include "PitchChange.h"

namespace AudioEngine
{
    void AudioCallback(float* outputBuffer, const unsigned frameCount, const unsigned channels, void* userData);

    namespace MixEventTypes
    {
        enum Enum { MixSemaphore, StopMixing, MixThreadExit, Count };
    }

    class SoundAsset;
    class SoundInstance;
    class ThreadedSoundInstance;
    class Functor;

    typedef std::unordered_map<InstanceIDType, SoundInstance*> InstanceMapType;

#define MAX_GROUPS 20

    struct InstanceGroupData
    {
        bool mPitchShifting_Thr = false;
        float mVolume_Thr = 1.0f;
        Interpolator VolumeInterpolator_Thr;
        PitchChangeHandler PitchHandler_Thr;
        bool mInUse = false;
        float mVolume = 1.0f;
        float mPitch = 1.0f;
    };

    //----------------------------------------------------------------------------------------------------------- Mixer
    class Mixer
    {
    public:
        Mixer();
        ~Mixer();

        AE_RESULT Initialize();
        void ShutDown();
        void Update();
        void AddTaskForMixThread(Functor* task);
        void AddTaskForMainThread(Functor* task);
        void MixLoopThreaded();
        static unsigned _stdcall StartThread(void* param);
        unsigned GetSampleRate() const { return mSampleRate; }
        SoundInstance* FindInstance(InstanceIDType instanceID);
        AE_RESULT GetNewInstanceGroup(GroupIDType* id);
        AE_RESULT ReleaseInstanceGroup(GroupIDType groupID);
        AE_RESULT SetGroupPaused(GroupIDType groupID, bool paused);
        AE_RESULT StopGroup(GroupIDType groupID);
        AE_RESULT GetGroupVolume(GroupIDType groupID, float* volume);
        AE_RESULT SetGroupVolume(GroupIDType groupID, float volume);
        AE_RESULT GetGroupPitch(GroupIDType groupID, float* pitch);
        AE_RESULT SetGroupPitch(GroupIDType groupID, float pitch);

        void* Events[MixEventTypes::Count];
        RingBuffer OutputRingBuffer;

    private:
        static const unsigned cBufferSize = 4096;
        const float cBaseVolume = 0.8f;

        float RingBuffer[cBufferSize];
        InstanceMapType InstanceMap;
        InstanceIDType mNextInstanceID = 1;
        bool mMixing = false;
        Vec3 mListenerPosition;
        unsigned mChannels = 0;
        unsigned mSampleRate = 0;
        InList<ThreadedSoundInstance> InstancesByGroup_Thr[MAX_GROUPS];
        InstanceGroupData DataPerGroup[MAX_GROUPS];

        std::vector<Functor*> TasksForMixThread[2];
        AtomicType mMixTasksReadIndex = 0;
        AtomicType mMixTasksWriteIndex = 1;
        std::vector<Functor*> TasksForMainThread[2];
        AtomicType mMainTasksReadIndex = 0;
        AtomicType mMainTasksWriteIndex = 1;

        WasapiOutput AudioOutput;

        void AddInstanceThreaded(ThreadedSoundInstance* instance, unsigned group);
        void RemoveInstanceThreaded(ThreadedSoundInstance* instance, unsigned group);
        void DeleteInstance(InstanceIDType instanceID);
        void SetGroupPausedThreaded(GroupIDType groupID, bool paused);
        void StopGroupThreaded(GroupIDType groupID);
        void SetGroupVolumeThreaded(GroupIDType groupID, float volume);
        void SetGroupPitchThreaded(GroupIDType groupID, float pitch);

        friend class ThreadedSoundInstance;
        friend class SoundInstance;
        friend class SoundAsset;
    };

    extern Mixer* gMixer;
}

