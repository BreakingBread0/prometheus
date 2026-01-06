#pragma once
#include "../window_manager/window_manager.h"
#include "../idadefs.h"
#include "../entity_admin.h"
#include "../game.h"
#include "../hash_exporter.h"

class game_data_window : public window {
	WINDOW_DEFINE(game_data_window, "Game", "Game Data", true);

	std::string getClientType(int input) {
		switch (input) {
		case 0:
			return "CLIENTTYPE_SRV";
		case 1:
			return "CLIENTTYPE_WIN";
		case 2:
			return "CLIENTTYPE_UNK";
		case 3:
			return "CLIENTTYPE_UNK2";
		}
		return "CLIENTTYPE Unknown (" + std::to_string(input) + ")"; 
	}

	/*std::string getClientMode(int input) {
		switch (input) {
		case 0:
			return "CLIENTMODE_DEV";
		case 1:
			return "CLIENTMODE_WIN";
		case 2:
			return "CLIENTMODE_WINLF";
		}
		return "CLIENTMODE Unknown (" + std::to_string(input) + ")";
	}*/

	/*std::string getClientState(int input) {
		switch (input) {
		case 0:
			return "CLIENTSTATE_DEV";
		case 1:
			return "CLIENTSTATE_EXT";
		}
		return "CLIENTSTATE Unknown (" + std::to_string(input) + ")";
	}*/

	__int64 __fastcall FLIP_BITS(unsigned __int64 a1)
	{
		unsigned __int64 v1; // rdx
		unsigned __int64 v2; // rdx
		unsigned __int64 v3; // r8
		unsigned __int64 v4; // rcx

		v1 = (2 * a1) ^ ((2 * a1) ^ (a1 >> 1)) & 0x5555555555555555i64;
		v2 = (4 * v1) ^ ((4 * v1) ^ (v1 >> 2)) & 0x3333333333333333i64;
		v3 = (16 * v2) ^ ((16 * v2) ^ (v2 >> 4)) & 0xF0F0F0F0F0F0F0Fi64;
		v4 = (v3 << 8) ^ ((v3 << 8) ^ (v3 >> 8)) & 0xFF00FF00FF00FFi64;
		return __ROL8__((v4 << 16) ^ ((v4 << 16) ^ (v4 >> 16)) & 0xFFFF0000FFFFi64, 32);
	}

	std::string getLocaleStr(int input) {
		switch (input) {
		case 0:
			return "LOCALE_DEV";
		case 1:
			return "LOCALE_ENUS";
		}
		return "LOCALE Unknown (" + std::to_string(input) + ")";
	}

	std::string getRegionString(int type) {
		if (type == 0)
			return "REGION_DEV";
		if (type == 1)
			return "REGION_US";
		return "REGION Unknown (" + std::to_string(type) + ")";
	}

	std::map<__int64, __int64> _mapheaders;

