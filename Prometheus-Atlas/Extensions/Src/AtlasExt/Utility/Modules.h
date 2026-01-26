// Hazno - 2026

#pragma once

#include <Windows.h>
#include <tlhelp32.h>
#include <memory>
#include <string>

#include "Type.h"
#include "Atlas/Common.h"

namespace Atlas::Utility::Modules
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

            [[nodiscard]] uint64 RVA(uint64 absolute) const;

            template <class Ret = uint64, class A> requires (std::is_integral_v<std::remove_reference_t<A>> || std::is_pointer_v<std::remove_reference_t<A>>)
            [[nodiscard]] Ret RVA(A absolute) const
            {
                if constexpr (std::is_same_v<Ret, uint64>) {
                    return RVA(Type::ToUInt64(absolute));
                } else {
                    return reinterpret_cast<Ret>(RVA(Type::ToUInt64(absolute)));
                }
            }

            [[nodiscard]] uint64 VA(uint64 relative) const;

            template <class Ret = uint64, class A> requires (std::is_integral_v<std::remove_reference_t<A>> || std::is_pointer_v<std::remove_reference_t<A>>)
            [[nodiscard]] Ret VA(A relative) const
            {
                if constexpr (std::is_same_v<Ret, uint64>) {
                    return VA(Type::ToUInt64(relative));
                } else {
                    return reinterpret_cast<Ret>(VA(Type::ToUInt64(relative)));
                }
            }

            [[nodiscard]] uint64 Base() const { return base; }
            [[nodiscard]] uint64 End() const { return end; }
            [[nodiscard]] uint64 Size() const { return size; }
            [[nodiscard]] const std::string& Name() const { return name; }
    };

    const ModuleBounds& ProgramBounds();
    const ModuleBounds& RuntimeBounds();

#ifdef PROMETHEUS_CORE
    inline const ModuleBounds& GameBounds() { return ProgramBounds(); }
#endif

    void Initialize();
    void Uninitialize();

    const ModuleBounds* FindModuleForAddress(uint64 addr);

    template <class T> requires std::is_pointer_v<T>
    const ModuleBounds* FindModuleForAddress(T addr) {
        return FindModuleForAddress(reinterpret_cast<uint64>(addr));
    }
}
