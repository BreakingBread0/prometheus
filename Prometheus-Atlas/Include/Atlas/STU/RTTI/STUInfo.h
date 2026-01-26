// Hazno - 2026

#pragma once

#include <memory>
#include <ranges>
#include <stdexcept>

#include "STUArgumentInfo.h"
#include "Atlas/Common.h"

#ifdef ATLAS_EXTENSIONS
#include "AtlasExt/STU/RTTI/Views/STUArgumentInfoView.h"
#include "AtlasExt/STU/RTTI/Views/STUInfoView.h"
#endif

namespace Atlas::STU::RTTI
{
    /**
     *  <b> InheritanceInfo </b> \n
     *      Inheritance information for STUInfo types.
     *
     *
     *  \n
     *  \n  Size:           0x10
     *  \n  Factory:        N/A
     *  \n  VT:             N/A
     *  \n
     */
    struct InheritanceInfo
    {
        InheritanceInfo*    Self;
        InheritanceInfo*    Next;
    };

    /**
     *  <b> STUInfo </b> \n
     *      Describes a STU type.
     *
     *
     *  \n
     *  \n  Size:           0x60
     *  \n  Factory:        N/A
     *  \n  VT:             N/A
     *  \n
     */
    struct STUInfo
    {
        /**
         * Name of the STU type. (Usually stripped)
         */
        char* Name_str;

        InheritanceInfo* RTTI_Info;

        /**
         * Start of sibling chain which all inherit from this base.
         */
        STUInfo* Child;

        /**
         * The next STUInfo with the same base.
         */
        STUInfo* Sibling;

        /**
         * Parent STUInfo.
         */
        STUInfo* Parent;


        STUArgumentInfo* Args;

        PADDING(0x10);

        void* (*CreateInstance)();         //TODO: Convert back to returning STUBase<STUBase_vt>*
        void* (*ClearInstance)(int64);     //TODO: Convert back to returning STUBase<STUBase_vt>*

        /**
         * A unique CRC32 hash of the STU type name. Unlike Name_str, this is not stripped and can be used for identity.
         */
        uint32 Hash;

        int32 ArgsCount;

        /**
         * If 0, this is an array type -- a more accurate name is probably a bool "IsSingleValue"?
         */
        int32 IsArrayInt;

        /**
         * Size (in bytes) of an allocated instance.
         */
        int32 InstanceSize;

#ifdef ATLAS_EXTENSIONS

        /**
         * Enumerates all STUArgumentInfo types for this STUInfo.
         * @param includeParents Whether to include parent STUInfo argument infos.
         * @return STUArgumentInfoView of argument infos.
         */
        [[nodiscard]] STUArgumentInfoView RangeArgs(bool includeParents = true) const;

        /**
         * Enumerates all sibling STUInfo types. Starting from this type's next sibling.
         * @return STUInfoView of sibling types.
         */
        [[nodiscard]] STUInfoView RangeSiblings() const;

        /**
         * Enumerates all direct child STUInfo types. Starting from this type's first child.
         * @return STUInfoView of child types.
         */
        [[nodiscard]] STUInfoView RangeChildren() const;

        /**
         * Enumerates all descendant STUInfo types (children and their siblings). Starting from this type's first child.
         * @return STUInfoView of descendant types.
         */
        [[nodiscard]] STUInfoView RangeDescendants() const;

        /**
         * Enumerates all direct parent STUInfo types. Starting from this type's parent.
         * @return STUInfoView of parent types.
         */
        [[nodiscard]] STUInfoView RangeParents() const;

        /**
         * Enumerates all ancestor STUInfo types (parents and their siblings). Starting from this type's parent.
         * @return STUInfoView of ancestor types.
         */
        [[nodiscard]] STUInfoView RangeAncestors() const;

        [[nodiscard]] bool AssignableToHash(uint64 assign) const;
        [[nodiscard]] bool AssignableToHashes(std::initializer_list<uint64> assign) const;
        [[nodiscard]] bool AssignableToRTTI(uint64 assign) const;

        template <class T> requires (std::is_integral_v<std::remove_reference_t<T>> || std::is_pointer_v<std::remove_reference_t<T>>)
        [[nodiscard]] bool AssignableToRTTI(T assign) const
        {
            if constexpr (std::is_pointer_v<std::remove_reference_t<T>>) {
                return AssignableToRTTI(reinterpret_cast<uint64>(assign));
            } else {
                return AssignableToRTTI(static_cast<uint64>(assign));
            }
        }

        [[nodiscard]] STUArgumentInfo* ArgByHash(uint32 hash, bool search_parents = true) const;
#endif
    };

    ATLAS_ASSERT_SIZE(STUInfo, 0x60);
}