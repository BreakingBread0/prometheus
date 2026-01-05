#pragma once
#include "globals.h"
#include "game.h"
#include <memory>
#include "idadefs.h"
#include <stdexcept>
#include <map>
#include <vector>

struct Entity;
struct EntityAdminBase;
struct Component_23_Statescript;
struct View;
struct teFreeLookView;
struct MisalignedResourceLoadEntry;
struct PrometheusSystem;
struct ComponentBase;

inline void EntityIDToString(uint* entid, char* buf, int sz) {
    ((void(*)(uint*, char*, int))(globals::gameBase + 0x7f91d0))(entid, buf, sz);
}

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

struct Entity {
	union {
        Entity* parent;
        STRUCT_PLACE(Entity*, child, 0x8);
        STRUCT_PLACE(Entity*, next_child, 0x10);
        STRUCT_PLACE(EntityAdminBase*, entity_admin_backref, 0x28);
        //STRUCT_PLACE(Entity*, field_20_ent, 0x20); // falsch
		//Immer count - 1
		STRUCT_PLACE(teList<ComponentBase*>, component_list, 0x50);
        STRUCT_PLACE(uint32, entity_global_id, 0x90);
        STRUCT_PLACE(uint32, entity_id, 0x94);
        STRUCT_PLACE(MisalignedResourceLoadEntry*, resload_entry, 0x30);
        STRUCT_PLACE(MisalignedResourceLoadEntry*, resload_entry2, 0x38); //F�r STUSkinTheme
        STRUCT_PLACE(MisalignedResourceLoadEntry*, resload_entry3, 0x40); //F�r STUSkinBase
    };

    inline ComponentBase* getById(int id) {
        for (int i = 0; i < component_list.num - 1; i++) {
            if (component_list.ptr[i]->component_id == id)
                return component_list.ptr[i];
        }
        return nullptr;
    }

    inline bool hasComponent(int id) {
        return getById(id) != nullptr;
    }
    
    template <typename T>
    inline T* getById(int id) {
        return (T*)getById(id);
    }
};

struct Component_50_Input {
    struct InputStruct {
        char tick0[0xc3];
        char tick1[0xc3];
    };
    union {
        ComponentBase base;
        STRUCT_PLACE(int, tick10, 0x40);
        STRUCT_PLACE(char, field_4C, 0x4C);
        STRUCT_PLACE(char, not_ingame, 0x4D);
        STRUCT_PLACE(float, pause_input_for, 0x144);
        STRUCT_PLACE(InputStruct, stru1, 0x148);
        STRUCT_PLACE(InputStruct, stru2, 0x2CE);
        STRUCT_PLACE(InputStruct, stru3, 0x454);
        STRUCT_PLACE_CUSTOM(stru4, 0x5E6, char stru4[0xc3]);
    };
};

struct teScene {
    union {
        // 0: load stopped
        // 1: loading
        // 3: Map Load Complete
        // 4: map loaded
        STRUCT_PLACE(int, load_map_state, 0x160); //not world system / engine state!
    };
};

struct Component_2_AssetManager {
    union {
        ComponentBase base;
        STRUCT_PLACE(teScene*, scene, 0x3A8);
    };
};

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

struct Component_20_ModelReference {
    union {
        ComponentBase base;
        STRUCT_PLACE(uint32, aim_entid, 0x90);
        STRUCT_PLACE(uint32, cam_attach_entid, 0x94);
        STRUCT_PLACE(uint32, movement_attach_entid, 0x98);
    };
};

//STUPvPGameComponent
struct Component_24_PvPGame {
    union {
        ComponentBase base;
        //0x20-0x98 (0xC0): MirrorData
        STRUCT_PLACE(__int64, current_gamestate_d8, 0x98);
        STRUCT_PLACE(int, overtime_remaining, 0xA0);
        STRUCT_PLACE(float, field_A4, 0xA4);
        STRUCT_PLACE(float, field_A8, 0xA8);
        STRUCT_PLACE(float, field_AC, 0xAC);
        STRUCT_PLACE(int, some_team_id, 0xB0);
        STRUCT_PLACE(int, field_B4, 0xB4);
        STRUCT_PLACE(int, current_round, 0xB8);
        STRUCT_PLACE(int, counter_decreasing, 0xC0);
    };
};

struct Component_3F_PlayerInfo {
    union {
        ComponentBase base;
        STRUCT_PLACE(Component_20_ModelReference*, pet_ref, 0x18);
        
        //Alles ist MirrorData
        STRUCT_PLACE(__int64, selected_heroid, 0x78 + 0x20);
        STRUCT_PLACE(__int64, unconformed_selected_heroid, 0x90 + 0x20); //When hero is not finally chosen yet
        STRUCT_PLACE(char, m_slot, 0xa0 + 0x20);
        STRUCT_PLACE(char, localplayer_heroselected, 0xa1 + 0x20);
        STRUCT_PLACE(char, hero_was_selected, 0xa2 + 0x20);
        STRUCT_PLACE(char, hero_select_enabled, 0xa3 + 0x20);
    };
};

