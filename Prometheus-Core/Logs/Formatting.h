// Hazno - 2026

#pragma once

#include <fmt/format.h>

#include "Utility/Modules.h"
#include "idadefs.h"

//
// Avoid including this file directly, include "Logs.h" instead.
//

#define FmtAddr(addr) Logs::Format::Address(addr)

namespace Logs::Format
{
    struct Address
    {
        uint64 value;

        explicit Address(const uint64 val) : value(val) {}
        template <class T> requires std::is_pointer_v<T>
        explicit Address(T val) : value(reinterpret_cast<uint64>(val)) {}
    };
}

namespace fmt
{
    template<> struct formatter<Logs::Format::Address> : formatter<std::string>
    {
        private:
            bool show_module = true; //N == Dont try print module
            bool show_value = false; //V == Also show decimal value
            int len = 16;

        public:
            constexpr auto parse(const parse_context<>& ctx)
            {
                auto it = ctx.begin();

                for (const auto end = ctx.end(); it != end && *it != '}'; it++) {
                    switch (*it) {
                        case 'n': show_module = false; continue;
                        case 'v': show_value = true; continue;
                        default: return ctx.begin();
                    }
                }

                //TODO: Length parse

                return it;
            }

            auto format(const Logs::Format::Address& s, const format_context& ctx) const
            {
                const auto addr = s.value;
                auto out = ctx.out();

                auto end = format_to(out, "0x{:0{}X}", addr, len);

                if (show_value) {
                    end = format_to(out, " {:0>10}", addr);
                }

                if (show_module) {
                    if (const auto m = Utility::Modules::FindModuleForAddress(addr)) {
                        end = format_to(out, " ({}+0x{:08X})", m->Name(), m->RVA(addr));
                    }
                }

                return end;
            }
    };
}

