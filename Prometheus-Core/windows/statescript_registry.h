#pragma once
#include "../window_manager/window_manager.h"
#include "../Statescript.h"

class statescript_registry : public window {
	WINDOW_DEFINE(statescript_registry, "Statescript", "Statescript Registry", true);
	StatescriptRTTI* selected = nullptr;

	inline void render() override {
		static int count = 0;
		if (open_window()) {
			StatescriptRTTI* rtti = *(StatescriptRTTI**)(globals::gameBase + 0x18f8e88);
			STURegistry* header = *(STURegistry**)(globals::gameBase + 0x18f74e0);
			ImGui::Text("Size: %d", count);

			count = 0;

			const auto curStyle = ImGui::GetStyle();
			imgui_helpers::beginTable("SSRTTI", { "STUInfo->Hash" ,"field_8", "*field_8", "*(field_8+8)", "factory_func", "inherit", "field_20", "field_28", "*field_28"},
				ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersH | ImGuiTableFlags_HighlightHoveredColumn | ImGuiTableFlags_RowBg);

			while (rtti) {
				ImGui::TableNextRow();
				ImGui::PushID(rtti);
				count++;
				auto temp = header;
				ImGui::TableNextColumn();

				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, curStyle.Colors[ImGuiCol_Header]);
				ImGui::PushStyleColor(ImGuiCol_HeaderActive, curStyle.Colors[ImGuiCol_Header]);
				if (ImGui::Selectable("", selected == rtti, ImGuiSelectableFlags_NoAutoClosePopups | ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap, ImVec2(0, 20.0f))) {
					selected = rtti;
				}
				ImGui::PopStyleColor(2);

				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(6.0f, 6.0f));

				while (temp != nullptr) {
					if (temp->info->rtti_info == rtti->stuclass_inherit) {
						ImGui::SameLine();
						allmighty_hash_lib::display_hash(temp->info->name_hash);
						break;
					}
					temp = temp->next;
				}

				ImGui::TableNextColumn();
				if (rtti->field_8 - (__int64)rtti != 0x10) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 0, 0, 255));
					ImGui::Text("%x", rtti->field_8 - (__int64)rtti);
					ImGui::PopStyleColor();
				}
				else {
					ImGui::Text("ok");
				}

				ImGui::TableNextColumn();
				display_addr(*(__int64*)rtti->field_8);

				ImGui::TableNextColumn();
				display_addr(*(__int64*)(rtti->field_8 + 8));

				ImGui::TableNextColumn();
				display_addr((__int64)rtti->factory_func);

				ImGui::TableNextColumn();
				display_addr((__int64)rtti->stuclass_inherit);

				ImGui::TableNextColumn();
				display_addr(rtti->field_20);

				ImGui::TableNextColumn();
				display_addr(rtti->field_28);

				ImGui::TableNextColumn();
				display_addr(*(int*)(rtti->field_28 + 4));

				ImGui::PopID();
				ImGui::PopStyleVar();
				rtti = rtti->next_instance;
			}

			ImGui::EndTable();
		}
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(statescript_registry);
