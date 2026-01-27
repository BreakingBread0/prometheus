// Hazno - 2026

#pragma once
#include <iterator>

#include "Atlas/Common.h"


namespace Atlas::STU::RTTI
{
    struct STUInfo;
    struct STUArgumentInfo;

    struct STUArgumentInfoView
    {
        private:
            const STUInfo*  m_info;
            bool            m_includeParents;

            explicit STUArgumentInfoView(const STUInfo* info, const bool includeParents = true) :
                m_info(info),
                m_includeParents(includeParents) {}

        public:
            STUArgumentInfoView() = delete;

            static STUArgumentInfoView Create(const STUInfo* info, const bool includeParents = true)
            {
                return STUArgumentInfoView(info, includeParents);
            }

            struct Iter
            {
                const STUInfo*  m_info;
                int32           m_currentPos;
                bool            m_includeParents;

                using iterator_category = std::forward_iterator_tag;
                using value_type        = std::pair<const STUInfo*, STUArgumentInfo*>;
                using difference_type   = std::ptrdiff_t;

                Iter() = default;

                Iter(const STUInfo* info, const bool includeParents) :
                    m_info(info),
                    m_currentPos(-1),
                    m_includeParents(includeParents) {
			        this->operator++();
                }

                Iter& operator++();
                Iter operator++(int);
                bool operator==(const Iter& it) const;
                value_type operator*() const;
            };

            [[nodiscard]] Iter begin() const
            {
                return Iter{m_info, m_includeParents};
            }

            [[nodiscard]] Iter end() const
            {
                return Iter{nullptr, m_includeParents};
            }
    };
}
