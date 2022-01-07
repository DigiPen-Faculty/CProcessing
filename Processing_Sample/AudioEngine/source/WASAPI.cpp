//---------------------------------------------------------------------------------------------------------------------
// file:    WASAPI.cpp
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#include "Definitions.h"
#include "WASAPI.h"

#include <mmreg.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <functiondiscoverykeys.h>
#include <process.h>
#include <avrt.h>

#include <string>

#pragma comment(lib,"avrt.lib")

namespace AudioEngine
{

#define SAFE_CLOSE(h) if ((h) != nullptr) { CloseHandle((h)); (h) = nullptr; }
#define SAFE_RELEASE(object) if ((object) != nullptr) { (object)->Release(); (object) = nullptr; }
#define SAFE_FREE(object) if ((object) != nullptr) { CoTaskMemFree(object); (object) = nullptr; }

    const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
    const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
    const IID IID_IAudioClient = __uuidof(IAudioClient);
    const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

    //*****************************************************************************************************************
    std::string GetMessageForHresult(HRESULT hr)
    {
        if (HRESULT_FACILITY(hr) == FACILITY_WINDOWS)
            hr = HRESULT_CODE(hr);
        TCHAR* errorMessage;
        char fullMessage[100];

        if (FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            nullptr,
            hr,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&errorMessage,
            0,
            nullptr) != 0)
        {
            snprintf(fullMessage, 100, "Error 0x%08x: %s", hr, errorMessage);

            LocalFree(errorMessage);
        }
        else
            snprintf(fullMessage, 100, "Error 0x%08x", hr);

