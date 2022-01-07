//---------------------------------------------------------------------------------------------------------------------
// file:    SoundAsset.h
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#pragma once

#include "Definitions.h"
#include "FileDecoder.h"
#include "AudioEngine.h"

namespace AudioEngine
{
    //------------------------------------------------------------------------------------------------------ SoundAsset
    class SoundAsset
    {
    public:
        SoundAsset();
        ~SoundAsset();

        AE_RESULT Load(const char* fileName, bool streaming);
        AE_RESULT Play(InstanceIDType* newID, bool loop, bool startPaused, unsigned group);
        bool HasInstances() const;
        bool IsValid() const;

    private:
        void AppendSamplesThreaded(BufferType* buffer, unsigned howManySamples, unsigned startingIndex);
        // Called by the decoder to pass off decoded samples
        void DecodingCallback(DecodedPacket* packet);

        AtomicType mCurrentInstances_Shared = 0;
        AtomicType mSamplesAvailable_Shared = 0;
        AudioFileDecoder mDecoder;
        BufferType mSamples;
        bool mStreaming_Thr = false;
        unsigned mChannels = 0;
        unsigned mFrames = 0;

        friend class ThreadedSoundInstance;
        friend class AudioFileDecoder;
    };
}
