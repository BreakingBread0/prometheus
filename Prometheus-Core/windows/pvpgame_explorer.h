#pragma once
#include "../window_manager/window_manager.h"
#include "../entity_admin.h"
#include "entity_window.h"

class pvpgame_explorer : public window {
	WINDOW_DEFINE_ARG(pvpgame_explorer, "Game", "PvPGame Explorer", Component_24_PvPGame*);

	inline void render() override {
		if (open_window(nullptr, 0, ImVec2(400, 600))) {
			display_addr((__int64)_arg, "Address");
			if (IsBadReadPtr(_arg, sizeof(_arg))) {
				ImGui::Text("Invalid Component");
				return;
			}
			if (ImGui::Button(EMOJI_SHARE)) {
				entity_window::get_latest_or_create(this)->nav_to_ent(_arg->base.entity_backref);
				//statescript_list::get_latest_or_create(this)->display_compo(_pvp->base.entity_backref->getById<StatescriptComponent>(0x23));
			}

			STRUCT_MODIFIABLE(_arg, current_gamestate_d8);
			STRUCT_MODIFIABLE(_arg, overtime_remaining);
			STRUCT_MODIFIABLE(_arg, field_A4);
			STRUCT_MODIFIABLE(_arg, field_A8);
			STRUCT_MODIFIABLE(_arg, field_AC);
			STRUCT_MODIFIABLE(_arg, some_team_id);
			STRUCT_MODIFIABLE(_arg, field_B4);
			STRUCT_MODIFIABLE(_arg, current_round);
			STRUCT_MODIFIABLE(_arg, counter_decreasing);
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(pvpgame_explorer);