struct Component_10_FilterBits {
    union {
        ComponentBase base;

        //Alle values vom ReplayController
        STRUCT_PLACE(short, field_8C, 0x8C);
        STRUCT_PLACE(char, field_8E, 0x8e);
        STRUCT_PLACE(__int32, field_20, 0x20);
        STRUCT_PLACE(__int32, field_48, 0x48); 
        STRUCT_PLACE(__int64, pre_filter_bits, 0x90); //0x400001
        STRUCT_PLACE(__int64, filter_bits, 0x98); //0x400001
    };

    void set_team(int team) {
        owassert(team >= 0 && team < 4);
        filter_bits &= ~(__int64)0x3C0000;
        switch (team) {
        case 0:
            filter_bits |= 0x40000;
            break;
        case 1:
            filter_bits |= 0x80000;
            break;
        case 2:
            filter_bits |= 0x100000;
            break;
        case 3:
            filter_bits |= 0x200000;
            break;
        }
    }

    //default values f�r controller entity
    void set_filterbits_spectator() {
        field_8C = 0x101;
        field_8E = 1;
        field_20 |= 1;
        field_48 |= 1;
        pre_filter_bits = 0x400001;
    }

    void FilterBitsMakeFinal() {
        reinterpret_cast<void(*__fastcall)(Component_10_FilterBits*)>(globals::gameBase + 0xcbf660)(this);
    }
};

struct View;
struct OverrideView {
    View* view_ptr;
    char is_enabled;
};

struct Component_1F_Backref {
    union {
        ComponentBase base;
        STRUCT_PLACE(uint32, backref_entid, 0x18);
        STRUCT_PLACE(uint32, backref_global_entid, 0x1C);

        STRUCT_PLACE(teList<OverrideView>, override_views, 0x128);
    };
};

struct HealthPart {
    union {
        int part_enabled;
        STRUCT_PLACE(float, max_health, 4);
        STRUCT_PLACE(float, curr_health, 8);
        STRUCT_PLACE(char, field_10, 0x10);
        STRUCT_PLACE(char, some_other_flag, 0x11);
        STRUCT_PLACE(__int64, field_12, 0x12);
    };
};

struct Component_28_STUHealthComponent;
struct STUHealthComponent_vt {
    Component_vt component;
    //WARN: Those are all return address checked!
    float(__fastcall* GetCurrentHealth)(Component_28_STUHealthComponent*, int healthType);
    float(__fastcall* GetMaxHealth)(Component_28_STUHealthComponent*, int healthType);
    float(__fastcall* GetCurrHealth_IfFlag)(Component_28_STUHealthComponent*, int healthType);
    float(__fastcall* GetMaxHealth_IfFlag)(Component_28_STUHealthComponent*, int healthType);
    float(__fastcall* GetFullCurrentHealth)(Component_28_STUHealthComponent*);
    float(__fastcall* GetFullCurrentHealth_IfFlag)(Component_28_STUHealthComponent*);
    float(__fastcall* GetFullMaxHealth)(Component_28_STUHealthComponent*);
    float(__fastcall* GetFullMaxHealth_IfFlag)(Component_28_STUHealthComponent*);
};

struct Component_28_STUHealthComponent {
    union {
        ComponentBase base;
        STUHealthComponent_vt* vfptr;

        //TODO: May be wrong. This is STU MirrorData
        //Plus multiple fields have the saame name. WTF!
        STRUCT_PLACE_ARRAY(HealthPart, normal_health, 0x10, 0x98);
        STRUCT_PLACE_ARRAY(HealthPart, armour_health, 0x10, 0x1D8);
        STRUCT_PLACE_ARRAY(HealthPart, shield_health, 0x10, 0x318);
    };
};

enum class WalkDirectionLR : unsigned char {
    NONE = 0,
    MOVE_RIGHT = 0x7F,
    MOVE_LEFT = 0x81
};
enum class WalkDirectionFB : unsigned char {
    NONE = 0,
    MOVE_FORWARD = 0x7F,
    MOVE_BACKWARD = 0x81
};

struct WalkDirection {
    WalkDirectionLR left_right;
    WalkDirectionFB forward_backward;

    std::string to_string() {
        std::string result = "";
        if (left_right == WalkDirectionLR::MOVE_RIGHT) {
            result = "right";
        }
        else if (left_right == WalkDirectionLR::MOVE_LEFT) {
            result = "left";
        }
        if (forward_backward == WalkDirectionFB::MOVE_BACKWARD) {
            if (left_right != WalkDirectionLR::NONE)
                result += "&";
            result += "backward";
        }
        else if (forward_backward == WalkDirectionFB::MOVE_FORWARD) {
            if (left_right != WalkDirectionLR::NONE)
                result += "&";
            result += "forward";
        }
        return result;
    }
};

