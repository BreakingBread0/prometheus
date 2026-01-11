#pragma once
#include "../window_manager/window_manager.h"
#include <vector>
#include "entitydef_list.h"
#include "../stringhash_library.h"
#include "entity_window.h"
#include "../stu_resources.h"
#include "../player_spawner.h"

//2f und 33 sind bei controller
//-> ist prob local player
//3f ist wsl bei model NEIN -> bei controller, controller ist local entity
//1f backref ist zu controller
//aim ent ist model
//mov attach ist model
//local entity ist das mit 3f
//local entity muss team haben in filter bits
//mov attach == local ent??? maybe, crasht nicht
//local ent muss camera stuff (c20) haben
//Component mit 2f muss c20 haben und wird benotzt

//SPECTATE:
//Entity mit C20 muss filterbits 0x400000 haben
//Ein Spectator kann alles spectaten wenn kein team in filterbits ist
//LocalEnt muss flag 0x400000 haben

//set pos: 2f <- 12 <- 15 (adds grav?)
//GameClientApp.exe+102B000
//GameClientApp.exe+CCEDDD

class player_spawner_deluxe : public window {
	WINDOW_DEFINE(player_spawner_deluxe, "Game", "Player Spawner DeLuxe", true);

	void render_ent_radios(std::string label, int* input) {
		ImGui::PushID(label.c_str());
		ImGui::Text("%s:", label.c_str());
		ImGui::SameLine();
		ImGui::RadioButton("None", input, 0);
		ImGui::SameLine();
		ImGui::RadioButton("Controller", input, 1);
		ImGui::SameLine();
		ImGui::RadioButton("Model", input, 2);
		ImGui::PopID();
	}

	void render_ent_options(player_spawner::spawn_info* inf) {
		imgui_helpers::InputHex("Resource ID", &inf->resource_id);
		/*if (ImGui::Button("Choose")) {
			window_manager::add_window(new entitydef_list(true, [inf](__int64 ent_out) {
				inf->resource_id = ent_out;
			}), this);
		}*/
		if (allmighty_hash_lib::comments.find(inf->resource_id) != allmighty_hash_lib::comments.end()) {
			ImGui::SameLine();
			ImGui::Text("(%s)", allmighty_hash_lib::comments[inf->resource_id].c_str());
		}
		/*if (inf->latest_entid) {
			display_addr(inf->latest_entid & 0xFFFFFFFF, "EntID");
			auto ent = GameEntityAdmin()->getEntById(inf->latest_entid);
			if (ent) {
				ImGui::SameLine();
				if (ImGui::Button("Delete")) {
					GameEntityAdmin()->delEnt(ent);
				}
				ImGui::SameLine();
				if (ImGui::Button(EMOJI_SHARE)) {
					entity_window::get_latest_or_create(this)->nav_to_ent(ent);
				}
			}
		}*/
		ImGui::NewLine();
		ImGui::Checkbox("Load 2F & 33", &inf->load_2f_33);
		if (inf->load_2f_33) {
			ImGui::SameLine();
			ImGui::Checkbox("Movable", &inf->set_2f_movable);
		}
		ImGui::Checkbox("Load 3F", &inf->load_3f);
		ImGui::Checkbox("Set Health", &inf->set_health);
		ImGui::Checkbox("Set Local", &inf->set_localent);
		ImGui::Checkbox("Controller FilterBits", &inf->set_filterbits_controller);
		//ImGui::Checkbox("call_component2_vodoo", &inf->call_component2_vodoo);
		ImGui::Checkbox("Weapon Attach", &inf->weapon_attach);

		ImGui::NewLine();
		render_ent_radios("Cam Attach", &inf->cam_attach);
		render_ent_radios("Aim Attach", &inf->aim_attach);
		render_ent_radios("Movement Attach", &inf->movement_attach);
		render_ent_radios("Backref Attach", &inf->petbackref_attach);
		ImGui::TextUnformatted("Team");
		ImGui::SameLine();
		ImGui::RadioButton("None", &inf->team, -1);
		ImGui::SameLine();
		ImGui::RadioButton("0", &inf->team, 0);
		ImGui::SameLine();
		ImGui::RadioButton("1", &inf->team, 1);
		ImGui::SameLine();
		ImGui::RadioButton("2", &inf->team, 2);
		ImGui::SameLine();
		ImGui::RadioButton("3", &inf->team, 3);
	}

