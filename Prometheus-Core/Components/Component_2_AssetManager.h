#ifndef PROMETHEUS_COMPONENT_2_ASSETMANAGER_H
#define PROMETHEUS_COMPONENT_2_ASSETMANAGER_H

#include "globals.h"
#include "Component.h"

//TODO: teStagedObjectMgr

struct Component_2_AssetManager;

//https://github.com/overtools/OWLib/blob/develop/TankLib/Enums.cs
//NOTE: Not everything is implemented in 0.8 yet (for example Text is not)
enum teMAP_PLACEABLE_TYPE : char {
    UNKNOWN = 0,
    MODEL_GROUP = 0x1,
    SINGLE_MODEL = 0x2,
    OCCLUDER = 0x3,
    REFLECTIONPOINT = 0x4,
    CLUTTER = 0x5,
    LABEL = 0x6,
    TEXT = 0x7,
    MODEL = 0x8,
    LIGHT = 0x9,
    AREA = 0xA,
    ENTITY = 0xB,
    SOUND = 0xC,
    EFFECT = 0xD,
    FOG = 0xE,
    INDOORBOX = 0xF,
    POSTPROCESSING = 0x10,
    PLANAR_REFLECTION_SURFACE = 0x11,
    // MAP_REGION = 0x12,
    // SOUNDAREA = 0x13
    // COLLISION = 0x14, ??
    // PATHING = 0x15,

    SEQUENCE = 0x18
};

inline std::string teMAP_PLACEABLE_toString(teMAP_PLACEABLE_TYPE type)
{
    switch (type)
    {
    case MODEL_GROUP:
        return "MODEL_GROUP";
    case SINGLE_MODEL:
        return "SINGLE_MODEL";
    case OCCLUDER:
        return "OCCLUDER";
    case REFLECTIONPOINT:
        return "REFLECTIONPOINT";
    case CLUTTER:
        return "CLUTTER";
    case LABEL:
        return "LABEL";
    case TEXT:
        return "TEXT";
    case MODEL:
        return "MODEL";
    case LIGHT:
        return "LIGHT";
    case AREA:
        return "AREA";
    case ENTITY:
        return "ENTITY";
    case SOUND:
        return "SOUND";
    case EFFECT:
        return "EFFECT";
    case FOG:
        return "FOG";
    case INDOORBOX:
        return "INDOORBOX";
    case POSTPROCESSING:
        return "POSTPROCESSING";
    case PLANAR_REFLECTION_SURFACE:
        return "PLANAR_REFLECTION_SURFACE";
    case UNKNOWN:
    default:
        return "UNKNOWN (" + std::to_string(type) + ")";
    }
}

enum Placeable_LoadState : char
{
    LoadState_Error = 2,
    LoadState_Loading = 3,
    LoadState_Loaded = 4,
    LoadState_Spawned = 5, //Final state
    LoadState_NeedsEntitySpawn = 8,
};

//This is a flag enum
enum Placeable_FilterFlags : unsigned char
{
    FilterFlag_NeedsFilter = 0x40, //Server side placeable => not loaded by client?
    FilterFlag_DontLoad = 0x80, //Dont load resource, loadstate will stay at 0
};


//This is loaded directly out of CASC.
//The OWLib name for this is CommonStructure.
struct Placeable_Base
{
    teUUID uuid;
    char field_10; //is server side?
    char field_11;
    char field_12;
    int size;
};

struct Placeable_LoadEntry
{
    union
    {
        //forward and back? idk... the PlaceableLoader is basically just an array in ResourceLoader
        Placeable_LoadEntry* field_0;
        STRUCT_PLACE(Placeable_LoadEntry*, field_8, 8);

        STRUCT_PLACE(__int64, hash, 0x10); //fnv1a_16bytes von ptr_for_loadFilter
        STRUCT_PLACE(Placeable_Base*, placeable_resource, 0x18);
        STRUCT_PLACE(MisalignedResourceLoadEntry*, resload_entry, 0x20);
        STRUCT_PLACE(uint, global_entity_id, 0x28);

        //Used as a short in code - but it seems those are two different enums.
        STRUCT_PLACE(Placeable_LoadState, load_state, 0x30);
        STRUCT_PLACE(Placeable_FilterFlags, filter_flags, 0x31);

        STRUCT_PLACE(teMAP_PLACEABLE_TYPE, map_placeable_type, 0x32);
        STRUCT_PLACE(char, field_33, 0x33);
        STRUCT_PLACE(int, field_34, 0x34);
    };
};

struct teScene_ResourceLoader
{
    union
    {
        Component_2_AssetManager* comp2_backref;
        STRUCT_PLACE(MisalignedResourceLoadEntry*, resload_entry, 0x8);
        STRUCT_PLACE(__int32, field_10, 0x10);
        STRUCT_PLACE(__int32, placeables_num, 0x14);
        STRUCT_PLACE(__int64, field_18, 0x18);
        STRUCT_PLACE(__int32, field_20, 0x20);
        STRUCT_PLACE(__int32, field_24, 0x24);
        STRUCT_PLACE(__int16, field_28, 0x28);
        STRUCT_PLACE(__int16, field_2A, 0x2A);
        STRUCT_PLACE(__int32, field_2C, 0x2C);
        STRUCT_PLACE(Placeable_LoadEntry*, placeables, 0x30);
    };
};

struct teScene {
    union {
        __int64 fiwld_0; //seems to be a subclass of teStagedObject (used by teStagedObjectManager)
        STRUCT_PLACE(__int64, entity_id_to_load, 0x8);
        STRUCT_PLACE(__int64, field_10, 0x10); //0C00000000000003
        STRUCT_PLACE(__int64, field_18, 0x18); //0B000000000004AB
        STRUCT_PLACE(__int64, field_20, 0x20); //0C000000000015DF
        STRUCT_PLACE(__int64, field_28, 0x28); //0D00000000000E4B
        STRUCT_PLACE(__int64, field_30, 0x30); //0980000000000FF2
        STRUCT_PLACE(__int64, field_38, 0x38); //02A0000000000051
        STRUCT_PLACE(__int64, field_40, 0x40); //0C000000000054E3
        STRUCT_PLACE(__int64, field_48, 0x48); //0C000000000054E4
        STRUCT_PLACE(__int64, field_50, 0x50); //0DE000000000192A "THE UNFEELING VOID"
        STRUCT_PLACE(__int64, field_58, 0x58);

        //...

        STRUCT_PLACE(__int64, wanted_map_id, 0x150);
        STRUCT_PLACE(__int64, field_158, 0x158);

        // 0: load stopped
        // 1: loading
        // 3: Map Load Complete
        // 4: map loaded
        STRUCT_PLACE(int, load_map_state, 0x160); //not world system / engine state!
        STRUCT_PLACE(teScene_ResourceLoader*, resource_loader, 0x168);
    };
};

struct Component_2_AssetManager {
    //Component 2 cannot hurt me if I have not created an IDA structure for it
    //checkmate, science!
    union {
        ComponentBase base;
        STRUCT_PLACE(teScene*, scene, 0x3A8);
    };
};

#endif //PROMETHEUS_COMPONENT_2_ASSETMANAGER_H