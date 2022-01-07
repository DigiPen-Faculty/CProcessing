//---------------------------------------------------------------------------------------------------------------------
// file:    Mixer.cpp
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#include "Mixer.h"
#include "SoundInstance.h"
#include "Functor.hpp"
#include "Atomics.h"

#include <process.h>

namespace AudioEngine
{
    Mixer* gMixer = nullptr;

    //*****************************************************************************************************************
    void AudioCallback(float* outputBuffer, const unsigned frameCount, const unsigned channels, void* userData)
    {
        // Cast the pointer to the mixer
        Mixer* audioMixer = (Mixer*)userData;
        // Get the available samples from the ring buffer
        unsigned samplesNeeded = frameCount * channels;
        unsigned samples = audioMixer->OutputRingBuffer.Read(outputBuffer, samplesNeeded);
        // If not enough available, set the rest to 0
        if (samples < samplesNeeded)
            memset(outputBuffer + samples, 0, sizeof(float) * (samplesNeeded - samples));
        // Notify the mix thread to create more audio data
        ReleaseSemaphore(audioMixer->Events[MixEventTypes::MixSemaphore], 1, nullptr);
    }

    //----------------------------------------------------------------------------------------------------------- Mixer

    //*****************************************************************************************************************
    Mixer::Mixer()
    {
        gMixer = this;

        // Create the thread events
        Events[MixEventTypes::MixSemaphore] = CreateSemaphore(nullptr, 1, 10, nullptr);
        for (int i = 1; i < MixEventTypes::Count; ++i)
            Events[i] = CreateEvent(nullptr, false, false, nullptr);

        // Set up the ring buffer
        OutputRingBuffer.Initialize(sizeof(float), cBufferSize, (void*)RingBuffer);
    }

    //*****************************************************************************************************************
    Mixer::~Mixer()
    {
        if (mMixing)
            ShutDown();

        for (int i = 0; i < MAX_GROUPS; ++i)
        {
            while (!InstancesByGroup_Thr[i].Empty())
            {
                ThreadedSoundInstance* temp = InstancesByGroup_Thr[i].PopFront();
                delete temp;
            }
        }

        // Close the thread events
        for (int i = 0; i < MixEventTypes::Count; ++i)
            CloseHandle(Events[i]);

        std::vector<Functor*>* list = &TasksForMixThread[0];
        for (unsigned i = 0; i < list->size(); ++i)
            delete (*list)[i];
        list = &TasksForMixThread[1];
        for (unsigned i = 0; i < list->size(); ++i)
            delete (*list)[i];
        list = &TasksForMainThread[0];
        for (unsigned i = 0; i < list->size(); ++i)
            delete (*list)[i];
        list = &TasksForMainThread[1];
        for (unsigned i = 0; i < list->size(); ++i)
            delete (*list)[i];
    }

    //*****************************************************************************************************************
    AE_RESULT Mixer::Initialize()
    {
        mMixing = AudioOutput.Initialize(&AudioCallback, this);
        if (mMixing)
        {
            mChannels = AudioOutput.GetChannels();
            mSampleRate = AudioOutput.GetSampleRate();
            _beginthreadex(nullptr, 0, &Mixer::StartThread, this, 0, nullptr);
            return AE_OK;
        }
        else
            return AE_ERROR_STARTING_AUDIO_OUTPUT;
    }

    //*****************************************************************************************************************
    void Mixer::ShutDown()
    {
        if (!mMixing)
            return;

        SignalObjectAndWait(Events[MixEventTypes::StopMixing], Events[MixEventTypes::MixThreadExit],
            INFINITE, false);
        AudioOutput.StopOutput();

        mMixing = false;
    }

    //*****************************************************************************************************************
    void Mixer::Update()
    {
        // Swap the task buffer indexes
        mMainTasksReadIndex = mMainTasksWriteIndex;
        AtomicExchange(&mMainTasksWriteIndex, (mMainTasksReadIndex + 1) % 2);

        std::vector<Functor*>& taskBuffer = TasksForMainThread[mMainTasksReadIndex];

        // Execute all tasks
        for (unsigned i = 0; i < taskBuffer.size(); ++i)
        {
            taskBuffer[i]->Execute();
            delete taskBuffer[i];
        }

        taskBuffer.clear();
    }

    //*****************************************************************************************************************
    void Mixer::AddTaskForMixThread(Functor* task)
    {
        TasksForMixThread[mMixTasksWriteIndex].push_back(task);
    }

    //*****************************************************************************************************************
    void Mixer::AddTaskForMainThread(Functor* task)
    {
        TasksForMainThread[mMainTasksWriteIndex].push_back(task);
    }