	inline void render() override {
		if (open_window()) {
			typedef char* (__fastcall* getEngineState_fn)();
			getEngineState_fn getEngineState = (getEngineState_fn)(globals::gameBase + 0x7d88d0);
			ImGui::Text("Game Engine State: %s (%d)\n", getEngineState(), *(int*)(globals::gameBase + 0x18582ec));
			ImGui::Text("Last Assertion: %s", globals::gameBase + 0x18b2130);

			int clientType = *(int*)(globals::gameBase + 0x17a16dc);
			ImGui::Text("Client Type: %s (%d)", getClientType(clientType).c_str(), clientType);
			int region = *(int*)(globals::gameBase + 0x17a16e4);
			ImGui::Text("Client Region: %s (%d)", getRegionString(region).c_str(), region);
			int locale = *(int*)(globals::gameBase + 0x17a16e8);
			ImGui::Text("Locale: %s (%d)", getLocaleStr(locale).c_str(), locale);
			int audlocale = *(int*)(globals::gameBase + 0x17a16ec);
			ImGui::Text("Audio Locale: %s (%d)", getLocaleStr(audlocale).c_str(), audlocale);

			ImGui::Checkbox("Tabbing Enabled", (bool*)(globals::gameBase + 0x1859dcc));

			if (ImGui::Button("hash export")) {
				hash_exporter::export_hashes();
			}

			ImGui::Text("");
			ImGui::Text("");
			ImGui::Text("");
			if (ImGui::Button("Test")) {
				*(__int64*)(globals::gameBase + 0x18af678) = 0x080000000000005B;
				typedef char* (__fastcall* testcall_fn)(void);
				testcall_fn testcall = (testcall_fn)(globals::gameBase + 0xbfe670);
			}

			static __int64 bitflip{};
			imgui_helpers::InputHex("Bitflip", &bitflip);
			display_addr(FLIP_BITS(bitflip), "Bitflip Result");

			ImGui::NewLine();
			ImGui::NewLine();
			ImGui::NewLine();

			//if (ImGui::Button("Test call")) {
			//	typedef __int64(__stdcall* consoleAlloc_fn)();
			//	consoleAlloc_fn consoleAlloc = (consoleAlloc_fn)(globals::gameBase + 0xa7da60);
			//	imgui_helpers::openCopyWindow(consoleAlloc());
			//	/*printf("1\n");
			//	typedef __int64(__stdcall* dataPacKLoad_fn)(__int64, __int64, __int64, __int64);
			//	dataPacKLoad_fn dataPackLoad = (dataPacKLoad_fn)(globals::gameBase + 0x9c4670);
			//	__int64 vars[2]{};
			//	dataPackLoad(0x6E000000000008D, (__int64)vars, 0, globals::gameBase + 0x17a996a);
			//	dataPackLoad(0x6E00000000000B5, (__int64)vars, 0, globals::gameBase + 0x17a996b);
			//	printf("2\n");
			//	typedef __int64(__stdcall* afterDataPacKLoad_fn)(int, __int64, __int64, __int64, __int64);
			//	afterDataPacKLoad_fn afterDataPackLoad = (afterDataPacKLoad_fn)(globals::gameBase + 0xf3eb10);
			//	afterDataPackLoad(*(int*)(globals::gameBase + 0x17b7fe8), 0x6E0000000000079, 0, globals::gameBase + 0x17a996b, 4);*/
			//}

			//typedef __int64(*mapLoad_fn)(uint64_t map);
			//mapLoad_fn mapLoad = (mapLoad_fn)(globals::gameBase + 0xc3db90);

			//typedef int (*fn)(__int64**, __int64);
			//fn resource_load = (fn)(globals::gameBase + 0x9c8330);

			//static char buf[32];
			static __int64 entid = 0x400000000000001;
			imgui_helpers::InputHex("Entity ID", &entid);
			//ImGui::Text("Entity ResID: %p", entid);
			//static bool load_2f_33 = false;
			//static bool load_3f = false;
			//static bool set_in_camera = false;
			//static bool set_localent = false;
			//static bool set_health = false;
			//static bool set_pos = false;
			//static bool f_freeze = false;
			//static bool stack = false;
			//ImGui::Checkbox("Load 2F & 33", &load_2f_33);
			//ImGui::Checkbox("Load 3F", &load_3f);
			//ImGui::Checkbox("Set as Local Entity", &set_localent);
			//ImGui::Checkbox("Set in Camera", &set_in_camera);
			//ImGui::Checkbox("Set Health", &set_health);
			//ImGui::Checkbox("Set Pos & Misc", &set_pos);
			//ImGui::Checkbox("Comp 1F Freeze", &f_freeze);
			//ImGui::Checkbox("Stack on LocalEnt", &stack);
			//auto game_admin = GameEntityAdmin();
			//if (ImGui::Button("Test Spawn")) {
			//	EntityLoader loader{};
			//	auto constructor = (void(__fastcall*)(EntityLoader*))(globals::gameBase + 0x9d4c10);
			//	constructor(&loader);
			//	loader.vfptr = (EntityLoader_vt*)(globals::gameBase + 0x15c2978);
			//	loader.field_1850 = 0xF;
			//	loader.field_1854 = 0xF;
			//	loader.stu_id = entid;
			//	loader.Create2FAnd33 = load_2f_33;
			//	loader.Create3F = load_3f;
			//	//loader.additional_stu_id = 0x0AE0000000000123; //ist das vielleicht ein replay ent?
			//	if (entid == 0x400000000000001) {
			//		loader.aplist_extra = 0x0A50000000001711;
			//		loader.additional_stu_id = 0x0AE00000000003EA;
			//	}
			//	if (set_pos) {
			//		auto comp = &loader.loader_entries[0x30];
			//		comp->component_id = 0x30;
			//		comp->a1 = 0;
			//		comp->a2 = 0;
			//		comp->a3 = 0;
			//		/*comp = &loader.loader_entries[0x70];
			//		comp->component_id = 0x70;
			//		comp->a1 = 0;
			//		comp->a2 = 0;
			//		comp->a3 = 0;*/
			//	}
			//	auto load_ent2 = (__int64(__fastcall*)(__int64))(globals::gameBase + 0x9c6470);
			//	__int64 result_resload = load_ent2(entid);
			//	if (!result_resload/* == 0xFFFFFFFFFFFFFFFFLL
			//		|| ((unsigned __int64)result_resload & 0xFFFFFFFFFFFFFFC0uLL) == 0
			//		|| (((unsigned __int8)result_resload ^ *(_BYTE*)(((unsigned __int64)result_resload & 0xFFFFFFFFFFFFFFC0uLL) + 8)) & 0x3F) != 0*/)
			//	{
			//		printf("EL %p failed: Resource is null.\n", entid);
			//	}
			//	else {
			//		/*auto load_ent2 = (__int64(__fastcall*)(__int64))(globals::gameBase + 0x9c6470);*/
			//		//printf("%p\n", def & 0xFFFFFFFFFFFFFFC0uLL);
			//		//printf("%p\n", result_resload);
			//		/*printf("Breakpoint at: %p\n", &loader.additional_stu_id);
			//		system("pause");*/
			//		if (loader.vfptr->load_entity_into_loader(&loader, result_resload /*& 0xFFFFFFFFFFFFFFC0uLL*/)) {
			//			//loader.field_1850 = 3;
			//			typedef Entity* (__fastcall* load_entity_fn)(EntityAdminBase*, int* entid, EntityLoader*);
			//			load_entity_fn load = (load_entity_fn)(globals::gameBase + 0x80f7c0);
			//			int* default_entid = (int*)(globals::gameBase + 0x185a0c8);
			//			//int custom_entid = 0x80000001; //<- das funktioniert
			//			if (stack) {
			//				//default_entid = &game_admin->mby_local_entity;
			//				static int test = 0x800001a7;
			//				default_entid = &test;
			//			}
			//			auto entity = load(game_admin, default_entid, &loader);
			//			if (entity) {
			//				printf("Entity successfully spawned: %p %x\n", entity, entity->entity_id);
			//				if (set_localent) {
			//					/*game_admin->local_entity_id = */game_admin->mby_local_entity = entity->entity_id;
			//				}

			//				if (set_in_camera) {
			//					Component_20_Pet* cam = (Component_20_Pet*)entity->getById(0x20);
			//					if (cam)
			//						cam->aim_entid = cam->cam_attach_entid = cam->movement_attach_entid = entity->entity_id;
			//					else
			//						printf("Failed to find camera\n");
			//				}
			//				
			//				if (set_health) {
			//					Component_28_STUHealthComponent* health = (Component_28_STUHealthComponent*)entity->getById(0x28);
			//					if (health) {
			//						auto& part = health->normal_health[0];
			//						part.part_enabled = 1;
			//						part.curr_health = 169;
			//						part.max_health = 200;
			//						part.some_other_flag = 1;
			//						part.field_10 = 1;
			//					}
			//					else
			//						printf("Failed to find healthc omp\n");
			//				}

			//				if (set_pos) {
			//					//__int64 unk, component_2f, vector4 pos
			//					auto setpos = (void(__fastcall*)(__int64, ComponentBase*, Vector4))(globals::gameBase + 0xca6fb0);
			//					auto comp_2f = entity->getById(0x2F);
			//					if (comp_2f) {
			//						//*(char*)((__int64)comp_2f + 0x7d0) = 1;
			//						setpos(0, comp_2f, Vector4(1, 50, 1, 0));
			//					}

			//					auto component_3f = (__int64)entity->getById(0x3F);
			//					if (component_3f) {
			//						*(__int64*)(component_3f + 0x98) = 0x2e0000000000003;
			//						*(__int64*)(component_3f + 0xA0) = 0x0AE00000000003EA; //STUSkin
			//						*(char*)(component_3f + 0xC3) = 1; //Hero Select Enable
			//						*(char*)(component_3f + 0xC2) = 1; //Use alternative heroid?
			//						*(__int64*)(component_3f + 0xB0) = 0x2e0000000000003;
			//					}

			//					//entity->resload_entry2 = try_load_resource_2(0x0A50000000001711);

			//					/*auto test = (void(__fastcall*)(ComponentBase*, float, float))(globals::gameBase + 0x101bf90);
			//					test(LobbyEntityAdmin()->component_iterator[0x88].component_list.ptr[0], .1f, .1f);*/

			//					auto spawn = (void(__fastcall*)(char*, __int64, char*))(globals::gameBase + 0xca8660);
			//					char combatsys[0x10];
			//					*(__int64*)&combatsys[8] = (__int64)GameEntityAdmin();
			//					char jammsg[0x90];
			//					*(int*)&jammsg[0x78] = entity->entity_id;
			//					*(Vector4*)&jammsg[0x68] = Vector4(1, 50, 1, 0);
			//					spawn(combatsys, 0, jammsg);
			//					
			//					auto get_combatsys = (CombatSystem * (__fastcall*)(EntityAdminBase*))(globals::gameBase + 0xc71600);
			//					CombatSystem* combat_sys = get_combatsys(game_admin);
			//					printf("combat sys: %p\n", combat_sys);
			//					auto client_spawn = (__int64(__fastcall*)(CombatSystem * combat_sys, __int64, __int64 jammsg))(globals::gameBase + 0xca8410);
			//					char buf[0x100];
			//					client_spawn(combat_sys, 0, (__int64)buf);
			//					combat_sys = get_combatsys(game_admin);
			//					combat_sys->list_with_entids.emplace_item(entity->entity_id);

			//					auto get_sys36 = (__int64(__fastcall*)(EntityAdminBase*))(globals::gameBase + 0xc03750);
			//					auto localplayer_shit = (void(__fastcall*)(__int64, ComponentBase*))(globals::gameBase + 0xc27800);
			//					__int64 sys = get_sys36(game_admin);
			//					if (sys && comp_2f) {
			//						localplayer_shit(sys, comp_2f);
			//						auto move = (void(__fastcall*)(__int64, int, int, char, char))(globals::gameBase + 0xc27560);
			//						move(sys, 5, 5, 1, 1);
			//					}

			//					auto comp_12 = (__int64)entity->getById(0x12);
			//					if (comp_12) {
			//						*(int*)(comp_12 + 0x14394) = 1;
			//						*(double*)(comp_12 + 0x14398) = *(double*)(globals::gameBase + 0x181e320);
			//						*(int*)(comp_12 + 0x143A0) = 1;
			//					}
			//				}

			//				if (f_freeze) {
			//					auto component_1f = (__int64)entity->getById(0x1F);
			//					if (component_1f) {
			//						*(int*)((__int64)component_1f + 0x18) = entity->entity_id;
			//						*(int*)((__int64)component_1f + 0x1c) = entity->entity_global_id;
			//					}
			//				}

			//				/*auto fix_camera_fn = (void(__fastcall*)(__int64*))(globals::gameBase + 0xc0b520);
			//				__int64 fix_cam_data[3]{};
			//				fix_cam_data[0] = (__int64)game_admin;
			//				fix_camera_fn(&fix_cam_data[1]);*/

			//				/*for (int i = 0; i < entity->component_list.num - 1; i++) {
			//					if (entity->component_list.ptr[i]->component_id == 0x2F) {
			//						__int64 sys = get_sys36(game_admin);
			//						printf("%p Calling wtffunc\n", sys);
			//						__int64 compo = (__int64)entity->component_list.ptr[i];
			//						localplayer_shit(sys, (__int64)compo);
			//					}
			//				}*/
			//			}
			//			else {
			//				printf("Entity failed to spawn\n");
			//			}
			//		}
			//		else {
			//			printf("Entityloader refused entity %p\n", entid);
			//		}
			//	}
			//}
			//ZeroMemory(buf, 32);
			//static __int64 ssid = 0x400000000000001;
			//if (ImGui::InputText("SS ID", buf, 32)) {
			//	ssid = _strtoi64(buf, nullptr, 16);
			//}
			//ImGui::Text("SS ResID: %p", ssid);
			///*if (ImGui::Button("Load Script on Local Ent")) {
			//	auto game_ea = GameEntityAdmin();
			//	auto ent = game_ea->getEntById(game_ea->mby_local_entity);
			//	if (ent) {
			//		auto ss = (StatescriptComponent*)ent->getById(0x23);
			//		if (ss) {
			//			ss->base.vfptr->load_statescript_script(ss, ssid, 0, 0);
			//		}
			//		else {
			//			printf("Failed to get Statescript Comp!\n");
			//		}
			//	}
			//	else {
			//		printf("Local Ent not found!\n");
			//	}
			//}*/

			///*auto pscall = (void(__fastcall*)(__int64, int))(globals::gameBase + 0xc65110);
			//if (ImGui::Button("PS 0")) {
			//	pscall(*(__int64*)(globals::gameBase + 0x17b7fa8) + 0x20, 0);
			//}
			//ImGui::SameLine();
			//if (ImGui::Button("PS 1")) {
			//	pscall(*(__int64*)(globals::gameBase + 0x17b7fa8) + 0x20, 1);
			//}
			//ImGui::SameLine();
			//if (ImGui::Button("PS 2")) {
			//	pscall(*(__int64*)(globals::gameBase + 0x17b7fa8) + 0x20, 2);
			//}
			//ImGui::SameLine();
			//if (ImGui::Button("PS 3")) {
			//	pscall(*(__int64*)(globals::gameBase + 0x17b7fa8) + 0x20, 3);
			//}*/

			if (ImGui::Button("Test Load Resource")) {
				//auto load_ent = (__int64(__fastcall*)(__int64))(globals::gameBase + 0x9c6470);
				//0x2E00000000000014
				/*__int64* result_var;
				__int64 entity_def = resource_load(&result_var, entid);
				if (((uint64_t)result_var & 0xFFFFFFFFFFFFFFC0) != 0 && *(uint64_t*)((uint64_t)result_var & 0xFFFFFFFFFFFFFFC0) != 0) {
					printf("Res successfully spawned: %p\n", *(uint64_t*)((uint64_t)result_var & 0xFFFFFFFFFFFFFFC0));
				}
				else {
					printf("Failed to load res %p\n", entid);
				}*/
				auto result = try_load_resource(entid);
				printf("Resource Load 1: %p\n", result);
				auto result2 = try_load_resource_2(entid);
				printf("Resourde Load 2: %p\n", result2);
			}
			if (ImGui::Button("Test STUBreakable")) {
				for (auto comp : GameEntityAdmin()->component_iterator[0x2d].component_list) {
					__int64 comp_2d = (__int64)comp;
					*(uint*)(comp_2d + 0x98) = 1;
					((void(*)(__int64))(globals::gameBase + 0xcc3b00))(comp_2d);
				}
			}

			if (ImGui::Button("Replace State")) {
				auto inst = (STUStatescriptGraph*)try_load_resource(0x5800000000009d0);
				if (inst) {
					for (auto state : inst->m_states) {
						if (state->ss_base.graph_node.base.vfptr->GetSTUInfo()->name_hash == STU_NAME::STUStatescriptStateUXHeroSelectScreen) {
							state->ss_base.graph_node.base.vfptr = (STUBase_vt*)(globals::gameBase + 0x1509020);
							auto create_res_cv = (__int64(__fastcall*)(void))(globals::gameBase + 0x746ab0);
							auto find_impl_fn = (__int64(__fastcall*)(__int64 unused, __int64 stu_state))(globals::gameBase + 0xf43450);
							auto res_cv = create_res_cv();
							auto impl = find_impl_fn(0, res_cv);
							printf("stu/impl: %p %p\n", res_cv, impl);
							*(__int64*)(res_cv + 8) = impl;
							*(__int64*)(res_cv + 0x18) = 0x2e0000000000003;
							*(__int64*)((__int64)state + 0x150) = res_cv;
							printf("Found- replaced\n");
						}
					}
					/*for (auto state : inst->m_nodes) {
						if (state->graph_node.base.vfptr->GetSTUInfo()->name_hash == STU_NAME::STUStatescriptStateUXHeroSelectScreen) {
							state->graph_node.base.vfptr = (STUBase_vt*)(globals::gameBase + 0x1509020);
							auto create_res_cv = (__int64(__fastcall*)(void))(globals::gameBase + 0x746ab0);
							auto res_cv = create_res_cv();
							*(__int64*)(res_cv + 0x10) = 0x400000000000001;
							*(__int64*)((__int64)state + 0x150) = res_cv;
							printf("Found- replaced 2\n");
						}
					}*/
				}
				else {
					imgui_helpers::messageBox("Failed to replace state: Could not find graph", this);
				}
			}

			/*if (ImGui::Button("ARL")) {
				auto fn = (__int64(__fastcall*)(__int64, __int64, int, int, __int64, char))(globals::gameBase + 0x9c2d30);
				for (int i = 1; i < 7; i++) {
					__int64 result = fn(entid, 0, 1, i, 0, 0);
					printf("ARL Load: %p %p\n", entid, result);
				}
			}
			if (ImGui::Button("Asdf test")) {
				auto fn = (void(__fastcall*)(__int64))(globals::gameBase + 0xd35340);
				fn(0);
			}

			if (ImGui::Button("Inject Replay Controller")) {
				mapfunc_vt** replay_controller = *(mapfunc_vt***)(globals::gameBase + 0x17b7fc0);

				GameEntityAdmin()->GameEA_mapfunc_arr.emplace_item(replay_controller);
			}*/


			//ImGui::NewLine();
			/*if (ImGui::Button("Test ClientSpawn")) {
				auto get_combatsys = (CombatSystem*(__fastcall*)(EntityAdminBase*))(globals::gameBase + 0xc71600);
				CombatSystem* combat_sys = get_combatsys(game_admin);
				printf("combat sys: %p\n", combat_sys);
				auto client_spawn = (__int64(__fastcall*)(CombatSystem* combat_sys, __int64, __int64 jammsg))(globals::gameBase + 0xca8410);
				char buf[0x100];
				client_spawn(combat_sys, 0, (__int64)buf);
				combat_sys->list_with_entids.emplace_item();
			}*/
			//if (ImGui::Button("Get BlockTransferSystem")) {
			//	auto get_sys = (__int64(__fastcall*)(EntityAdminBase*))(globals::gameBase + 0xc83f60);
			//	__int64 block_sys = get_sys(game_admin);
			//	printf("block transfer sys: %p\n", block_sys);
			//}//

			//if (ImGui::Button("Load Maps")) {
			//	_mapheaders.clear();
			//	for (auto type : types) {
			//		printf("Loading map %p\n", type);
			//		__int64 result = mapLoad(type);
			//		if (result) {
			//			_mapheaders.emplace(type, result);
			//		}
			//	}
			//}

			//if (_mapheaders.size() > 0) {
			//	beginTable("Maps", { "Type", "Addr" });
			//	for (auto& header : _mapheaders) {
			//		ImGui::TableNextRow();

			//		ImGui::TableNextColumn();
			//		display_addr(header.first);

			//		ImGui::TableNextColumn();
			//		display_addr(header.second);
			//	}
			//	ImGui::EndTable();
			//}

			//int i = 0;
			//for (uint64_t typ : types) {
			//	char buf[64];
			//	sprintf_s(buf, "Test Map %p", typ);
			//	if (ImGui::Button(buf)) {
			//		//mapLoad(typ);
			//		__int64* output = 0;
			//		int result = resource_load(&output, typ);
			//		if ((long long)output == -1LL
			//			|| ((unsigned __int64)output & 0xFFFFFFFFFFFFFFC0uLL) == 0
			//			|| (((unsigned __int8)output ^ *(_BYTE*)(((unsigned __int64)output & 0xFFFFFFFFFFFFFFC0uLL) + 8)) & 0x3F) != 0)
			//		{
			//			//printf("Invalid return: " + std::to_string(result));
			//		}
			//		else {
			//			printf("VALID: %d\n", result);
			//		}
			//		imgui_helpers::openCopyWindow((__int64)((unsigned __int64)output & 0xFFFFFFFFFFFFFFC0uLL));
			//	}
			//	i++;
			//}

			/*__int64 mapdata_ptr = *(__int64*)(*(__int64*)(*(__int64*)(globals::gameBase + 0x17B7F90) + 0xd8) + 0x28);
			display_addr(mapdata_ptr, "Map Data PTR");

			__int64 mapupdates_ptr = *(__int64*)(mapdata_ptr + 8);
			display_addr(mapupdates_ptr, "Map Update PTR");
			for (int i = 0; i < *(int*)(mapupdates_ptr + 0x188); i++) {
				char buf[64];
				__int64 entry = *(__int64*)(*(__int64*)(mapupdates_ptr + 0x180) + i * 8);
				__int64 entry_vt = *(__int64*)(entry);
				sprintf_s(buf, "Update PTR %d", i);
				display_addr(entry, buf);
				memset(buf, 0, 64);
				sprintf_s(buf, "Update VT %d", i);
				display_addr(entry_vt, buf);
				ImGui::NewLine();
			}*/

		}
		ImGui::End();
	}
};

WINDOW_REGISTER(game_data_window)