#pragma once
#include "../window_manager/window_manager.h"

class lobbymap_tester : public window {
	WINDOW_DEFINE(lobbymap_tester, "Tools", "LobbyMap tester", true);

	inline void render() override {
		if (open_window()) {
			//Anubis 0x80000000000005b
			//Watchpoint 0x800000000000184
			//Dorado 0x8000000000002c3

			//TODO: This crashes the game once deallocating the entity
			//for (auto map : _maps) {
			//	if (map.ent) {
			//		auto comp54 = map.ent->getById<Component_54_Lobbymap>(0x54);
			//		if (comp54->component_2_ref && comp54->component_2_ref->scene && comp54->component_2_ref->scene->load_map_state != 4) {
			//			((void(*)(EntityAdminBase*))(globals::gameBase + 0x8115d0))(comp54->embedded_game_ea); //Do some entity admin magic
			//			((void(*)(Component_2_AssetManager*, float, char))(globals::gameBase + 0x9f6770))(comp54->component_2_ref, 0, 1);
			//		}
			//	}
			//}

			if (ImGui::Button("Load All")) {
				_load_all = true;
			}

			int i = 0;
			bool load = _load_all;
			for (auto& map : _maps) {
				ImGui::PushID(map.map);
				bool loaded = !!map.ent;
				ImGui::RadioButton("Select", &_camera_for, i);
				ImGui::SameLine();
				if (load)
					_camera_for = i;
				if (ImGui::Checkbox(map.name.c_str(), &loaded) || load && !map.ent) {
					if (map.ent) {
						LobbyEntityAdmin()->delEnt(map.ent);
						map.ent = nullptr;
					}
					else {
						//globals::switchGameEA = true;
						auto spawn = EntityLoader::Create(0, 0, false, false);
						spawn->loader_entries[0x54].component_id = 0x54;
						auto ent = spawn->Spawn(LobbyEntityAdmin());
						auto comp54 = ent->getById<Component_54_Lobbymap>(0x54);
						if (!comp54) {
							printf("Comp54 failed\n");
						}
						else {
							comp54->LoadMap(map.map);
						}
						map.ent = ent;
					}
				}
				load = false;
				if (map.ent) {
					auto comp54 = map.ent->getById<Component_54_Lobbymap>(0x54);
					if (comp54->component_2_ref && comp54->component_2_ref->scene) {
						ImGui::SameLine();
						ImGui::Text("State: %d", comp54->component_2_ref->scene->load_map_state);
						load = _load_all && comp54->component_2_ref->scene->load_map_state == 4;
					}
				}
				i++;
				ImGui::PopID();
			}
			if (load)
				_load_all = false;

			auto& selected = _maps[_camera_for];
			display_addr((__int64)selected.ent, "Entity");
			ImGui::Text("Selected map: %s", selected.name.c_str());
			if (selected.ent) {
				auto comp54 = selected.ent->getById<Component_54_Lobbymap>(0x54);
				display_addr((__int64)comp54, "Comp54");
				display_addr((__int64)comp54->embedded_game_ea, "Comp54 EA");
				auto cam = comp54->embedded_game_ea->getSingletonComponent<Component_4F_Camera>(0x4F);
				if (cam) {
					if (cam->override_views.num == 0) {
						cam->override_views.emplace_item(OverrideView((View*)teFreeLookView::create(), true));
					}
					auto view = cam->override_views.ptr[0];
					STRUCT_MODIFIABLE(view.view_ptr, view_position.X);
					STRUCT_MODIFIABLE(view.view_ptr, view_position.Y);
					STRUCT_MODIFIABLE(view.view_ptr, view_position.Z);
				}
			}
		}
		ImGui::End();
	}

private:
	struct info {
		Entity* ent;
		__int64 map;
		std::string name;
	};

	static inline Component_54_Lobbymap*(*origSwitchCam)();
	static Component_54_Lobbymap* hookSwitchCam() {
		auto& mapst = _maps[_camera_for];
		if (mapst.ent != nullptr) {
			auto replacement = mapst.ent->getById<Component_54_Lobbymap>(0x54);
			return replacement;
			//origSwitchCam(replacement); //0xce6da0 0xc45400
		}
		return LobbyEntityAdmin()->getSingletonComponent<Component_54_Lobbymap>(0x54);
	}

	static inline int _camera_for = 0;
	static inline std::vector<info> _maps = {
		info{ nullptr, 0x80000000000005b, "Anubis" },
		info{ nullptr, 0x800000000000184, "Watchpoint" },
		info{ nullptr, 0x8000000000002c3, "Dorado" }
	};
	bool _load_all = false;

	inline void preStartInitialize() override {
		/*MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xc45400), hookSwitchCam, (PVOID*)&origSwitchCam));
		MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xc45400)));*/
	}
	//inline void initialize() override {}
};

WINDOW_REGISTER(lobbymap_tester);
