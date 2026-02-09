#include "Atlas/STU/RTTI/STURegistry.h"
#include "stu_explorer.h"
#include "STU_Editable.h"
#include "stu_resources.h"
#include "stu_primitive_edit.h"
#include "stu_object_edit.h"
#include "stu_enum_window.h"

void stu_explorer::navigate_to(STUInfo* info, __int64 instance, StatescriptInstance* ss_inst = nullptr) {
	navigate_internal(info, instance, ss_inst, true, {});
}

void stu_explorer::navigate_to_resource(__int64 resource) {
	auto item = stu_resources::GetByID(resource);
	if (item->valid()) {
		navigate_internal(item->vfptr->GetSTUInfo(), (__int64)item, nullptr, true, {});
		root_item = _history.current_item;
		root_item_resource = resource;
	}
}

void stu_explorer::navigate_with_history(STUInfo* stu_info, __int64 current_instance, FollowedItem followed_via) {
	navigate_internal(stu_info, current_instance, _history.current_item.ss, false, followed_via);
}

void stu_explorer::navigate_internal(STUInfo* info, __int64 instance, StatescriptInstance* ss_inst, bool remove_root, FollowedItem followItem) {
	Item new_item;
	new_item.current_instance = instance;
	new_item.stu_info = info;
	new_item.ss = ss_inst;
	if (followItem.item) {
		new_item.followed_path = _history.current_item.followed_path;
		new_item.followed_path.push_back(followItem);
	}

	_history.push_item(new_item);

	if (remove_root) {
		root_item_resource = 0;
		root_item = {};
	}
}

inline void stu_explorer::render() {
	if (open_window(nullptr, 0, ImVec2(1100, 500))) {
		_history.display_history_buttons();

		ImGui::PushID("history");
		int follow_amount = _history.current_item.followed_path.size();
		if (follow_amount > 0) {
			bool first = true;
			for (auto item_followed : _history.current_item.followed_path) {
				ImGui::PushID(follow_amount);
				if (!first) {
					ImGui::SameLine();
					ImGui::BeginDisabled();
					ImGui::TextUnformatted(">>");
					ImGui::EndDisabled();
					ImGui::SameLine();
				} else {
					first = false;
				}

				if (ImGui::ArrowButton("##back", ImGuiDir_Left)) {
					for (int i = 0; i < follow_amount; i++)
						_history.history_back();
					ImGui::PopID();
					break;
				}
				ImGui::SameLine();
				ImGui::Text("[0x%x]", item_followed.item->Offset);
				if (item_followed.index != -1) {
					ImGui::SameLine();
					ImGui::Text("(idx 0x%x)", item_followed.index);
				}
				ImGui::SameLine();
				imgui_helpers::display_type(item_followed.item->Hash, false, false, false);

				follow_amount--;
				ImGui::PopID();
			}
		}
		ImGui::PopID();

		if (_history.current_item.current_instance) {
			display_addr(_history.current_item.current_instance, "Current Instance");
			if (IsBadReadPtr((void*)_history.current_item.current_instance, 1)) {
				ImGui::SameLine();
				ImGui::PushFont(imgui_helpers::BoldFont);
				ImGui::TextUnformatted("Invalid!");
				ImGui::PopFont();
			}
		}

		if (_history.current_item.stu_info) {
			auto instance = _history.current_item.stu_info;
			STU_Object obj(instance, IsBadReadPtr((void*)_history.current_item.current_instance, _history.current_item.stu_info->InstanceSize) ? nullptr : (void*)_history.current_item.current_instance);
			instance = (obj = obj.get_runtime_root()).struct_info;
			int children = 0;
			auto child = _history.current_item.stu_info->Child;
			while (child) {
				children++;
				child = child->Sibling;
			}
			ImGui::Text("Arguments (top): %d - Size: %x - Children %d - Hash: ", _history.current_item.stu_info->ArgsCount, _history.current_item.stu_info->InstanceSize, children);
			ImGui::SameLine();
			while (instance) {
				//__try {
				imgui_helpers::display_type(instance->Hash, true, true, true);
				if (render_table()) {
					render_stu(obj);
					ImGui::EndTable();
				}

				if (instance->Parent) {
					ImGui::Text("Next:");
					ImGui::SameLine();
				}
				/*}
				__except (EXCEPTION_EXECUTE_HANDLER) {
					ImGui::Text("Exception while rendering");
				}*/
				obj = STU_Object(instance = instance->Parent, obj.value);
			}
		}
	}
	ImGui::End();
}

void stu_explorer::render_resref(STUResourceReference* ref) {
	if (ref->has_resource()) {
		imgui_helpers::display_type(ref->resource_id, true, true, true);
		ImGui::PushID("ref_value");
		if (ref->is_resource_loaded()) {
			display_addr(ref->resource_load_entry->align()->resource_ptr);
			ImGui::SameLine();
		}
		auto stu = stu_resources::GetByID(ref->resource_id);
		if (stu && imgui_helpers::TooltipButton(EMOJI_FORWARD, "Follow - Will end editing session for current resource")) {
			navigate_to_resource(ref->resource_id);
		}
		ImGui::SameLine();
		if (ImGui::Button(EMOJI_EDIT)) {
			STU_Primitive item((void*)&ref->resource_id, STU_NAME::Primitive::u64);
			window_manager::open_modal<stu_primitive_edit>(this, item);
		}
		ImGui::PopID();
	}
	else {
		ImGui::Text("null");
	}
}

