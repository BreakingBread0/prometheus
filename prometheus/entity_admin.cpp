#include "entity_admin.h"
#include "Statescript.h"
#include "game.h"
#include <imgui.h>
#include <MinHook.h>
#include "STU_Editable.h"
#include "player_spawner.h"
#include "StatescriptVar.h"
#include "stu_resources.h"
#include "MovementState.h"
#include "Viewmodel.h"
#include "state_replicator.h"
#include "serialization.h"
#include "demo_cammoves.cpp"

static const char* playerUsername = "Jeff Kaplan";

static bool     g_autoSpawnEnabled = true;
static uint64_t g_autoSpawnLastMap  = 0;   // map we already applied on
static int      g_autoSpawnDelayTicks = 45;   // try 30–90; 45 is a good start
static int      g_autoSpawnPendingTicks = 0;
static uint64_t g_autoSpawnPendingMapId = 0;
static uint32_t g_autoSpawnPendingControllerId = 0;
static int      g_autoSpawnReapplyTicks = 0;   // re-apply to beat spawn/state overwrites
static uint64_t g_autoSpawnReapplyMapId = 0;
static uint32_t g_autoSpawnReapplyControllerId = 0;

// Define specific teleport locations for each map ID
std::map<uint64_t, Vector4> s_mapTeleportCoords = {
    { 0x80000000000005bULL, Vector4(-44.724f, 1.511f, 38.358f, 0) },        // Anubis
    { 0x800000000000184ULL, Vector4(-10.494f, -0.072f, -161.437f, 0) },    	// Watchpoint
    { 0x800000000000165ULL, Vector4(108.33f, 0.99f, 0.14f, 0) },    		// Hanamura
	{ 0x8000000000001dbULL, Vector4(17.790f, -4.979f, 1.851f, 0) }, 		// Volskaya Industries
	{ 0x8000000000001d4ULL, Vector4(87.241f, 0.074f, -4.300f, 0) }, 		// Numbani
	{ 0x8000000000002c3ULL, Vector4(89.102f, 6.728f, -0.336f, 0) }, 		// Dorado
	{ 0x8000000000000d4ULL, Vector4(-17.149f, -0.116f, -33.180f, 0) }, 		// King's Row
	{ 0x8000000000002afULL, Vector4(-28.136f, 1.604f, -3.228f, 0) }, 		// Hollywood
	{ 0x800000000000223ULL, Vector4(17.129f, -5.159f, -79.195f, 0) }, 		// Lobby Final

};

static bool LooksLikeMapId(uint64_t v)
{
    const uint64_t hi = (v & 0xFF00000000000000ULL);
    // Your build uses 0x08...... for maps (e.g. 0x80000000000005b), but keep 0x80...... too for safety.
    return (hi == 0x0800000000000000ULL) || (hi == 0x8000000000000000ULL);
}

static void DumpWorldMapCandidates_F9(EntityAdminBase* gameEA)
{
    auto world = get_system27_WorldEngineSystem(gameEA);
    if (!world) {
        printf("[mapdump] world == null\n");
        return;
    }

    printf("[mapdump] world=%p world_state=%d wanted_map_id=0x%llx\n",
        world,
        (int)world->world_state,
        (unsigned long long)world->wanted_map_id
    );

    // Scan more than 0x400; structs often store ids farther in
    uint8_t* base = (uint8_t*)world;

    int found = 0;
    for (size_t off = 0; off < 0x2000; off += 8)
    {
        uint64_t v = *(uint64_t*)(base + off);

        // Best signal: exact match to one of your teleport keys
        if (s_mapTeleportCoords.find(v) != s_mapTeleportCoords.end())
        {
            printf("[mapdump] MATCH +0x%04zx : 0x%llx\n", off, (unsigned long long)v);
            found++;
            continue;
        }

        // Otherwise print likely candidates
        if (LooksLikeMapId(v))
        {
            printf("[mapdump] cand  +0x%04zx : 0x%llx\n", off, (unsigned long long)v);
            found++;
        }
    }

    if (!found)
        printf("[mapdump] (no 0x08/0x80... candidates found in first 0x2000 bytes)\n");
}

static bool TryGetCurrentMapId(EntityAdminBase* gameEA, uint64_t& outMapId)
{
    outMapId = 0;
    if (!gameEA) return false;

    auto world = get_system27_WorldEngineSystem(gameEA);
    if (!world) return false;

    // Confirmed by your dump: current mapId sits at WorldEngineSystem + 0x20
    uint64_t mapId = *(uint64_t*)((uint8_t*)world + 0x20);
    if (mapId != 0) {
        outMapId = mapId;
        return true;
    }

    // Fallback: sometimes valid during loads
    if (world->wanted_map_id != 0) {
        outMapId = (uint64_t)world->wanted_map_id;
        return true;
    }

    return false;
}

static Vector4* GetLocalPlayerPosPtr(Entity* controllerEnt, EntityAdminBase* gameEA)
{
    if (!controllerEnt || !gameEA)
        return nullptr;

    auto ref_comp = controllerEnt->getById<Component_20_ModelReference>(0x20);
    if (!ref_comp || ref_comp->aim_entid <= 0)
        return nullptr;

    auto modelEnt = gameEA->getEntById(ref_comp->aim_entid);
    if (!modelEnt)
        return nullptr;

    auto comp15 = modelEnt->getById<__int64>(0x15);
    if (!comp15)
        return nullptr;

    return (Vector4*)((__int64)comp15 + 0xC70); // your known position offset
}

static bool TeleportControllerToMapSpawn(Entity* controller, EntityAdminBase* gameEA, uint64_t mapId)
{
    auto it = s_mapTeleportCoords.find(mapId);
    if (it == s_mapTeleportCoords.end())
        return false;

    Vector4* pos = GetLocalPlayerPosPtr(controller, gameEA);
    if (!pos)
        return false;

    *pos = it->second;
    return true;
}


__int64 PrometheusSystem::get_inheritance() {
	return NULL;
}
bool PrometheusSystem::is_assignable_to(__int64) {
	return false;
}
bool PrometheusSystem::is_instance_of(__int64) {
	return false;
}

const char* PrometheusSystem::get_system_name() {
	return "PrometheusSystem";
}
void PrometheusSystem::deallocate() {
	
}

void PrometheusSystem::OnInitialize() {
	_local_controller_entities.ptr = nullptr;
	_local_controller_entities.num = 0;
	_local_controller_entities.max = 0;
	memset(_old_lbutton_states, 0, sizeof(_old_lbutton_states)); //Removing this crashes the game on initialization. You know what? idc.
	_input_component = nullptr;
	_world = nullptr;

	//0xcc6110 0xcc67f5
	//0xcc57d3 0xcc4c22
}

void PrometheusSystem::field_1() {
	auto str = stacktrace_str();
	printf("System field_1:\n%s\n", str.c_str());
}
void PrometheusSystem::PreDelete() {
	auto str = stacktrace_str();
	printf("System PreDelete:\n%s\n", str.c_str());
}

