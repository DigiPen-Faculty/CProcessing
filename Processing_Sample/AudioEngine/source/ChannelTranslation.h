//---------------------------------------------------------------------------------------------------------------------
// file:    ChannelTranslation.h
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#pragma once

namespace AudioEngine
{
    //---------------------------------------------------------------------------------------------- ChannelTranslation

    class ChannelTranslation
    {
    public:

        static void Translate(const float* inputFrame, float* outputFrame, unsigned inputChannels, unsigned outputChannels);

    private:
        enum Channels { FrontLeft, FrontRight, Center, LowFreq, SideLeft, SideRight, BackLeft, BackRight };
        static void CopySamples(const float* source, float* destination, unsigned channels);
    };
}
