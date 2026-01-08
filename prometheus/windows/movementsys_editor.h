#pragma once
#include "../window_manager/window_manager.h"
#include "../entity_admin.h"
#include <vector>

class movementsys_editor : public window {
	WINDOW_DEFINE_2(movementsys_editor, "Game", "Movementsys Editor", true, true);

	void do_modification() {
		auto ea = GameEntityAdmin();
		ea->movement_systems.clear();
		for (int i = 0; i < _orig_cnt; i++) {
			for (auto& movsys : s_mov_vtables) {
				if ((__int64)(*_orig_list[i]) == globals::gameBase + movsys.vfptr && movsys.enabled) {
					ea->movement_systems.emplace_item(_orig_list[i]);
				}
			}
		}
	}

	inline void render() override {
		if (open_window()) {
			for (auto& item : s_mov_vtables) {
				ImGui::PushID(item.vfptr);

				char buf[64];
				sprintf_s(buf, "%s (%p)", item.name.c_str(), item.vfptr);
				if (ImGui::Checkbox(buf, &item.enabled)) {
					do_modification();
				}

				ImGui::PopID();
			}
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	inline void initialize() override {
		auto ea = GameEntityAdmin();
		_orig_list = new movement_vt**[_orig_cnt = ea->movement_systems.num];
		memcpy(_orig_list, ea->movement_systems.begin(), sizeof(movement_vt**) * ea->movement_systems.num);
	}
private:
	struct movsys_state {
		__int64 vfptr;
		std::string name;
		bool enabled;
	};
	static inline std::vector<movsys_state> s_mov_vtables = {
		{ 0x15c17b0, "stuprojectile", true },
		{ 0x15c1c30, "charmover", true },
		{ 0x15c1ed8, "unsynchronized_mover", true },
		{ 0x15c1ef8, "stuweapon", true },
		{ 0x15c2d90, "statescript", true },
		{ 0x15c4da0, "simplemover", true },
		{ 0x15c4e28, "localplayer", true }
	};
	movement_vt*** _orig_list;
	int _orig_cnt;
};

WINDOW_REGISTER(movementsys_editor);
