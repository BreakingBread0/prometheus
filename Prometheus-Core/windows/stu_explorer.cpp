#include "stu_explorer.h"
#include "../STU_Editable.h"
#include "../stu_resources.h"

void stu_explorer::navigate_to(STUInfo* info, __int64 instance, StatescriptInstance* ss_inst) {
	_forward_history.clear();
	_history.push_back(_current_item);
	_current_item.cls = info;
	_current_item.current_instance = instance;
	_current_item.ss = ss_inst;
}

inline void stu_explorer::render() {
	if (open_window(nullptr, 0, ImVec2(1100, 500))) {
		auto history_disabled = _history.size() == 1;
		if (history_disabled)
			ImGui::BeginDisabled();
		if (ImGui::Button(EMOJI_BACK)) {
			auto& item = _history.back();
			_forward_history.push_back(_current_item);
			_current_item = item;
			_history.pop_back();
		}
		if (history_disabled)
			ImGui::EndDisabled();

		ImGui::SameLine();
		bool forward_history_disabled = _forward_history.size() == 0;
		if (forward_history_disabled)
			ImGui::BeginDisabled();
		if (ImGui::Button(EMOJI_FORWARD)) {
			auto& item = _forward_history.back();
			_history.push_back(_current_item);
			_current_item = item;
			_forward_history.pop_back();
		}
		if (forward_history_disabled)
			ImGui::EndDisabled();

		if (_history.size() > 0 && _history.back().followed != 0) {
			for (auto it = _history.rbegin(); it != _history.rend(); it++) {
				if (it->followed == 0) {
					while (it-- != _history.rbegin()) {
						imgui_helpers::display_type(it->followed, false, false, false);
						if (it->followed_index != -1) {
							ImGui::SameLine();
							ImGui::Text("(index %d)", it->followed_index);
						}
						ImGui::SameLine();
						ImGui::TextDisabled(">>");
						ImGui::SameLine();
					}
					break;
				}
			}
			ImGui::NewLine();
		}

		if (_current_item.current_instance) {
			display_addr(_current_item.current_instance, "Current Instance");
			if (IsBadReadPtr((void*)_current_item.current_instance, 1)) {
				ImGui::SameLine();
				ImGui::TextUnformatted("Invalid");
			}
		}

		if (_current_item.cls) {

			auto instance = _current_item.cls;
			STU_Object obj(instance, IsBadReadPtr((void*)_current_item.current_instance, _current_item.cls->instance_size) ? nullptr : (void*)_current_item.current_instance);
			instance = (obj = obj.get_runtime_root()).struct_info;
			int children = 0;
			auto child = _current_item.cls->child;
			while (child) {
				children++;
				child = child->sibling;
			}
			ImGui::Text("Arguments (top): %d - Size: %x - Children %d - Hash: ", _current_item.cls->argument_count, _current_item.cls->instance_size, children);
			ImGui::SameLine();
			while (instance) {
				//__try {
				imgui_helpers::display_type(instance->name_hash, true, true, true);
				if (render_table()) {
					render_stu(obj);
					ImGui::EndTable();
				}

				if (instance->base_stu) {
					ImGui::Text("Next:");
					ImGui::SameLine();
				}
				/*}
				__except (EXCEPTION_EXECUTE_HANDLER) {
					ImGui::Text("Exception while rendering");
				}*/
				obj = STU_Object(instance = instance->base_stu, obj.value);
			}
		}
	}
	ImGui::End();
}

void stu_explorer::render_resref(STUResourceReference* ref) {
	if (ref->has_resource()) {
		imgui_helpers::display_type(ref->resource_id, true, true, true);
		ImGui::PushID("ref_value");
		if (ref->is_resource_loaded())
			display_addr(ref->resource_load_entry->align()->resource_ptr);
		auto stu = stu_resources::GetByID(ref->resource_id);
		if (stu && ImGui::Button("follow")) {
			navigate_to(stu->to_editable().struct_info, (__int64)stu, nullptr);
		}
		ImGui::PopID();
	}
	else {
		ImGui::Text("null");
	}
}

