#ifndef PROMETHEUS_COMPONENT_54_LOBBYMAP_H
#define PROMETHEUS_COMPONENT_54_LOBBYMAP_H
#include "Component_2_AssetManager.h"

//Embedded Entity Admin!
struct Component_54_Lobbymap {
    union {
        ComponentBase base;
        STRUCT_PLACE(EntityAdminBase*, embedded_game_ea, 0x28);
        STRUCT_PLACE(__int64, wanted_map_id, 0x30);
        STRUCT_PLACE(Component_2_AssetManager*, component_2_ref, 0x18);
    };

    void UnloadMap() {
        auto forceunload_lobbymap = (void(*)(Component_54_Lobbymap*))(globals::gameBase + 0xce74a0);
        forceunload_lobbymap(this);
    }

    void LoadMap(__int64 mapId) {
        wanted_map_id = mapId;
        auto forceload_lobbymap = (void(*)(Component_54_Lobbymap*))(globals::gameBase + 0xce6ef0);
        forceload_lobbymap(this);
    }

    int GetMapLoadState() {
        if (component_2_ref && component_2_ref->scene)
            return component_2_ref->scene->load_map_state;
        return 0;
    }
};

#endif //PROMETHEUS_COMPONENT_54_LOBBYMAP_H