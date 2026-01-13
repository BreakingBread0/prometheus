//#pragma once
//#include "../window_manager/window_manager.h"
//#include "entitydef_explorer.h"
//#include <functional>
//#include "../search_helper.h"
//
//class entitydef_list : public window {
//	WINDOW_DEFINE(entitydef_list, "Resource", "Entity Def List", true);
//
//	static inline __int64 s_stuheroes[] = {
//		0x02e0000000000003,
//		0x02e0000000000002,
//		0x02e00000000000dd,
//		0x02e000000000000a,
//		0x02e0000000000079,
//		0x02e0000000000068,
//		0x02e0000000000065,
//		0x02e0000000000042,
//		0x02e0000000000040,
//		0x02e0000000000029,
//		0x02e0000000000020,
//		0x02e0000000000016,
//		0x02e0000000000015,
//		0x02e0000000000009,
//		0x02e0000000000008,
//		0x02e000000000007a,
//		0x02e0000000000007,
//		0x02e000000000006e,
//		0x02e0000000000006,
//		0x02e0000000000005,
//		0x02e0000000000004
//	};
//
//	void display_ent(__int64 entid, STUEntityDefinition* ent) {
//		__try {
//			ImGui::PushID(entid);
//			if (ImGui::RadioButton("", _selected_ent == entid)) {
//				_selected_ent = entid;
//				if (_select_mode) {
//					_selection_callback(_selected_ent);
//					this->queue_deletion();
//				}
//				else {
//					entitydef_explorer::get_latest_or_create(this)->stu_entity = ent;
//				}
//			}
//			ImGui::SameLine();
//			if (imgui_helpers::TooltipButton(EMOJI_COPY, "Copy address"))
//				imgui_helpers::openCopyWindow((__int64)ent);
//			ImGui::SameLine();
//			imgui_helpers::display_type(entid, true, true, true);
//
//			for (int i = 0; i < ent->m_childEntityData.count(); i++) {
//				auto& entry = ent->m_childEntityData.list()[i];
//				ImGui::Indent();
//				auto& ent = entry.entity;
//				if ((ent.resource_id & 0x400000000000000) != 0 && ent.is_resource_loaded()) {
//					display_ent(ent.resource_id, ent.resource_load_entry->align()->resource_ptr);
//				}
//				else {
//					ImGui::Text("Not loaded: %p", ent.resource_id);
//				}
//				if (entry.unk) {
//					display_addr(entry.unk, "Unk");
//				}
//				if (entry.another_res_1.has_resource()) {
//					display_addr(entry.another_res_1.resource_id, "AR1");
//					if (entry.another_res_1.is_resource_loaded()) {
//						ImGui::SameLine();
//						ImGui::Text("(Loaded)");
//					}
//				}
//				if (entry.another_res_2.has_resource()) {
//					display_addr(entry.another_res_2.resource_id, "AR2");
//					if (entry.another_res_2.is_resource_loaded()) {
//						ImGui::SameLine();
//						ImGui::Text("(Loaded)");
//					}
//				}
//				ImGui::Unindent();
//			}
//			/*int count = ent->m_childEntityData.count();
//			if (count > 0) {
//				ImGui::Text("Children: %d", count);
//			}*/
//
//			ImGui::PopID();
//		}
//		__except (EXCEPTION_EXECUTE_HANDLER) {
//			ImGui::Text("Exception");
//		}
//	}
//
//	void render() override {
//		if (open_window(_select_mode ? "SELECT ENTITY" : nullptr)) {
//			if (ImGui::Button("Explore All")) {
//				_entities.clear();
//				for (int i = 1; i < 0x10000; i++) {
//					__int64 entid = 0x400000000000000 | i;
//					__int64 resource = try_load_resource(entid);
//					if (resource)
//						_entities.emplace(entid, (STUEntityDefinition*)resource);
//				}
//			}
//			ImGui::SameLine();
//			ImGui::Text("Count: %d (%x)", _entities.size(), _entities.size());
//			if (!_select_mode) {
//				ImGui::SameLine();
//				if (ImGui::Button("Set Names from STUHeroes")) {
//					for (auto hero : s_stuheroes) {
//						STUHero* instance = (STUHero*)try_load_resource(hero);
//						if (instance) {
//							if (!instance->hero_name.is_resource_loaded()) {
//								printf("Hero %p has no name\n", hero);
//								continue;
//							}
//							std::string name = *instance->hero_name.resource_load_entry->align()->resource_ptr;
//							allmighty_hash_lib::add_comment(instance->m_gameplayEntity.resource_id, name);
//							allmighty_hash_lib::add_comment(instance->entity_heroselect_lineup.resource_id, name + " (HeroSelect)");
//							allmighty_hash_lib::add_comment(instance->entity_herolineup.resource_id, name + "(Lineup)");
//							allmighty_hash_lib::add_comment(instance->entity_potg.resource_id, name + "(POTG)");
//						}
//						else {
//							printf("Failed to load hero %p\n", hero);
//						}
//					}
//				}
//			}
//
//			_search.search_box("Search Entities");
//			ImGui::TextWrapped("Search includes: Component names, hashes, resource ids, statescript script ids (SS Comp only)");
//			if (ImGui::BeginListBox("", ImVec2(-1, -1))) {
//				for (auto it = _entities.begin(); it != _entities.end(); it++) {
//					if (IsBadReadPtr(it->second, sizeof(STUEntityDefinition*))) {
//						_entities.erase(it);
//						it--;
//						continue;
//					}
//					if (_search.needs_haystack()) {
//						bool has = false;
//						_search.haystack_hex(it->first);
//						if (allmighty_hash_lib::comments.find(it->first) != allmighty_hash_lib::comments.end()) {
//							_search.haystack(allmighty_hash_lib::comments[it->first]);
//						}
//						for (int i = 0; i < it->second->m_componentMap.count(); i++) {
//							auto& comp_def = it->second->m_componentMap.list()[i];
//							_search.haystack(comp_def.component_hash);
//							auto hash_name = allmighty_hash_lib::hashes.find((int)comp_def.component_hash);
//							if (hash_name != allmighty_hash_lib::hashes.end()) {
//								_search.haystack(hash_name->second);
//							}
//							/*char hash_buf[32];
//							sprintf_s(hash_buf, "%p", comp_def.component_hash);
//							printf("%p\n", comp_def.component_hash);
//							if (has = icontains(hash_buf, search_buf))
//								break;*/
//							if (comp_def.component_hash == STU_NAME::STUStatescriptComponent) {
//								auto scripts = (STUBullshitListFull<STUResourceReference<__int64>>*)((__int64)comp_def.STUEntityComponent_data + 8);
//								for (int j = 0; j < scripts->count(); j++) {
//									auto& script = scripts->list()[j];
//									if (script.has_resource()) {
//										_search.haystack(script.resource_id);
//									}
//								}
//							}
//						}
//					}
//					if (_search.found_needle(it->first))
//						display_ent(it->first, it->second);
//				}
//				ImGui::EndListBox();
//			}
//		}
//		ImGui::End();
//	}
//
//	entitydef_list() {
//
//	}
//	entitydef_list(bool select_mode, std::function<void(__int64)> selection_callback) {
//		_select_mode = true;
//		_selection_callback = selection_callback;
//	}
//
//	//inline void preStartInitialize() override {}
//	//inline void initialize() override {}
//private:
//	std::map<__int64, STUEntityDefinition*> _entities;
//	__int64 _selected_ent;
//	bool _select_mode = false;
//	std::function<void(__int64)> _selection_callback;
//	search_helper_imgui _search;
//};
//
//WINDOW_REGISTER(entitydef_list);
