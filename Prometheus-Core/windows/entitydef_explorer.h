//#pragma once
//#include "../window_manager/window_manager.h"
//#include "stu_explorer.h"
//
//class entitydef_explorer : public window {
//	WINDOW_DEFINE(entitydef_explorer, "Resource", "Entity Def Explorer", true);
//
//	STUEntityDefinition* stu_entity;
//	inline void render() override {
//		if (open_window()) {
//			if (IsBadReadPtr(stu_entity, sizeof(STUEntityDefinition))) {
//				ImGui::Text("Invalid Entity Def");
//			}
//			else {
//				if (imgui_helpers::beginTable("compos", { "Hash", "Component" })) {
//					for (int i = 0; i < stu_entity->m_componentMap.count(); i++) {
//						auto& item = stu_entity->m_componentMap.list()[i];
//						ImGui::PushID(&item);
//						ImGui::TableNextRow();
//
//						ImGui::TableNextColumn();
//						imgui_helpers::display_type(item.component_hash, true, true, false);
//
//						ImGui::TableNextColumn();
//						display_addr((__int64)item.STUEntityComponent_data);
//						ImGui::SameLine();
//						if (imgui_helpers::TooltipButton(EMOJI_FORWARD, "Show in STU explorer")) {
//							stu_explorer::get_latest_or_create(this)->navigate_to(item.STUEntityComponent_data->vfptr->GetSTUInfo(), (__int64)item.STUEntityComponent_data, nullptr);
//						}
//						ImGui::PopID();
//					}
//					ImGui::EndTable();
//				}
//			}
//		}
//		ImGui::End();
//	}
//
//	//inline void preStartInitialize() override {}
//	//inline void initialize() override {}
//};
//
//WINDOW_REGISTER(entitydef_explorer);