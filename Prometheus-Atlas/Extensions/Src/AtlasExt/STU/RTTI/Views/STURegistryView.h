// Hazno - 2026

#pragma once

#include <iterator>

namespace Atlas::STU::RTTI
{
    struct STURegistry;
    struct STUInfo;
    struct STUArgumentInfo;

    struct STURegistryView
    {
        private:
            const STURegistry*  m_registry;

           explicit STURegistryView(const STURegistry* m_registry) :
               m_registry(m_registry) {}

        public:
            STURegistryView() = delete;

            static STURegistryView Create(const STURegistry* registry)
            {
                return STURegistryView(registry);
            }

            struct Iter
            {
                const STURegistry*        m_current;

                using iterator_category = std::forward_iterator_tag;
                using value_type        = std::pair<const STUInfo*, const STURegistry*>;
                using difference_type   = std::ptrdiff_t;

                Iter() :
                    m_current(nullptr) {}

                Iter(const STURegistry* registry) :
                    m_current(registry) {}

                Iter& operator++();
                Iter operator++(int);
                bool operator==(const Iter& it) const;
                value_type operator*() const;
            };

            [[nodiscard]] Iter begin() const
            {
                return Iter{m_registry};
            }

            [[nodiscard]] Iter end() const
            {
                return Iter{nullptr};
            }
    };

    inline auto begin(const STURegistry* r) {
        return STURegistryView::Create(r).begin();
    }

    inline auto end(const STURegistry* r) {
        return STURegistryView::Create(r).end();
    }
}
