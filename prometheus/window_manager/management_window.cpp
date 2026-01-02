#include "management_window.h"
#include <memory>
#include <vector>
#include <map>
#include "../stu_resources.h"
#include <format>
#include "../entity_admin.h"

std::map<int, std::string> s_worldstates = {
	{ 0, "Shutdown" },
	{ 1, "Shutting Down" },
	{ 2, "Waiting" },
	{ 3, "Global Loading" },
	{ 4, "World Loading" },
	{ 5, "World Loaded" },
	{ 6, "World Replicating" },
	{ 7, "Gamemode Loading" },
	{ 8, "World Running" }
};

struct window_context_invis {
	std::vector<std::shared_ptr<window>> children;
	std::vector<std::shared_ptr<window>> docked_dependents;
};

struct window_context : public window_context_invis {
	std::vector<std::shared_ptr<window>> dependents;
};

std::map<int, window_context_invis> invisible_window_group_children{};
std::set<int> invisible_child_windows{};

//returns true = default focus / unfocus behaviour
bool render_window(const char* buf, std::shared_ptr<window> main_window, /*std::vector<std::shared_ptr<window>>& children,*/ bool is_collapsed, bool bold) {
	ImGui::PushID(main_window->window_id);

	if (is_collapsed)
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
	if (bold)
		ImGui::PushFont(imgui_helpers::BoldFont);
	bool result = ImGui::MenuItem(buf);
	if (bold)
		ImGui::PopFont();
	/*if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
		main_window->queue_deletion();
		for (auto& child : children)
			child->queue_deletion();
	}*/
	if (is_collapsed)
		ImGui::PopStyleColor();
	ImGui::PopID();
	return result;
}

std::map<std::string, __int64> get_maps() {
	std::map<std::string, __int64> maps;
	for (auto& item : stu_resources::GetIfAssignableTo(stringHash("STUMapHeader"))) {
		auto map_header = item.second->to_editable();
		const char* map_path = map_header.get_argument_primitive("m_mapName").get_value<const char*>();
		const char* map_internal_name = map_header.get_argument_primitive(0x39824653).get_value<const char*>(); //sometimes null
		const char* display_name = map_header.get_argument_resource("m_displayName")->get_UXDisplayString();

		// (carter): TODO: Don't skip silently, these should likely be logged. Potential pita if not.
		if (!map_path || !map_internal_name || !display_name) continue;

		std::string name_out = map_path;
		if (strlen(map_internal_name) > 0)
			name_out += " (" + std::string(map_internal_name) + ")";
		name_out += " - " + std::string(display_name);
		__int64 map_id_1 = map_header.get_argument_resource("m_map")->resource_id;
		//map id 2 seems to be a different platform -> server map low poly / no lighting?
		//__int64 map_id_2 = map_header.get_argument_resource(0xccf37178)->resource_id;
		maps.emplace(name_out + std::format(": {:x}", map_id_1), map_id_1);
		//maps.emplace(name_out + std::format(" (2): {:x}", map_id_2), map_id_2);
	}
	return maps;
}