struct Component_2F_LocalPlayer {
    struct MovementCommand {
        int field_0;
        short left_right_lookdir;
        short up_down_lookdir; //0 = center, - = up, + = down
        //char left_right_movement; //0x7f: move right, 0x81: move left
        //char forward_backward_movement; //0x7f: move forward, 0x81: move backward
        WalkDirection direction;
        short ability_button_states; // 1: shoot, 2: right click, 0x10: ability1, 0x20: ability2, 0x40: ult, 0x100: jump, 0x200: crouch
        char field_C;
        char field_D;
    };

    union {
        ComponentBase base;

        STRUCT_PLACE(Component_20_ModelReference*, pet_reference, 0x18);
        STRUCT_PLACE(float, field_40, 0x40);
        STRUCT_PLACE(float, field_44, 0x44);
        STRUCT_PLACE(int, mover_type, 0x50);
        STRUCT_PLACE(int, is_thirdperson_model, 0x2D0);
        STRUCT_PLACE(__int64, field_550, 0x550);
        STRUCT_PLACE(int, field_558, 0x558);

        STRUCT_PLACE(char, is_able_to_aim, 0x7D0);
        STRUCT_PLACE(char, field_7D1, 0x7D1); //wird am anfang vom movementsys auf 0 geforced, is_initializing?
        STRUCT_PLACE(MovementCommand, field_1460, 0x1460);
        STRUCT_PLACE(MovementCommand, curr_movcommand, 0x1470);
        STRUCT_PLACE(MovementCommand, copied_movcommand, 0x1480);

        STRUCT_PLACE(float, mouse_rel_x, 0x1490);
        STRUCT_PLACE(float, mouse_rel_y, 0x1494);
        STRUCT_PLACE(float, field_1498, 0x1498);
        STRUCT_PLACE(float, field_149C, 0x149C);
        STRUCT_PLACE(char, crouch_toggle, 0x14A0);

        //View Projection Matrix
        /*STRUCT_PLACE(Vector4, viewproj_matrix_0, 0x14B0);
        STRUCT_PLACE(Vector4, viewproj_matrix_1, 0x14c0);
        STRUCT_PLACE(Vector4, viewproj_matrix_2, 0x14d0);
        STRUCT_PLACE(Vector4, viewproj_matrix_3, 0x14e0);*/
        STRUCT_PLACE(Vector4, view_angles_radian, 0x14D0); //2D VECTOR

        STRUCT_PLACE(Vector4, empty_vec, 0x14f0);
        STRUCT_PLACE(Vector4, current_position, 0x1500);
        STRUCT_PLACE(Vector4, current_position_copied, 0x1510);
        STRUCT_PLACE(int, some_entid, 0x1520);

        STRUCT_PLACE(teList<__int64>, list_1530, 0x1530); //size unk
        STRUCT_PLACE(char, render_thirdperson, 0x15D0);
        STRUCT_PLACE(char, field_15D1, 0x15D1);

        STRUCT_PLACE_ARRAY(uint32, entids_array, 32, 0x15E0);
        STRUCT_PLACE(int, field_17E0, 0x17E0);
        STRUCT_PLACE(int, field_17F0, 0x17F0);
        STRUCT_PLACE(int, field_17F4, 0x17F4);
        STRUCT_PLACE(teList<__int64>, list_1800, 0x1800); //size unk

        STRUCT_PLACE(int, field_18C0, 0x18C0);
        STRUCT_PLACE(int, field_18C4, 0x18C4);
        STRUCT_PLACE(__int64, field_18C8, 0x18C8);
        STRUCT_PLACE(int, field_18D0, 0x18D0);
        STRUCT_PLACE(int, field_18D4, 0x18D4);
        STRUCT_PLACE(__int64, field_18D8, 0x18D8);
        STRUCT_PLACE(float, add_to_angle_x, 0x18E0);
        STRUCT_PLACE(float, add_to_angle_y, 0x18E4);
        STRUCT_PLACE(float, field_18E8, 0x18E8);
        STRUCT_PLACE(float, field_18EC, 0x18EC);
    };

    void SetRotation(Vector4 rot) {
        ((void(*)(__int64, Component_2F_LocalPlayer*, Vector4*))(globals::gameBase + 0xca6fb0))(0, this, &rot);
    }
};

struct Component_4F_Camera {
    struct Camera {
        union {
            __int64 vfptr;
            STRUCT_PLACE(View*, current_view, 0x260);
        };
    };
    union {
        ComponentBase base;
        STRUCT_PLACE(teList<OverrideView>, override_views, 0x18);
        STRUCT_PLACE(View*, default_view, 0x50);
        STRUCT_PLACE(Camera*, camera, 0x40);
        STRUCT_PLACE(__int64, crosshair_struct, 0x48);
    };

