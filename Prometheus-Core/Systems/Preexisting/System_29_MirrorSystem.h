#ifndef PROMETHEUS_MIRRORSYSTEM_H
#define PROMETHEUS_MIRRORSYSTEM_H
#include "Systems/SystemBase.h"

struct System_29_MirrorSystem;
struct NetworkEventSystem_Callback_vt;

struct System_29_MirrorSystem
{

    struct DeserializeCallback
    {
        struct CallData
        {
            ComponentBase* component;
            __int64 mirror_data;
            System_29_MirrorSystem* mirror_system;
        };

        __int64 (*function)(ComponentBase*, __int64 mirror_data, CallData* call_data, __int64 user_data);
        __int64 user_data;
    };

    struct EntityCreationCallback
    {
        void (*Function)(Entity*, __int64 user_data);
        __int64 user_data;
    };

    typedef TemplatedHashMap<__int64, teList<DeserializeCallback>, false, 0x100> ComponentUpdateCallbackHashmap;

    union
    {
        SystemVTBase* vfptr;
        STRUCT_PLACE(EntityAdminBase*, game_ea, 8);
        STRUCT_PLACE(NetworkEventSystem_Callback_vt*, network_callback, 0x10);
        STRUCT_PLACE(__int64, replaycontroller_eventhandler, 0x18);
        STRUCT_PLACE(__int64, vfptr_3, 0x20);
        STRUCT_PLACE(mapsystem_callback_vt*, map_change_callback, 0x28);

        STRUCT_PLACE(ComponentUpdateCallbackHashmap, pre_deserialize_callback, 0x30);
        STRUCT_PLACE(ComponentUpdateCallbackHashmap, post_deserialize_callback, 0x58);
        STRUCT_PLACE(teList<DeserializeCallback>, entity_creation_callback, 0x80);
        STRUCT_PLACE(teList<EntityCreationCallback>, entity_creation_callbacks, 0x80);
        STRUCT_PLACE(teList<__int64>, mirrored_ent_ops, 0x98); //Pretty useless, mirrored ent operations size 0x1A0
        //Client waits for these placeables to be placed. If there are any state is not changed from World Replicating to World Running
        //Placed in that sense means scaling, and MovementState applied
        //The ints here are indices for Placeable_LoadEntry in Placeable_ResourceLoader
        //These are sent from the server which implies these are deterministic (.... duh (..... ?))
        STRUCT_PLACE(teList<int>, outstanding_server_placeables, 0xB0);
    };

    static System_29_MirrorSystem* Get(EntityAdminBase* ea)
    {
        auto fn = (System_29_MirrorSystem*(*)(EntityAdminBase*))(globals::gameBase + 0xc038d0);
        return fn(ea);
    }
};

#endif //PROMETHEUS_MIRRORSYSTEM_H