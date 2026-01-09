#pragma once
#include "../window_manager/window_manager.h"
#include "../entity_admin.h"
#include "../stringhash_library.h"

class entadmin_compodependencies : public window {
	WINDOW_DEFINE(entadmin_compodependencies, "ECS", "Component Dependencies", true);

	inline void render() override {
		if (open_window()) {
			auto creation_info = GameEntityAdmin()->creation_info;
			display_addr((__int64)creation_info, "EA Creation Info");
			ImGui::Text("Max: %d (0x%x)", creation_info->MAX_COMPONENT_ID, creation_info->MAX_COMPONENT_ID);
			if (imgui_helpers::beginTable("Dependencies", { "Component", "Dependencies" })) {
				for (int i = 0; i < creation_info->MAX_COMPONENT_ID; i++) {
					ImGui::TableNextRow();
					
					ImGui::TableNextColumn();
					ImGui::Text("%x", i);
					
					ImGui::TableNextColumn();
					auto& dependencies = creation_info->component_dependencies[i];
					for (int j = 0; j < dependencies.num; j++) {
						stringhash_library::display_component(dependencies.ptr[j]);
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

WINDOW_REGISTER(entadmin_compodependencies);
