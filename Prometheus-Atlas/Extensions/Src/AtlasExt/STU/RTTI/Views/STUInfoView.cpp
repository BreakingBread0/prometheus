// Hazno - 2026

#include "STUInfoView.h"

#include "Atlas/STU/RTTI/STUInfo.h"

namespace Atlas::STU::RTTI
{
    STUInfoView::Iter& STUInfoView::Iter::operator++()
    {
        if (!m_current || !m_root) {
            return *this;
        }

        if (Utility::Enums::HasAnyFlags(m_flags, SiblingsTraverse) && m_current->Sibling) {
            m_current = m_current->Sibling;
            return *this;
        }

        if (Utility::Enums::HasAnyFlags(m_flags, ParentsTraverse) && m_root->Parent) {
            m_root = m_root->Parent;
            m_current = m_root;
            return *this;
        }

        if (Utility::Enums::HasAnyFlags(m_flags, ChildrenTraverse) && m_root->Child) {
            m_root = m_root->Child;
            m_current = m_root;
            return *this;
        }

        m_current = nullptr;
        m_root = nullptr;
        return *this;
    }

    STUInfoView::Iter STUInfoView::Iter::operator++(int)
    {
        const Iter tmp = *this;
        ++*this;
        return tmp;
    }

    bool STUInfoView::Iter::operator==(const Iter& it) const
    {
        return this->m_current == it.m_current && this->m_root == it.m_root;
    }

    STUInfoView::Iter::value_type STUInfoView::Iter::operator*() const
    {
        return m_current;
    }
}
