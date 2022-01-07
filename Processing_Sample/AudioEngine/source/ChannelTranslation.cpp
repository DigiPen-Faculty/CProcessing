//---------------------------------------------------------------------------------------------------------------------
// file:    ChannelTranslation.cpp
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#include "ChannelTranslation.h"
#include "AEMath.h"
#include "Definitions.h"

namespace AudioEngine
{
    namespace AudioChannelTranslation
    {
        using namespace AudioConstants;

        // Matrix columns are FrontLeft, FrontRight, Center, LowFreq, SideLeft, SideRight, BackLeft, BackRight

        const float cSqrt2Inv = 1.0f / sqrtf(2.0f);

        const float ChannelMatrix1[cMaxChannels] =
        {
          cSqrt2Inv, cSqrt2Inv, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.5f
        };

        const float ChannelMatrix2[cMaxChannels * 2] =
        {
          1.0f, 0.0f, cSqrt2Inv, 0.0f, cSqrt2Inv, 0.0f, cSqrt2Inv, 0.0f,
          0.0f, 1.0f, cSqrt2Inv, 0.0f, 0.0f, cSqrt2Inv, 0.0f, cSqrt2Inv
        };

        const float ChannelMatrix3[cMaxChannels * 3] =
        {
          1.0f, 0.0f, 0.0f, 0.0f, cSqrt2Inv, 0.0f, cSqrt2Inv, 0.0f,
          0.0f, 1.0f, 0.0f, 0.0f, 0.0f, cSqrt2Inv, 0.0f, cSqrt2Inv,
          0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
        };

        const float ChannelMatrix4[cMaxChannels * 4] =
        {
          1.0f, 0.0f, cSqrt2Inv, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 1.0f, cSqrt2Inv, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f
        };

        const float ChannelMatrix5[cMaxChannels * 5] =
        {
          1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f
        };

        const float ChannelMatrix6[cMaxChannels * 6] =
        {
          1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f
        };

        const float ChannelMatrix7[cMaxChannels * 7] =
        {
          1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
        };

        const float ChannelMatrix8[cMaxChannels * 8] =
        {
          1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
        };

        const float* Matrices[AudioConstants::cMaxChannels + 1] = {
            nullptr,
            ChannelMatrix1,
            ChannelMatrix2,
            ChannelMatrix3,
            ChannelMatrix4,
            ChannelMatrix5,
            ChannelMatrix6,
            ChannelMatrix7,
            ChannelMatrix8,
        };

    } // namespace AudioChannelTranslation


    //---------------------------------------------------------------------------------------------- ChannelTranslation

    //*****************************************************************************************************************
    void ChannelTranslation::Translate(const float* inputFrame, float* outputFrame, unsigned inputChannels, 
        unsigned outputChannels)
    {
        if (outputChannels != inputChannels)
        {
            float output[AudioConstants::cMaxChannels] = { 0 };

            // Down-mixing
            if (outputChannels < inputChannels)
            {
                for (unsigned outChannel = 0; outChannel < outputChannels; ++outChannel)
                {
                    for (unsigned i = 0; i < AudioConstants::cMaxChannels; ++i)
                    {
                        output[outChannel] += inputFrame[i] * 
                            AudioChannelTranslation::Matrices[outputChannels][i + (outChannel * AudioConstants::cMaxChannels)];
                    }
                }
            }
            // Up-mixing
            else
            {
                if (inputChannels == 1)
                {
                    for (unsigned i = 0; i < outputChannels; ++i)
                        output[i] = inputFrame[0];

                    output[LowFreq] = 0.0f;
                }
                else
                {
                    if (inputChannels < 5)
                    {
                        output[0] = inputFrame[0];
                        output[1] = inputFrame[1];

                        if (inputChannels == 3)
                            output[Center] = inputFrame[Center];
                        else
                            output[Center] = (inputFrame[0] + inputFrame[1]) * AudioChannelTranslation::cSqrt2Inv;

                        output[BackLeft] = output[BackRight] = output[SideLeft] = output[SideRight]
                            = (inputFrame[0] - inputFrame[1]) * AudioChannelTranslation::cSqrt2Inv;
                    }
                    else
                    {
                        memcpy(output, inputFrame, sizeof(float) * 6);
                        memcpy(output + BackLeft, inputFrame + SideLeft, sizeof(float) * 2);
                    }
                }
            }

            CopySamples(output, outputFrame, outputChannels);
        }
        else
            CopySamples(inputFrame, outputFrame, outputChannels);
    }

    //*****************************************************************************************************************
    void ChannelTranslation::CopySamples(const float* source, float* destination, unsigned channels)
    {
        switch (channels)
        {
        case 1:
            destination[0] = source[0];
            break;
        case 2:
            memcpy(destination, source, sizeof(float) * 2);
            break;
        case 3:
            memcpy(destination, source, sizeof(float) * 3);
            break;
        case 4:
            memcpy(destination, source, sizeof(float) * 2);
            memcpy(destination + 2, source + BackLeft, sizeof(float) * 2);
            break;
        case 5:
            memcpy(destination, source, sizeof(float) * 3);
            memcpy(destination + 3, source + SideLeft, sizeof(float) * 2);
            break;
        case 6:
            memcpy(destination, source, sizeof(float) * 6);
            break;
        case 7:
            memcpy(destination, source, sizeof(float) * 3);
            memcpy(destination + 3, source + SideLeft, sizeof(float) * 4);
            break;
        case 8:
            memcpy(destination, source, sizeof(float) * 8);
            break;
        default:
            break;
        }
    }
}
