// Hazno - 2026

#pragma once
#include <string>
#include <nlohmann/json.hpp>

#include "Atlas/Common.h"

namespace Generation::Probing
{
    struct ProbeData_Hex
    {
        uint64 Value;

        ProbeData_Hex() = default;

        ProbeData_Hex(const uint64 value) : Value(value) {}

        operator uint64() const {
            return Value;
        }

        template <typename BasicJsonType, nlohmann::detail::enable_if_t<
               nlohmann::detail::is_basic_json<BasicJsonType>::value, int> = 0>
        friend void to_json(BasicJsonType& nlohmann_json_j, const ProbeData_Hex& nlohmann_json_t)
        {
            char buf[32];
            sprintf_s(buf, "%p", nlohmann_json_t.Value);
            nlohmann_json_j = std::string(buf);
        }

        template <typename BasicJsonType, nlohmann::detail::enable_if_t<
                      nlohmann::detail::is_basic_json<BasicJsonType>::value, int> = 0>
        friend void from_json(const BasicJsonType& nlohmann_json_j, ProbeData_Hex& nlohmann_json_t)
        {
            const ProbeData_Hex nlohmann_json_default_obj{};
            nlohmann_json_t.Value = !nlohmann_json_j.is_null()
                                        ? _strtoi64(nlohmann_json_j.template get<std::string>().c_str(), nullptr, 16)
                                        : nlohmann_json_default_obj.Value;
        }
    };
}
