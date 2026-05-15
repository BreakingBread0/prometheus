#pragma once
#include "../window_manager/window_manager.h"
#include "entity_admin.h"

class entityadmin_timing : public window {
	WINDOW_DEFINE(entityadmin_timing, "ECS", "GameEntityAdmin Timing", true);

	inline void render() override {
		if (open_window()) {
			auto ea = GameEntityAdmin();

			ImGui::Text("Timestamp CF (EA): %d", ea->timestamp_cf);

			auto local_ent = ea->getLocalEnt();
			if (local_ent)
			{
				ImGui::Text("Local Ent: %s", local_ent->toString().c_str());
				auto fn = (double(*)(Entity*, EntityAdminBase*))(globals::gameBase + 0x103eb50);
				double timing_frame = fn(local_ent, ea);
				ImGui::Text("Timing Frame: %lf", timing_frame);
			}

			auto timing = ea->GameEA_timing;

			display_addr((__int64)timing, "Timing Addr");
			if (!timing) {
				ImGui::Text("Timing Invalid");
				return;
			}

			ImGui::NewLine();
			STRUCT_MODIFIABLE(timing, cf_timer_1.command_frame_length_secs);
			STRUCT_MODIFIABLE(timing, cf_timer_1.timer_secs_double);
			STRUCT_MODIFIABLE(timing, cf_timer_1.timer_secs);
			STRUCT_MODIFIABLE(timing, cf_timer_2.command_frame_length_secs);
			STRUCT_MODIFIABLE(timing, cf_timer_2.timer_secs_double);
			STRUCT_MODIFIABLE(timing, cf_timer_2.timer_secs);

			ImGui::NewLine();
			STRUCT_MODIFIABLE(timing, tick_count);
			STRUCT_MODIFIABLE(timing, field_3C);
			STRUCT_MODIFIABLE(timing, field_40);
			STRUCT_MODIFIABLE(timing, field_48);
			STRUCT_MODIFIABLE(timing, field_50);
			STRUCT_MODIFIABLE(timing, field_58);
			STRUCT_MODIFIABLE(timing, field_60);
			STRUCT_MODIFIABLE(timing, list_70.num);

			ImGui::NewLine();
			STRUCT_MODIFIABLE(timing, field_D0);
			STRUCT_MODIFIABLE(timing, field_D4);
			STRUCT_MODIFIABLE(timing, field_D8);
			STRUCT_MODIFIABLE(timing, field_DC);
			STRUCT_MODIFIABLE(timing, field_E0);

			ImGui::NewLine();
			STRUCT_MODIFIABLE(timing, field_E4);
			STRUCT_MODIFIABLE(timing, field_E8);
			STRUCT_MODIFIABLE(timing, field_EC);

			ImGui::NewLine();
			STRUCT_MODIFIABLE(timing, field_F0);
			STRUCT_MODIFIABLE(timing, field_F4);
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(entityadmin_timing);
