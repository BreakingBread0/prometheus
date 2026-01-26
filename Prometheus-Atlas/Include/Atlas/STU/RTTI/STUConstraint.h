// Hazno - 2026

#pragma once

#include "Atlas/Common.h"

namespace Atlas::STU::RTTI
{
    enum STUConstraintType {
        STU_ConstraintType_Primitive = 0x0,
        STU_ConstraintType_BSList_Primitive = 0x1,              // List of values
        STU_ConstraintType_Object = 0x2,
        STU_ConstraintType_BSList_Object = 0x3,
        STU_ConstraintType_InlinedObject = 0x4,
        STU_ConstraintType_BSList_InlinedObject = 0x5,          // List of values, not pointers
        STU_ConstraintType_Map = 0x7,
        STU_ConstraintType_Enum = 0x8,                          // Has a size of 4
        STU_ConstraintType_BSList_Enum = 0x9,
        STU_ConstraintType_NonSTUResourceRef = 0xA,
        STU_ConstraintType_BSList_NonSTUResourceRef = 0xB,
        STU_ConstraintType_ResourceRef = 0xC,
        STU_ConstraintType_BSList_ResourceRef = 0xD,            // Inlined list of resourceref
    };

#ifdef ATLAS_EXTENSIONS
    const char* STUConstraintType_ToString(STUConstraintType type);
#endif

    /**
     *  <b> STUConstraint </b> \n
     *      Description TBC
     *
     *
     *  \n
     *  \n  Size:           Unk
     *  \n  Factory:        N/A
     *  \n  VT:             Base Unk. Der Example: 0x14DFFE8
     *  \n
     */
    class STUConstraint {
        public:
            STUB(int64*);
            STUB(int64*, int64*, int64*);
            STUB(int64*);
            STUB(int64*);
            STUB(int64*);
            STUB(int64*);

            virtual int64   GetStuHashWithTypeFlag() = 0;
            virtual uint64  GetTypeFlag() = 0;

            virtual uint64  GetNameHash() = 0;

            virtual uint64  SetDefaultValue(int64 u8_type, int8* stu_value) = 0;
            virtual void    ClearValue(int64, int8*) = 0;
            virtual void    ClearObjectValue(int8 *) = 0; // Can also happen on primitive (teString). No impl on u8*

            virtual const char* GetName() = 0;

            virtual int64   GetSomeFlagObject() = 0;
            virtual int64   GetSomeFlagPrimitive() = 0;
            virtual int64   GetFieldSize() = 0;
            virtual int8    IsFieldNull(uint8*)  = 0;

            STUB();
            STUB(int64, int64, int64);

            virtual uint8   SerializeToBitstream(void* bsDataStore, int64, uint8* input_value) = 0;
            STUB(int64, int64, int64);
            STUB(int64, char*, int64, int64, int64, int64, int8);
            virtual uint8   DeserializeFromBitStream(void* bsDataStore, void* bsDataStore2, uint8* output_with_offs)  = 0;
            virtual int64   Clone_Unsafe(uint8**)  = 0; // Call on a FRESH target instance!
            STUB();
            STUB(uint8*, int8*);
            virtual void    DeserializeSelf() = 0;
            virtual int64   GetAdditionalHash() = 0;
            STUB();
            STUB();

#ifdef ATLAS_EXTENSIONS
            int64 GetSTUType();
            STUConstraintType ToConstraintType();
            bool IsPrimitive();
            bool IsList();
#endif

        protected:
            ~STUConstraint() = delete;
    };


}