void management_window::render() {
	auto root_pos = ImGui::GetMainViewport()->Pos;
	ImGui::SetNextWindowPos(root_pos, ImGuiCond_Once);
	if (ImGui::Begin("Feel Good Inc.", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking)) {
		if (ImGui::BeginMenuBar()) {
			for (auto& category : window_manager::get_window_categories()) {
				if (ImGui::BeginMenu(category.c_str(), true)) {
					for (auto& window : window_manager::get_all_windows()) {
						if (window.second.category != category)
							continue;
						
						char buf[128];
						int count = window.second.active_windows_cnt;
						if (count > 0) {
							sprintf_s(buf, "%s [%d]##%s", window.second.name.c_str(), count, window.second.name.c_str());
						}
						else {
							sprintf_s(buf, "%s##%s", window.second.name.c_str(), window.second.name.c_str());
						}

						if (!window.second.helper_instance->allow_auto_creation())
							ImGui::BeginDisabled();
						if (ImGui::MenuItem(buf))
							window_manager::add_default_window(window.first);
						if (!window.second.helper_instance->allow_auto_creation())
							ImGui::EndDisabled();
					}
					ImGui::EndMenu();
				}
			}
			if (ImGui::Button("DP Load")) {
				printf("Loading lobby data packs...\n");
				auto msg5017 = (__int64(__fastcall*)(__int64, __int64, char* stru))(globals::gameBase + 0xc7f090);
				char buf[0x80]{};
				msg5017(0, 0, buf);
			}
			/*if (ImGui::Button("Test")) {
				auto fn = (void(__fastcall*)(Entity*, uint*))(globals::gameBase + 0xd07830);
				auto controller = GameEntityAdmin()->getLocalEnt();
				auto pet = controller->getById<Component_20_Pet>(0x20);
				auto aim_ent = GameEntityAdmin()->getEntById(pet->aim_entid);
				fn(aim_ent, &controller->entity_id);
			}*/
			if (!globals::isDemo && ImGui::Button("Demo")) {
				globals::isDemo = true;
			}
			auto game_ea = GameEntityAdmin();
			if (game_ea) {
				auto world = get_system27_WorldEngineSystem(game_ea);
				if (ImGui::BeginMenu("Map")) {
					for (auto& item : get_maps()) {
						if (ImGui::MenuItem(item.first.c_str())) {
							world->wanted_map_id = item.second;
							world->world_state = 2;
						}
					}

					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Lobby Map")) {
					for (auto& item : get_maps()) {
						if (ImGui::MenuItem(item.first.c_str())) {
							auto lobby_map_comp = LobbyEntityAdmin()->getSingletonComponent<Component_54_Lobbymap>(0x54);
							if (!lobby_map_comp)
								imgui_helpers::messageBox("Failed to get lobby map entity admin!");
							else {
								lobby_map_comp->UnloadMap();
								lobby_map_comp->LoadMap(item.second);
							}
						}
					}

					ImGui::EndMenu();
				}
				char buf[64];
				int worldstate = world->world_state;
				if (worldstate >= 2 && worldstate <= 8) {
					sprintf_s(buf, "Engine (%hhd %s)###ENG", worldstate, s_worldstates[worldstate].c_str());
				}
				else {
					sprintf_s(buf, "Engine Invalid");
				}
				if (ImGui::BeginMenu(buf)) {
					for (auto& item : s_worldstates) {
						char buf[64];
						sprintf_s(buf, "%hhd - %s", item.first, item.second.c_str());
						if (ImGui::MenuItem(buf, nullptr, worldstate == item.first)) {
							world->world_state = item.first;
						}
					}

					ImGui::EndMenu();
				}
			}
			ImGui::BeginDisabled();
			auto game_state = GetGameState();
			if (game_state) {
				ImGui::MenuItem(GetGameStateStr(game_state->current_state));
				//ImGui::MenuItem(game_state->state_str);
			}
			ImGui::MenuItem(" - ");
			ImGui::EndDisabled();
			/*if (ImGui::MenuItem(EMOJI_COPY " Base")) {
				copyAddress(globals::gameBase);
			}*/
			if (ImGui::MenuItem("CE")) {
				const char* path = "C:\\Program Files\\Cheat Engine\\cheatengine-x86_64.exe";
				//ShellExecuteA(NULL, "open", path, NULL, NULL, 1);
			}
			/*if (ImGui::MenuItem("CMD")) {
				const char* path = "C:\\windows\\system32\\cmd.exe";
				ShellExecuteA(NULL, "open", path, NULL, NULL, 1);
			}*/
			if (ImGui::MenuItem("DBG")) {
				const char* path = "C:\\vs\\Common7\\IDE\\Remote Debugger\\x64\\msvsmon.exe";
				const char* args = "/noclrwarn /nowowwarn /nofirewallwarn /anyuser /noauth /nosecuritywarn";
				//ShellExecuteA(NULL, "open", path, args, NULL, 1);
			}
			if (ImGui::MenuItem("Exit")) {
				globals::exit_normal = true;
				exit(0);
			}
			if (ImGui::MenuItem("Restart")) {
				globals::exit_normal = true;
				char szPath[_MAX_PATH];
				::GetModuleFileNameA(GetModuleHandleA(NULL), szPath, _MAX_PATH);

				// re execute.
				system((std::string("start ") + szPath).c_str());
				//ShellExecuteA(NULL, "open", szPath, NULL, NULL, 1);
				exit(0);
			}
			ImGui::EndMenuBar();
		}
	}
	ImGui::End();

	ImGuiWindowClass topmost;
	topmost.ViewportFlagsOverrideSet = ImGuiViewportFlags_TopMost;
	ImGui::SetNextWindowClass(&topmost);
	if (ImGui::Begin("menuswitch", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking)) {
		ImGui::SetWindowPos(ImVec2(root_pos.x, root_pos.y + ImGui::GetMainViewport()->Size.y - ImGui::GetWindowHeight()), ImGuiCond_Always);

		std::map<ImGuiWindow*, std::pair<std::shared_ptr<window>, window_context>> grouped_windows;
		if (ImGui::BeginMenuBar()) {
			for (auto& window : window_manager::get_window_list()) {
				if (window->im_id == 0)
					continue;
				if (window->is_docked)
					continue;

				ImGui::PushID(window.get());

				ImGui::BeginDisabled();
				ImGui::MenuItem(" - ");
				ImGui::EndDisabled();
				auto children = window->get_docked();

				if (window->is_collapsed)
					ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));

				//if (children.size() > 0) {
				//	std::string str = std::format("{:s}({:x}) ^^^", window->get_window_type().c_str(), window->window_id);
				//	if (ImGui::BeginMenu(str.c_str())) {
				//		for (auto& child : children) {
				//			ImGui::PushID(child.get());
				//			std::string child_str = std::format("{:s}({:x})", child->get_window_type(), child->window_id);
				//			if (ImGui::MenuItem(child_str.c_str())) {
				//				window->set_collapsed(!window->is_collapsed);
				//			}
				//			ImGui::SameLine();
				//			//TODO: Funktioniert nicht
				//			if (ImGui::MenuItem(EMOJI_CROSS)) {
				//				child->queue_deletion();
				//			}
				//			ImGui::PopID();
				//		}
				//		
				//		ImGui::EndMenu();
				//	}
				//	else if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
				//		window->set_focus_next_frame(true);
				//	}
				//}
				//else {
					if (ImGui::MenuItem(std::format("{:s}({:x})", window->get_window_type().c_str(), window->window_id).c_str())) {
						window->set_collapsed(!window->is_collapsed);
						//This code is al dente
						/*for (auto child : children) {
							child->set_collapsed(!window->is_collapsed);
						}*/
					} else if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
						window->set_focus_next_frame(true);
					}
				//}

				ImGui::SameLine();
				if (ImGui::MenuItem(EMOJI_CROSS)) {
					window->queue_deletion();
					for (auto& child : window->get_docked()) {
						child->queue_deletion();
					}
				}
				if (window->is_collapsed)
					ImGui::PopStyleColor();

				ImGui::PopID();
			}
			//If this code makes you cry, you are not alone.
			//Map: ImGuiWindow -> root window + vector(docked windows)
			//int lowest_focusid = 0;
			//for (auto window : window_manager::get_window_list()) {
			//	if (!window->im_id)
			//		continue;
			//	if (invisible_child_windows.find(window->window_id) != invisible_child_windows.end())
			//		continue;
			//	auto root_imw = ImGui::FindWindowByID(window->im_id)->RootWindowDockTree;
			//	if (root_imw->FocusOrder > lowest_focusid)
			//		lowest_focusid = root_imw->FocusOrder;
			//	auto& group = grouped_windows[root_imw];
			//	if (window->is_dependent) {
			//		auto dependant = window->created_by.lock();
			//		if (dependant) {
			//			grouped_windows[ImGui::FindWindowByID(dependant->im_id)->RootWindowDockTree].second.dependents.push_back(window);
			//		}
			//	}
			//	if (root_imw->ID == window->im_id || 
			//		root_imw->DockNodeAsHost && get_leftmost_window(root_imw->DockNodeAsHost)->ID == window->im_id
			//		/*root_imw->DockNode && get_leftmost_window(root_docknode(root_imw->DockNode))->ID == window->im_id*/) {
			//		if (!window->is_dependent)
			//			group.first = window;
			//	}
			//	else {
			//		if (window->is_dependent) {
			//			if (window->created_by.lock() != group.first) { //input ist ein vector also safe to assume dass das main window richtig ist
			//				ImGui::DockContextQueueUndockWindow(ImGui::GetCurrentContext(), ImGui::FindWindowByID(window->im_id));
			//			}
			//			group.second.docked_dependents.push_back(window);
			//		}
			//		else {
			//			group.second.children.push_back(window);
			//		}
			//	}
			//}
			//int i = 0;
			//for (auto group = grouped_windows.begin(); group != grouped_windows.end(); group++) {
			//	auto main_window = group->second.first;
			//	if (!main_window)
			//		continue;

			//	bool is_collapsed = main_window->is_collapsed;
			//	for (auto& window : group->second.second.children) {
			//		window->is_collapsed = is_collapsed;
			//	}
			//	char buf[64];
			//	char* name = ImGui::FindWindowByID(main_window->im_id)->Name;
			//	int size = group->second.second.children.size();
			//	if (main_window->is_collapsed)
			//		size = invisible_window_group_children[main_window->window_id].children.size();
			//	if (size > 0) {
			//		sprintf_s(buf, "[%d] %s", size + 1, name);
			//	}
			//	else {
			//		strcpy(buf, name);
			//	}

			//	ImGui::PushID(main_window->im_id);

			//	ImGui::BeginDisabled();
			//	ImGui::MenuItem(" - ");
			//	ImGui::EndDisabled();

			//	if (render_window(buf, main_window, is_collapsed ? invisible_window_group_children[main_window->window_id].children : group->second.second.children, is_collapsed, true)) {
			//		if (is_collapsed) {
			//			main_window->is_collapsed = false;
			//			for (auto& child : invisible_window_group_children[main_window->window_id].children) {
			//				child->is_collapsed = false;
			//				invisible_child_windows.erase(child->window_id);
			//			}
			//			for (auto& child : invisible_window_group_children[main_window->window_id].docked_dependents) {
			//				child->is_collapsed = false;
			//				invisible_child_windows.erase(child->window_id);
			//			}
			//			invisible_window_group_children.erase(main_window->window_id);
			//		}
			//		else if (group->first->FocusOrder == lowest_focusid) {
			//			main_window->is_collapsed = true;
			//			for (auto& child : group->second.second.children) {
			//				child->is_collapsed = true;
			//				invisible_child_windows.emplace(child->window_id);
			//			}
			//			for (auto& child : group->second.second.docked_dependents) {
			//				child->is_collapsed = true;
			//				invisible_child_windows.emplace(child->window_id);
			//			}
			//			invisible_window_group_children.emplace(main_window->window_id, group->second.second);
			//		}
			//		else {
			//			ImGui::FocusWindow(group->first);
			//		}
			//	}
			//	static auto empty_list = std::vector<std::shared_ptr<window>>{};
			//	for (auto dependent : group->second.second.dependents) {
			//		auto dep_window = ImGui::FindWindowByID(dependent->im_id);
			//		if (render_window(dep_window->Name, dependent, empty_list, dependent->is_collapsed, false)) {
			//			if (dependent->is_collapsed) {
			//				dependent->is_collapsed = false;
			//			}
			//			else if (dep_window->FocusOrder == lowest_focusid) {
			//				dependent->is_collapsed = true;
			//			}
			//			else {
			//				ImGui::FocusWindow(dep_window);
			//			}
			//		}
			//	}
			//	ImGui::PopID();
			//}

			ImGui::EndMenuBar();
		}
	}
	ImGui::End();
}