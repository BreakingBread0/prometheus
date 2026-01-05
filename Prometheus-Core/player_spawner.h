#pragma once
#include <vector>
#include "entity_admin.h"

class player_spawner {
public:
	struct spawn_info {
		//Load
		__int64 resource_id = 0;
		__int64 skin_theme_id = 0;
		__int64 heroid = 0;
		//uint32 latest_entid;
		bool load_2f_33 = false;
		bool load_3f = false;
		//value: initData ptr
		std::vector<EntityLoader_Entry> additional_compids{};

		//Modify
		int cam_attach = 0;
		int aim_attach = 0;
		int movement_attach = 0;
		int petbackref_attach = 0;
		int team = -1;
		bool set_health = false;
		bool set_2f_movable = false;
		bool set_localent = false;
		bool set_filterbits_controller = false;
		bool weapon_attach = false;
		bool fixup_player_statescript = false;
		//bool call_component2_vodoo;
	};

private:
	void modify_ent(spawn_info info, Entity* ent);
	Entity* spawn_ent(spawn_info& info);

public:
	spawn_info controller_info{};
	spawn_info model_info{};
	Entity* controller_ent;
	Entity* model_ent;

	void template_localPlayer();

	player_spawner() {
		template_localPlayer();
	}

	player_spawner(__int64 model_entid) {
		template_localPlayer();
		model_info.resource_id = model_entid;
	}

	//returns key: controller - value: model
	std::pair<Entity*, Entity*> spawn();
};