#pragma once
#include "../window_manager/window_manager.h"
#include "Systems/Preexisting/System_29_MirrorSystem.h"

class mirror_system_explorer : public window {
	WINDOW_DEFINE(mirror_system_explorer, "ECS", "Mirror System Explorer", true);

	void render_callbacks(System_29_MirrorSystem::ComponentUpdateCallbackHashmap& hashmap)
	{
		ImGui::Indent();
		for (auto item : hashmap)
		{
			ImGui::Text("Component: %x", item.first);
			ImGui::PushID(item.first);
			for (auto callback : *item.second)
			{
				ImGui::Indent();
				ImGui::PushID(&callback);

				ImGui::Bullet();
				ImGui::SameLine();
				display_addr((__int64)callback.function, "Func");
				ImGui::SameLine();
				display_addr((__int64)callback.user_data, "Data");

				ImGui::PopID();
				ImGui::Unindent();
			}
			ImGui::PopID();
		}
		ImGui::Unindent();
	}

	inline void render() override {
		if (open_window()) {
			auto ea = GameEntityAdmin();
			auto mirror_sys = System_29_MirrorSystem::Get(ea);

			display_addr((__int64)mirror_sys, "Mirror System");
			ImGui::Text("PreDeserialize:");
			ImGui::PushID("pre");
			render_callbacks(mirror_sys->pre_deserialize_callback);
			ImGui::PopID();

			ImGui::Text("PostDeserialize:");
			ImGui::PushID("post");
			render_callbacks(mirror_sys->post_deserialize_callback);
			ImGui::PopID();


			ImGui::Text("Ent Creation Callbacks:");
			for (auto list : mirror_sys->entity_creation_callbacks)
			{
				ImGui::PushID(&list);
				ImGui::Bullet();
				display_addr((__int64)list.Function, "function");
				display_addr(list.user_data, "User Data");

				ImGui::PopID();
			}

			ImGui::Text("Outstanding Placeables");
			bool enable_outlines = ImGui::Button("Highlight");
			for (auto item : mirror_sys->outstanding_server_placeables)
			{
				if (enable_outlines)
				{
					auto comp_2 = (*ea->system_1_sceneSystem)->Get_teScene_Comp2(ea->system_1_sceneSystem);
					auto resloader = comp_2->scene->resource_loader;
					if (item >= resloader->placeables_num)
						LOG_CORE(Warn, "Placeable {:d}: Out of bounds", item);
					else
					{
						auto placeable = &resloader->placeables[item];
						if (!placeable->placeable_resource)
							LOG_CORE(Warn, "Placeable {:d}: Has no resload entry", item);
						ea->getByUUID(placeable->placeable_resource->uuid)->Debug_Highlight(0x00FF00FF);
					}
				}
				ImGui::BulletText("0x%x", item);
			}
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(mirror_system_explorer);