        return std::string(fullMessage);
    }

    //---------------------------------------------------------------------------------------------------- WasapiOutput

    //*****************************************************************************************************************
    WasapiOutput::WasapiOutput() :
        AudioClient(nullptr),
        Format(nullptr),
        RenderClient(nullptr),
        mBufferFrameCount(0),
        WasapiEvent(nullptr),
        StopRequest(nullptr),
        ThreadExit(nullptr),
        mStreamOpen(false),
        ClientCallback(nullptr),
        ClientData(nullptr)
    {

    }

    //*****************************************************************************************************************
    WasapiOutput::~WasapiOutput()
    {
        StopOutput();
        ReleaseData();
        CoUninitialize();
    }

    //*****************************************************************************************************************
    bool WasapiOutput::Initialize(StreamCallback* callback, void* clientData)
    {
        IMMDevice* device = nullptr;
        IMMDeviceEnumerator* enumerator = nullptr;

        HRESULT result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (FAILED(result))
            goto ErrorExit;

        // Create the device enumerator
        result = CoCreateInstance(CLSID_MMDeviceEnumerator, nullptr, CLSCTX_ALL, IID_IMMDeviceEnumerator,
            (void**)&enumerator);
        if (FAILED(result))
            goto ErrorExit;

        result = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
        if (FAILED(result))
            goto ErrorExit;

        DWORD deviceState;
        result = device->GetState(&deviceState);
        if (deviceState != DEVICE_STATE_ACTIVE)
            goto ErrorExit;

        result = device->Activate(IID_IAudioClient, CLSCTX_ALL, nullptr, (void**)&AudioClient);
        if (FAILED(result))
            goto ErrorExit;

        result = AudioClient->GetMixFormat(&Format);
        if (FAILED(result))
            goto ErrorExit;

        result = AudioClient->Initialize(
            AUDCLNT_SHAREMODE_SHARED,            // Shared mode, not exclusive
            AUDCLNT_STREAMFLAGS_EVENTCALLBACK,   // Use event callback mode
            0,                                   // Pass in zero to get smallest buffer size
            0,                                   // Period must be 0 in shared mode
            Format,                              // Device format
            nullptr);                            // Audio session GUID value
        if (FAILED(result))
            goto ErrorExit;

        result = AudioClient->GetBufferSize(&mBufferFrameCount);

        result = AudioClient->GetService(IID_IAudioRenderClient, (void**)&RenderClient);
        if (FAILED(result))
            goto ErrorExit;

        StopRequest = CreateEvent(nullptr, false, false, nullptr);
        ThreadExit = CreateEvent(nullptr, false, false, nullptr);
        WasapiEvent = CreateEvent(nullptr, false, false, nullptr);

        result = AudioClient->SetEventHandle(WasapiEvent);
        if (FAILED(result))
            goto ErrorExit;

        // Get the entire buffer from the RenderClient
        byte* data;
        result = RenderClient->GetBuffer(mBufferFrameCount, &data);
        if (FAILED(result))
            goto ErrorExit;
        // Set the buffer to zero
        memset(data, 0, mBufferFrameCount * sizeof(float));
        // Release the buffer
        result = RenderClient->ReleaseBuffer(mBufferFrameCount, 0);
        if (FAILED(result))
            goto ErrorExit;

        ClientCallback = callback;
        ClientData = clientData;

        _beginthreadex(nullptr, 0, &WasapiOutput::StartThread, this, 0, nullptr);

        mStreamOpen = true;

        SAFE_RELEASE(device);
        SAFE_RELEASE(enumerator);
        return true;

    ErrorExit:

        printf(GetMessageForHresult(result).c_str());
        ReleaseData();
        SAFE_RELEASE(device);
        SAFE_RELEASE(enumerator);
        return false;
    }

    //*****************************************************************************************************************
    void WasapiOutput::ProcessingLoop()
    {
        LPCTSTR name = "Audio";
        DWORD value = 0;
        HANDLE task = AvSetMmThreadCharacteristics(name, &value);

        AudioClient->Start();

        while (true)
        {
            // If we time out waiting for WASAPI, stop
            if (WaitForSingleObject(WasapiEvent, 1000) == WAIT_TIMEOUT)
                break;

            // Check if the event for stopping has been signaled
            if (WaitForSingleObject(StopRequest, 0) != WAIT_TIMEOUT)
                break;

            // Get the number of frames that currently have audio data
            unsigned frames;
            HRESULT result = AudioClient->GetCurrentPadding(&frames);

            if (result == S_OK)
            {
                // Frames to request are total frames minus frames with data
                frames = mBufferFrameCount - frames;

                // Get a buffer section from WASAPI
                byte* data;
                result = RenderClient->GetBuffer(frames, &data);

                // If successful, call the client callback function
                if (result == S_OK)
                    (*ClientCallback)((float*)data, frames, Format->nChannels, ClientData);

                // Release the buffer
                result = RenderClient->ReleaseBuffer(frames, 0);
            }
        }

        // Stop the audio client
        AudioClient->Stop();

        // Revert the thread priority
        AvRevertMmThreadCharacteristics(task);

        // Mark that the stream is closed
        mStreamOpen = false;

        // Set the thread exit event
        SetEvent(ThreadExit);
    }

    //*****************************************************************************************************************
    void WasapiOutput::StopOutput()
    {
        if (mStreamOpen)
            SignalObjectAndWait(StopRequest, ThreadExit, INFINITE, false);
    }

    //*****************************************************************************************************************
    unsigned WasapiOutput::GetChannels()
    {
        if (Format)
            return Format->nChannels;
        else
            return 0;
    }

    //*****************************************************************************************************************
    unsigned WasapiOutput::GetSampleRate()
    {
        if (Format)
            return Format->nSamplesPerSec;
        else
            return 0;
    }

    //*****************************************************************************************************************
    void WasapiOutput::ReleaseData()
    {
        SAFE_RELEASE(AudioClient);
        SAFE_RELEASE(RenderClient);

        SAFE_FREE(Format);

        SAFE_CLOSE(WasapiEvent);
        SAFE_CLOSE(StopRequest);
        SAFE_CLOSE(ThreadExit);
    }

    //*****************************************************************************************************************
    unsigned _stdcall WasapiOutput::StartThread(void* param)
    {
        if (param)
            ((WasapiOutput*)param)->ProcessingLoop();

        return 0;
    }
}