char* PrometheusSystem::GetSubscriptions(int* count) {
	static char subscribed_components[] = {
		0x2F
	};
	*count = sizeof(subscribed_components);
	return subscribed_components;
}
void PrometheusSystem::OnCreationAfterEmplaced(Entity* ent) {
	printf("OnDeferredCreation: %x\n", ent->entity_id);
}
void PrometheusSystem::OnCreation(Entity* cb_ent) {
	printf("OnCreation: %x\n", cb_ent->entity_id);
	if (cb_ent->getById(0x2F)) {
		_local_controller_entities.emplace_item(cb_ent);
		_newly_added_entities.push_back(cb_ent);
		_trigger_helloVoiceLine_ticks = 50;
	}
}
void PrometheusSystem::field_3() {
	auto str = stacktrace_str();
	printf("System field_3:\n%s\n", str.c_str());
}
void PrometheusSystem::OnDeletion(Entity* ent) {
	//TODO This does apparently not work
	for (int i = 0; i < _local_controller_entities.num; i++) {
		if (_local_controller_entities.ptr[i] == ent)
			_local_controller_entities.remove_item(i--);
	}
}

string_rep* (*get_displayString_orig)(string_rep*, __int64 id, __int64);
string_rep* get_displayString_func(string_rep* a1, __int64 id, __int64 a3) {
    if (id == 0xde00000000005df) {
        char msg[256];

        // %s because playerUsername is a C-string (const char*)
        int n = sprintf_s(msg, sizeof(msg), "Welcome Back, %s!", playerUsername);

        if (n < 0) {
            const char* fallback = "Welcome Back!";
            a1->extend_to(strlen(fallback));
            strcpy(a1->get(), fallback);
            a1->actual_size = (int)strlen(fallback);
            return a1;
        }

        size_t len = (size_t)n;

        a1->extend_to(len);                 // if extend_to needs +1 for '\0', use (len + 1)
        memcpy(a1->get(), msg, len + 1);    // copy including null terminator
        a1->actual_size = (int)len;
        return a1;
    }

    return get_displayString_orig(a1, id, a3);
}

void play_game_btn() {
	PrometheusSystem::instance()->demo_join_game = true;
}

//0xd774b0
char SendHeroSelection(StatescriptAction_vt** impl, StatescriptState* state, StatescriptInstance* ss, STUBase<>* stu_data) {
	auto stu = stu_data->to_editable();
	auto hero = stu.get_argument_object("m_hero");
	auto hero_cv = (STUConfigVar*)hero.value;
	StatescriptPrimitive hero_value;
	hero_cv->get_value(ss, &hero_value);
	owassert(hero_value.type == StatescriptPrimitive_INT64);

	auto behavior = stu.get_argument_primitive("m_behavior").get_value<int>();
	/*printf("HeroSelect behavior: %hx\n", behavior);*/
	//0 = just send teammate preview
	if (hero_value.value != 0 && behavior == 1) {
		printf("Selected Hero: %p\n", hero_value.value);

		auto old_local = GameEntityAdmin()->getLocalEnt();
		if (old_local) {
			auto hero_info = old_local->getById<Component_3F_PlayerInfo>(0x3F);
			if (hero_info->selected_heroid == hero_value.value) {
				return 1;
			}
		}

		PrometheusSystem::instance()->DeleteLocalEnt();

		auto hero_stu = stu_resources::GetByID(hero_value.value);
		auto model_entid = hero_stu->to_editable().get_argument_resource("m_gameplayEntity")->resource_id;

		player_spawner spawner(model_entid);
		spawner.controller_info.heroid = hero_value.value;
		Component_1_SceneRendering::InitData init{};
		auto local_ents = spawner.spawn();

		//TODO Move
		state_replicator::ChangeHeroMessage msg;
		msg.heroid = hero_value.value;
		msg.skinid = 0;
		state_replicator::emplace_hero_update(msg);
	}
	return 1;
}

//0xd94930
char matchmaking_cv(__int64, __int64, __int64, StatescriptPrimitive* result) {
	result->type = StatescriptPrimitive_BYTE;
	result->value = PrometheusSystem::instance()->demo_join_game;
	return 1;
}

void pachipachi(Vector4 position, Vector4 rotation, Vector4 scaling) {
	auto res = try_load_resource(0x04000000000006CA);
	if (res == 0) //Not valid on all maps, this is only for hanamura
		return;
	auto ea = GameEntityAdmin();
	auto loader = EntityLoader::Create(0x04000000000006CA, res, false, false);
	auto init_data = Component_1_SceneRendering::InitData{};
	init_data.position = position;
	init_data.rotation = rotation;
	init_data.scale = scaling;

	loader->loader_entries[1].init_data = (__int64)&init_data;
	loader->Spawn(ea);
}

void spawn_pachis() {
	//pachipachi(Vector4(6.444336, 0.000000, -4.373047, 0), Vector4(0.000000, 0.697113, 0.000000, 0.716962), Vector4(1.000000, 1.000000, 1.000000, 1.000000));
}

std::map<state_replicator::playerid, std::pair<Entity*, Entity*>> _awful_server_entities;

