// Hazno - 2026

#pragma once
#include <type_traits>

#include "Atlas/Common.h"

namespace Atlas::Utility::Type
{
    template <class A> requires (std::is_integral_v<std::remove_reference_t<A>> || std::is_pointer_v<std::remove_reference_t<A>>)
    [[nodiscard]] static constexpr uint64 ToUInt64(A addr)
    {
        if constexpr (std::is_pointer_v<std::remove_reference_t<A>>) {
            return reinterpret_cast<uint64>(addr);
        } else {
            return  static_cast<uint64>(addr);
        }
    }
}