    Vector3 WorldToScreen(Vector3 input) {
        auto w2s_fn = (void(*)(__int64 crosshair_stru, Vector3*, Vector3*))(globals::gameBase + 0xa1fa70);
        Vector3 result;
        w2s_fn(crosshair_struct, &input, &result);
        return result;
    }
};

struct Component_1_SceneRendering {
    struct InitData {
        Vector4 position;
        Vector4 scale;
        Vector4 rotation;
    };

    union {
        ComponentBase base;

        STRUCT_PLACE(uint64, rendering_flags, 0x180);
        STRUCT_PLACE(Vector4, rotation, 0x40);
        STRUCT_PLACE(Vector4, scaling, 0x50);
        STRUCT_PLACE(Vector4, position, 0x60);
    };

    void RemoveFlag(uint64 flag) {
        auto func = (void(*)(Component_1_SceneRendering*, uint64))(globals::gameBase + 0x9e5b70);
        func(this, flag);
    }

    void AddFlag(uint64 flag) {
        auto func = (void(*)(Component_1_SceneRendering*, uint64))(globals::gameBase + 0x9e5c80);
        func(this, flag);
    }

    //From 0xd5b460
    void SetInvisible() {
        RemoveFlag(0x200000000000LL);
        RemoveFlag(0x8000);
    }

    void SetVisible() {
        AddFlag(0x200000000000LL);
        AddFlag(0x8000);
    }

    bool IsVisible() {
        return (rendering_flags & 0x200000008000) != 0x200000008000;
    }

    //Only x,y,z
    void SetScale(Vector4 scale) {
        ((void(*)(Component_1_SceneRendering*, Vector4*))(globals::gameBase + 0xa009a0))(this, &scale);
    }

    void SetPosRotation(Vector4 position, Vector4 rotation) {
        ((void(*)(Component_1_SceneRendering*, Vector4 * rot, Vector4 * pos))(globals::gameBase + 0xa04000))(this, &rotation, &position);
    }
};

struct HashMap_vt {
    void deallocate(HashMap_vt**, char deallocate);
    void deallocate_outer(HashMap_vt**);
};

//TODO
//key must be >= 8 bytes!!!
template <typename key, typename value>
struct TemplatedHashMap {
    struct HashMapEntry {
        HashMapEntry* next_ptr;
        value* value_ptr;
        key id_unhashed;
        __int64 hash;
    };

    union {
        HashMap_vt* vfptr;
        STRUCT_PLACE(char, field_8, 8); //0x20 default
        STRUCT_PLACE(uint32, flags, 0xC);
        STRUCT_PLACE(int, root_entries_cnt, 0x10); //default 0x40-0x100 
        STRUCT_PLACE(int, items_num, 0x14);
        STRUCT_PLACE(HashMapEntry**, entries, 0x18); //Base map is 0x200 elements big
    };

    /*static __int64 HashKey(key input) {
        return 0x80000001
            * ((0x15
                * ((0x109 * ((~v10 + (v10 << 0x15)) ^ ((unsigned __int64)(~v10 + (v10 << 0x15)) >> 0x18)))
                    ^ ((0x109 * ((~v10 + (v10 << 0x15)) ^ ((unsigned __int64)(~v10 + (v10 << 0x15)) >> 0x18))) >> 0xE)))
                ^ ((0x15
                    * ((0x109 * ((~v10 + (v10 << 0x15)) ^ ((unsigned __int64)(~v10 + (v10 << 0x15)) >> 0x18)))
                        ^ ((0x109 * ((~v10 + (v10 << 0x15)) ^ ((unsigned __int64)(~v10 + (v10 << 0x15)) >> 0x18))) >> 0xE))) >> 0x1C));
    }*/
};

struct Component_53_Settings {
    union {
        ComponentBase base;
        STRUCT_PLACE(char, tutorial_dialog_dismissed, 0x148);
        //hashmap 0xD0 size 0x98
    };
};

struct EntityIntermediate // sizeof=0x10
{
    uint32 entitiy_id;
	__int32 field_4;
	Entity* ent;
};

struct EntityListItem // sizeof=0x10
{
	EntityIntermediate* entity_im;
	__int32 is_possessed_by_a_demon_;
	__int32 EIGHT;
};

struct StatescriptFlagger_vt // sizeof=0x18
{                                       // XREF: sub_7FF650CE84A0+A4/r
    void(__fastcall* field_0)(StatescriptFlagger_vt*);
    void(__fastcall* field_8)(StatescriptFlagger_vt*);
    void (* SetStatescriptFlags)(StatescriptFlagger_vt**, uint);
};

