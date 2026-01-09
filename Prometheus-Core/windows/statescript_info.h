#pragma once
#include "../window_manager/window_manager.h"
#include "statescript_window.h"

class statescript_info : public window {
	WINDOW_DEFINE(statescript_info, "Statescript", "Statescript Info", false);

	inline void render() override {
		auto ss_wind = get_creator_as<statescript_window>();
		if (ss_wind->get_instance_invalid())
			return;
		auto ss = ss_wind->get_curr_instance();
		if (open_window(nullptr, 0, ImVec2(300, 800))) {
			display_addr((__int64)ss, "StatescriptInstance");

			ImGui::Separator();
			auto inner = ss->ss_inner;
			display_addr((__int64)inner->component_ref, "Component");
			display_addr((__int64)inner, "SyncMgr");
			display_addr((uint32)inner->component_ref->base.entity_backref->entity_id, "EntID");
			ImGui::NewLine();
			
			ImGui::Text("CF: %d %lld %lld", inner->increasing_counter1, inner->cf_timestamp, inner->increasing_counter3);
			ImGui::NewLine();
			
			ImGui::Checkbox("needs_process_e (if 0)", &inner->needs_e_process_0);
			ImGui::Checkbox("needs_process_e", &inner->needs_process_e);
			ImGui::Checkbox("field_358", &inner->field_358);
			ImGui::Checkbox("field_35A", &inner->field_35A);
			display_addr(inner->field_360, "field_360");
			display_addr(inner->ontick_updates_enabled, "tick_updates_enabled");
			display_addr(inner->field_36C, "field_36C");
			display_addr(inner->field_370, "field_370");
			ImGui::NewLine();

			ImGui::Checkbox("flag_374", &inner->flag_374);
			ImGui::Checkbox("is_instantiating", &inner->is_instantiating);
			ImGui::Checkbox("has_clientonly_scripts", &inner->has_clientonly_scripts);
			ImGui::Checkbox("latest_logbtn_down", &inner->latest_logbtn_down);
			ImGui::Text("latest_logbtn: %d (%x): %s", inner->latest_logbtn, inner->latest_logbtn, LogicalButtonById(inner->latest_logbtn)->name);
			ImGui::Text("LogicalBtns (%d):", inner->logicalBtn_list.num);
			for (auto btn : inner->logicalBtn_list) {
				ImGui::BulletText("%d (%s)", btn, LogicalButtonById(btn)->name);
			}
			ImGui::NewLine();

			display_addr(inner->field_3C8, "field_3c8");
			display_addr(inner->field_3CC, "field_3cc");
			ImGui::Text("Entity Admin Scope:");
			ImGui::SameLine();
			switch (inner->entity_admin_scope) {
			case 0:
				ImGui::Text("OWNER_ENTADM");
				break;
			case 1:
				ImGui::Text("GAME_EA");
				break;
			case 2:
				ImGui::Text("LOBBY_EA");
				break;
			case 3:
				ImGui::Text("EMBED_EA");
				break;
			default:
				ImGui::Text("LOBBYMAP_EA");
				break;
			}
			ImGui::Checkbox("field_444", &inner->field_444);
			display_addr(inner->has_player_entbackref, "has_entbackref");
			ImGui::Checkbox("field_4D0", &inner->field_4D0);
			ImGui::Checkbox("field_4D1", &inner->field_4D1);
			ImGui::Checkbox("is_aim_ent", &inner->is_aim_ent);
			ImGui::Checkbox("is_aim_ent_assigned", &inner->is_aim_ent_assigned);
			
			ImGui::Text("CVD Callbacks (%d):", inner->cvd_get_callbacks.num);
			for (auto callback : inner->cvd_get_callbacks) {
				ImGui::PushID(callback);
				display_addr((__int64)callback);
				ImGui::SameLine();
				ImGui::TextUnformatted("->");
				ImGui::SameLine();
				display_addr(*callback);
				ImGui::PopID();
			}
		}
		ImGui::End();
	}

	statescript_info() {
		is_dependent = true;
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(statescript_info);
