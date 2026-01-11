// Hazno - 2026

#pragma once

#include "Common.h"

class ComponentCreator
{
    public:
        int8            ComponentID{};
        void*           UnkVt{};    //IDK?
        PADDING(0x10){};
        void*           IClass{};   //StructPageAllocator
        int64           IWhich_flags{};

        virtual int64*  Destroy(char a2);
        virtual int64   Construct();
        virtual int64   Unk1(ComponentCreator* a1, void (__fastcall ***a2)(int64, int64));
        virtual int64   Unk2(int64 a1); //StructPageAllocator

    protected:
        ~ComponentCreator() = default;
};

class ComponentCreator28_STU_HEALTH_COMPONENT : public ComponentCreator
{
    protected:
        ~ComponentCreator28_STU_HEALTH_COMPONENT() = default;
        int64 Construct() override;
};