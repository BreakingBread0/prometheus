#pragma once
#include "statescript_list.h"
#include "statescript_window.h"
#include "statescript_node_search.h"
#include "statescript_fake_logicalbtn.h"
#include "CVD_VarbagSelector.h"

void statescript_list::render() {
	char buf[64];
	if (IsBadReadPtr(_ss, sizeof(Component_23_Statescript))) {
		strcpy_s(buf, "Invalid Component");
	}
	else {
		sprintf_s(buf, "Entity %x (%d)", _ss->base.entity_backref->entity_id, _ss->ss_inner.g1_instanceArr.num);
	}
	//ImGui::SetNextWindowSizeConstraints(ImVec2(250, 600), ImVec2(0, 0));
	if (open_window(buf, ImGuiWindowFlags_MenuBar, ImVec2(450, 600))) {
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("Tools")) {
				if (ImGui::MenuItem("Search for Nodes")) {
					statescript_node_search::get_latest_or_create(this)->set(_ss);
				}
				if (ImGui::MenuItem("Fake LogicalButton")) {
					auto size = ImGui::GetWindowSize();
					size.x /= 0.7f;
					ImGui::SetWindowSize(size);
					dock_item_right(window_manager::get_latest_or_create<statescript_fake_logicalbtn>(this, true, true).get(), 0.7);
				}
				if (ImGui::MenuItem("ConfigVarDynamic Editor")) {
					CVD_VarbagSelector::get_latest_or_create(this)->set(&_ss->ss_inner);
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		if (ImGui::BeginListBox("", ImVec2(-10, -30))) {
			for (auto script : _ss->ss_inner.g1_instanceArr) {
				if (!script->parent_instance_id) {
					list_script(script);
				}
			}
			ImGui::EndListBox();
				
			imgui_helpers::InputHex("", &_load_script);
			ImGui::SameLine();
			if (ImGui::Button("Load Script")) {
				_ss->vfptr->load_statescript_script(_ss, _load_script, 0, 0);
			}
		}
	}
	ImGui::End();
}

void statescript_list::display_compo(Component_23_Statescript* ss) {
	_ss = ss;
	_curr_inst_id = 0;
}

void statescript_list::list_script(StatescriptInstance* script) {
	ImGui::PushID(script);
	if (ImGui::Button(EMOJI_CROSS)) {
		script->vfptr->StopScript(script);
	}
	ImGui::SameLine();
	if (ImGui::RadioButton(std::format("({:d}) {:x}", script->instance_id, script->script_id).c_str(), _curr_inst_id == script->instance_id)) {
		_curr_inst_id = script->instance_id;
		statescript_window::get_latest_or_create(this)->display_instance(script);
	}
	imgui_helpers::display_type(script->script_id, true, true, false);
	if (!script->graph) {
		ImGui::Text("No Graph!");
	}
	ImGui::Separator();
	for (auto& other_script : _ss->ss_inner.g1_instanceArr) {
		if (other_script != script && other_script->parent_instance_id == script->instance_id) {
			ImGui::Indent();
			list_script(other_script);
			ImGui::Unindent();
		}
	}
	ImGui::PopID();
}