struct EntityAdmin_vt // sizeof=0xF8
{
    InheritanceInfo_destructor_vt rtti;
    __int64(__fastcall * field_20)(__int64);
    char(__fastcall* PreDeleteMby)(__int64);
    __int64(__fastcall * *(__fastcall * field_30)(_DWORD*))();
    __int64(__fastcall* field_38)(__int64);
    void(__fastcall * nullsub_1)();
    void(__fastcall* field_0)();
    void(__fastcall * field_1)();
    void(__fastcall* field_2)();
    __int64(__fastcall * field_60)(__int64);
    __int64* (__fastcall* field_68)(__int64*);
    __int64* (__fastcall * field_3)(__int64*);
    __int64 (*ret_0)();
    StatescriptFlagger_vt** (__fastcall* GetStatescriptFlagger)(EntityAdminBase*);
    __int64 (*field_4)();
    __int64 (*field_90)();
    __int64 (*field_5)();
    __int64 (*field_6)();
    __int64 (*field_7)();
    __int64 (*field_9)();
    __int64 (*field_11)();
    __int64 (*field_12)();
    const char* (*GetIsLiveReplayOrDeathStar)();
    __int64(__fastcall * field_D0)(__int64, __int64);
    __int64(__fastcall * field_D8)(__int64);
	    __int64(__fastcall * StrangeFuncWithGamepadRenderDebug)(__int64);
	    __int64 (*field_E8)();
    __int64(__fastcall * some_deallocator)(__int64);
};

struct EntityAdmin_ComponentIterator // sizeof=0x18
{
    teList<ComponentBase*> component_list;
    __int64* field_10;
};

struct system_vt // sizeof=0x70
{
    InheritanceInfo_vt inheritance;
    char* (*GetSystemName)();
    _QWORD* (__fastcall* deallocate)(_QWORD*, char);
    void (*OnInitialize)();
    void(__fastcall* nullsub_1)();
    void(__fastcall* PreDeleteMby)();
    char* (*get_subscriptions)(__int64, int* components_out_cnt);
    void(__fastcall* field_2)(system_vt**, Entity*);
    void(__fastcall* subscribe_oncreate_callback)(system_vt**, Entity*);
    void(__fastcall* field_4)();
    void(__fastcall* subscribe_ondelete_callback)(__int64, Entity*);
    void(__fastcall* Tick_mby)(__int64, float);
};

//aka callbacks for various map states
struct mapsystem_callback_vt {
    void(__fastcall* field_0)();
    void(__fastcall* nullsub_1)(__int64);
    void(__fastcall* change_state_3_globalloading)(__int64);
    void(__fastcall* change_state_4_worldloading)(__int64);
    void(__fastcall* change_state_5_worldloaded)();
    void(__fastcall* change_state_6_world_replicating)(__int64);
    void(__fastcall* change_state_7_gamemode_loading)(__int64);
    void(__fastcall* maybe_on_gamemode_unload)(__int64);
    void(__fastcall* OnGamemodeLoadingInGameEA)(__int64);
    void(__fastcall* maybe_on_map_unload)(__int64); // or map load finished?
    void(__fastcall* OnWorldLoadedAndInGameEnttiyAdmin)(__int64);
    void(__fastcall* OnWorldLoadingAndInGameEntityAdmin)(__int64);
    void(__fastcall* field_5)(__int64);
};

struct movement_vt // sizeof=0x18
{
    //Calling order each tick: field_1, field_0, field_2
    __int64(__fastcall * field_0)(__int64, unsigned int current_cf); // Tick funktion?
    void(__fastcall* field_1)(__int64);
    void(__fastcall * field_2)(__int64);
};

struct CommandFrameTime // sizeof=0x18
{                                       
    float command_frame_length_secs;    // 60fps = 0,016
    double timer_secs_double;
    __int64 timer_secs; // * 1000
};

struct EntityAdminTiming {
    union {
        EntityAdminBase* ea_backref;
        STRUCT_PLACE(CommandFrameTime, cf_timer_1, 8);
        STRUCT_PLACE(CommandFrameTime, cf_timer_2, 0x20);
        STRUCT_PLACE(int, tick_count, 0x38);
        STRUCT_PLACE(int, field_3C, 0x3C);
        STRUCT_PLACE(__int64, field_40, 0x40);
        STRUCT_PLACE(float, field_48, 0x48);
        STRUCT_PLACE(float, field_50, 0x50);
        STRUCT_PLACE(__int64, field_58, 0x58);
        STRUCT_PLACE(int, field_60, 0x60);
        STRUCT_PLACE(teList<__int64>, list_70, 0x70);

        STRUCT_PLACE(float, field_D0, 0xD0);
        STRUCT_PLACE(float, field_D4, 0xD4);
        STRUCT_PLACE(float, field_D8, 0xD8);
        STRUCT_PLACE(float, field_DC, 0xDC);
        STRUCT_PLACE(float, field_E0, 0xE0);

