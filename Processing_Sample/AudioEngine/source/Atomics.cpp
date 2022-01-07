//---------------------------------------------------------------------------------------------------------------------
// file:    Atomics.cpp
// author:  
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#include "Atomics.h"


namespace AudioEngine
{
    //*****************************************************************************************************************
    AtomicType AtomicExchange(volatile AtomicType* target, AtomicType value)
    {
#ifndef _WIN64
        return InterlockedExchange((long*)target, (long)value);
#else
        return InterlockedExchange64((long long*)target, (long long)value);
#endif
    }

    //*****************************************************************************************************************
    AtomicType AtomicCompareExchange(volatile AtomicType* destination, AtomicType exchange,
        AtomicType comperand)
    {
#ifndef _WIN64
        return InterlockedCompareExchange((long*)destination, (long)exchange, (long)comperand);
#else
        return InterlockedCompareExchange64((long long*)destination, (long long)exchange, (long long)comperand);
#endif
    }

    //*****************************************************************************************************************
    void* AtomicSetPointer(void** target, void* value)
    {
        return InterlockedExchangePointer(target, value);
    }

    //*****************************************************************************************************************
    void* AtomicCompareExchangePointer(void** destination, void* exchange, void* comperand)
    {
        return InterlockedCompareExchangePointer(destination, exchange, comperand);
    }
}