struct demo_lobbyview {
	__int64 map_id;
	Entity* ent;
};
struct demo_move {
	enum class typ {
		switch_map,
		camera_move,
		preload,
		join_game,
		wait_1s,
		play_welcome,
		spawn_lineups,
		change_music
	};
	typ type;
	std::map<double, cammove>* move;
	int mapindex;
};
demo_lobbyview s_demo_lobbyviews[] = {
	demo_lobbyview{ 0x80000000000005b, nullptr }, //0 Anubis
	demo_lobbyview{ 0x800000000000184, nullptr }, //1 Watchpoint
	demo_lobbyview{ 0x8000000000002af, nullptr }, //2 Hollywood
	demo_lobbyview{ 0x8000000000002c3, nullptr }, //3 Dorado
	demo_lobbyview{ 0x8000000000001d4, nullptr }, //4 Numbani
	demo_lobbyview{ 0x8000000000000d4, nullptr }, //5 Kings Row
	demo_lobbyview{ 0x800000000000165, nullptr }, //6 Hanamura
	demo_lobbyview{ 0x8000000000001db, nullptr }  //7 Volskaya
};
auto watchpoint_static_pos = std::map<double, cammove>{
	{ 0, cammove(Vector4(-14.808542, 1.334642, -158.170334, 0), Vector4(0.615470, 0.022686, 0.787834, 0))},
	{ 1, cammove(Vector4(-14.808542, 1.334642, -158.170334, 0), Vector4(0.615470, 0.022686, 0.787834, 0))},
	//{ 50000, cammove(Vector4(-9.047516, 1.845441, -150.246689, 0), Vector4(0.500749, 0.054075, 0.863902, 0))},
	{ 3, cammove(Vector4(-9.047516, 1.845441, -150.246689, 0), Vector4(0.500749, 0.054075, 0.863902, 0))},
	{ 6, cammove(Vector4(0.074877, 7.994227, -141.168823, 0), Vector4(0.435241, -0.083772, 0.896408, 0))},
	{ 10, cammove(Vector4(6.267166, 7.839251, -146.952255, 0), Vector4(0.249528, -0.082033, 0.964887, 0))},
	{ 13, cammove(Vector4(8.284057, 7.592375, -152.224564, 0), Vector4(0.991530, -0.057660, -0.116376, 0))},
	{ 16, cammove(Vector4(11.941032, 7.483008, -152.943985, 0), Vector4(0.724711, 0.041773, 0.687786, 0))},
	//{ 16, cammove(Vector4(11.941032, 7.483008, -152.943985, 0), Vector4(0.724711, 0.041773, 0.687786, 0))},
};
demo_move s_demo_moves[] = {
	demo_move{ demo_move::typ::preload, nullptr, 0 },

	demo_move{ demo_move::typ::switch_map, nullptr, 1 },
	demo_move{ demo_move::typ::spawn_lineups, nullptr, 0 },
	demo_move{ demo_move::typ::camera_move, &watchpoint_static_pos, 0 },

	//demo_move{ demo_move::typ::wait_1s, nullptr, 0 },
	//demo_move{ demo_move::typ::play_welcome, nullptr, 0 },

	demo_move{ demo_move::typ::wait_1s, nullptr, 0 },

	demo_move{ demo_move::typ::switch_map, nullptr, 0 },
	demo_move{ demo_move::typ::camera_move, &anubis_1, 0 },
	//demo_move{ demo_move::typ::camera_move, &anubis_2, 0 },

	//demo_move{ demo_move::typ::switch_map, nullptr, 1 },
	//demo_move{ demo_move::typ::camera_move, &watchpoint_1, 0 },
	//demo_move{ demo_move::typ::camera_move, &watchpoint_2, 0 },
	//demo_move{ demo_move::typ::camera_move, &watchpoint_3, 0 },
	//demo_move{ demo_move::typ::camera_move, &hollywood_2, 0 },

	demo_move{ demo_move::typ::switch_map, nullptr, 3 },
	//demo_move{ demo_move::typ::camera_move, &dorado_1, 0 },
	demo_move{ demo_move::typ::camera_move, &dorado_2, 0 },

	demo_move{ demo_move::typ::switch_map, nullptr, 4 },
	demo_move{ demo_move::typ::camera_move, &numbani_1, 0 },
	//demo_move{ demo_move::typ::camera_move, &numbani_2, 0 },

	demo_move{ demo_move::typ::switch_map, nullptr, 6 },
	//demo_move{ demo_move::typ::camera_move, &hanamura_1, 0 },
	demo_move{ demo_move::typ::camera_move, &hanamura_2, 0 },

	demo_move{ demo_move::typ::switch_map, nullptr, 2 },
	demo_move{ demo_move::typ::camera_move, &hollywood_1, 0 },

	demo_move{ demo_move::typ::switch_map, nullptr, 7 },
	demo_move{ demo_move::typ::camera_move, &volskaya_1, 0 },

	demo_move{ demo_move::typ::switch_map, nullptr, 5 },
	//demo_move{ demo_move::typ::camera_move, &kingsrow_1, 0 },
	demo_move{ demo_move::typ::change_music, nullptr, 0 },
	demo_move{ demo_move::typ::camera_move, &kingsrow_2, 0 },
	//demo_move{ demo_move::typ::camera_move, &volskaya_2, 0 },
	//demo_move{ demo_move::typ::camera_move, &volskaya_3, 0 },

	demo_move{ demo_move::typ::join_game, nullptr, 0 }
};

struct lineup {
	__int64 hero;
	Vector4 position;
	Vector4 rotation;
};

auto s_lineups = std::vector<lineup>{
	lineup(0x400000000000dd7, Vector4(19.110352, -5.095703, -88.195312, 0), Vector4(0.000000, 0.993958, 0.000000, 0)),
	lineup(0x40000000000063c, Vector4(-12.725586, 0.085938, -152.776367, 0), Vector4(0.000000, 0.996279, 0.000000, 0)),
	lineup(0x400000000000dce, Vector4(-9.442383, 0.316406, -145.820312, 0), Vector4(0.000000, 0.994236, 0.000000, 0)),
	lineup(0x40000000000086d, Vector4(-6.450195, 2.524414, -142.608398, 0), Vector4(0.000000, 0.990009, 0.000000, 0)),
	lineup(0x400000000000dd5, Vector4(-5.743164, 2.032227, -144.719727, 0), Vector4(0.000000, 0.970287, 0.000000, 0)),
	lineup(0x400000000000ddf, Vector4(5.125977, 6.073242, -141.152344, 0), Vector4(0.000000, 0.920815, 0.000000, 0)),
	lineup(0x400000000000dd9, Vector4(-11.636719, 0.080078, -149.741211, 0), Vector4(0.000000, 0.998717, 0.000000, 0)),
	lineup(0x400000000000ddb, Vector4(11.440430, 6.112305, -153.666992, 0), Vector4(0.000000, 0.375229, 0.000000, 0)),
	lineup(0x400000000000dde, Vector4(-7.588867, 0.092773, -150.149414, 0), Vector4(0.000000, 0.890197, 0.000000, 0)),
	lineup(0x400000000000dd4, Vector4(-8.099609, 1.330078, -144.375000, 0), Vector4(0.000000, 0.991657, 0.000000, 0)),
	lineup(0x400000000000ddd, Vector4(-2.491211, 5.317383, -138.779297, 0), Vector4(0.000000, 0.984183, 0.000000, 0)),
	lineup(0x400000000000dd0, Vector4(-7.530273, 0.688477, -146.667969, 0), Vector4(0.000000, 0.970287, 0.000000, 0)),
	lineup(0x400000000000ddc, Vector4(-5.279297, 3.378906, -141.364258, 0), Vector4(0.000000, 0.984804, 0.000000, 0)),
	lineup(0x400000000000dd8, Vector4(2.912109, 6.102539, -138.940430, 0), Vector4(0.000000, 0.961486, 0.000000, 0)),
	lineup(0x400000000000dcf, Vector4(-8.442383, 0.119141, -152.231445, 0), Vector4(0.000000, -0.839088, 0.000000, 0)),
	lineup(0x400000000000dd2, Vector4(-3.981445, 4.208984, -140.291992, 0), Vector4(0.000000, 0.975126, 0.000000, 0)),
	lineup(0x400000000000dd6, Vector4(0.616211, 6.078125, -136.959961, 0), Vector4(0.000000, 0.978721, 0.000000, 0)),
	lineup(0x400000000000dda, Vector4(-9.895508, 0.076172, -153.852539, 0), Vector4(0.000000, 0.958059, 0.000000, 0)),
	lineup(0x400000000000dd3, Vector4(-3.987305, 3.366211, -142.756836, 0), Vector4(0.000000, 0.961486, 0.000000, 0)),
	lineup(0x400000000000dd1, Vector4(6.980469, 6.061523, -143.701172, 0), Vector4(0.000000, 0.930375, 0.000000, 0)),
	lineup(0x4000000000012f5, Vector4(-2.083008, 4.671875, -140.976562, 0), Vector4(0.000000, 0.956042, 0.000000, 0)),
};