        STRUCT_PLACE(int, field_E4, 0xE4);
        STRUCT_PLACE(int, field_E8, 0xE8);
        STRUCT_PLACE(int, field_EC, 0xEC);

        STRUCT_PLACE(float, field_F0, 0xF0);
        STRUCT_PLACE(float, field_F4, 0xF4);
    };
};

struct System1_idk_vt {
    union {
        Component_2_AssetManager* (*GetComponent2WarumAuchImmer)(System1_idk_vt**);
    };
};

struct EntityAdminCreationInfo {
    union {
        //may not be teList, ignore max
        STRUCT_PLACE(teList<char>*, component_dependencies, 0x18);
        STRUCT_PLACE(uchar, MAX_COMPONENT_ID, 0x20);
    };
};


const int ENTITYLIST_MAX = 0x1000;
//Just threw together GameEntityAdmin, LobbyEntityAdmin, LobbyMapEntityAdmin and ReplayEntityAdmin
struct EntityAdminBase {
    union {
        EntityAdmin_vt* vfptr;
        STRUCT_PLACE(EntityListItem*, entity_list_array, 0x10);
        STRUCT_PLACE(EntityAdmin_ComponentIterator*, component_iterator, 0xb0);
        STRUCT_PLACE(EntityAdmin_ComponentIterator*, it2, 0xb8);
        STRUCT_PLACE(teList<system_vt**>, systems_array, 0xD8);
        //STRUCT_PLACE(int, local_entity_id, 0x108);
        STRUCT_PLACE(uint, local_entid, 0x220);
        STRUCT_PLACE(teList<movement_vt**>, movement_systems, 0xF0);
        STRUCT_PLACE(System1_idk_vt**, system_1_ref, 0x130); //all EA

        STRUCT_PLACE(teList<mapsystem_callback_vt**>, GameEA_mapfunc_arr, 0x180);
        STRUCT_PLACE(EntityAdminTiming*, GameEA_timing, 0x218);
        STRUCT_PLACE(EntityAdminCreationInfo*, creation_info, 0x60);

    };

    inline Entity* getEntById(uint32 id) {
        EntityListItem* entities = entity_list_array;
        for (int i = 0; i < ENTITYLIST_MAX; i++) {
            if (entities->is_possessed_by_a_demon_ && entities->entity_im->entitiy_id == id)
                return entities->entity_im->ent;
            entities++;
        }
        return nullptr;
    }

    // ==> CONTROLLER ENTITY
    inline Entity* getLocalEnt() {
        return getEntById(local_entid);
    }

    template <typename T>
    inline T* getSingletonComponent(int compid) {
        auto it = &component_iterator[compid];
        if (it->component_list.num > 0)
            return (T*)it->component_list.ptr[0];
        return nullptr;
    }

    inline void delEnt(Entity* ent) {
        if (!ent)
            return;
        auto delete_ent_fn = (void(__fastcall*)(EntityAdminBase*, Entity*, char))(globals::gameBase + 0x80ff50);
        delete_ent_fn(this, ent, 0);
    }

    class EntityIterator {
    public:
        using value_type = Entity*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;

        EntityIterator() {}
        EntityIterator(EntityAdminBase* ea) : _ea(ea) {
            this->operator++();
        }
        EntityIterator& operator++() {
            do {
                if (++_current_pos >= ENTITYLIST_MAX) {
                    _ea = nullptr;
                    _current_pos = 0;
                    break;
                }
            } while (!_ea->entity_list_array[_current_pos].is_possessed_by_a_demon_);
            return *this;
        }

        EntityIterator operator++(int) {
            EntityIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        value_type operator*() const {
            if (!_ea)
                throw std::out_of_range("Iterator out of range");
            return _ea->entity_list_array[_current_pos].entity_im->ent;
        }

        bool operator==(const EntityIterator& it) const {
            return it._current_pos == _current_pos && it._ea == _ea;
        }
    private:
        EntityAdminBase* _ea = nullptr;
        int _current_pos = 0;
    };


    EntityIterator begin() {
        return EntityIterator(this);
    }

