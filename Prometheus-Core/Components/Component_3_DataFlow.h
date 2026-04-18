//
// Created by cereal on 18.04.26.
//

#ifndef PROMETHEUS_COMPONENT_3_DATAFLOW_H
#define PROMETHEUS_COMPONENT_3_DATAFLOW_H

#include "Component.h"
#include "globals.h"
#include "idadefs.h"
#include "STU.h"

struct DataFlowComponent_ValueStack;
struct DataFlow_ComponentRefBase;
struct Component_1_SceneRendering;

struct DataFlowValue
{
    union
    {
        Entity* value_ent;
        float value_flt;
        Vector3 value_vec23;
        Vector4 value_vec4;
        union
        {
            __int64 value_uuid_1;
            __int64 value_uuid_2;
        };
    };
};

struct STUDataFlowType
{
    struct Float
    {
        STUBase<> vfptr;
        int m_default;
        int m_min;
        int m_max;
        int unk_1;
        uint unk_2;
        char unk_3;
    };

    struct Vec2
    {
        STUBase<> vfptr;
        float m_default[2]; //teVec2
    };

    struct Vec3
    {
        STUBase<> vfptr;
        Vector3 m_default;
    };

    struct Vec4
    {
        STUBase<> vfptr;
        Vector4 m_default;
    };

    struct Entity
    {
        STUBase<> vfptr;
        //such empty, much wow
    };

    struct GUID
    {
        STUBase<> vfptr;
        __int64 m_default[2];
    };
    //static inline uint StuDataFlowType_Float = stringHash("STUDataFlowType_Float");
    //static inline uint StuDataFlowType_Vec2 = stringHash("StuDataFlowType_Vec2");
    //static inline uint StuDataFlowType_Vec3 = stringHash("StuDataFlowType_Vec3");
    //static inline uint StuDataFlowType_Vec4 = stringHash("StuDataFlowType_Vec4");
    //static inline uint StuDataFlowType_Entity = stringHash("StuDataFlowType_Entity");
    //static inline uint StuDataFlowType_GUID = stringHash("StuDataFlowType_GUID");

    std::string to_string()
    {

    }

    union
    {
        STUBase<> vfptr;
        STRUCT_PLACE(Float*, float_value, 8);
        STRUCT_PLACE(Vec2*, vec2_value, 8);
        STRUCT_PLACE(Vec3*, vec3_value, 8);
        STRUCT_PLACE(Vec4*, vec4_value, 8);
        STRUCT_PLACE(Entity*, ent_value, 8);
        STRUCT_PLACE(GUID*, guid_value, 8);
    };
};

struct STUDataFlow
{
    STUBase<>* vfptr;
    STUDataFlowType* m_type;
    int m_animBlendRule;
    char m_onFailureKeepLastValue;
    char m_visibleOnServer;
};

struct HardEntRef_vt // sizeof=0x20
{                                       // XREF: DataFlow_GetValue_GUID+10E/r
    // DataFlow_GetValue_Float+111/r ...
    _QWORD *(__fastcall *deallocate)(_QWORD *, char);
    __int64 (__fastcall *GetHardEntRefValue)(DataFlow_ComponentRefBase *, Entity *, unsigned int id, __int64 *value_out);
    // XREF: DataFlow_GetValue_GUID+10E/r
    // DataFlow_GetValue_Float+111/r ...
    char (__fastcall *UpdateValueStack)(DataFlow_ComponentRefBase *, Entity *, DataFlowComponent_ValueStack *);
    // XREF: DataFlowComp_UpdateValueStacks+75/r
    __int64 (*ret_0)();
};

struct DataFlow_ComponentRefBase
{
    HardEntRef_vt* vfptr;
    __int32 field_8;
    __int32 field_8_1;
    __int32 field_10;
};

//size 0x100
struct Dataflow_Cache
{
    union
    {
        int mutex;
        STRUCT_PLACE(teList<DataFlowValue*>, dataflow_values, 0x18);
        STRUCT_PLACE(teList<int>, id_to_cache_index, 0x70);
        STRUCT_PLACE(teList<char>, value_moves, 0xA0);
        STRUCT_PLACE(teList<__int64>, list_C0, 0xC0); //unk size
    };
};

struct DataFlowComponent_ValueStack // sizeof=0xC0
{
    DataFlowValue value_stack[8];
    int value_stack_dataflow_ids[8];
    char value_stack_assigned[8];
    DataFlow_ComponentRefBase* entref;
    char stack_idx_bottom;
    char stack_idx_top;
    char needs_default_value_bits;
    char needs_comp2_1_fun;
};

/* 865 */
struct DataFlowComponent_IdMappingItem //sizeof=0x20
{
    int *id_list;
};



struct DataFlowComponent_IdMapping
{
    DataFlowComponent_IdMappingItem split_mappings[16];
    char mapping_values[16][8];
    char split_item_counts[16];
    DataFlowComponent_IdMapping *next;
};

/// Value Modes:
/// > 4: Dont get from cache, re-get from dataflow provider
/// >= 3: Get from Override cache / if not exists get default
/// != 3: Emplace in transient cache
/// FirstPerson DFProvider: if <= 2 return value_mode sonst 3
struct Component_3_DataFlow
{
    union
    {
        ComponentBase base;
        STRUCT_PLACE(Component_1_SceneRendering*, comp1_ref, 18);
        STRUCT_PLACE(teList<DataFlow_ComponentRefBase*>, component_refs, 0x20);
        STRUCT_PLACE(teList<__int64>, field_38, 0x38);
        STRUCT_PLACE(DataFlow_ComponentRefBase, dataflow_ref, 0x50);
        STRUCT_PLACE(Dataflow_Cache, immediate_cache, 0x70);
        STRUCT_PLACE(Dataflow_Cache, transient_cache, 0x170);
        STRUCT_PLACE(Dataflow_Cache, value_overrides, 0x170);
        STRUCT_PLACE(teList<DataFlowComponent_ValueStack*>, value_stacks, 0x370);
        STRUCT_PLACE(DataFlowComponent_IdMapping, id_to_stack_mapping, 0x3A0);
        STRUCT_PLACE(int, last_accessed_commandFrame_2, 0x638);
        STRUCT_PLACE(int, last_accessed_commandFrame, 0x63C);
        STRUCT_PLACE(char, field_640, 0x640);
        STRUCT_PLACE(char, field_641, 0x641);
    };
};

#endif //PROMETHEUS_COMPONENT_3_DATAFLOW_H