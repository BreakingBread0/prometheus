// Hazno - 2026

#pragma once
#include "Atlas/Common.h"

namespace Probing
{
    struct ProbeData_STUProbe
    {
        uint32  Hash;
        uint32  ParentHash;

        int32   ArgumentCount;
        int32   Size;

        uint64  RVA;

        bool    IsArray;
    };

    struct ProbeResult
    {
        ProbeData_STUProbe STU[];
    };

    //ProbeResult GenerateResult();
}
