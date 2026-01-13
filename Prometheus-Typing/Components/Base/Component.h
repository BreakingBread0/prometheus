// Hazno - 2026

#pragma once
#include "Common.h"

class Component
{
    public:
        void*           EntityBackref{};
        uint8           ComponentID{};
        int32           IsMirrored{}; //TODO that is wrong. Why do some components have 1 and some have 0?

        virtual int64   Deallocate(char);
        virtual void    CallForDeserialize(void* mirrorDataHelper);
        virtual uint8   OnCreate(int64 loader);
        virtual void    Field1();
        virtual void    Field2();
        virtual uint8   Field3();

    protected:
        ~Component() = default;
};
