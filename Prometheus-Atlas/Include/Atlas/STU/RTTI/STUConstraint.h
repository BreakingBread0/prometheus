// Hazno - 2026

#pragma once

#include "Atlas/Common.h"

struct bsDataStore;

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
     *  NOTE: "void* stu_value" means the pointer to the actual STU value youre operating on. A helper accessor function is available on STU_Editable::get_argument_raw
     *
     *  \n
     *  \n  Size:           Unk
     *  \n  Factory:        None
     *  \n  VT:             No base / unnecessary to know base
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

            virtual uint64  SetDefaultValue(void* stu_value) = 0;
            virtual void    ClearValue(void* stu_value) = 0;
            virtual void    ClearObjectValue(void* stu_balue) = 0; // Can also happen on primitive (teString). No impl on u8*

            virtual const char* GetName() = 0;

            virtual int64   GetSomeFlag_Object() = 0;
            virtual int64   GetSomeFlag_Primitive() = 0;
            virtual int64   GetFieldSize() = 0;
            virtual int8    IsFieldNull(void* stu_value)  = 0;

            STUB();
            STUB(int64, int64, int64);

            virtual uint8   SerializeToBitstream(bsDataStore* store, int64 _unused, void* input_value) = 0;
            STUB(int64, int64, int64);
            STUB(int64, char*, int64, int64, int64, int64, int8);
            virtual uint8   DeserializeFromBitStream(bsDataStore* store, int64 _unused, void* stu_value)  = 0;
            // Call on a FRESH target instance!
            // Unsafe because old value is not cleared.
            virtual int64   Clone_Unsafe(uint8**)  = 0;
            STUB();
            STUB(uint8*, int8*);
            //From casc archives. Not useful
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