	//void template_localplayer() {
	//	_spawner.controller_info.resource_id = 0x40000000000000E;
	//	_spawner.controller_info.aim_attach = 2;
	//	_spawner.controller_info.cam_attach = 2;
	//	_spawner.controller_info.set_localent = true;
	//	_spawner.controller_info.set_2f_movable = true;
	//	_spawner.controller_info.load_2f_33 = true;
	//	_spawner.controller_info.load_3f = true;
	//	_spawner.controller_info.movement_attach = 2; //Doch model sonst funktioniert F1 nicht
	//	_spawner.controller_info.set_filterbits_controller = false;
	//	_spawner.model_info.resource_id = 0x400000000000001;
	//	_spawner.model_info.petbackref_attach = 1;
	//	_spawner.model_info.set_health = true;
	//	_spawner.model_info.weapon_attach = true;
	//}

	void template_teamplayer() {
		_spawner.controller_info.resource_id = 0x40000000000000E;
		_spawner.controller_info.aim_attach = 2;
		_spawner.controller_info.cam_attach = 2;
		_spawner.controller_info.set_localent = false;
		_spawner.controller_info.set_2f_movable = false;
		_spawner.controller_info.load_2f_33 = false;
		_spawner.controller_info.load_3f = true;
		_spawner.controller_info.movement_attach = 2;
		_spawner.controller_info.set_filterbits_controller = false;
		_spawner.model_info.resource_id = 0x400000000000001;
		_spawner.model_info.petbackref_attach = 1;
		_spawner.model_info.set_health = true;
	}

