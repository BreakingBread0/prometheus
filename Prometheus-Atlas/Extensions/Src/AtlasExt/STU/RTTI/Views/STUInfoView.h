// Hazno - 2026

#pragma once

#include <iterator>

#include "Atlas/Common.h"
#include "AtlasExt/Utility/Enums.h"

namespace Atlas::STU::RTTI
{
    struct STUInfo;
    struct STUArgumentInfo;

    enum STUArgumentViewFlags
    {
        INVALID                 = 0 << 0,
        SiblingsTraverse        = 1 << 0,
        SiblingsNoTraverse      = 1 << 1,
        ParentsTraverse         = 1 << 2,
        ChildrenTraverse        = 1 << 3,
    };

    constexpr STUArgumentViewFlags operator|(const STUArgumentViewFlags a, STUArgumentViewFlags b) {
        return static_cast<STUArgumentViewFlags>(
            static_cast<uint32>(a) | static_cast<uint32>(b)
        );
    }

    struct STUInfoView
    {
        private:
            const STUArgumentViewFlags  m_flags;
            const STUInfo*              m_info;

           explicit STUInfoView(const STUInfo* info, const STUArgumentViewFlags flags) :
               m_flags(flags),
               m_info(info) {}

        public:
            STUInfoView() = delete;

            template <STUArgumentViewFlags T>
            static constexpr STUInfoView Create(const STUInfo* info)
            {
                static_assert(!Utility::Enums::HasAllFlags(T, ParentsTraverse | ChildrenTraverse),
                    "Cannot traverse both parents and children simultaneously.");

                return STUInfoView(info, T);
            }

            struct Iter
            {
                const STUArgumentViewFlags m_flags;

                const STUInfo*            m_root;
                const STUInfo*            m_current;

                using iterator_category = std::forward_iterator_tag;
                using value_type        = const STUInfo*;
                using difference_type   = std::ptrdiff_t;

                Iter() :
                    m_flags(INVALID),
                    m_root(nullptr),
                    m_current(nullptr) {}

                Iter(const STUInfo* info, const STUArgumentViewFlags m_flags) :
                    m_flags(m_flags),
                    m_root(info),
                    m_current(info) {}

                Iter& operator++();
                Iter operator++(int);
                bool operator==(const Iter& it) const;
                value_type operator*() const;
            };

            [[nodiscard]] Iter begin() const
            {
                return Iter{m_info, m_flags};
            }

            [[nodiscard]] Iter end() const
            {
                return Iter{nullptr, m_flags};
            }
    };
}
