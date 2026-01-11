#include "player_spawner.h"
#include "window_manager/window_manager.h"
#include "stu_resources.h"
#include "Statescript.h"
#include "StatescriptVar.h"
#include "Logs/Logs.h"

void player_spawner::modify_ent(spawn_info info, Entity* ent) {
	auto game_ea = GameEntityAdmin();
	//Entity* ent = game_ea->getEntById(info.latest_entid);
	if (!ent) {
		imgui_helpers::messageBox("Failed to modify ent");
		return;
	}
	if (info.set_localent) {
		game_ea->local_entid = ent->entity_id;
	}

	Component_20_ModelReference* pet_comp = ent->getById<Component_20_ModelReference>(0x20);
	if (pet_comp) {
		if (info.cam_attach > 0) {
			pet_comp->cam_attach_entid = info.cam_attach == 1 ? controller_ent->entity_id : model_ent->entity_id;
		}
		if (info.aim_attach > 0) {
			pet_comp->aim_entid = info.aim_attach == 1 ? controller_ent->entity_id : model_ent->entity_id;
		}
		if (info.movement_attach > 0) {
			pet_comp->movement_attach_entid = info.movement_attach == 1 ? controller_ent->entity_id : model_ent->entity_id;
		}
	}

	Component_1F_Backref* petref = ent->getById<Component_1F_Backref>(0x1F);
	if (petref && info.petbackref_attach > 0) {
		Entity* backref_ent = info.petbackref_attach == 1 ? controller_ent : model_ent;
		petref->backref_entid = backref_ent->entity_id;
		petref->backref_global_entid = backref_ent->entity_global_id;
	}

	Component_28_STUHealthComponent* health = ent->getById<Component_28_STUHealthComponent>(0x28);
	if (health && info.set_health) {
		auto& part = health->normal_health[0];
		part.part_enabled = 1;
		part.curr_health = 169;
		part.max_health = 200;
		part.some_other_flag = 1;
		part.field_10 = 1;
	}

	Component_2F_LocalPlayer* lp = ent->getById<Component_2F_LocalPlayer>(0x2F);
	if (lp && info.set_2f_movable) {
		//lp->is_able_to_aim = true;

		auto move = (void(__fastcall*)(__int64, int, int, char, char))(globals::gameBase + 0xc27560);
		auto get_sys36 = (__int64(__fastcall*)(EntityAdminBase*))(globals::gameBase + 0xc03750);
		__int64 sys = get_sys36(GameEntityAdmin());
		move(sys, 1000, 1000, 1, 1);
	}

	Component_3F_PlayerInfo* player_info = ent->getById<Component_3F_PlayerInfo>(0x3F);
	if (player_info) {
		player_info->hero_select_enabled = true;
		player_info->hero_was_selected = true;
		player_info->m_slot = 3;
		player_info->selected_heroid = info.heroid;
		player_info->localplayer_heroselected = true;
		//player_info->selected_heroid_2 = info.heroid;
		//player_info->unk_1 = 1;
	}

	Component_10_FilterBits* filter_bits = ent->getById<Component_10_FilterBits>(0x10);
	if (filter_bits) {
		if (info.set_filterbits_controller) {
			filter_bits->set_filterbits_spectator();
		}
		if (info.team != -1) {
			filter_bits->set_team(info.team);
		}
		filter_bits->filter_bits = 0x800000;
		filter_bits->FilterBitsMakeFinal();
	}

	Component_23_Statescript* statescript = ent->getById<Component_23_Statescript>(0x23);
	if (statescript) {
		if (info.weapon_attach) {
			auto entity_def = stu_resources::GetByID(info.resource_id)->to_editable();
			auto component_map = entity_def.get_argument_map("m_componentMap");
			auto weapon_comp = ((STUBase<>*)component_map.value->get_by_key(stringHash("STUWeaponComponent"))->value)->to_editable(); //
			statescript->loadScript(weapon_comp.get_argument_resource("m_managerScript")->resource_id);

			//filler stub for the entry to 0x0D8000000000001D (valid weapon idxs dict)
			const StatescriptPrimitive wpnDictionaryBotox{0, StatescriptPrimitive_INT};
			statescript->ss_inner.rid_entity_varbag->SetArray({0x0D8000000000001D, StatescriptVar_ENTITY_VARBAG}, wpnDictionaryBotox, wpnDictionaryBotox, 1);

			auto weaponId = 1;
			for (auto weapon : weapon_comp.get_argument_objectlist("m_weapons")) {
				if (auto script = weapon.get_argument_resource("m_script"); script->has_resource()) {
					statescript->loadScript(script->resource_id);

					if (auto inst = statescript->ss_inner.getByResourceId(script->resource_id)) {
						LOG_INFO("Successfully created weapon script: {:#x} (inst: {:#x}) with TargetID {}", script->resource_id, reinterpret_cast<int64>(inst), weaponId);

						//Fix selected weapon targetIDs for weapon graphs (0xd8000000000001c)
						const StatescriptPrimitive selectedWeaponTargetID{weaponId, StatescriptPrimitive_INT};
						inst->rid_instance_varbag->SetVar({ StatescriptVar_INSTANCE_VARBAG, 0xd8000000000001c }, selectedWeaponTargetID);

						//Fix weapon IDx array for weapon switching.
						const StatescriptPrimitive wpnDictionaryId{weaponId++, StatescriptPrimitive_INT};
						statescript->ss_inner.rid_entity_varbag->SetArray({0x0D8000000000001D, StatescriptVar_ENTITY_VARBAG}, wpnDictionaryId, wpnDictionaryId, weaponId);
					} else {
						LOG_ERROR("Failed to create weapon script: {:#x}", script->resource_id);
						weaponId++;
					}
				}
			}
		}

		if (info.fixup_player_statescript) {
			StatescriptPrimitive cv{};
			cv.type = StatescriptPrimitive_BYTE;
			cv.value = 1;
			statescript->ss_inner.rid_entity_varbag->SetVar({ StatescriptVar_ENTITY_VARBAG, 0x0D800000000000FD }, cv);
		}
	}

	/*if (info.call_component2_vodoo) {
		auto comp_2 = (*ent->entity_admin_backref->system_1_ref)->GetComponent2WarumAuchImmer(ent->entity_admin_backref->system_1_ref);
		printf("Comp2: %p\n", comp_2);
		auto comp2_vodoo = (void(*)(Component_2_AssetManager*, Entity*))(globals::gameBase + 0x9f2290);
	}*/
}

