#include "viewmodel_write_blocker.h"
#include "viewmodel_viewer.h"

void viewmodel_write_blocker::add_block(ViewModel* vm, __int64 key) {
	_blocked_viewmodels.try_emplace({ vm, key });
}

void viewmodel_write_blocker::redo_blocks() {
	for (int i = 0; i < ViewModelPropertyType_Max; i++) {
		s_blocked_viewmodels[i].clear();
		if (_blocked_types[i]) {
			s_blocked_viewmodels[i].insert(_blocked_viewmodels.begin(), _blocked_viewmodels.end());
		}
	}
}

void viewmodel_write_blocker::render() {
	if (open_window()) {
		ImGui::TextUnformatted("Block:");
		for (auto i : s_proptypes_declared) {
			if (ImGui::Checkbox(VMPropTypeToString((ViewModelPropertyType)i).c_str(), &_blocked_types[i])) {
				redo_blocks();
			}
		}
		if (imgui_helpers::beginTable("blocked viewmodels", { "Addr", "" })) {
			for (auto it = _blocked_viewmodels.begin(); it != _blocked_viewmodels.end(); it++) {
				if (IsBadReadPtr(it->first.first, sizeof(ViewModel))) {
					_blocked_viewmodels.erase(it);
					it--;
					continue;
				}

				ImGui::PushID(&it->second);
				ImGui::TableNextRow();

				ImGui::TableNextColumn();
				if (it->first.first) {
					ImGui::Text("[%x]", it->first.first->id);
					ImGui::SameLine();
					if (ImGui::Button(EMOJI_LOCATE)) {
						viewmodel_viewer::get_latest_or_create(this)->selected_vm = it->first.first;
					}
					ImGui::SameLine();
				}
				imgui_helpers::display_type(it->first.second, true, true, false);

				ImGui::TableNextColumn();
				if (ImGui::Checkbox(EMOJI_DENY, &it->second.blocked)) {
					redo_blocks();
				}
				ImGui::SameLine();
				if (ImGui::Checkbox(EMOJI_COMMENT, &it->second.logged)) {
					redo_blocks();
				}
				ImGui::SameLine();
				if (ImGui::Checkbox(EMOJI_COMMENT_2, &it->second.logged_ultradeluxe)) {
					redo_blocks();
				}
				ImGui::SameLine();
				if (ImGui::Button(EMOJI_TRASH)) {
					_blocked_viewmodels.erase(it);
					it--;
					redo_blocks();
				}
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();
}

bool viewmodel_write_blocker::isBlocked(ViewModel* vm, __int64 new_value, __int64 a4, ViewModelPropertyListItem* prop, __int64 return_addr, bool with_vm) {
	auto result = s_blocked_viewmodels[prop->property.type].find({ with_vm ? vm : nullptr, prop->key });
	if (result != s_blocked_viewmodels[prop->property.type].end()) {
		if (return_addr > globals::gameBase && return_addr - globals::gameBase < globals::gameSize)
			return_addr -= globals::gameBase;
		if (result->second.logged)
			printf("[%x] (%p): Changed from %p to %p\n", vm->id, return_addr, prop->property.value, new_value);
		if (result->second.logged_ultradeluxe) {
			stacktrace();
		}
		return result->second.blocked;
	}
	return false;
}

//true = block
std::pair<ViewModelProperty*, bool> viewmodel_write_blocker::block_behaviour(ViewModel* vm, __int64 key, __int64 new_value, __int64 a4, __int64 return_addr) {
	ViewModelPropertyListItem* prop = vm->getByKey(key);
	if (!prop)
		return { nullptr, false };
	if (isBlocked(vm, new_value, a4, prop, return_addr, true) || isBlocked(vm, new_value, a4, prop, return_addr, false))
		return { &prop->property, true };
	return{ nullptr, false };
}