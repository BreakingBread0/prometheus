#pragma once
#include "../window_manager/window_manager.h"
#include <map>
#include "../entity_admin.h"

class entityadmin_systems_window : public window {
	WINDOW_DEFINE(entityadmin_systems_window, "ECS", "Systems", true);

	static inline std::vector<std::pair<std::string, long>> s_systems = {
		{ "System 1", 0x15c4440 },
		{ "System 1 Primitive", 0x15d2e70 },
		{ "System 2", 0x15c4630 },
		{ "System 3", 0x15c46f8 },
		{ "System 4", 0x15c41c0 },
		{ "System 5", 0x15c2248 },
		{ "System 6", 0x15c4150 },
		{ "System 7", 0x15c47b8 },
		{ "System 8", 0x15c4840 },
		{ "System 9", 0x15c48d0 },
		{ "System 10", 0x15c4998 },
		{ "System 11", 0x15c22c8 },
		{ "System 12", 0x15c20f0 },
		{ "System 13", 0x15c2080 },
		{ "System 14", 0x15c21d0 },
		{ "System 15", 0x15c2160 },
		{ "System 16", 0x15c1f90 },
		{ "System 17", 0x15c4a08 },
		{ "System 18", 0x15c1f20 },
		{ "System 19", 0x15c2010 },
		{ "System 29 (EntityStartup)", 0x15c4ac8 },
		{ "System 21", 0x15c4b78 },
		{ "System 22 (NetworkEventSystem?)", 0x15c3b90 },
		{ "System 23", 0x15c4be8 },
		{ "System 24", 0x15c23a8 },
		{ "System 25 (JamSystem)", 0x15c3920 },
		{ "System 26", 0x15c3a58 },
		{ "System 27 (MapSystem)", 0x15c3118 },
		{ "System 28 (PvpSystem)", 0x15c1c48 },
		{ "System 29 (MirrorSystem)", 0x15c28c8 },
		{ "System 30", 0x15c2528 },
		{ "System 31", 0x15c1d28 },
		{ "System 32", 0x15c1cb8 },
		{ "System 33", 0x15c3eb8 },
		{ "System 34", 0x15c2c18 },
		{ "System 35", 0x15c4cc0 },
		{ "System 36", 0x15c2b40 },
		{ "System 37", 0x15c1b48 },
		{ "System 38", 0x15c1ac8 },
		{ "System 39", 0x15c4d30 },
		{ "System 40", 0x15c1bb8 },
		{ "System 41", 0x15c4db8 },
		{ "System 42", 0x15c1e68 },
		{ "System 43", 0x15c2e20 },
		{ "System 44", 0x15c1d98 },
		{ "System 45", 0x15c17c8 },
		{ "System 46", 0x15c4e40 },
		{ "System 47", 0x15c1650 },
		{ "System 48", 0x15c15e0 },
		{ "System 49 (CombatSystem)", 0x15c4ec8 },
		{ "System 50", 0x15c4f68 },
		{ "System 51", 0x15c4fd8 },
		{ "System 52", 0x15c2338 },
		{ "System 53 (CameraSystem)", 0x15c26e0},
		{ "System 54", 0x15c5048 },
		{ "System 55", 0x15c16d8 }, 
		{ "System 56", 0x15c1a58 },
		{ "System 57", 0x15c5120 },
		{ "System 58", 0x15c3198 },
		{ "System 59", 0x15c5200 },
		{ "System 60", 0x15c5288 },
		{ "System 61", 0x15c52f8 },
		{ "System 62", 0x15c5368 },
		{ "System 63", 0x15c32e8 },
		{ "System 64", 0x15c43a0 },
		{ "System 65", 0x15c3d18 },
		{ "System 66", 0x15c3d88 },
		{ "System 67", 0x15c3e48 },
		{ "System 68",0x15c2ed0 },
		{ "System 69 (noice)", 0x15c3f38 },
		{ "System 70", 0x15c2428 },
		{ "System 71 (embEA 54 support)", 0x15da328 }
	};

	inline void render() override {
		if (open_window()) {
			std::vector<std::pair<std::string, EntityAdminBase*>> entity_admins;

			auto lobby_ea = *(EntityAdminBase**)(globals::gameBase + 0x1833e18);
			entity_admins.push_back({ "Lobby", lobby_ea });
			//Compo54 ist eh nur in lobby
			if (lobby_ea->component_iterator[0x54].component_list.num > 0) {
				auto comp54 = (Component_54_Lobbymap*)lobby_ea->component_iterator[0x54].component_list.ptr[0];
				if (comp54->embedded_game_ea) {
					entity_admins.push_back({ "Lobby Comp54", comp54->embedded_game_ea });
				}
			}
			entity_admins.push_back({ "Game", *(EntityAdminBase**)(globals::gameBase + 0x17b7f90) });
			auto temp = *(__int64*)(globals::gameBase + 0x17b7fc0);
			if (temp) {
				auto embed = *(EntityAdminBase**)(temp + 0xB0);
				if (embed) {
					entity_admins.push_back({ "Embedded (Replay?)", embed });
				}
			}

			std::vector<std::string> headers = { "System" };
			for (auto& ea : entity_admins) {
				headers.push_back(ea.first);
			}
			if (imgui_helpers::beginTable("Systemslelelel", headers)) {
				for (auto& system : s_systems) {
					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					ImGui::TextUnformatted(system.first.c_str());
					auto get_inheritance_fn = (__int64(__fastcall*)())(*(__int64*)(globals::gameBase + system.second));
					display_addr(get_inheritance_fn(), "I");

					for (auto& ea : entity_admins) {
						ImGui::TableNextColumn();

						for (int i = 0; i < ea.second->systems_array.num; i++) {
							__int64 sys = (__int64)ea.second->systems_array.ptr[i];
							auto sys_typ = *(__int64*)sys - globals::gameBase;
							if (sys_typ == system.second) {
								display_addr(sys);
							}
						}
					}
				}

				ImGui::EndTable();
			}
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(entityadmin_systems_window);