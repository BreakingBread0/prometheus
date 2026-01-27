// Hazno - 2026

#pragma once

#include "STUInfo.h"

#ifdef ATLAS_EXTENSIONS
#include "AtlasExt/Utility/Modules.h"
#include "AtlasExt/STU/RTTI/Views/STURegistryView.h"
#endif

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
        STURegistry*    Next;
        STUInfo*        Info;

#ifdef ATLAS_EXTENSIONS

        /**
         * Get the singleton instance of the STURegistry list head.
         * @return Pointer to the first STURegistry in the linked list.
         */
        static STURegistry* Get();
        static STURegistry* Get(const Utility::Modules::ModuleBounds& module);

        STUInfo* GetSTUInfoByHash(uint32 hash) const;

        [[nodiscard]] STURegistryView Range() const;
#endif
    };

    ATLAS_ASSERT_SIZE(STURegistry, 0x10);
}
