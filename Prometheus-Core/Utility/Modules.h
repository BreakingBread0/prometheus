// Hazno - 2026

#pragma once

#include <memory>
#include <string>
#include <tlhelp32.h>

#include "idadefs.h"

namespace Utility::Modules
{
    struct ModuleBounds {
        private:
            uint64          base;
            uint64          end;
            uint64          size;
            std::string     name;

        public:
            explicit ModuleBounds(MODULEENTRY32 modEntry) :
                base(reinterpret_cast<uint64>(modEntry.modBaseAddr)),
                end(reinterpret_cast<uint64>(modEntry.modBaseAddr + modEntry.modBaseSize)),
                size(static_cast<uint64>(modEntry.modBaseSize)),
                name(modEntry.szModule) {}

            [[nodiscard]] uint64 RVA(uint64 address) const;

            template <class T> requires std::is_pointer_v<T>
            [[nodiscard]] uint64 RVA(T address) const
            {
                return RVA(reinterpret_cast<uint64>(address));
            }

            [[nodiscard]] uint64 Base() const { return base; }
            [[nodiscard]] uint64 End() const { return end; }
            [[nodiscard]] uint64 Size() const { return size; }
            [[nodiscard]] const std::string& Name() const { return name; }
    };

    const ModuleBounds& GameBounds();
    const ModuleBounds& CoreBounds();

    void Initialize();
    void Uninitialize();

    const ModuleBounds* FindModuleForAddress(uint64 addr);

    template <class T> requires std::is_pointer_v<T>
    const ModuleBounds* FindModuleForAddress(T addr) {
        return FindModuleForAddress(reinterpret_cast<uint64>(addr));
    }
}
