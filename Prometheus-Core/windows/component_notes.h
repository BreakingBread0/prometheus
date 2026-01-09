#pragma once
#include "../window_manager/window_manager.h"
#include "../stringhash_library.h"
#include "textedit_window.h"
#include "../search_helper.h"

class component_notes : public window {
	WINDOW_DEFINE(component_notes, "Tools", "Component Notes", true);

	search_helper_imgui search;

	inline void render() override {
		if (open_window(nullptr, 0, ImVec2(1500, 550))) {
			search.search_box("Search Components");

			imgui_helpers::beginTable("components", {"Component", "Name", "Comment"});

			for (auto& component : stringhash_library::components) {
				ImGui::PushID(component.first);
				if (search.needs_haystack()) {
					search.haystack_hex(component.first);
					search.haystack(component.second.name);
					search.haystack(component.second.comment);
				}
				bool is_bg = !search.found_needle(component.first);
				if (is_bg)
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(80, 80, 80, 255));
				
				ImGui::TableNextRow();
				int compid = component.first;

				ImGui::TableNextColumn();
				ImGui::Text("%x", compid);

				ImGui::TableNextColumn();
				ImGui::PushID("second");
				if (ImGui::Button(EMOJI_EDIT)) {
					window_manager::add_window(new textedit_window(component.second.name, [compid](std::string new_value){
						stringhash_library::components[compid].name = new_value;
						stringhash_library::save_all();
					}), this);
				}
				ImGui::PopID();
				ImGui::SameLine();
				ImGui::TextUnformatted(component.second.name.c_str());
				
				ImGui::TableNextColumn();
				if (ImGui::Button(EMOJI_EDIT)) {
					window_manager::add_window(new textedit_window(component.second.comment, [compid](std::string new_value) {
						stringhash_library::components[compid].comment = new_value;
						stringhash_library::save_all();
					}), this);
				}
				ImGui::SameLine();
				ImGui::TextWrapped(component.second.comment.c_str());
				if (is_bg)
					ImGui::PopStyleColor();
				ImGui::PopID();
			}

			ImGui::EndTable();
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(component_notes);
