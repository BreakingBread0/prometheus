// Hazno - 2026

#include "Atlas/STU/RTTI/STUConstraint.h"

#include <string>

#include "AtlasExt/STU/Statics/StaticLookup.h"

namespace Atlas::STU::RTTI
{
    const char* STUConstraintType_ToString(const STUConstraintType type)
    {
        switch (type) {
            case STU_ConstraintType_Primitive:
                return "Primitive";
            case STU_ConstraintType_BSList_Primitive:
                return "List: Primitive";
            case STU_ConstraintType_Object:
                return "Object";
            case STU_ConstraintType_BSList_Object:
                return "List: Object";
            case STU_ConstraintType_InlinedObject:
                return "InlinedObject";
            case STU_ConstraintType_BSList_InlinedObject:
                return "List: InlinedObject";
            case STU_ConstraintType_Map:
                return "Map";
            case STU_ConstraintType_Enum:
                return "Enum";
            case STU_ConstraintType_BSList_Enum:
                return "List: Enum";
            case STU_ConstraintType_NonSTUResourceRef:
                return "NonSTUResourceRef";
            case STU_ConstraintType_BSList_NonSTUResourceRef:
                return "List: NonSTUResourceRef";
            case STU_ConstraintType_ResourceRef:
                return "ResourceRef";
            case STU_ConstraintType_BSList_ResourceRef:
                return "List: ResourceRef";
            default:
                return ("Unknown: " + std::to_string(type)).c_str();
        }
    }

    int64 STUConstraint::GetSTUType() {
        return this->GetStuHashWithTypeFlag() & 0xFFFFFFFF;
    }

    STUConstraintType STUConstraint::ToConstraintType() {
        return static_cast<STUConstraintType>(this->GetStuHashWithTypeFlag() >> 32 & 0xFFFFFFFF);
    }

    bool STUConstraint::IsPrimitive() {
        const auto type = ToConstraintType();
        return type == STU_ConstraintType_Primitive && Statics::Primitive::all.contains(type);
    }

    bool STUConstraint::IsList() {
        const auto type = ToConstraintType();
        return
            type == STU_ConstraintType_BSList_Primitive ||
            type == STU_ConstraintType_BSList_Object ||
            type == STU_ConstraintType_BSList_InlinedObject ||
            type == STU_ConstraintType_BSList_Enum ||
            type == STU_ConstraintType_BSList_ResourceRef ||
            type == STU_ConstraintType_BSList_NonSTUResourceRef;
    }
}