int _curr_demomove = -1;
double _demomove_starttime = 0;
int _curr_lobbymap = -1;

Component_54_Lobbymap* (*origSwitchCam)();
Component_54_Lobbymap* hookSwitchCam() {
	if (_curr_lobbymap != -1) {
		auto& mapst = s_demo_lobbyviews[_curr_lobbymap];
		if (mapst.ent != nullptr) {
			auto replacement = mapst.ent->getById<Component_54_Lobbymap>(0x54);
			return replacement;
			//origSwitchCam(replacement); //0xce6da0 0xc45400
		}
	}
	return LobbyEntityAdmin()->getSingletonComponent<Component_54_Lobbymap>(0x54);
}


void PrometheusSystem::state_replicator_do() {
	if (state_replicator::is_connected) {
		while (true) {
			auto update = state_replicator::get_outstanding_message();
			if (update->type == state_replicator::MsgBase::MessageType::None)
				break;
			switch (update->type) {
			case state_replicator::MsgBase::MessageType::ChangeHero: {
				auto msg = dynamic_cast<state_replicator::ChangeHeroMessage*>(update.get());
				if (msg->player_id == state_replicator::local_playerid)
					continue;
				auto player = _awful_server_entities.find(msg->player_id);
				if (player != _awful_server_entities.end()) {
					_game_ea->delEnt(player->second.first);
					_game_ea->delEnt(player->second.second);
				}
				auto hero = stu_resources::GetByID(msg->heroid);
				owassert(hero);
				auto model_entid = hero->to_editable().get_argument_resource("m_gameplayEntity")->resource_id;
				player_spawner spawn(model_entid);
				spawn.controller_info.heroid = model_entid;
				spawn.controller_info.load_2f_33 = false;
				spawn.controller_info.set_localent = false;
				spawn.model_info.skin_theme_id = msg->skinid;
				_awful_server_entities[msg->player_id] = spawn.spawn();
				printf("spawned hero %llx for ent %s\n", msg->heroid, msg->player_id.c_str());
			}
																   break;
			case state_replicator::MsgBase::MessageType::JoinInstance: {
				auto msg = dynamic_cast<state_replicator::JoinInstanceMessage*>(update.get());
				if (msg->player_id == state_replicator::local_playerid)
					continue;
				printf("Player joined: %s (%s)\n", msg->player_id.c_str(), msg->name.c_str());
			}
																	 break;
			case state_replicator::MsgBase::MessageType::LeaveInstance: {
				auto msg = dynamic_cast<state_replicator::LeaveInstanceMessage*>(update.get());
				if (msg->player_id == state_replicator::local_playerid)
					continue;
				auto player = _awful_server_entities.find(msg->player_id);
				if (player != _awful_server_entities.end()) {
					_game_ea->delEnt(player->second.first);
					_game_ea->delEnt(player->second.second);
				}
				printf("Player left: %s (%s)\n", msg->player_id.c_str());
			}
																	  break;
			case state_replicator::MsgBase::MessageType::PositionUpdate: {
				auto msg = dynamic_cast<state_replicator::PositionUpdateMessage*>(update.get());
				if (msg->player_id == state_replicator::local_playerid)
					continue;
				MovementState state = msg->data.get<MovementState>();
				if (state.command_frame == -1)
					continue;
				auto player = _awful_server_entities.find(msg->player_id);
				if (player != _awful_server_entities.end()) {
					auto comp12 = player->second.second->getById<Component_12_STUMovementStateComponent>(0x12);
					auto result = comp12->PasteMovementState(&state);
					//printf("%p (%d) %d - %f %f %f\n", comp12, result, state.command_frame, state.absolute_position.X, state.absolute_position.Y, state.absolute_position.Z);
				}
				else {
					printf("Unable to find hero %s locally!\n", msg->player_id.c_str());
				}
			}
																	   break;
			}
		}

		Entity* local_controller = _game_ea->getLocalEnt();
		Entity* local_model = nullptr;

		if (local_controller) {
			auto pet_controller = local_controller->getById<Component_20_ModelReference>(0x20);
			if (pet_controller) {
				local_model = _game_ea->getEntById(pet_controller->cam_attach_entid);
			}
		}
		if (local_model) {
			auto comp15 = local_model->getById<Component_15_STUCharacterMoverComponent>(0x15);
			state_replicator::emplace_positon_update(&comp15->movement_state_2);
		}
	}
	else {
		for (auto ent : _awful_server_entities) {
			_game_ea->delEnt(ent.second.first);
			_game_ea->delEnt(ent.second.second);
		}
	}
}

void PrometheusSystem::state_replicator_exhandled() {
	__try {
		state_replicator_do();
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		printf("state replicator exception\n");
	}
}


