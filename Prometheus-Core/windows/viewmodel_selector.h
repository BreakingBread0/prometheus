#pragma once
#include "../window_manager/window_manager.h"
#include "../Viewmodel.h"
#include "viewmodel_viewer.h"

class viewmodel_selector : public window {
	WINDOW_DEFINE(viewmodel_selector, "Viewmodel", "Viewmodel Selector", true);

	void render_vm(ViewModel* vm/*, std::set<ViewModel*> already_done*/) {
		ImGui::PushID(vm);
		/*if (already_done.find(vm) != already_done.end())
			return;
		already_done.emplace(vm);*/
		char buf[64];
		sprintf_s(buf, "[%x] %s [%p]", vm->id, vm->vfptr->GetName());
		if (ImGui::RadioButton(buf, selected_vm == vm)) {
			selected_vm = viewmodel_viewer::get_latest_or_create(this)->selected_vm = vm;
		}
		for (int i = 0; i < vm->properties.num; i++) {
			auto& prop = vm->properties.ptr[i];
			if (prop.property.type == ViewModelPropertyType_VIEWMODEL) {
				ViewModel* vm_inner = (ViewModel*)prop.property.value;
				ImGui::Indent();
				imgui_helpers::display_type(prop.key, true, true, false);
				render_vm(vm_inner/*, already_done*/);
				ImGui::Unindent();
			}
			else if (prop.property.type == ViewModelPropertyType_ARRAY) {
				ImGui::Indent();
				ViewModelArray* arr = (ViewModelArray*)prop.property.value;
				imgui_helpers::display_type(prop.key, true, true, false);
				ImGui::SameLine();
				ImGui::Text("(%d/%d)", arr->viewmodel_list.num, arr->viewmodel_list.max);
				for (int j = 0; j < arr->viewmodel_list.num; j++) {
					ViewModel* vm_inner = arr->viewmodel_list.ptr[j];
					ImGui::Text("%d:", j);
					ImGui::SameLine();
					render_vm(vm_inner/*, already_done*/);
				}

				ImGui::Unindent();
			}
		}
		ImGui::PopID();
	}

	ViewModel* selected_vm = nullptr;
	inline void render() override {
		if (open_window()) {
			ViewModelRoot* root = GetViewModelRoot();
			if (ImGui::BeginListBox("##Viewmodels", ImVec2(-10, -10))) {
				for (int i = 0; i < root->viewmodel_list.num; i++) {
					ViewModel* vm = root->viewmodel_list.ptr[i];
					if (vm == vm->this_or_parent) {
						render_vm(vm);
					}
				}
				ImGui::EndListBox();
			}
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(viewmodel_selector);