Entity* player_spawner::spawn_ent(spawn_info& info) {
	auto res_ptr = try_load_resource(info.resource_id);
	if (!res_ptr) {
		printf("PlayerSpawner failed: No Resource! (Resource ID: %p)\n", info.resource_id);
		return nullptr;
	}
	auto spawner = EntityLoader::Create(info.resource_id, res_ptr, info.load_2f_33, info.load_3f);
	for (auto additional : info.additional_compids) {
		auto spawner_elem = &spawner->loader_entries[additional.component_id];
		spawner_elem->component_id = additional.component_id;
		if (additional.init_data)
			spawner_elem->init_data = additional.init_data;
		if (additional.stu_component_data)
			spawner_elem->stu_component_data = additional.stu_component_data;
		if (additional.stu_instance_data)
			spawner_elem->stu_instance_data = additional.stu_instance_data;
		//memcpy(elem, &info, sizeof(EntityLoader_Entry));
	}
	spawner->aplist_extra = info.skin_theme_id;
	return spawner->Spawn(GameEntityAdmin());
}

void player_spawner::template_localPlayer() {
	controller_info.resource_id = 0x40000000000000E;
	controller_info.aim_attach = 2;
	controller_info.cam_attach = 2;
	controller_info.set_localent = true;
	controller_info.set_2f_movable = true;
	controller_info.load_2f_33 = true;
	controller_info.load_3f = true;
	controller_info.movement_attach = 2; //Doch model sonst funktioniert F1 nicht
	controller_info.set_filterbits_controller = false;
	model_info.resource_id = 0x400000000000001;
	model_info.petbackref_attach = 1;
	model_info.set_health = true;
	model_info.weapon_attach = true;
	model_info.fixup_player_statescript = true;
}

//returns key: controller - value: model
std::pair<Entity*, Entity*> player_spawner::spawn() {
	controller_ent = spawn_ent(controller_info);
	model_ent = spawn_ent(model_info);
	if (!controller_ent || !model_ent) {
		return {};
	}

	//controller_entity.latest_entid = c_ent->entity_id;
	//model_entity.latest_entid = m_ent->entity_id;
	modify_ent(controller_info, controller_ent);
	modify_ent(model_info, model_ent);
	return { controller_ent, model_ent };
}