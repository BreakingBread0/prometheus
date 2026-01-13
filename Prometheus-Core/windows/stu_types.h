#pragma once
#include "../window_manager/window_manager.h"
#include "../STU.h"
#include "../stringhash_library.h"
#include "stu_explorer.h"
#include "../search_helper.h"

class stu_types : public window {
	WINDOW_DEFINE(stu_types, "STU", "STU Types", true);

	inline void render() override {
		if (open_window()) {
			_search.search_box("Search");
			imgui_helpers::InputHex("argument constraint type search", &_arg_constraint_typ);
			if (imgui_helpers::beginTable("e", { "Hash", "" })) {
				for (auto info : _infos) {
					std::set<STUInfo*> found;
					auto current = info;
					while (current) {
						if (_search.needs_haystack()) {
							_search.haystack_stringhash(current->name_hash);
						}
						if (_search.found_needle(info != current ? info->name_hash ^ current->name_hash : info->name_hash)) {
							found.emplace(current);
						}
						current = current->base_stu;
					}
					if (found.size() == 0)
						continue;

					bool found_typ = false;
					if (_arg_constraint_typ != 0) {
						//only search top level
						for (int i = 0; i < info->argument_count; i++) {
							auto arg = info->arguments[i];
							if (arg.constraint && arg.constraint->get_type_flag() == _arg_constraint_typ) {
								found_typ = true;
								break;
							}
						}
						if (!found_typ)
							continue;
					}

					ImGui::TableNextRow();
					ImGui::PushID(info);

					ImGui::TableNextColumn();
					imgui_helpers::display_type(info->name_hash, found.find(info) != found.end(), true, false);
					ImGui::SameLine();
					if (ImGui::Button(EMOJI_FORWARD)) {
						stu_explorer::get_latest_or_create(this)->navigate_to(info, 0, nullptr);
					}

					ImGui::TableNextColumn();
					current = info->base_stu;
					while (current) {
						ImGui::PushID(current);
						imgui_helpers::display_type(current->name_hash, found.find(current) != found.end(), true, false);
						ImGui::SameLine();
						if (ImGui::Button(EMOJI_FORWARD)) {
							stu_explorer::get_latest_or_create(this)->navigate_to(current, 0, nullptr);
						}
						ImGui::PopID();
						current = current->base_stu;
					}

					ImGui::PopID();
				}
				ImGui::EndTable();
			}
		}
		ImGui::End();
	}

	inline void initialize() override {
		STURegistry* header = GetSTURegistry();
		while (header) {
			_infos.push_back(header->info);
			header = header->next;
		}
	}
private:
	search_helper_imgui _search{};
	std::vector<STUInfo*> _infos;
	__int64 _arg_constraint_typ = 0;

	//inline void preStartInitialize() override {}
};

WINDOW_REGISTER(stu_types);
