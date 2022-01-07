//---------------------------------------------------------------------------------------------------------------------
// file:    WASAPI.h
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#pragma once


struct IAudioClient;
struct tWAVEFORMATEX;
struct IAudioRenderClient;

namespace AudioEngine
{
    typedef void StreamCallback(float* outputBuffer, const unsigned frameCount, const unsigned channels, void* userData);

    //---------------------------------------------------------------------------------------------------- WasapiOutput

    class WasapiOutput
    {
    public:
        WasapiOutput();
        ~WasapiOutput();

        bool Initialize(StreamCallback* callback, void* clientData);
        void ProcessingLoop();
        void StopOutput();
        unsigned GetChannels();
        unsigned GetSampleRate();

    private:
        IAudioClient* AudioClient;
        tWAVEFORMATEX* Format;
        IAudioRenderClient* RenderClient;
        unsigned mBufferFrameCount;
        void* WasapiEvent;
        void* StopRequest;
        void* ThreadExit;
        bool mStreamOpen;
        StreamCallback* ClientCallback;
        void* ClientData;

        void ReleaseData();
        static unsigned _stdcall StartThread(void* param);
    };
}