void stu_explorer::render_stu(STU_Object value) {
	for (int i = 0; i < value.struct_info->argument_count; i++) {
		ImGui::PushID(value.struct_info);
		auto arg = &value.struct_info->arguments[i];
		auto type = arg->constraint->get_type_flag();
		auto arg_type_hash = arg->constraint->get_stu_type();
		ImGui::PushID(arg->name_hash);
		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		imgui_helpers::display_type(arg->name_hash, false, true, false);

		ImGui::TableNextColumn();
		bool* expand_list = ImGui::GetStateStorage()->GetBoolRef(arg->name_hash, false);
		if (value.valid()) {
			if (arg->constraint->is_list()) {
				auto list = (STUBullshitListFull<__int64>*)((__int64)value.value + arg->offset);
				if (list->valid()) {
					if (imgui_helpers::TooltipButton(EMOJI_FORWARD, "Follow without address")) {
						navigate_to(GetSTUInfoByHash(arg_type_hash), 0, 0);
						auto& old = _history.back();
						old.followed = arg->name_hash;
					}
					ImGui::SameLine();
					ImGui::Text("Count: %d", list->count());
					ImGui::SameLine();
					ImGui::Checkbox("Show", expand_list);
				}
				else {
					ImGui::Text("Invalid");
				}
			}
			else if (type == STU_ConstraintType_Map) {
				auto list = value.get_argument_map(arg);
				if (list.valid()) {
					if (imgui_helpers::TooltipButton(EMOJI_FORWARD, "Follow without address")) {
						navigate_to(GetSTUInfoByHash(arg_type_hash), 0, 0);
						auto& old = _history.back();
						old.followed = arg->name_hash;
					}
					ImGui::SameLine();
					ImGui::Text("Count: %d", list.count());
					ImGui::SameLine();
					ImGui::Checkbox("Show", expand_list);
				}
				else {
					ImGui::Text("Invalid");
				}
			}
			else {
				switch (type) {
				case STU_ConstraintType_Primitive: {
					auto primitive = value.get_argument_primitive(arg);
					imgui_helpers::render_primitive(primitive, arg_type_hash);
					break;
				}
				case STU_ConstraintType_Object:
				case STU_ConstraintType_InlinedObject: {
					auto object = value.get_argument_object(arg);
					if (imgui_helpers::TooltipButton(EMOJI_FORWARD, "Follow")) {
						navigate_to(object.struct_info, (__int64)object.value, _current_item.ss);
						auto& old = _history.back();
						old.followed = arg->name_hash;
					}
					if (!object.value) {
						ImGui::SameLine();
						ImGui::Text("null");
					}
					break;
				}
				case STU_ConstraintType_Enum: {
					auto primitive = value.get_argument_primitive(arg);
					ImGui::Text("%x", primitive.get_value<uint>());
					break;
				}
				case STU_ConstraintType_NonSTUResourceRef:
				case STU_ConstraintType_ResourceRef: {
					auto res = value.get_argument_resource(arg);
					render_resref(res);
					break;
				}
				}
			}
		}
		else {
			if (type == STU_ConstraintType_Object || type == STU_ConstraintType_InlinedObject || arg->constraint->is_list()) {
				if (imgui_helpers::TooltipButton(EMOJI_FORWARD, "Follow")) {
					navigate_to(GetSTUInfoByHash(arg_type_hash), 0, 0);
					auto& old = _history.back();
					old.followed = arg->name_hash;
				}
			}
		}

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(STU_ConstraintType_ToString(type).data());

		ImGui::TableNextColumn();
		imgui_helpers::display_type(arg_type_hash, true, true, false);

		ImGui::TableNextColumn();
		ImGui::Text("%x", arg->offset);

		ImGui::PopID();
		ImGui::PopID();

		if (*expand_list) {
			ImGui::EndTable();

			ImGui::PushID(value.struct_info);
			ImGui::PushID(arg->name_hash);
			switch (type) {
			case STU_ConstraintType_BSList_InlinedObject:
			case STU_ConstraintType_BSList_Object: {
				auto list = value.get_argument_objectlist(arg);
				for (int i = 0; i < list.count(); i++) {
					auto item = list[i];

					ImGui::PushID(i);
					ImGui::BulletText("%d:", i);
					ImGui::SameLine();
					display_addr((__int64)item.value);
					ImGui::SameLine();
					imgui_helpers::display_type(item.get_runtime_root().struct_info->name_hash, true, true, false);
					ImGui::SameLine();
					if (imgui_helpers::TooltipButton(EMOJI_FORWARD, "Follow")) {
						navigate_to(item.struct_info, (__int64)item.value, _current_item.ss);
						auto& old = _history.back();
						old.followed = arg->name_hash;
						old.followed_index = i;
					}
					ImGui::PopID();
				}
				break;
			}
			case STU_ConstraintType_BSList_Enum:
			case STU_ConstraintType_BSList_Primitive: {
				auto list = value.get_argument_primitivelist(arg);
				for (int i = 0; i < list.count(); i++) {
					auto item = list[i];

					ImGui::PushID(i);
					ImGui::BulletText("%d:", i);
					ImGui::SameLine();
					imgui_helpers::render_primitive(item, arg_type_hash);
					ImGui::PopID();
				}
				break;
			}
			case STU_ConstraintType_BSList_NonSTUResourceRef:
			case STU_ConstraintType_BSList_ResourceRef: {
				auto list = value.get_argument_resreflist(arg);
				for (int i = 0; i < list.count(); i++) {
					auto item = list[i];

					ImGui::PushID(i);
					ImGui::BulletText("%d:", i);
					ImGui::SameLine();
					render_resref(item);
					ImGui::PopID();
				}
				break;
			}
			case STU_ConstraintType_Map: {
				auto map = value.get_argument_map(arg);
				for (int i = 0; i < map.count(); i++) {
					auto item = map[i];

					ImGui::PushID(i);
					ImGui::BulletText("%d - Key: ", i);
					ImGui::SameLine();
					imgui_helpers::display_type(item.first, false, true, true);
					display_addr((__int64)item.second.value);
					ImGui::SameLine();
					imgui_helpers::display_type(item.second.get_runtime_root().struct_info->name_hash, true, true, false);
					ImGui::SameLine();
					if (imgui_helpers::TooltipButton(EMOJI_FORWARD, "Follow")) {
						navigate_to(item.second.struct_info, (__int64)item.second.value, _current_item.ss);
						auto& old = _history.back();
						old.followed = arg->name_hash;
						old.followed_index = i;
					}
					ImGui::PopID();
				}
				break;
			}
			}

			ImGui::PopID();
			ImGui::PopID();

			render_table_noheader();
		}
	}
}