void PrometheusSystem::OnTick(float tick) {
	Entity* local_controller = _game_ea->getLocalEnt();
	Entity* local_model = nullptr;

	if (local_controller) {
		auto pet_controller = local_controller->getById<Component_20_ModelReference>(0x20);
		if (pet_controller) {
			local_model = _game_ea->getEntById(pet_controller->cam_attach_entid);
			if (local_model) {
				auto ss = local_model->getById<Component_23_Statescript>(0x23);
				auto health_stru = local_model->getById<Component_28_STUHealthComponent>(0x28);

				float health = health_stru->vfptr->GetFullCurrentHealth(health_stru);
				float health_max = health_stru->vfptr->GetFullMaxHealth(health_stru);

				StatescriptPrimitive health_cv{};
				health_cv.type = StatescriptPrimitive_FLT;

				*(float*)&health_cv.value = health;
				ss->ss_inner.rid_entity_varbag->SetVar({ StatescriptVar_ENTITY_VARBAG, 0x0D800000000006C4 }, health_cv);

				*(float*)&health_cv.value = health_max;
				ss->ss_inner.rid_entity_varbag->SetVar({ StatescriptVar_ENTITY_VARBAG, 0x0D800000000006C9 }, health_cv);

				StatescriptPrimitive wpn_1{};
				wpn_1.type = StatescriptPrimitive_INT;
				wpn_1.value = 1;
				ss->ss_inner.rid_entity_varbag->SetArray({ 0x0D8000000000001D }, wpn_1, wpn_1, 0);
			}
		}
	}

	if (globals::isDemo) {
		if (!_applied_demo) {
			_applied_demo = true;

			MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0x8537c0), get_displayString_func, (PVOID*)&get_displayString_orig));
			MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0x8537c0)));

			MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xc87220), play_game_btn, (PVOID*)0));
			MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xc87220)));

			MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xd774b0), SendHeroSelection, (PVOID*)0));
			MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xd774b0)));

			MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xd94930), matchmaking_cv, (PVOID*)0));
			MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xd94930)));

			MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xc45400), hookSwitchCam, (PVOID*)&origSwitchCam));
			MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xc45400)));

			auto lobby_ss = LobbyEntityAdmin()->getSingletonComponent<Component_23_Statescript>(0x23);
			lobby_ss->ss_inner.getByResourceId(0x58000000000041a)->StartStopState(9, true); //developer mode

			//*(char*)(globals::gameBase + 0x1859db4) = 1; //tank_showMode
			//*(char*)(globals::gameBase + 0x1859dcc) = 1; //feature flag 11 (developer mode?)

			//0xc87220

			auto settings = LobbyEntityAdmin()->getSingletonComponent<Component_53_Settings>(0x53);
			settings->tutorial_dialog_dismissed = 1;
		}

		if (demo_join_game) {
			//1: searching
			//2: match found
			//3: joining game
			auto matchmake_info_comp = (__int64)LobbyEntityAdmin()->getSingletonComponent<__int64>(0x56);
			if (*(int*)(matchmake_info_comp + 0x20) == 0) {
				*(int*)(matchmake_info_comp + 0x20) = 1;
			}
			if (_curr_demomove == -1)
				_curr_demomove = 0;
			if (_demomove_starttime == 0)
				_demomove_starttime = GetGameRuntimeSecs();
			auto& demomove = s_demo_moves[_curr_demomove];
			//static std::vector<Component_1_SceneRendering*> test{};
			if (demomove.type == demo_move::typ::preload) {
				auto lobby_ss = LobbyEntityAdmin()->getSingletonComponent<Component_23_Statescript>(0x23);
				auto map_ss = lobby_ss->ss_inner.getByResourceId(0x580000000000b47);
				if (map_ss)
					map_ss->Stop();
				auto vmroot = GetViewModelRoot();
				bool all_loaded = true;
				int loaded_cnt = 0;
				for (int i = 0; i < 8; i++) {
					auto& map = s_demo_lobbyviews[i];
					if (map.ent == nullptr) {
						auto spawn = EntityLoader::Create(0, 0, false, false);
						spawn->loader_entries[0x54].component_id = 0x54;
						auto ent = spawn->Spawn(LobbyEntityAdmin());
						auto comp54 = ent->getById<Component_54_Lobbymap>(0x54);
						if (!comp54) {
							printf("Comp54 failed!\n");
						}
						else {
							comp54->LoadMap(map.map_id);
						}
						map.ent = ent;
						_curr_lobbymap = i;
					}
					auto comp54 = map.ent->getById<Component_54_Lobbymap>(0x54);
					if (comp54->GetMapLoadState() != 4) {
						all_loaded = false;
						loaded_cnt = i;
						break;
					}
				}
				for (auto vm : vmroot->viewmodel_list) {
					auto name = vm->vfptr->GetName();
					if (!strcmp(name, "LobbyMainMenuVM")) {
						auto play_enabled = vm->getByKey(0x99363eb);
						if (play_enabled) {
							play_enabled->property.has_value = true;
							string_rep* str = (string_rep*)play_enabled->property.value;
							str->extend_to(64);
							if (all_loaded) {
								str->actual_size = 0;
							}
							else {
								strcpy(str->get(), std::format("Load Demo {:d}/8", loaded_cnt + 1).c_str());
							}
							SendViewModelPropUpdate(vm, play_enabled);
						}
					}
				}
				if (all_loaded) {
					*(int*)(matchmake_info_comp + 0x20) = 2;
					lobby_ss->ss_inner.getByResourceId(0x5800000000001e5)->ExecuteNode(87); //POTG music
					auto ss = lobby_ss->ss_inner.getByResourceId(0x5800000000002d8); //Chat
					if (ss)
						ss->Stop();
					ss = lobby_ss->ss_inner.getByResourceId(0x580000000000158); //Lobby Menu
					if (ss)
						ss->Stop();
					ss = lobby_ss->ss_inner.getByResourceId(0x580000000000138); //Social Menu
					if (ss)
						ss->Stop();
					_curr_demomove++;
					_demomove_starttime = 0;
				}
			}
			else if (demomove.type == demo_move::typ::join_game) {
				_curr_lobbymap = -1;
				for (int i = 0; i < 8; i++) {
					auto& map = s_demo_lobbyviews[i];
					LobbyEntityAdmin()->delEnt(map.ent);
					map.ent = nullptr;
				}
				auto lobby_ss = LobbyEntityAdmin()->getSingletonComponent<Component_23_Statescript>(0x23);
				lobby_ss->ss_inner.getByResourceId(0x58000000000041a)->StartStopState(7, true); //For "Welcome to Overwatch"
				printf("Demo Finished!\n");
				*(int*)(matchmake_info_comp + 0x20) = 3;

				lobby_ss->ss_inner.getByResourceId(0x5800000000001e5)->graph->m_nodes.list()[91]->graph_node.base.to_editable().get_argument_object("m_outPlug").get_argument_objectlist("m_links").value->set_count(0);

				auto world = get_system27_WorldEngineSystem(GameEntityAdmin());
				world->wanted_map_id = 0x800000000000165; //Hanamura
				world->world_state = 2;

				demo_join_game = false;
			}
			else if (demomove.type == demo_move::typ::play_welcome) {
				auto lobby_ss = LobbyEntityAdmin()->getSingletonComponent<Component_23_Statescript>(0x23);
				auto inst = lobby_ss->ss_inner.getByResourceId(0x5800000000004f6);
				if (inst) {
					inst->ExecuteNode(1); //"Welcome to Overwatch"
				}
				_curr_demomove++;
				_demomove_starttime = 0;
			}
			else if (demomove.type == demo_move::typ::switch_map) {
				_curr_lobbymap = demomove.mapindex;
				_curr_demomove++;
				_demomove_starttime = 0;
			}
			else if (demomove.type == demo_move::typ::change_music) {
				auto lobby_ss = LobbyEntityAdmin()->getSingletonComponent<Component_23_Statescript>(0x23);
				lobby_ss->ss_inner.getByResourceId(0x5800000000001e5)->ExecuteNode(5); //29 start match music
				_curr_demomove++;
				_demomove_starttime = 0;
			}
			else if (demomove.type == demo_move::typ::wait_1s) {
				if ((GetGameRuntimeSecs() - _demomove_starttime) > 1) {
					_curr_demomove++;
					_demomove_starttime = 0;
				}
			}
			else if (demomove.type == demo_move::typ::spawn_lineups) {
				auto ea = s_demo_lobbyviews[1].ent->getById<Component_54_Lobbymap>(0x54)->embedded_game_ea;
				for (auto& lineup : s_lineups) {
					auto loader = EntityLoader::Create(lineup.hero, try_load_resource(lineup.hero), false, false);
					/*Component_1_SceneRendering::InitData data{};
					data.position = lineup.position;
					data.rotation = lineup.rotation;
					data.scale = Vector4(1, 1, 1, 0);
					loader->loader_entries[1].init_data = (__int64)&data;*/
					auto ent = loader->Spawn(ea);
					auto sr = ent->getById<Component_1_SceneRendering>(1);
					sr->SetPosRotation(lineup.position, lineup.rotation);
					sr->SetScale(Vector4(1, 1, 1, 1));
					//sr->SetVisible();
					//test.push_back(sr);
				}
				_curr_demomove++;
				_demomove_starttime = 0;
			}
			else if (demomove.type == demo_move::typ::camera_move) {
				typedef std::reverse_iterator<std::map<double, cammove>::iterator> rit;
				auto delta = GetGameRuntimeSecs() - _demomove_starttime;
				rit it = demomove.move->rbegin();
				rit upper(it);
				while (it->first > delta) {
					upper = rit(it);
					it++;
				}
				rit lower = it;
				if (it == demomove.move->rbegin()) {
					_curr_demomove++;
					_demomove_starttime = 0;
				}
				Vector4 pos{};
				Vector4 rot{};
				pos = lower->second.position;
				rot = lower->second.rotation;
				auto timespan = upper->first - lower->first;
				if (timespan != 0) {
					double pos_delta = (delta - lower->first) / (upper->first - lower->first);
					pos.X += (upper->second.position.X - lower->second.position.X) * pos_delta;
					pos.Y += (upper->second.position.Y - lower->second.position.Y) * pos_delta;
					pos.Z += (upper->second.position.Z - lower->second.position.Z) * pos_delta;

					rot.X += (upper->second.rotation.X - lower->second.rotation.X) * pos_delta;
					rot.Y += (upper->second.rotation.Y - lower->second.rotation.Y) * pos_delta;
					rot.Z += (upper->second.rotation.Z - lower->second.rotation.Z) * pos_delta;
				}

				auto map = s_demo_lobbyviews[_curr_lobbymap].ent->getById<Component_54_Lobbymap>(0x54);
				auto camera = map->embedded_game_ea->getSingletonComponent<Component_4F_Camera>(0x4F);
				if (camera->override_views.num == 0) {
					camera->override_views.emplace_item(OverrideView{ (View*)teFreeLookView::create(), true });
				}
				auto view = camera->override_views.ptr[0].view_ptr;
				view->view_position = pos;
				view->view_rotation = rot;
				view->view_roll = Vector4(0,0,0,0);
			}
			//if (demo_join_game == 200) {
			//	*(int*)(matchmake_info_comp + 0x20) = 2;
			//}
			//else if (demo_join_game == 20) {
			//	*(int*)(matchmake_info_comp + 0x20) = 3;
			//}
			//else if (demo_join_game == 0) {
			//	auto world = get_system27_WorldEngineSystem(GameEntityAdmin());
			//	world->wanted_map_id = 0x800000000000165; //Hanamura
			//	world->world_state = 2;
			//}
			//else 
		}
	
		auto vmroot = GetViewModelRoot();
		for (auto vm : vmroot->viewmodel_list) {
			auto name = vm->vfptr->GetName();
			if (!strcmp(name, "LobbyMainMenuVM")) {
				__int64 buttons[] = { 0xac0000000000223, 0xac0000000000916, 0xac0000000000917, 0xac0000000000918, 0xac0000000000936, 0xad2ab0c7 };
				for (auto button : buttons) {
					auto play_enabled = vm->getByKey(button);
					if (play_enabled) {
						play_enabled->property.has_value = true;
						play_enabled->property.value = !demo_join_game;
						SendViewModelPropUpdate(vm, play_enabled);
					}
				}
			}
			else if (!strcmp(name, "LobbyPartyMemberMeVM")) {
				{
					auto name = vm->getByKey(0xfe11d138);
					string_rep* rep = (string_rep*)name->property.value;
					const char* new_name = playerUsername;
					rep->extend_to(strlen(new_name));
					rep->actual_size = strlen(new_name);
					strcpy(rep->get(), new_name);
					SendViewModelPropUpdate(vm, name);
				}
				{
					auto background = vm->getByKey(0x7cac602a);
					background->property.value = 0xac00000000002cd;
					SendViewModelPropUpdate(vm, background);
				}
				{
					auto icon = vm->getByKey(0x11db7719);
					icon->property.value = 0xc0000000000135c;
					SendViewModelPropUpdate(vm, icon);
				}
			}
		}
	}
	if (!_input_component) {
		_input_component = LobbyEntityAdmin()->getSingletonComponent<Component_50_Input>(0x50);
	}
	else {
		/*char* curr = _input_component->tick10 ? _input_component->stru1.tick0 : _input_component->stru1.tick1;
		char* prev = _old_lbutton_states;*/

		//for (int i = 0; i < 0xc3; i++) {
		//	if (curr[i] != prev[i]) {
		//		//printf("button state changed: %x [%s] - %d\n", i, LogicalButtonById(i)->name, curr[i]);

		//		for (auto ent : _local_controller_entities) {
		//			auto ref_comp = ent->getById<Component_20_Pet>(0x20);
		//			auto aim_entid = ref_comp->aim_entid;
		//			//Many checks are probably not needed but better be safe than sorry...
		//			if (aim_entid > 0) {
		//				auto aim_ent = _game_ea->getEntById(aim_entid);
		//				if (aim_ent) {
		//					auto comp = aim_ent->getById<StatescriptComponent>(0x23);
		//					for (auto& script : comp->ss_inner) {
		//						auto event = (StatescriptNotification_LogicalButton*)comp->ss_inner.vfptr->Allocate_StatescriptE(&comp->ss_inner, ETYPE_LOGICALBUTTON);
		//						event->base.m_timestamp = comp->ss_inner.cf_timestamp;
		//						event->base.m_instanceId = script->instance_id;
		//						event->m_logicalButton = i;
		//						event->m_bButtonGoingDown = curr[i];
		//						script->vfptr->EnqueueE(script, (StatescriptNotification_Base*)event);

		//					}
		//				}
		//			}
		//		}
		//	}
		//}

		if (ImGui::IsKeyPressed(ImGuiKey_F8, false)) {
				g_autoSpawnEnabled = !g_autoSpawnEnabled;
				printf("[autospawn] %s\n", g_autoSpawnEnabled ? "enabled" : "disabled");
			}

			if (ImGui::IsKeyPressed(ImGuiKey_M, false)) {
			//0x580000000000352
			if (local_model) {
				auto ss = local_model->getById<Component_23_Statescript>(0x23);
				if (ss) {
					auto script = ss->ss_inner.getByResourceId(0x580000000000352);
					if (script) {
						script->ToggleState(17); //toggle_map logical button does not work
					}
				}
			}
		}

		if (ImGui::IsKeyPressed(ImGuiKey_F9, false))
		{
		DumpWorldMapCandidates_F9(_game_ea);
		}

		// Print current character coordinates + try to find facing (quat scan)
		if (ImGui::IsKeyPressed(ImGuiKey_J, false)) {
			int printed = 0;
			for (auto ent : _local_controller_entities) {
				if (!ent) continue;

				auto ref_comp = ent->getById<Component_20_ModelReference>(0x20);
				if (!ref_comp) continue;

				auto aim_entid = ref_comp->aim_entid;
				if (aim_entid <= 0) continue;

				auto aim_ent = _game_ea->getEntById(aim_entid);
				if (!aim_ent) continue;

				auto comp = aim_ent->getById<__int64>(0x15);
				if (!comp) continue;

				Vector4* pos = (Vector4*)((__int64)comp + 0xC70);
				printf("[coords] controller=%d model=%d -> (%.3f, %.3f, %.3f)\n",
					(int)ent->entity_id, (int)aim_entid, pos->X, pos->Y, pos->Z);
				printed++;
			}

			if (!printed) {
				printf("[coords] Unable to resolve local player position (no controller/model)\n");
			}
		}

		// Teleport to map-specific location
		if (ImGui::IsKeyPressed(ImGuiKey_U, false))
		{
			uint64_t mapId = 0;
			if (!TryGetCurrentMapId(_game_ea, mapId))
			{
				printf("[teleport] Could not read map id\n");
			}
			else
			{
				auto it = s_mapTeleportCoords.find(mapId);
				if (it == s_mapTeleportCoords.end())
				{
					printf("[teleport] No coords for mapId = 0x%llx\n", (unsigned long long)mapId);
				}
				else
				{
					const Vector4& target = it->second;

					for (auto controllerEnt : _local_controller_entities)
					{
						if (Vector4* pos = GetLocalPlayerPosPtr(controllerEnt, _game_ea))
						{
							*pos = target;

							// Optional: if you find you “rubber-band” back, also try clearing velocity
							// auto modelEnt = _game_ea->getEntById(controllerEnt->getById<Component_20_ModelReference>(0x20)->aim_entid);
							// auto comp15 = modelEnt ? modelEnt->getById<__int64>(0x15) : 0;
							// if (comp15) *(Vector4*)((__int64)comp15 + 0xC80) = Vector4(0,0,0,0); // guessed vel offset; verify!
						}
					}

					printf("[teleport] Teleported on map 0x%llx -> (%.3f, %.3f, %.3f)\n",
						(unsigned long long)mapId, target.X, target.Y, target.Z);
				}
			}
		}

		// Teleport up and down
		if (ImGui::IsKeyPressed(ImGuiKey_UpArrow, false)) {
			for (auto ent : _local_controller_entities) {
				auto ref_comp = ent->getById<Component_20_ModelReference>(0x20);
				auto aim_entid = ref_comp->aim_entid;
				//Many checks are probably not needed but better be safe than sorry...
				if (aim_entid > 0) {
					auto aim_ent = _game_ea->getEntById(aim_entid);
					if (aim_ent) {
						auto comp = aim_ent->getById<__int64>(0x15);
						if (comp) {
							((Vector4*)((__int64)comp + 0xC70))->Y += 10.f;
						}
					}
				}
			}
		} else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow, false)) {
			for (auto ent : _local_controller_entities) {
				auto ref_comp = ent->getById<Component_20_ModelReference>(0x20);
				auto aim_entid = ref_comp->aim_entid;
				//Many checks are probably not needed but better be safe than sorry...
				if (aim_entid > 0) {
					auto aim_ent = _game_ea->getEntById(aim_entid);
					if (aim_ent) {
						auto comp = aim_ent->getById<__int64>(0x15);
						if (comp) {
							((Vector4*)((__int64)comp + 0xC70))->Y -= 2.f;
						}
					}
				}
			}
		}

		// Change physics timescale
		/* else if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false)) {
			(*globalTimeScale()) -= 0.5f;
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow, false)) {
			(*globalTimeScale()) += 0.5f;
		} */

		//memcpy(_old_lbutton_states, curr, sizeof(_old_lbutton_states));
	}
	
	if (_trigger_helloVoiceLine_ticks > 0) {
		auto heroSelectActive_cvget = (char(*)(__int64, __int64, __int64, StatescriptPrimitive*))(globals::gameBase + 0xd8ee00);
		StatescriptPrimitive cv{};
		if (heroSelectActive_cvget(0, 0, 0, &cv) && !cv.is_truthy()) {
			if (--_trigger_helloVoiceLine_ticks == 0 && local_model) {
				if (local_model->resload_entry->valid()) {
					auto model_resid = local_model->resload_entry->align()->resource_id;
					auto mapping = s_modelEntToHelloVoiceMap.find(model_resid);
					if (mapping != s_modelEntToHelloVoiceMap.end()) {
						VoiceSystem_PlayVoice play_stru;
						play_stru.entity_1 = play_stru.entity_2 = play_stru.instigator_ent = local_model->entity_id;

						play_stru.field_20 = 0;
						play_stru.field_21 = 0;
						play_stru.voice_stimulus = 0xee0000000000001; //just taking the first one worked... lol
						play_stru.voice_line = mapping->second;

						auto voice_sys = GetVoiceSystem(_game_ea);
						voice_sys->vfptr->PlayVoice(voice_sys, &play_stru);
					}
				}
			}
		}
	}

	if (local_freeLookView) {
		for (auto ent : _local_controller_entities) {
			auto ref_comp = ent->getById<Component_20_ModelReference>(0x20);
			auto aim_entid = ref_comp->aim_entid;
			//Many checks are probably not needed but better be safe than sorry...
			if (aim_entid > 0) {
				auto aim_ent = _game_ea->getEntById(aim_entid);
				if (aim_ent) {
					auto filterbits = aim_ent->getById<Component_10_FilterBits>(0x10);
					filterbits->filter_bits |= 0x10 | 0x400000;
				}
			}
		}

		Vector4 position_delta{};
		/*if (_local_controller_entities.num > 0) {
			auto lp = _local_controller_entities.ptr[0]->getById<Component_2F_LocalPlayer>(0x2f);
			if (lp) {
				local_freeLookView->base.vfptr->field_30(&local_freeLookView->base, lp->mouse_rel_x, lp->mouse_rel_y);
			}
		}*/
		auto getMouseRel_fn = (void(__fastcall*)(Component_50_Input*, float* x, float* y))(globals::gameBase + 0xcee3f0);
		float x, y;
		if (_input_component) {
			__int64 viewmodel_list = *(__int64*)(globals::gameBase + 0x18a6688);
			if (viewmodel_list && !*(char*)(viewmodel_list + 0x95A)) {
				getMouseRel_fn(_input_component, &x, &y);
				local_freeLookView->base.vfptr->field_30(&local_freeLookView->base, x, y);

				float delta = freeLookView_movDelta;
				if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
					delta *= 2;
				}
				if (ImGui::IsKeyDown(ImGuiKey_W)) {
					position_delta.Z += delta;
				}
				else if (ImGui::IsKeyDown(ImGuiKey_S)) {
					position_delta.Z -= delta;
				}
				if (ImGui::IsKeyDown(ImGuiKey_A)) {
					position_delta.X -= delta;
				}
				else if (ImGui::IsKeyDown(ImGuiKey_D)) {
					position_delta.X += delta;
				}
			}
		}
		local_freeLookView->position_delta = position_delta;
	}
	else {
		for (auto ent : _local_controller_entities) {
			auto ref_comp = ent->getById<Component_20_ModelReference>(0x20);
			auto aim_entid = ref_comp->aim_entid;
			//Many checks are probably not needed but better be safe than sorry...
			if (aim_entid > 0) {
				auto aim_ent = _game_ea->getEntById(aim_entid);
				if (aim_ent) {
					auto filterbits = aim_ent->getById<Component_10_FilterBits>(0x10);
					filterbits->filter_bits &= ~(0x10 | 0x400000);
				}
			}
		}
	}

	for (auto it = _newly_added_entities.begin(); it != _newly_added_entities.end();) {
		Entity* controller = *it;
		Entity* model = _game_ea->getEntById(controller->getById<Component_20_ModelReference>(0x20)->aim_entid);
		if (model) {
			//Moved into player spawner
			/*auto ss = model->getById<StatescriptComponent>(0x23);
			StatescriptPrimitive cv{};
			cv.type = StatescriptPrimitive_BYTE;
			cv.value = 1;
			ss->ss_inner.rid_entity_varbag->SetVar({ StatescriptVar_ENTITY_VARBAG, 0x0D800000000000FD }, cv);*/
			if (globals::isDemo) {
				auto localplayer = controller->getById<Component_2F_LocalPlayer>(0x2F);
				localplayer->SetRotation(Vector4(0.000000, -0.713039, 0.000000, 0));
				auto comp = model->getById<__int64>(0x15);
				if (comp) {
					*((Vector4*)((__int64)comp + 0xC70)) = Vector4(108.336914, 0.999023, 0.142578, 0);
					//Is inside MovementState, i need to finisht the demo this is ugly ;(
				}
			}



// Auto-spawn teleport: arm a delayed teleport once the local model exists
if (!globals::isDemo && g_autoSpawnEnabled)
{
    uint64_t mapId = 0;
    if (TryGetCurrentMapId(_game_ea, mapId))
    {
        if (mapId != 0 && mapId != g_autoSpawnLastMap)
        {
            if (s_mapTeleportCoords.find(mapId) != s_mapTeleportCoords.end())
            {
                g_autoSpawnPendingTicks = g_autoSpawnDelayTicks;
                g_autoSpawnPendingMapId = mapId;
                g_autoSpawnPendingControllerId = controller->entity_id;

                printf("[autospawn] armed map 0x%llx in %d ticks\n",
                    (unsigned long long)mapId, g_autoSpawnDelayTicks);
            }
            else
            {
                printf("[autospawn] no mapping for mapId 0x%llx (not arming)\n",
                    (unsigned long long)mapId);
            }
        }
    }
}

			auto tell_statescript_this_is_localent = (void(__fastcall*)(Entity*, uint*))(globals::gameBase + 0xd07830);
			tell_statescript_this_is_localent(model, &controller->entity_id);

			if (globals::isDemo) {
				spawn_pachis();
			}

			_newly_added_entities.erase(it);
			continue;
		}

		it++;
	}

	if (!_world)
		_world = get_system27_WorldEngineSystem(_game_ea);
	else {
		if (_world->world_state == 6) {
			_world->Call_OnStateChance();

			auto init_ss_fn = (void(*)(Component_23_Statescript*))(globals::gameBase + 0x103acb0);
			for (auto ent : *_game_ea) {
				auto ss = ent->getById<Component_23_Statescript>(0x23);
				if (ss)
					init_ss_fn(ss);
			}

			if (globals::isDemo) {
				//state_replicator::start_server();
			}
			
			if (globals::isDemo) {

			}
		}
		else if (_world->world_state == 5) {
			_world->Call_OnStateChance();
		}
	}
}