    EntityIterator end() {
        return EntityIterator();
    }
};

struct CombatSystem_SpawnQueue {
    union {
        STRUCT_MIN_SIZE(0x20);
        uint32 entity_id;
        STRUCT_PLACE(Vector4, position, 0x10);
    };
};

struct CombatSystem {
    union {
        system_vt* vfptr;
        STRUCT_PLACE(EntityAdminBase*, game_ea, 8);
        STRUCT_PLACE(teList<uint32>, list_with_entids, 0x10);
        STRUCT_PLACE(teList<CombatSystem_SpawnQueue>, spawn_queue, 0x20);
        STRUCT_PLACE(teList<uint32*>, list_with_entids2, 0x80);
    };
};

struct EntityLoader;
struct EntityLoader_Entry;
struct EntityLoader_vt {
    //retaddress checked
    /*EntityLoader_Entry* (__fastcall* Reset_LoaderIterator)(EntityLoader*);
    EntityLoader_Entry* (__fastcall* LoaderIterator_Next)(EntityLoader*);*/
    union {
        STRUCT_PLACE_CUSTOM(l1, 0x98, char(__fastcall* load_entity_into_loader)(EntityLoader*, __int64));
    };
};

struct EntityLoader_Entry {
    unsigned char component_id;
    __int64 stu_component_data; //On STU components
    __int64 init_data; //On component 1
    __int64 stu_instance_data;

    EntityLoader_Entry() {
        component_id = 0;
        stu_component_data = 0;
        init_data = 0;
        stu_instance_data = 0;
    }
};

struct EntityLoader;
//entid_input kann null sein
inline Entity* EntityAdmin_MagicalVodoo_SpawnEntity(EntityAdminBase* entity_admin, uint32* entid_input, EntityLoader* loader) {
    auto load = (Entity * (__fastcall*)(EntityAdminBase*, uint32*, EntityLoader*))(globals::gameBase + 0x80f7c0);
    uint32* default_entid = (uint32*)(globals::gameBase + 0x185a0c8); //eh, is always 0 anyways
    if (entid_input)
        default_entid = entid_input;
    return load(entity_admin, default_entid, loader);
}

struct EntityLoader {
    union {
        STRUCT_MIN_SIZE(/*0x1860*/0x2000);
        EntityLoader_vt* vfptr;
        STRUCT_PLACE(__int64, stu_id, 0x1848);
        STRUCT_PLACE(__int64, aplist_extra, 0x1838);
        STRUCT_PLACE(__int64, additional_stu_id, 0x1840);
        STRUCT_PLACE(char, Create2FAnd33, 0x1858);
        STRUCT_PLACE(char, Create3F, 0x1859);
        STRUCT_PLACE(int, field_1854, 0x1854); //set 1
        STRUCT_PLACE(int, field_1850, 0x1850); //set 5
        STRUCT_PLACE_ARRAY(EntityLoader_Entry, loader_entries, 0xc0, 0x8);
    };

    EntityLoader() {

    }

    //can return empty
    static std::unique_ptr<EntityLoader> Create(__int64 stu_id, __int64 stu_instance, bool load_2f_33, bool load_3f) {
        auto constructor = (void(__fastcall*)(EntityLoader*))(globals::gameBase + 0x9d4c10);

        auto loader = std::unique_ptr<EntityLoader>(new EntityLoader);
        constructor(loader.get());
        loader->vfptr = (EntityLoader_vt*)(globals::gameBase + 0x15c2978);
        //loader->field_1850 = 3;
        loader->field_1850 = 5;
        loader->field_1854 = 1;
        loader->stu_id = stu_id;
        loader->additional_stu_id = stu_id; //TODO UNK
        //loader->additional_stu_id = 0x0400000000000C57;
        loader->Create2FAnd33 = load_2f_33;
        loader->Create3F = load_3f;

        if (stu_instance)
            if (!loader->vfptr->load_entity_into_loader(loader.get(), stu_instance))
                return {};

        return loader;
    }

    inline Entity* Spawn(EntityAdminBase* ea) {
        return EntityAdmin_MagicalVodoo_SpawnEntity(ea, nullptr, this);
    }
};

struct System_27_WorldEngineSystem {
    union {
        system_vt* vfptr;
        STRUCT_PLACE(EntityAdminBase*, ea, 0x8);
        STRUCT_PLACE(char, world_state, 0x10);
        STRUCT_PLACE(__int64, wanted_map_id, 0x18);
        STRUCT_PLACE(__int64, loaded_map_id, 0x20);
        STRUCT_PLACE(__int64, field_28, 0x28);
        STRUCT_PLACE(__int64, field_30, 0x30);
        STRUCT_PLACE(__int64, field_38, 0x38);
        STRUCT_PLACE(__int64, field_40, 0x40);
        STRUCT_PLACE(int, field_48, 0x40);
        STRUCT_PLACE(int, field_4C, 0x40);
        STRUCT_PLACE(char, field_50, 0x40);
        STRUCT_PLACE(char, worldloaded_mby, 0x40);
        STRUCT_PLACE(char, gamemodeloaded_mby, 0x40);
        STRUCT_PLACE(char, field_53, 0x40);
    };

