#pragma once
#include "../window_manager/window_manager.h"
#include "../Viewmodel.h"
#include "string_rep_edit.h"
#include "viewmodel_write_blocker.h"

class viewmodel_viewer : public window {
	WINDOW_DEFINE(viewmodel_viewer, "Viewmodel", "Viewmodel Viewer", true);

	ViewModel* selected_vm = nullptr; 

	inline void render() override {
		if (open_window()) {
			if (!selected_vm || IsBadReadPtr(selected_vm, sizeof(ViewModel))) {
				ImGui::Text("Invalid Viewmodel");
			}
			else {
				display_addr((__int64)selected_vm, "Viewmodel");
				ImGui::SameLine();
				display_addr((__int64)selected_vm->this_or_parent, "This or Parent");
				if (imgui_helpers::beginTable("properties", { "Addr", "ID", "Type", "Value" })) {
					for (int i = 0; i < selected_vm->properties.num; i++) {
						auto* prop = &selected_vm->properties.ptr[i];
						ImGui::PushID(prop);
						ImGui::TableNextRow();

						ImGui::TableNextColumn();
						display_addr((__int64)prop);

						ImGui::TableNextColumn();
						imgui_helpers::display_type(prop->key, true, true, false);
						ImGui::SameLine();
						if (imgui_helpers::TooltipButton(EMOJI_DENY, "Block")) {
							viewmodel_write_blocker::get_latest_or_create(this)->add_block(selected_vm, prop->key);
						}

						ImGui::TableNextColumn();
						ImGui::TextUnformatted(prop->property.type_str().c_str());

						ImGui::TableNextColumn();
						if (imgui_helpers::TooltipButton(EMOJI_REFRESH, "Send Prop Update Notification")) {
							SendViewModelPropUpdate(selected_vm, prop);
						}
						ImGui::SameLine();
						ImGui::Text("%x", prop->property.has_value);
						ImGui::SameLine();
						if (prop->property.has_value) {
							if (ImGui::Button(EMOJI_CROSS)) {
								prop->property.has_value--;
							}
						}
						else if (ImGui::Button(EMOJI_CHECK)) {
							prop->property.has_value++;
						}
						ImGui::SameLine();
						display_addr(prop->property.value);

						if (prop->property.type == ViewModelPropertyType_STRING_REP) {
							string_rep* rep = (string_rep*)prop->property.value;
							ImGui::SameLine();
							if (imgui_helpers::TooltipButton(EMOJI_EDIT, "Edit String")) {
								window_manager::add_window(new string_rep_edit(rep), this);
							}
							if (rep) {
								ImGui::SameLine();
								ImGui::Text("str: %s", rep->get());
							}
						}
						else if (prop->property.type == ViewModelPropertyType_VIEWMODEL) {
							ViewModel* vm_child = (ViewModel*)prop->property.value;
							char buf[64];
							sprintf_s(buf, "[%x] %s", vm_child->id, vm_child->vfptr->GetName());
							ImGui::SameLine();
							ImGui::TextUnformatted(buf);
						}
						else if (prop->property.type == ViewModelPropertyType_LONG) {
							ImGui::SameLine();
							imgui_helpers::display_type(prop->property.value, true, true, false);
						}
						else if (prop->property.type == ViewModelPropertyType_FLT) {
							ImGui::SameLine();
							ImGui::Text("%f", prop->property.value);
						}
						else if (prop->property.type == ViewModelPropertyType_CHAR) {
							ImGui::SameLine();
							if (prop->property.value) {
								if (ImGui::Button(EMOJI_CROSS "##2")) {
									prop->property.value--;
								}
							}
							else if (ImGui::Button(EMOJI_CHECK "##2")) {
								prop->property.value++;
							}
						}
						ImGui::PopID();
					}

					ImGui::EndTable();
				}
			}
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(viewmodel_viewer);
