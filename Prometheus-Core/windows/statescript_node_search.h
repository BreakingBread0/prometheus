#pragma once
#include "../window_manager/window_manager.h"
#include "../Statescript.h"
#include "../search_helper.h"
#include "../stringhash_library.h"
#include "statescript_window.h"
#include <set>
#include "stu_explorer.h"

class statescript_node_search : public window {
	WINDOW_DEFINE_ARG(statescript_node_search, "Statescript", "Statescript Node Search", Component_23_Statescript*);

	struct Node {
		StatescriptInstance* ss;
		STUStatescriptBase* node;
		std::vector<std::pair<STUArgumentInfo*, STUConfigVar*>> config_vars;
	};

	inline void render() override {
		if (open_window()) {
			if (IsBadReadPtr(_arg, sizeof(_arg))) {
				ImGui::Text("Invalid Arg");
				return;
			}
			_search.search_box("Search");
			if (_search.needs_haystack()) {
				_nodes.clear();
				for (auto& ss : _arg->ss_inner) {
					if (ss->graph) {
						for (auto node : *ss->graph) {
							auto typ = node->graph_node.base.vfptr->GetSTUInfo();
							_search.haystack_stringhash(typ->name_hash);
							std::vector<std::pair<STUArgumentInfo*, STUConfigVar*>> cvs{};
							for (auto arg_info : *typ) {
								if (arg_info.first->name_hash == STU_NAME::STUStatescriptState ||
									arg_info.first->name_hash == STU_NAME::STUStatescriptAction)
									break;
								auto arg = arg_info.second;
								if (arg->constraint->get_type_flag() == STU_ConstraintType_Object) {
									auto arg_typ = GetSTUInfoByHash(arg->constraint->get_stu_type());
									if (arg_typ && arg_typ->assignable_to_hash(STU_NAME::STUConfigVar)) {
										STUConfigVar* cv = *(STUConfigVar**)((__int64)node + arg->offset);
										if (cv) {
											_search.haystack_stringhash(cv->base.vfptr->GetSTUInfo()->name_hash);
											cvs.push_back(std::pair<STUArgumentInfo*, STUConfigVar*>{arg, cv});
										}
									}
								}
							}
							if (_search.found_needle(node)) {
								_nodes[typ->name_hash].push_back({ss, node, cvs});
							}
						}
					}
				}
			}
			for (auto& node_typ : _nodes) {
				ImGui::PushID(node_typ.first);

				imgui_helpers::display_type(node_typ.first, true, true, true);

				for (auto& node : node_typ.second) {
					ImGui::PushID(node.ss);
					ImGui::PushID(node.node);
					
					if (!IsBadReadPtr(node.ss, sizeof(StatescriptInstance*))) {
						ImGui::BulletText("%d <%d>", node.ss->instance_id, node.node->idx_in_nodes);
						ImGui::SameLine();
						if (imgui_helpers::TooltipButton(EMOJI_SHARE, "Graph")) {
							statescript_window::get_latest_or_create(this)->display_instance(node.ss, node.node);
						}
						ImGui::SameLine();
						if (imgui_helpers::TooltipButton(EMOJI_LOCATE, "STU Explorer")) {
							auto explorer = stu_explorer::get_latest_or_create(this);
							explorer->navigate_to(node.ss->graph->base.vfptr->GetSTUInfo(), (__int64)node.ss->graph, node.ss);
							explorer->navigate_to(node.node->graph_node.base.vfptr->GetSTUInfo(), (__int64)node.node, node.ss);
						}
						ImGui::Indent();

						for (auto& cv : node.config_vars) {
							ImGui::PushID(cv.second);

							imgui_helpers::display_type(cv.first->name_hash, true, true, false);
							ImGui::SameLine();
							imgui_helpers::display_type(cv.second->base.vfptr->GetSTUInfo()->name_hash, false, true, false);
							ImGui::SameLine();
							ImGui::Text("- at offs 0x%x", cv.first->offset);

							ImGui::PopID();
						}

						ImGui::Unindent();
					}

					ImGui::PopID();
					ImGui::PopID();
				}

				ImGui::PopID();
			}
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
private:
	search_helper_imgui _search{};
	std::map<uint, std::vector<Node>> _nodes{};
};

WINDOW_REGISTER(statescript_node_search);
