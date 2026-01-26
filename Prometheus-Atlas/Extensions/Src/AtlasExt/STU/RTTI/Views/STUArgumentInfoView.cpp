// Hazno - 2026

#include "STUArgumentInfoView.h"

#include <stdexcept>
#include "Atlas/STU/RTTI/STUInfo.h"

namespace Atlas::STU::RTTI
{
    STUArgumentInfoView::Iter& STUArgumentInfoView::Iter::operator++()
    {
        m_currentPos++;

        if (!m_info) {
            return *this;
        }

        if (m_info->ArgsCount <= m_currentPos) {
            m_info = m_includeParents ? m_info->Parent : nullptr;
            m_currentPos = -1;
            return this->operator++();
        }

        return *this;
    }

    STUArgumentInfoView::Iter STUArgumentInfoView::Iter::operator++(int)
    {
        Iter tmp = *this;
        ++(*this);
        return tmp;
    }

    bool STUArgumentInfoView::Iter::operator==(const Iter& it) const
    {
        return this->m_currentPos == it.m_currentPos && this->m_info == it.m_info && this->m_includeParents == it.m_includeParents;
    }

    STUArgumentInfoView::Iter::value_type STUArgumentInfoView::Iter::operator*() const
    {
        if (!m_info) {
            throw std::out_of_range("Empty iterator called!");
        }

        if (m_info->ArgsCount <= m_currentPos) {
            throw std::out_of_range("Iterator called on invalid position!");
        }

        return { m_info, &m_info->Args[m_currentPos] };
    }
}
