#ifndef PROMETHEUS_COMPONENT_H
#define PROMETHEUS_COMPONENT_H

struct ComponentBase;
struct ComponentCreator;
struct ComponentCreator_vt {
    __int64 (*destruct)(ComponentCreator*, char);
    ComponentBase *(__fastcall *create_component)(ComponentCreator *);
    //TODO Dont know what those functions are
    __int64 (__fastcall *field_10)(__int64, void (__fastcall ***)(_QWORD, _QWORD));
    __int64 (__fastcall *field_18)(__int64);
};

struct ComponentCreator {
    union {
        ComponentCreator_vt* vfptr;
        STRUCT_PLACE(char, component_id, 8);
        STRUCT_PLACE(StructPageAllocator*, iclass, 0x28);
        STRUCT_PLACE(__int64, iwelche_flags, 0x30); //Always seems to be 0x14
    };
};

struct ComponentRTTI
{
    ComponentRTTI* next;
    __int16 component_id;
    ComponentCreator*(__fastcall *create_componentCreator)();
    __int64 field_18; //Every pointer seems to point at EntityAdminCreationInfo

    static ComponentRTTI* Get() {
        return *(ComponentRTTI**)(globals::gameBase + 0x18f74d0);
    }

    static ComponentRTTI* GetForComponent(int compoId) {
        auto rtti = Get();
        while (rtti) {
            if (rtti->component_id == compoId) {
                return rtti;
            }
            rtti = rtti->next;
        }
        owassert(false);
        return nullptr;
    }
};

struct ComponentBase;
struct Component_vt {
    class GetMirrorDataHelper {
        virtual void Callback(ComponentBase*, __int64* mirrorData) {
            MirrorDataPtr = mirrorData;
        }

    public:
        __int64* MirrorDataPtr = nullptr;
    };

    __int64(__fastcall* deallocate)(__int64, char);
    void(__fastcall * CallForDeserialize)(ComponentBase*, GetMirrorDataHelper*);
    char (*OnCreate)(ComponentBase*, __int64 /*EntityLoader**/);
    void(__fastcall* field_1)();
    void(__fastcall* field_2)();
    char (*field_3)();
};

struct Entity;
struct ComponentBase {
    Component_vt* vfptr;
    Entity* entity_backref;
    unsigned char component_id;
    int is_mirrored; //TODO that is wrong. Why do some components have 1 and some have 0?

    __int64* GetMirrorData() {
        Component_vt::GetMirrorDataHelper helper{};
        vfptr->CallForDeserialize(this, &helper);
        return helper.MirrorDataPtr;
    }
};

#endif //PROMETHEUS_COMPONENT_H