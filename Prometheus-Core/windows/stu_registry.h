#pragma once
#include "../window_manager/window_manager.h"
#include "../STU.h"
#include "../stringhash_library.h"
#include "stu_explorer.h"
#include "../search_helper.h"
#include "../stu_resources.h"
#include "stu_explorer.h"

class stu_registry : public window {
	WINDOW_DEFINE(stu_registry, "STU", "STU Registry", true);

	inline void add_search_recursive(STU_Object obj, std::set<void*>& visited_nodes) {
		_search.haystack_stringhash(obj.struct_info->name_hash);
		if (visited_nodes.find(obj.value) != visited_nodes.end())
			return;
		visited_nodes.insert(obj.value);
		for (auto arg : *obj.struct_info) {
			switch (arg.second->constraint->get_type_flag()) {
			case STU_ConstraintType_BSList_InlinedObject:
			case STU_ConstraintType_BSList_Object: {
				for (auto item : obj.get_argument_objectlist(arg.second)) {
					if (item.valid())
						add_search_recursive(item.get_runtime_root(), visited_nodes);
				}
			}
				break;
			case STU_ConstraintType_InlinedObject:
			case STU_ConstraintType_Object: {
				auto child = obj.get_argument_object(arg.second);
				if (child.valid())
					add_search_recursive(child.get_runtime_root(), visited_nodes);
			}
				break;
			case STU_ConstraintType_Map:
				for (auto item : obj.get_argument_map(arg.second)) {
					if (item.second.valid())
						add_search_recursive(item.second.get_runtime_root(), visited_nodes);
				}
				break;
			}
		}
	}

	inline void render() override {
		if (open_window()) {
			update_if_required();
			ImGui::Text("Total loaded STU Resources: %d", _stus.size()); 
			_search.search_box("Search");
			if (ImGui::Checkbox("Recursive STU Object search", &_search_recursive))
				_search.set_needs_haystack();
			if (imgui_helpers::beginTable("e", { "ID", "Actions" })) {
				for (auto& info : _stus) {
					if (!info.second->valid()) {
						//printf("Invalid Resource %p!\n", info.first);
						continue;
					}
					if (_search.needs_haystack()) {
						_search.haystack_stringhash(info.first);
						_search.haystack_stringhash(info.second->vfptr->GetSTUInfo()->name_hash);
						if (_search_recursive) {
							std::set<void*> vec{};
							add_search_recursive(info.second->to_editable().get_runtime_root(), vec);
						}
					}
					if (_search.found_needle(info.second)) {
						ImGui::TableNextRow();
						ImGui::PushID(info.second);

						ImGui::TableNextColumn();
						imgui_helpers::display_type(info.first, true, true, true);

						ImGui::TableNextColumn();
						
						if (ImGui::Button("Explore")) {
							stu_explorer::get_latest_or_create(this)->navigate_to(info.second->vfptr->GetSTUInfo(), (__int64)info.second, nullptr);
						}

						ImGui::PopID();
					}
				}
				ImGui::EndTable();
			}
		}
		ImGui::End();
	}

	void update_if_required() {
		if (resource_load_frame != stu_resources::GetResourceLoadFrame()) {
			resource_load_frame = stu_resources::GetResourceLoadFrame();
			_stus = stu_resources::GetAll();
			_search.set_needs_haystack();
		}
	}

	inline void initialize() override { 
	}
private:
	search_helper_imgui _search{};
	std::map<__int64, STUBase<>*> _stus;
	int resource_load_frame = 0;
	bool _search_recursive = false;

	//inline void preStartInitialize() override {}
};

WINDOW_REGISTER(stu_registry);
