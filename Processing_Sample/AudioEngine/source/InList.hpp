//---------------------------------------------------------------------------------------------------------------------
// file:    InList.hpp
// author:  Andy Ellinger
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//---------------------------------------------------------------------------------------------------------------------

#pragma once

namespace AudioEngine
{
    template <typename Type>
    class InListLink
    {
    public:
        InListLink() : mNext(nullptr), mPrev(nullptr) {}

        Type* mNext;
        Type* mPrev;
    };

    template <typename Type>
    class InList
    {
    public:
        InList() : mFirst(nullptr), mLast(nullptr) {}

        void PushBack(Type* item)
        {
            if (!mFirst)
            {
                mFirst = item;
                mLast = item;
            }
            else
            {
                mLast->Link.mNext = item;
                item->Link.mPrev = mLast;
                mLast = item;
            }
        }

        void Remove(Type* item)
        {
            if (!item->Link.mNext && !item->Link.mPrev && mFirst != mLast)
                return;

            if (item->Link.mPrev)
                item->Link.mPrev->Link.mNext = item->Link.mNext;

            if (item->Link.mNext)
                item->Link.mNext->Link.mPrev = item->Link.mPrev;

            if (mLast == item)
                mLast = item->Link.mPrev;

            if (mFirst == item)
                mFirst = item->Link.mNext;

            item->Link.mNext = item->Link.mPrev = nullptr;
        }

        Type* Front()
        {
            return mFirst;
        }

        Type* Back()
        {
            return mLast;
        }

        Type* PopFront()
        {
            Type* temp = mFirst;

            if (temp)
                Remove(temp);

            return temp;
        }

        bool Empty()
        {
            return mFirst == nullptr;
        }

        class iterator
        {
        public:
            void operator++() { if (mItem) mItem = mItem->Link.mNext; }
            Type* operator->() { return mItem; }
            Type* operator*() { return mItem; }
            bool operator==(const iterator& rhs) { return mItem == rhs.mItem; }
            bool operator!=(const iterator& rhs) { return mItem != rhs.mItem; }

        private:
            iterator(Type* item) : mItem(item) {}

            Type* mItem;

            friend class InList;
        };

        iterator begin()
        {
            return iterator(mFirst);
        }

        iterator end()
        {
            return iterator(nullptr);
        }

    private:
        Type* mFirst;
        Type* mLast;
    };
}
