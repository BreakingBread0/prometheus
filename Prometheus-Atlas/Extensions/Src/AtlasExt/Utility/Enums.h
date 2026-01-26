// Hazno - 2026

#pragma once
#include <type_traits>

namespace Atlas::Utility::Enums
{
    template<typename T>
    concept Enum = requires(T a) {
        std::is_enum_v<T> == true;
        std::is_integral_v<std::underlying_type_t<T>> == true;
    };

    /**
     * Determine if any flags are set in the given enum value.
     * @tparam T Enum Type
     * @param value Current enum value.
     * @param flags Target flag(s) to evaluate.
     * @return True if any of the specified flags are set, false otherwise.
     */
    template<Enum T>
    bool constexpr HasAnyFlags(const T value, const T flags) {
        return (value & flags) != 0;
    }

    /**
     * Determine if all the flags are set in the given enum value.
     * @tparam T Enum Type
     * @param value Current enum value.
     * @param flags Target flag(s) to evaluate.
     * @return True if all the specified flags are set, false otherwise.
     */
    template<Enum T>
    bool constexpr HasAllFlags(const T value, const T flags) {
        return (value & flags) == flags;
    }

};