#pragma once
#include "../window_manager/window_manager.h"
#include "../STU_Editable.h"
#include "stu_explorer.h"

class stu_object_edit : public window {
public:
	typedef std::pair<STU_Object, STUArgumentInfo*> arg_typ;
	WINDOW_DEFINE_ARG(stu_object_edit, "STU", "ObjectEditor", arg_typ);

	inline void render() override {
		if (open_window()) {
			if (is_modal)
				modal_force_focus = true;
			if (_arg.first.valid()) {
				if (_arg.second == nullptr) {
					ImGui::Text("Please select object");
					if (ImGui::BeginListBox("")) {
						for (auto arg : *_arg.first.struct_info) {
							auto arg_obj = _arg.first.get_argument_object(arg.second);
							if (arg_obj.struct_info != nullptr) {
								ImGui::PushID(arg.second);

								if (ImGui::RadioButton("", false)) {
									_arg.second = arg.second;
								}
								ImGui::Text("[offs 0x%x]", arg.second->offset);
								ImGui::SameLine();
								imgui_helpers::display_type(arg.second->name_hash, false, true, false);
								ImGui::TextUnformatted("(");
								imgui_helpers::display_type(arg_obj.get_runtime_root().struct_info->name_hash, true, true, false);
								ImGui::TextUnformatted(")");
								
								ImGui::PopID();
							}
						}
						ImGui::EndListBox();
					}
				}
				else {
					auto arginfo = _arg.first.get_argument_object(_arg.second);
					if (arginfo.valid() && !_arg.second->constraint->is_list()) {
						if (imgui_helpers::TooltipButton(EMOJI_CROSS, "Set value to NULL")) {
							_arg.first.set_object(_arg.second, nullptr);
						}
						ImGui::SameLine();
					}
					if (imgui_helpers::TooltipButton(EMOJI_EDIT, "Set / Replace Value"))
						_replace_value = true;
					if (_replace_value) {
						if (ImGui::RadioButton("Base ", false))
							emplace_object(STU_Object::create(arginfo.struct_info));
						ImGui::SameLine();
						imgui_helpers::display_type(_arg.second->constraint->get_stu_type(), true, true, false);
						if (ImGui::BeginChild("", ImVec2(-10, 250), ImGuiChildFlags_AutoResizeX)) {
							display_object_recursive(arginfo.struct_info);
						}
						ImGui::EndChild();
					}
				}
			}
			else {
				ImGui::Text("Invalid STU Object");
			}
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}

private:
	bool _replace_value{};

	void emplace_object(STU_Object obj) {
		if (_arg.second->constraint->is_list()) {
			auto list = _arg.first.get_argument_objectlist(_arg.second);
			if (!list.valid()) {
				imgui_helpers::messageBox("List is InlinedObject invalid!");
			}
			else {
				list.push_back_object(obj);
			}
		}
		else {
			_arg.first.set_object(_arg.second, obj);
		}
	}

	void display_object_recursive(STUInfo* type) {
		auto typ = type->child;
		ImGui::Indent();
		while (typ) {
			ImGui::PushID(typ);
			if (ImGui::RadioButton("", false)) {
				emplace_object(STU_Object::create(typ));
			}
			ImGui::SameLine();
			imgui_helpers::display_type(typ->name_hash, true, true, false);
			if (typ->child) {
				display_object_recursive(typ);
			}
			typ = typ->sibling;

			ImGui::PopID();
		}
		ImGui::Unindent();
	}
};

WINDOW_REGISTER(stu_object_edit);