	inline void render() override {
		if (open_window(nullptr, 0, ImVec2(1050, 550))) {
			ImGui::Text("Player Spawner DeLuxe at your service.");
			if (ImGui::BeginChild("replay entity", ImVec2(0, 0), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY)) {
				ImGui::Text("Controller Entity");
				render_ent_options(&_spawner.controller_info);
			}
			ImGui::EndChild();
			ImGui::SameLine();
			if (ImGui::BeginChild("model entity", ImVec2(0, 0), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY)) {
				ImGui::Text("Model Entity");
				STU_Object _selected_hero = STU_Object(nullptr, nullptr);
				if (ImGui::BeginListBox("H", ImVec2(0, 150))) {
					for (auto hero : stu_resources::GetIfAssignableTo(stringHash("STUHero"))) {
						auto stu = hero.second->to_editable();
						auto ent_id = stu.get_argument_resource("m_gameplayEntity")->resource_id;
						auto name = stu.get_argument_resource(0xcd16baa)->get_UXDisplayString();
						auto selected = _spawner.model_info.heroid == hero.first;

						if (selected)
							_selected_hero = stu;
						if (ImGui::Selectable(name, selected)) {
							_spawner.model_info.resource_id = ent_id;
							_spawner.model_info.heroid = hero.first;
						}
					}
					ImGui::EndListBox();
				}

				if (_selected_hero.valid()) {
					//if (ImGui::BeginListBox("S", ImVec2(600, 150))) { //auto resize doesnt work here...
						for (auto skin_id : _selected_hero.get_argument_resreflist("m_skins")) {
							auto skin = skin_id->get_STU()->to_editable();
							std::string name = std::format("{:x}", skin_id->resource_id);
							//Dont bother no skin has a name in this version
							/*auto name1 = skin.get_argument_primitive("m_internalName").get_value<const char*>();
							auto name2 = skin.get_argument_primitive("m_skinCode").get_value<const char*>();
							auto name3 = skin.get_argument_primitive(0x5c415784).get_value<const char*>();
							if (strlen(name1) > 0)
								name += name1 + std::string(" ");
							if (strlen(name2) > 0)
								name += name2 + std::string(" ");
							if (strlen(name3) > 0)
								name += name3 + std::string(" ");*/
							for (auto skin_theme_id : skin.get_argument_resreflist("m_skinThemes")) {
								auto skin_theme = skin_theme_id->get_STU()->to_editable();
								if (ImGui::RadioButton(std::format("##{:x}", skin_theme_id->resource_id).c_str(), _spawner.model_info.skin_theme_id == skin_theme_id->resource_id)) {
									_spawner.model_info.skin_theme_id = skin_theme_id->resource_id;
								}
								ImGui::SameLine();
								ImGui::TextDisabled(name.c_str());
								ImGui::SameLine();
								imgui_helpers::display_type(skin_theme_id->resource_id, true, true, false);
							}
						}
						//ImGui::EndListBox();
					//}
				}

				render_ent_options(&_spawner.model_info);
			}
			ImGui::EndChild();
			if (ImGui::Button("Spawn")) {
				_spawner.spawn();
				/*auto game_admin = GameEntityAdmin();
				auto get_combatsys = (CombatSystem * (__fastcall*)(EntityAdminBase*))(globals::gameBase + 0xc71600);
				CombatSystem* combat_sys = get_combatsys(game_admin);
				printf("combat sys: %p\n", combat_sys);
				auto client_spawn = (__int64(__fastcall*)(CombatSystem * combat_sys, __int64, __int64 jammsg))(globals::gameBase + 0xca8410);
				char buf[0x100];
				client_spawn(combat_sys, 0, (__int64)buf);
				combat_sys = get_combatsys(game_admin);
				combat_sys->list_with_entids.emplace_item(controller_entity.latest_entid);
				combat_sys->list_with_entids.emplace_item(model_entity.latest_entid);


				auto spawn = (void(__fastcall*)(char*, __int64, char*))(globals::gameBase + 0xca8660);
				char combatsys[0x10];
				*(__int64*)&combatsys[8] = (__int64)GameEntityAdmin();
				char jammsg[0x90];
				*(int*)&jammsg[0x78] = m_ent->entity_id;
				*(Vector4*)&jammsg[0x68] = Vector4(1, 50, 1, 0);
				spawn(combatsys, 0, jammsg);

				auto localplayer_shit = (void(__fastcall*)(__int64, ComponentBase*))(globals::gameBase + 0xc27800);
				auto get_sys36 = (__int64(__fastcall*)(EntityAdminBase*))(globals::gameBase + 0xc03750);
				__int64 sys = get_sys36(game_admin);
				if (sys) {
					localplayer_shit(sys, c_ent->getById(0x2F));
					auto move = (void(__fastcall*)(__int64, int, int, char, char))(globals::gameBase + 0xc27560);
					move(sys, 5, 5, 1, 1);
				}*/

				/*auto comp_12 = (__int64)c_ent->getById(0x12);
				if (comp_12) {
					*(int*)(comp_12 + 0x14394) = 1;
					*(double*)(comp_12 + 0x14398) = *(double*)(globals::gameBase + 0x181e320);
					*(int*)(comp_12 + 0x143A0) = 1;
				}*/

				//Setzt rotation und nicht position??
				/*auto setpos = (void(__fastcall*)(__int64, ComponentBase*, Vector4*))(globals::gameBase + 0xca6fb0);
				auto comp_2f = c_ent->getById(0x2F);
				if (comp_2f) {
					auto pos = Vector4(0, 0, 0, 50);
					setpos(0, comp_2f, &pos);
				}*/
			}

			/*if (ImGui::Button("Call Replay ControllerAAAAA")) {
				mapfunc_vt** replay_controller = *(mapfunc_vt***)(globals::gameBase + 0x17b7fc0);
				auto func = (void(*)(mapfunc_vt**))(globals::gameBase + 0xc81c80);
				*(int*)((__int64)replay_controller + 0x100) = controller_entity.latest_entid;
				*(__int64*)((__int64)replay_controller + 0xB8) = (__int64)GameEntityAdmin();
				func(replay_controller);
			}

			if (ImGui::Button("Jank Movement Fix")) {
				char* ch = (char*)(globals::gameBase + 0xca0800);
				ch[0] = 0xc3;
			}

			if (ImGui::Button("Template: Local Player")) {
				template_localplayer();
			}
			ImGui::SameLine();
			if (ImGui::Button("Template: Team")) {
				template_teamplayer();
			}
			if (ImGui::Button("InGame Flag 0x200 Lobby")) {
				auto ea = LobbyEntityAdmin();
				memset((void*)(globals::gameBase + 0xc4ea69), 0x90, 0xc4f00a - 0xc4ea69);
				auto flagger = ea->vfptr->GetStatescriptFlagger(ea);
				(*flagger)->SetStatescriptFlags(flagger, 0x200);
			}
			if (ImGui::Button("InGame Flag 0x200 Game")) {
				auto ea = GameEntityAdmin();
				memset((void*)(globals::gameBase + 0xc60488), 0x90, 0xc60c40 - 0xc60488);
				auto flagger = ea->vfptr->GetStatescriptFlagger(ea);
				(*flagger)->SetStatescriptFlags(flagger, 0x200);
			}*/

			ImGui::NewLine();
			if (ImGui::RadioButton("0x400000000000013 (PvPGame 1)", _spawn_entid == 0x400000000000013)) {
				_spawn_entid = 0x400000000000013;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("0x40000000000012c (PvPGame 2)", _spawn_entid == 0x40000000000012c)) {
				_spawn_entid = 0x40000000000012c;
			}
			ImGui::SameLine();
			ImGui::Checkbox("Spawn in LobbyEA", &_spawn_lobby);
			ImGui::SameLine();
			ImGui::Checkbox("Set Location", &_spawn_setloc);
			imgui_helpers::InputHex("Ent", &_spawn_entid);
			ImGui::SameLine();
			if (ImGui::Button("Spawn Entity Simple")) {
				auto simple_res = try_load_resource(_spawn_entid);
				if (!simple_res) {
					imgui_helpers::messageBox("Failed to load entity " + std::format("{:x}", _spawn_entid), this);
					return;
				}
				auto loader = EntityLoader::Create(_spawn_entid, simple_res, false, false);
				Component_1_SceneRendering::InitData data{};
				if (_spawn_setloc) {
					Entity* controller = GameEntityAdmin()->getLocalEnt();
					Entity* model = GameEntityAdmin()->getEntById(controller->getById<Component_20_ModelReference>(0x20)->cam_attach_entid);
					auto comp1 = model->getById<Component_1_SceneRendering>(1);
					if (comp1) {
						data.position = comp1->position;
						data.rotation = comp1->rotation;
						data.scale = comp1->scaling;
						loader->loader_entries[1].init_data = (__int64)&data;
					}
				}
				auto created_ent = loader->Spawn(_spawn_lobby ? LobbyEntityAdmin() : GameEntityAdmin());
				if (created_ent) {
					_pvpgame_entid = created_ent->entity_id;
					printf("Spawned Entity Simple: %x\n", _pvpgame_entid);
				}
			}
			auto pvpgame_ent = GameEntityAdmin()->getEntById(_pvpgame_entid);
			if (pvpgame_ent) {
				ImGui::Text("%x", _pvpgame_entid);
				ImGui::SameLine();
				if (ImGui::Button("Delete")) {
					GameEntityAdmin()->delEnt(pvpgame_ent);
				}
				ImGui::SameLine();
				if (ImGui::Button(EMOJI_SHARE)) {
					entity_window::get_latest_or_create(this)->nav_to_ent(pvpgame_ent);
				}
			}

			ImGui::NewLine();
			auto sys = PrometheusSystem::instance();
			if (_dealloc_freelookview && sys->local_freeLookView) {
				sys->local_freeLookView = nullptr;
				ow_dealloc((__int64)sys->local_freeLookView);
				_dealloc_freelookview = false;
			}
			bool freelook = !!sys->local_freeLookView;
			if (ImGui::Checkbox("FreeLookView", &freelook)) {
				if (sys->local_freeLookView) {
					_dealloc_freelookview = true;

					auto cam = GameEntityAdmin()->getSingletonComponent<Component_4F_Camera>(0x4F);
					cam->override_views.num = 0;
					
					//*(char*)(globals::gameBase + 0xc0bb70) = 0x48;
				}
				else {
					sys->local_freeLookView = teFreeLookView::create();

					auto cam = GameEntityAdmin()->getSingletonComponent<Component_4F_Camera>(0x4F);
					cam->override_views.emplace_item(OverrideView{ &sys->local_freeLookView->base, 1 });

					//*(char*)(globals::gameBase + 0xc0bb70) = 0xc3; //Prevent writing default view / override views from player entity
				}
			}
			if (freelook) {
				ImGui::SliderFloat("Movement Delta", &sys->freeLookView_movDelta, 10, 50, "%.0f", ImGuiSliderFlags_AlwaysClamp);
			}

			if (_dealloc_thirdpers && _thirdpers_view) {
				ow_dealloc((__int64)_thirdpers_view);
				_thirdpers_view = nullptr;
				_dealloc_thirdpers = false;
			}
			//TODO: outside of the state view pos doesnt get updated properly.
			/*bool thirdpers = !!_thirdpers_view;
			if (ImGui::Checkbox("ThirdPersonView", &thirdpers)) {
				auto model_ent = GameEntityAdmin()->getEntById(GameEntityAdmin()->getLocalEnt()->getById<Component_20_Pet>(0x20)->cam_attach_entid);
				auto ent_backref = model_ent->getById<Component_1F_PetBackref>(0x1F);

				if (_thirdpers_view) {
					ent_backref->override_views.num = 0;
					_dealloc_thirdpers = true;
				}
				else {
					__int64 view_ptr = ow_memalloc(0x1E0);
					auto init_thirdperson_view = (View*(*)(__int64))(globals::gameBase + 0xdcd5b0);
					_thirdpers_view = init_thirdperson_view(view_ptr);
					printf("ThirdPersonView allocated: %p\n", _thirdpers_view);

					ent_backref->override_views.emplace_item(OverrideView{ _thirdpers_view, 1 });
				}
			}*/
			/*if (_thirdpers_view) {
				auto model_ent = GameEntityAdmin()->getEntById(GameEntityAdmin()->getLocalEnt()->getById<Component_20_Pet>(0x20)->cam_attach_entid);
				if (model_ent) {
					auto model_rendering = model_ent->getById<Component_1_SceneRendering>(1);
					if (model_rendering) {
						_thirdpers_view->view_position = model_rendering->position;
					}
				}
			}*/
			/*ImGui::TextUnformatted("Time Scale");
			ImGui::SameLine();
			auto time_scale = globalTimeScale();

			if (ImGui::RadioButton("0.1", *time_scale == 0.1)) {
				*time_scale = 0.1;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("0.5", *time_scale == 0.5)) {
				*time_scale = 0.5;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("1", *time_scale == 1)) {
				*time_scale = 1;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("2", *time_scale == 2)) {
				*time_scale = 2;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("3", *time_scale == 3)) {
				*time_scale = 3;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("5", *time_scale == 5)) {
				*time_scale = 5;
			}*/
			//0xdcd5b0
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	inline void initialize() override {
		_spawner.template_localPlayer();
	}

private:
	bool _dealloc_freelookview = false;
	bool _dealloc_thirdpers = false;
	View* _thirdpers_view = 0;
	player_spawner _spawner;

	__int64 _spawn_entid = 0x400000000000013;
	bool _spawn_lobby = false;
	bool _spawn_setloc = false;
	uint _pvpgame_entid = 0;
};

WINDOW_REGISTER(player_spawner_deluxe);