void stu_explorer::render_stu(STU_Object value) {
	const auto reg = STURegistry::Get();

	for (int i = 0; i < value.struct_info->ArgsCount; i++) {
		ImGui::PushID(value.struct_info);
		auto arg = &value.struct_info->Args[i];
		auto type = arg->Constraint->ToConstraintType();
		auto arg_type_hash = arg->Constraint->GetSTUType();
		ImGui::PushID(arg->Hash);
		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		imgui_helpers::display_type(arg->Hash, false, true, false);

		ImGui::TableNextColumn();
		bool* expand_list = ImGui::GetStateStorage()->GetBoolRef(arg->Hash, false);
		if (value.valid()) {

			if (arg->Constraint->IsList()) {
				auto list = (STUBullshitListFull<__int64>*)((__int64)value.value + arg->Offset);
				if (list->valid()) {
					if (imgui_helpers::TooltipButton(EMOJI_FORWARD, "Follow without address")) {
						navigate_with_history(reg->GetSTUInfoByHash(arg_type_hash), 0, FollowedItem{arg});
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
						navigate_with_history(reg->GetSTUInfoByHash(arg_type_hash), 0, FollowedItem{arg});
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
					if (ImGui::Button(EMOJI_EDIT)) {
						window_manager::open_modal<stu_primitive_edit>(this, primitive);
					}
					ImGui::SameLine();
					imgui_helpers::render_primitive(primitive);
					break;
				}
				case STU_ConstraintType_Object:
				case STU_ConstraintType_InlinedObject: {
					auto object = value.get_argument_object(arg);
					if (imgui_helpers::TooltipButton(EMOJI_FORWARD, "Follow")) {
						navigate_with_history(object.struct_info, (__int64)object.value, FollowedItem{arg});
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
					ImGui::SameLine();
					auto enum_type = STUFindEnum(arg_type_hash);
					auto enum_value = enum_type->findValue(primitive.get_value<uint>());
					if (enum_value) {
						imgui_helpers::display_type(enum_value->hash, true, true, false);
					} else {
						ImGui::Text("enum value invalid");
					}
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
			if (type == STU_ConstraintType_Object || type == STU_ConstraintType_InlinedObject || arg->Constraint->IsList()) {
				if (imgui_helpers::TooltipButton(EMOJI_FORWARD, "Follow")) {
					navigate_with_history(reg->GetSTUInfoByHash(arg_type_hash), 0, FollowedItem{arg});
				}
			}
		}

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(STUConstraintType_ToString(type));
		if (type == STU_ConstraintType_Enum || type == STU_ConstraintType_BSList_Enum) {
			ImGui::SameLine();
			auto def = STUFindEnum(arg_type_hash);
			if (!def)
				ImGui::BeginDisabled();

			if (ImGui::Button(EMOJI_SHARE)) {
				auto window = stu_enum_window::get_latest_or_create(this);
				dock_item_right(window, 0.8f);
				window->set(def);
			}

			if (!def)
				ImGui::EndDisabled();
		}

		ImGui::TableNextColumn();
		imgui_helpers::display_type(arg_type_hash, true, true, false);

		ImGui::TableNextColumn();
		ImGui::Text("%x", arg->Offset);

		ImGui::PopID();
		ImGui::PopID();

		if (*expand_list) {
			ImGui::EndTable();

			ImGui::PushID(value.struct_info);
			ImGui::PushID(arg->Hash);
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
					imgui_helpers::display_type(item.get_runtime_root().struct_info->Hash, true, true, false);
					ImGui::SameLine();
					if (imgui_helpers::TooltipButton(EMOJI_FORWARD, "Follow")) {
						navigate_with_history(item.struct_info, (__int64)item.value, FollowedItem{arg, i});
					}
					ImGui::SameLine();
					if (imgui_helpers::TooltipButton(EMOJI_CROSS, "Remove")) {
						list.remove_at_index(i);
					}
					ImGui::PopID();
				}
				if (ImGui::Button(" + ")) {
					if (type == STU_ConstraintType_BSList_InlinedObject) {
						list.push_back_inlinedObject();
					}
					else {
						window_manager::open_modal<stu_object_edit>(this, stu_object_edit::arg_typ{ value, arg });
					}
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
					if (ImGui::Button(EMOJI_EDIT)) {
						window_manager::open_modal<stu_primitive_edit>(this, item);
					}
					ImGui::SameLine();
					if (imgui_helpers::TooltipButton(EMOJI_CROSS, "Remove")) {
						list.remove_at_index(i);
					}
					ImGui::SameLine();
					imgui_helpers::render_primitive(item);
					ImGui::PopID();
				}
				if (ImGui::Button(" + ")) {
					list.push_back_default();
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
					ImGui::SameLine();
					if (imgui_helpers::TooltipButton(EMOJI_CROSS, "Remove")) {
						list.remove_at_index(i);
					}
					ImGui::PopID();
				}
				if (ImGui::Button(" + ")) {
					list.push_back(0);
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
					imgui_helpers::display_type(item.second.get_runtime_root().struct_info->Hash, true, true, false);
					ImGui::SameLine();
					if (imgui_helpers::TooltipButton(EMOJI_FORWARD, "Follow")) {
						navigate_with_history(item.second.struct_info, (__int64)item.second.value, FollowedItem{arg, i});
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
