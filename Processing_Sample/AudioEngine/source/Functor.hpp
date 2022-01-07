//---------------------------------------------------------------------------------------------------------------------
// file:    Functor.hpp
// author:  Andy Ellinger, Trevor Sundberg
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#pragma once

namespace AudioEngine
{
    class Functor
    {
    public:
        virtual void Execute() = 0;
    };

    // ----------------------------------------------------------------------------------------------------------------

    //*****************************************************************************************************************
    template <typename InstanceType>
    class Functor0 : public Functor
    {
    public:
        void Execute() override
        {
            (mInstance->*mFunctionPointer)();
        }

        void(InstanceType::* mFunctionPointer)();
        InstanceType* mInstance;
    };

    //*****************************************************************************************************************
    template <typename InstanceType>
    Functor* CreateFunctor(void(InstanceType::* functionPointer)(), InstanceType* instance)
    {
        if (!instance || !functionPointer)
            return nullptr;

        Functor0<InstanceType>* functor = new Functor0<InstanceType>();
        functor->mFunctionPointer = functionPointer;
        functor->mInstance = instance;
        return functor;
    }

    // ----------------------------------------------------------------------------------------------------------------

    //*****************************************************************************************************************
    template <typename InstanceType, typename P0>
    class Functor1 : public Functor
    {
    public:
        void Execute() override
        {
            (mInstance->*mFunctionPointer)(mP0);
        }

        void(InstanceType::* mFunctionPointer)(P0);
        InstanceType* mInstance;
        P0 mP0;
    };

    //*****************************************************************************************************************
    template <typename InstanceType, typename P0>
    Functor* CreateFunctor(void(InstanceType::* functionPointer)(P0), InstanceType* instance,
        P0 p0)
    {
        if (!instance || !functionPointer)
            return nullptr;

        Functor1<InstanceType, P0>* functor = new Functor1<InstanceType, P0>();
        functor->mFunctionPointer = functionPointer;
        functor->mInstance = instance;
        functor->mP0 = p0;
        return functor;
    }

    // ----------------------------------------------------------------------------------------------------------------

    //*****************************************************************************************************************
    template <typename InstanceType, typename P0, typename P1>
    class Functor2 : public Functor
    {
    public:
        void Execute() override
        {
            (mInstance->*mFunctionPointer)(mP0, mP1);
        }

        void(InstanceType::* mFunctionPointer)(P0, P1);
        InstanceType* mInstance;
        P0 mP0;
        P1 mP1;
    };

    //*****************************************************************************************************************
    template <typename InstanceType, typename P0, typename P1>
    Functor* CreateFunctor(void(InstanceType::* functionPointer)(P0, P1), InstanceType* instance,
        P0 p0, P1 p1)
    {
        if (!instance || !functionPointer)
            return nullptr;

        Functor2<InstanceType, P0, P1>* functor = new Functor2<InstanceType, P0, P1>();
        functor->mFunctionPointer = functionPointer;
        functor->mInstance = instance;
        functor->mP0 = p0;
        functor->mP1 = p1;
        return functor;
    }

    // ----------------------------------------------------------------------------------------------------------------

    //*****************************************************************************************************************
    template <typename InstanceType, typename MemberType>
    class FunctorMember : public Functor
    {
    public:
        void Execute() override
        {
            (mInstance->*mMember) = mValue;
        }

        MemberType InstanceType::* mMember;
        InstanceType* mInstance;
        MemberType mValue;
    };

    //*****************************************************************************************************************
    template <typename InstanceType, typename MemberType>
    Functor* CreateFunctor(MemberType InstanceType::* member, InstanceType* instance, MemberType value)
    {
        if (!member || !instance)
            return nullptr;

        FunctorMember<InstanceType, MemberType>* functor = new FunctorMember<InstanceType, MemberType>();
        functor->mMember = member;
        functor->mInstance = instance;
        functor->mValue = value;
        return functor;
    }
}
