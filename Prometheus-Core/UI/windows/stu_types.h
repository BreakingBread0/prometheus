#pragma once
#include "../window_manager/window_manager.h"
#include "STU.h"
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
							_search.haystack_stringhash(current->Hash);
						}
						if (_search.found_needle(info != current ? info->Hash ^ current->Hash : info->Hash)) {
							found.emplace(current);
						}
						current = current->Parent;
					}
					if (found.size() == 0)
						continue;

					bool found_typ = false;
					if (_arg_constraint_typ != 0) {
						//only search top level
						for (int i = 0; i < info->ArgsCount; i++) {
							auto arg = info->Args[i];
							if (arg.Constraint && arg.Constraint->ToConstraintType() == _arg_constraint_typ) {
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
					imgui_helpers::display_type(info->Hash, found.find(info) != found.end(), true, false);
					ImGui::SameLine();
					if (ImGui::Button(EMOJI_FORWARD)) {
						stu_explorer::get_latest_or_create(this)->navigate_to(info, 0, nullptr);
					}

					ImGui::TableNextColumn();
					current = info->Parent;
					while (current) {
						ImGui::PushID(current);
						imgui_helpers::display_type(current->Hash, found.find(current) != found.end(), true, false);
						ImGui::SameLine();
						if (ImGui::Button(EMOJI_FORWARD)) {
							stu_explorer::get_latest_or_create(this)->navigate_to(current, 0, nullptr);
						}
						ImGui::PopID();
						current = current->Parent;
					}

					ImGui::PopID();
				}
				ImGui::EndTable();
			}
		}
		ImGui::End();
	}

	inline void initialize() override {
		STURegistry* header = STURegistry::Get();
		while (header) {
			_infos.push_back(header->Info);
			header = header->Next;
		}
	}
private:
	search_helper_imgui _search{};
	std::vector<STUInfo*> _infos;
	__int64 _arg_constraint_typ = 0;

	//inline void preStartInitialize() override {}
};

WINDOW_REGISTER(stu_types);