    //*****************************************************************************************************************
    void Mixer::MixLoopThreaded()
    {
        // Buffer to get data from sound instances
        BufferType instanceBuffer(cBufferSize);
        // Buffer to accumulate mixed data
        BufferType mixingBuffer(cBufferSize);
        // Buffer to accumulate data for a group
        BufferType groupBuffer(cBufferSize);


        while (true)
        {
            // Wait until an event is signaled
            DWORD result = WaitForMultipleObjects(MixEventTypes::Count, Events, false, INFINITE);

            if (result == MixEventTypes::StopMixing)
                break;

            // Swap the task buffer indexes
            mMixTasksReadIndex = mMixTasksWriteIndex;
            AtomicExchange(&mMixTasksWriteIndex, (mMixTasksReadIndex + 1) % 2);

            std::vector<Functor*>& taskBuffer = TasksForMixThread[mMixTasksReadIndex];

            // Execute all tasks
            for (unsigned i = 0; i < taskBuffer.size(); ++i)
            {
                taskBuffer[i]->Execute();
                delete taskBuffer[i];
            }

            taskBuffer.clear();

            // Find out how many samples can be written to the ring buffer
            unsigned samplesToWrite = OutputRingBuffer.GetWriteAvailable();

            if (samplesToWrite == 0)
                continue;

            // Reset data in mixing buffer
            mixingBuffer.clear();
            mixingBuffer.resize(samplesToWrite, 0.0f);

            // Walk through the list of sound instances
            for (int group = 0; group < MAX_GROUPS; ++group)
            {
                if (InstancesByGroup_Thr[group].Empty())
                    continue;

                InstanceGroupData& data = DataPerGroup[group];
                unsigned groupSamples = samplesToWrite;

                // Check if we are pitch shifting the group
                if (data.mPitchShifting_Thr)
                {
                    data.PitchHandler_Thr.CalculateBufferSize(samplesToWrite, mChannels);
                    groupSamples = data.PitchHandler_Thr.GetInputSampleCount();
                }

                // Reset data in group buffer
                groupBuffer.clear();
                groupBuffer.resize(groupSamples, 0.0f);

                // Store the number of frames
                unsigned frameCount = groupSamples / mChannels;

                for (InList<ThreadedSoundInstance>::iterator it = InstancesByGroup_Thr[group].begin();
                    it != InstancesByGroup_Thr[group].end(); )
                {
                    // Get the data from the sound instance
                    bool stillPlaying = it->GetNextSamples(instanceBuffer.data(), frameCount, mChannels);

                    // Add all samples to the mixing buffer
                    for (unsigned i = 0; i < groupSamples; ++i)
                        groupBuffer[i] += instanceBuffer[i] * cBaseVolume;

                    // If the instance is still playing, move to the next in the list
                    if (stillPlaying)
                        ++it;
                    // If it's not playing, remove it
                    else
                    {
                        // Save the instance to be removed
                        ThreadedSoundInstance* toRemove = *it;
                        // Move to the next in the list
                        ++it;
                        // Remove the instance from the list
                        RemoveInstanceThreaded(toRemove, group);
                        // Tell the main thread to delete this sound instance
                        AddTaskForMainThread(CreateFunctor(&Mixer::DeleteInstance, this, toRemove->mID));
                    }
                }

                // Apply pitch change if needed
                if (data.mPitchShifting_Thr)
                {
                    BufferType adjustedSamples(samplesToWrite);
                    data.PitchHandler_Thr.ProcessBuffer(&groupBuffer, &adjustedSamples);
                    groupBuffer.swap(adjustedSamples);
                }

                // Check if we need to apply a volume change
                if (!data.VolumeInterpolator_Thr.Finished() || !AudioConstants::IsWithinLimit(data.mVolume_Thr, 1.0f, 0.01f))
                {
                    // If not interpolating, apply the volume as is
                    if (data.VolumeInterpolator_Thr.Finished())
                    {
                        for (unsigned i = 0; i < cBufferSize; ++i)
                            groupBuffer[i] *= data.mVolume_Thr;
                    }
                    // Otherwise, apply volume to each frame
                    else
                    {
                        for (unsigned i = 0; i < cBufferSize; i += mChannels)
                        {
                            data.mVolume_Thr = data.VolumeInterpolator_Thr.NextValue();
                            for (unsigned j = 0; j < mChannels; ++j)
                                groupBuffer[i + j] *= data.mVolume_Thr;
                        }
                    }
                }

                // Add group samples to the mixed buffer
                for (unsigned i = 0; i < samplesToWrite; ++i)
                    mixingBuffer[i] += groupBuffer[i];
            }

            // Write mixed data to the ring buffer
            OutputRingBuffer.Write(mixingBuffer.data(), samplesToWrite);
        }

        SetEvent(Events[MixEventTypes::MixThreadExit]);
    }

    //*****************************************************************************************************************
    unsigned _stdcall Mixer::StartThread(void* param)
    {
        ((Mixer*)param)->MixLoopThreaded();

        return 0;
    }

    //*****************************************************************************************************************
    SoundInstance* Mixer::FindInstance(InstanceIDType instanceID)
    {
        InstanceMapType::iterator it = InstanceMap.find(instanceID);
        if (it != InstanceMap.end())
            return it->second;
        else
            return nullptr;
    }

    //*****************************************************************************************************************
    AE_RESULT Mixer::GetNewInstanceGroup(GroupIDType* id)
    {
        for (unsigned i = 1; i < MAX_GROUPS; ++i)
        {
            if (!DataPerGroup[i].mInUse)
            {
                DataPerGroup->mInUse = true;
                *id = i;
                return AE_OK;
            }
        }

        *id = MAX_GROUPS;
        return AE_MAXIMUM_GROUPS_REACHED;
    }

