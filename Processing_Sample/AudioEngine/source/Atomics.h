//---------------------------------------------------------------------------------------------------------------------
// file:    Atomics.h
// author:  
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#pragma once

#include "Definitions.h"


namespace AudioEngine
{
    // Returns the initial value of the target before it was changed.
    AtomicType AtomicExchange(volatile AtomicType* target, AtomicType value);

    // If Destination == Comparand, Destination is set to Exchange. Returns the initial value of Destination.
    AtomicType AtomicCompareExchange(volatile AtomicType* destination, AtomicType exchange,
        AtomicType comperand);

    // Returns the initial value of the target before it was changed.
    void* AtomicSetPointer(void** target, void* value);

    // If Destination == Comparand, Destination is set to Exchange. Returns the initial value of Destination.
    void* AtomicCompareExchangePointer(void** destination, void* exchange, void* comperand);


//template <typename T>
//class Threaded
//{
//public:
//    Threaded() {}
//    Threaded(T value) { mValues[0] = value; mValues[1] = value; }

//    T Get(AudioThreads::Enum whichThread) { return mValues[whichThread]; }

//    void Set(T value, AudioThreads::Enum threadCalledOn)
//    {
//        mValues[threadCalledOn] = value;

//        if (threadCalledOn == AudioThreads::MainThread)
//            Z::gSound->Mixer.AddTask(CreateFunctor(&mValues[AudioThreads::MixThread], value), nullptr);
//        else
//            Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&mValues[AudioThreads::MainThread], value), nullptr);
//    }

//    void SetDirectly(T value) { mValues[0] = value; mValues[1] = value; }

//private:
//    T mValues[2];

//    T& operator=(const T& lhs) {}
//};


    //class ThreadedInt
    //{
    //public:
    //    ThreadedInt() {}
    //    ThreadedInt(const AtomicType& value) : mValue(value) {}

    //    AtomicType Get() { return mValue; }

    //    void Set(AtomicType value) { AtomicExchange(&mValue, value); }

    //private:
    //    volatile AtomicType mValue;
    //};
}
