// Hazno - 2026

#pragma once

#include "STUInfo.h"

namespace Atlas::STU::RTTI
{
    /**
     *  <b> STURegistry </b> \n
     *      Description TBC
     *
     *
     *  \n
     *  \n  Size:           0x10
     *  \n  Factory:        N/A
     *  \n  VT:             N/A
     *  \n
     */
    struct STURegistry
    {
        STURegistry* Next;
        STUInfo* Info;

#ifdef ATLAS_EXTENSIONS

        /**
         * Get the singleton instance of the STURegistry list head.
         * @return Pointer to the first STURegistry in the linked list.
         */
        static STURegistry* Get();

        STUInfo* GetSTUInfoByHash(uint32 hash) const;
#endif
    };

    ATLAS_ASSERT_SIZE(STURegistry, 0x10);
}
