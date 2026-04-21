#pragma once
#include "../window_manager/window_manager.h"
#include "entity_admin.h"
#include "Components/Component_2_AssetManager.h"

class teScene_window : public window {
	WINDOW_DEFINE_ARG(teScene_window, "Game", "teScene explorer", EntityAdminBase*);

	inline void render() override {
		if (open_window()) {
			display_addr((__int64)_arg, "EA");
			ImGui::SameLine();
			ImGui::Text("%s", _arg->vfptr->GetIsLiveReplayOrDeathStar(_arg));
			auto comp_2 = (*_arg->system_1_sceneSystem)->Get_teScene_Comp2(_arg->system_1_sceneSystem);
			if (!comp_2)
			{
				ImGui::Text("Invalid comp2");
				return;
			}
			display_addr((__int64)comp_2, "Component2");
			display_addr((__int64)comp_2->scene, "teScene");
			if (comp_2->scene)
			{
				ImGui::Text("Map load state: %x", comp_2->scene->load_map_state);
				display_addr((__int64)comp_2->scene->resource_loader, "Resource Loader");
			}
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(teScene_window);