PrometheusSystem* PrometheusSystem::create(EntityAdminBase* ea) {
	if (s_instance) {
		printf("PrometheusSystem already created! %p\n", s_instance);
		return nullptr;
	}

	auto sys = new PrometheusSystem;
	//sys->vfptr = new PrometheusSystem_vt;
	sys->_game_ea = ea;
	ea->GameEA_mapfunc_arr.emplace_item((mapsystem_callback_vt**)&sys->_mapfunc);
	s_instance = sys;

	MH_VERIFY_RET(MH_CreateHook((PVOID)(globals::gameBase + 0xcfe920), deallocate_view_hook, (LPVOID*)&deallocate_view_orig), nullptr);
	MH_VERIFY_RET(MH_EnableHook((PVOID)(globals::gameBase + 0xcfe920)), nullptr);
	return sys;
}

View* PrometheusSystem::deallocate_view_hook(View* view, char dealloc) {
	if (view == &s_instance->local_freeLookView->base) {
		s_instance->local_freeLookView = nullptr;
	}
	return deallocate_view_orig(view, dealloc);
}

void PrometheusSystem_Mapfunc::field_0() {
	printf("PrometheusSystem_Mapfunc: field_0\n");
}
void PrometheusSystem_Mapfunc::nullsub_1() {
	printf("PrometheusSystem_Mapfunc: nullsub_1\n");
}
void PrometheusSystem_Mapfunc::change_state_3_globalloading() {
	printf("PrometheusSystem_Mapfunc: change_state_3_globalloading\n");
}
void PrometheusSystem_Mapfunc::change_state_4_worldloading() {
	printf("PrometheusSystem_Mapfunc: change_state_4_worldloading\n");
}
void PrometheusSystem_Mapfunc::change_state_5_worldloaded() {
	printf("PrometheusSystem_Mapfunc: change_state_5_worldloaded\n");
}
void PrometheusSystem_Mapfunc::change_state_6_world_replicating() {
	printf("PrometheusSystem_Mapfunc: change_state_6_world_replicating\n");
}
void PrometheusSystem_Mapfunc::change_state_7_gamemode_loading() {
	printf("PrometheusSystem_Mapfunc: change_state_7_gamemode_loading\n");
}
void PrometheusSystem_Mapfunc::maybe_on_gamemode_unload() {
	printf("PrometheusSystem_Mapfunc: maybe_on_gamemode_unload\n");
}
void PrometheusSystem_Mapfunc::on_gamemode_loading() {
	printf("PrometheusSystem_Mapfunc: on_gamemode_loading\n");
}
void PrometheusSystem_Mapfunc::maybe_on_map_unload() {
	printf("PrometheusSystem_Mapfunc: maybe_on_map_unload\n");
}
void PrometheusSystem_Mapfunc::OnWorldLoadedAndInGameEnttiyAdmin() {
	printf("PrometheusSystem_Mapfunc: OnWorldLoadedAndInGameEnttiyAdmin\n");
}
void PrometheusSystem_Mapfunc::OnWorldLoadingAndInGameEntityAdmin() {
	printf("PrometheusSystem_Mapfunc: OnWorldLoadingAndInGameEntityAdmin\n");
}
void PrometheusSystem_Mapfunc::field_5() {
	printf("PrometheusSystem_Mapfunc: field_5\n");
}
