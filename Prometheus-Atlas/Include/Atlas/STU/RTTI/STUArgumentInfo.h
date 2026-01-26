// Hazno - 2026

#pragma once

#include "STUConstraint.h"
#include "Atlas/Common.h"

namespace Atlas::STU::RTTI
{
    /**
     *  <b> STUArgumentInfo </b> \n
     *      Description TBC
     *
     *
     *  \n
     *  \n  Size:           0xA0 (160) (Unconfirmed)
     *  \n  Factory:        N/A
     *  \n  VT:             N/A
     *  \n
     */
    struct STUArgumentInfo
    {
        char* Name_str;
        uint32 Hash;
        int32 Offset;
        STUConstraint* Constraint;
        int32 Field18;
        int32 Field1C;
        int64 Field20;
        int32 DoesntNeedResetValue;
        int32 Field2C;
        int64 Field30;
        int64 DefaultValue; //Unconfirmed!
        int64 Field40;
        int64 Field48;
        int32 Field50;
        int32 Field54;

        int64 Field58;
        int64 Field60;
        int64 Field68;

        int32 Field70;
        int32 Field74;
        int32 Field78;
        int32 Field7C;
        int32 Field80;
        int32 Field84;
        int32 Field88;
        int32 Field8C;

        int64 Field90;
        int64 Field98;
    };

    ATLAS_ASSERT_SIZE(STUArgumentInfo, 0xA0);
}