    //*****************************************************************************************************************
    AE_RESULT Mixer::ReleaseInstanceGroup(GroupIDType groupID)
    {
        if (groupID >= MAX_GROUPS)
            return AE_INVALID_GROUP_ID;

        DataPerGroup[groupID].mInUse = false;

        return AE_OK;
    }

    //*****************************************************************************************************************
    AE_RESULT Mixer::SetGroupPaused(GroupIDType groupID, bool paused)
    {
        if (groupID >= MAX_GROUPS)
            return AE_INVALID_GROUP_ID;

        AddTaskForMixThread(CreateFunctor(&Mixer::SetGroupPausedThreaded, this, groupID, paused));

        return AE_OK;
    }

    //*****************************************************************************************************************
    AE_RESULT Mixer::StopGroup(GroupIDType groupID)
    {
        if (groupID >= MAX_GROUPS)
            return AE_INVALID_GROUP_ID;

        AddTaskForMixThread(CreateFunctor(&Mixer::StopGroupThreaded, this, groupID));

        return AE_OK;
    }

    //*****************************************************************************************************************
    AE_RESULT Mixer::GetGroupVolume(GroupIDType groupID, float* volume)
    {
        if (groupID >= MAX_GROUPS)
            return AE_INVALID_GROUP_ID;

        *volume = DataPerGroup[groupID].mVolume;

        return AE_OK;
    }

    //*****************************************************************************************************************
    AE_RESULT Mixer::SetGroupVolume(GroupIDType groupID, float volume)
    {
        if (groupID >= MAX_GROUPS)
            return AE_INVALID_GROUP_ID;

        DataPerGroup[groupID].mVolume = volume;
        AddTaskForMixThread(CreateFunctor(&Mixer::SetGroupVolumeThreaded, this, groupID, volume));

        return AE_OK;
    }

    //*****************************************************************************************************************
    AE_RESULT Mixer::GetGroupPitch(GroupIDType groupID, float* pitch)
    {
        if (groupID >= MAX_GROUPS)
            return AE_INVALID_GROUP_ID;

        *pitch = DataPerGroup[groupID].mPitch;

        return AE_OK;
    }

    //*****************************************************************************************************************
    AE_RESULT Mixer::SetGroupPitch(GroupIDType groupID, float pitch)
    {
        if (groupID >= MAX_GROUPS)
            return AE_INVALID_GROUP_ID;

        DataPerGroup[groupID].mPitch = pitch;
        AddTaskForMixThread(CreateFunctor(&Mixer::SetGroupPitchThreaded, this, groupID, pitch));

        return AE_OK;
    }

    //*****************************************************************************************************************
    void Mixer::AddInstanceThreaded(ThreadedSoundInstance* instance, unsigned group)
    {
        if (group >= MAX_GROUPS)
            return;

        InstancesByGroup_Thr[group].PushBack(instance);
    }

    //*****************************************************************************************************************
    void Mixer::RemoveInstanceThreaded(ThreadedSoundInstance* instance, unsigned group)
    {
        if (group >= MAX_GROUPS)
            return;

        InstancesByGroup_Thr[group].Remove(instance);
    }

    //*****************************************************************************************************************
    void Mixer::DeleteInstance(InstanceIDType instanceID)
    {
        SoundInstance* instance = FindInstance(instanceID);
        if (instance)
            delete instance;
    }

    //*****************************************************************************************************************
    void Mixer::SetGroupPausedThreaded(GroupIDType groupID, bool paused)
    {
        for (ThreadedSoundInstance* instance : InstancesByGroup_Thr[groupID])
        {
            if (paused)
                AtomicExchange(&(instance->mPauseFlag_Shared), 1);
            else
                AtomicExchange(&(instance->mResumeFlag_Shared), 1);
        }
    }

    //*****************************************************************************************************************
    void Mixer::StopGroupThreaded(GroupIDType groupID)
    {
        for (ThreadedSoundInstance* instance : InstancesByGroup_Thr[groupID])
        {
            AtomicExchange(&(instance->mStopFlag_Shared), 1);
        }
    }

    //*****************************************************************************************************************
    void Mixer::SetGroupVolumeThreaded(GroupIDType groupID, float volume)
    {
        if (volume < 0.0f)
            volume = 0.0f;

        DataPerGroup[groupID].VolumeInterpolator_Thr.SetValues(volume, AudioConstants::cPropertyChangeFrames);
    }

    //*****************************************************************************************************************
    void Mixer::SetGroupPitchThreaded(GroupIDType groupID, float pitch)
    {
        // If the pitch factor didn't change, don't do anything
        if (AudioConstants::IsWithinLimit(pitch - DataPerGroup[groupID].PitchHandler_Thr.GetPitchFactor(), 0.0f, 0.01f))
            return;

        DataPerGroup[groupID].mPitchShifting_Thr = true;
        DataPerGroup[groupID].PitchHandler_Thr.SetPitchFactor(pitch, 0.01f);
    }

}
