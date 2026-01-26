// Hazno - 2026

#pragma once

#include "Atlas/STU/RTTI/STURegistry.h"
#include "AtlasExt/Utility/Modules.h"

namespace Atlas::STU::RTTI
{
    STURegistry* STURegistry::Get()
    {
        return *Utility::Modules::ProgramBounds().VA<STURegistry**>(0x18F74E0);
    }

    STUInfo* STURegistry::GetSTUInfoByHash(const uint32 hash) const
    {
        auto registry = this;
        while (registry) {
            if (registry->Info && registry->Info->Hash == hash) {
                return registry->Info;
            }

            registry = registry->Next;
        }

        return nullptr;
    }
}