    void Call_OnStateChance() {
        auto fn = (void(*)(System_27_WorldEngineSystem*))(globals::gameBase + 0xc39340);
        return fn(this);
    }
};

inline System_27_WorldEngineSystem* get_system27_WorldEngineSystem(EntityAdminBase* ea) {
    auto get_mapsys = (System_27_WorldEngineSystem*(*)(EntityAdminBase*))(globals::gameBase + 0xc1c790);
    return get_mapsys(ea);
}

struct System_63_WorldLoadingSystem {
    union {
        system_vt* vfptr;
        STRUCT_PLACE(EntityAdminBase*, ea, 0x8);
    };
};

inline System_63_WorldLoadingSystem* GetWorldLoadingSystem(EntityAdminBase* ea) {
    auto get_wls = (System_63_WorldLoadingSystem* (*)(EntityAdminBase*))(globals::gameBase + 0xc1c810);
    return get_wls(ea);
}

struct VoiceSystem_PlayVoice {
    int entity_1;
    int entity_2;
    int instigator_ent;

    __int64 voice_stimulus;
    __int64 voice_line;

    char field_20;
    char field_21;
};

struct VoiceSystem;
struct VoiceSystem_vt {
    union {
        mapsystem_callback_vt system_base;
        STRUCT_PLACE_CUSTOM(playvoice, 0xA8, void(*PlayVoice)(VoiceSystem*, VoiceSystem_PlayVoice*));
    };
};

struct VoiceSystem {
    union {
        VoiceSystem_vt* vfptr;
    };
};

inline VoiceSystem* GetVoiceSystem(EntityAdminBase* ea) {
    auto get_voicesys = (VoiceSystem* (*)(EntityAdminBase*))(globals::gameBase + 0xc71b50);
    return get_voicesys(ea);
}

class PrometheusSystem_Mapfunc {
    virtual void field_0();
    virtual void nullsub_1();
    virtual void change_state_3_globalloading();
    virtual void change_state_4_worldloading();
    virtual void change_state_5_worldloaded();
    virtual void change_state_6_world_replicating();
    virtual void change_state_7_gamemode_loading();
    virtual void maybe_on_gamemode_unload();
    virtual void on_gamemode_loading();
    virtual void maybe_on_map_unload();
    virtual void OnWorldLoadedAndInGameEnttiyAdmin();
    virtual void OnWorldLoadingAndInGameEntityAdmin();
    virtual void field_5();

    PrometheusSystem* backref;
};

class PrometheusSystem {
    virtual __int64 get_inheritance();
    virtual  bool is_assignable_to(__int64);
    virtual bool is_instance_of(__int64);

    virtual const char* get_system_name();
    virtual void deallocate();

    virtual void OnInitialize();
    virtual void field_1();
    virtual void PreDelete();

    virtual char* GetSubscriptions(int* count);
    virtual void OnCreationAfterEmplaced(Entity*);
    virtual void OnCreation(Entity*);
    virtual void field_3();
    virtual void OnDeletion(Entity*);

    virtual void OnTick(float);

    EntityAdminBase* _game_ea{};
    teList<Entity*> _local_controller_entities{};
    std::vector<Entity*> _newly_added_entities{};
    char _old_lbutton_states[0xc3]; //Finally not needed anymore
    Component_50_Input* _input_component{}; //LobbyEntityAdmin
    System_27_WorldEngineSystem* _world{};
    PrometheusSystem_Mapfunc _mapfunc{};
    int _trigger_helloVoiceLine_ticks = -1;
    bool _applied_demo = false;

    void state_replicator_do();
    void state_replicator_exhandled();

    static inline PrometheusSystem* s_instance = nullptr;
    static inline View* (*deallocate_view_orig)(View*, char);
    static View* deallocate_view_hook(View*, char);
public:
    static PrometheusSystem* create(EntityAdminBase* ea);

    static PrometheusSystem* instance() {
        return s_instance;
    }

    void DeleteLocalEnt() {
        auto local_ent = _game_ea->getLocalEnt();
        if (local_ent) {
            for (int i = 0; i < _local_controller_entities.num; i++) {
                if (_local_controller_entities.ptr[i] == local_ent) {
                    _local_controller_entities.remove_item(i--);
                }
            }

            auto pet_comp = local_ent->getById<Component_20_ModelReference>(0x20);
            if (pet_comp) {
                _game_ea->delEnt(_game_ea->getEntById(pet_comp->aim_entid));
                _game_ea->delEnt(_game_ea->getEntById(pet_comp->cam_attach_entid));
                _game_ea->delEnt(_game_ea->getEntById(pet_comp->movement_attach_entid));
            }
            _game_ea->delEnt(local_ent);
        }
    }

    teFreeLookView* local_freeLookView = nullptr;
    float freeLookView_movDelta = 10.f; //how fast freelookview is

    static inline std::map<__int64, __int64> s_modelEntToHelloVoiceMap = {
        { 0x400000000000001, 0x0760000000006C3D }
    };

    bool demo_join_game = false;
};

