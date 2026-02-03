#include "window_manager.h"
#include <string>
#include "memory.h"
#include <memory>
#include <mutex>
#include <Windows.h>
#include <vector>
#include <map>
#include <regex>
#include <imgui_internal.h>
#include "../windows/message_window.h"
#include "../windows/copy_window.h"
#include "window_regs.cpp"
#include "../windows/radio_selector_window.h"
//#include "../windows/radio_selector_window.h"

bool window_manager::window_id_exists(int window_id) {
	for (auto& window : s_windows) {
		if (window->window_id == window_id)
			return true;
	}
	return false;
}

void window_manager::register_window(window* instance) {
	const char* window_name = instance->window_name();
	auto ptr = std::unique_ptr<window>(instance);
	auto data = window_data{ std::move(ptr), instance->category_name(), window_name};
	s_all_windows().emplace(instance->get_window_type(), std::move(data));
	s_window_categories().emplace(instance->category_name());
}

void window_manager::call_preStartInitialize() {
	for (auto& window : s_all_windows()) {
		window.second.helper_instance->preStartInitialize();
	}
}

std::set<std::string>& window_manager::get_window_categories() {
	return s_window_categories();
}

const std::map<window_type, window_manager::window_data>& window_manager::get_all_windows() {
	return s_all_windows();
}

void window_manager::add_default_window(window_type typ) {
	auto result = s_all_windows().find(typ);
	if (result != s_all_windows().end()) {
		add_window(result->second.helper_instance->create_self()); //leaving parent empty for now.
	}
}

std::shared_ptr<window> window_manager::add_window(std::unique_ptr<window> window_reference, window* from) {
	if (!window_reference)
		return {};
	if (window_reference->is_singleton()) {
		auto result = get_latest_by_type(window_reference->get_window_type());
		if (result)
			return result;
	}
	int window_id = InterlockedAdd(&s_id_counter, 1);
	window_reference->window_id = window_id;
	if (from && !from->this_instance.expired()) //no need if its already expired
		window_reference->created_by = from->this_instance;
	{
		std::shared_ptr<window> new_window(window_reference.release());
		new_window->this_instance = new_window;
		s_window_add_queue.push_back(new_window);
		printf("Creating window %x (%s).\n", window_id, new_window->get_window_type().c_str());
		new_window->initialize();
		return new_window;
	}
}

void window_manager::remove_window_internal(window* window) {
	printf("delete window %d\n", window->window_id);
	auto dependant = window->created_by.lock();
	bool has_dependents = false;
	auto window_reference = window->this_instance.lock();
	std::erase_if(s_windows, [&](auto& pObject) {
		if (pObject->is_dependent && dependant && dependant == pObject->created_by.lock())
			has_dependents = true;
		return pObject->window_id == window->window_id ||
			(pObject->is_dependent && window_reference == pObject->created_by.lock());
	});
	if (dependant)
		dependant->_has_dependents = has_dependents;
}

void window_manager::kill_dependents(window* from) {
	if (!from)
		return;
	auto ref = from->this_instance.lock();
	if (!ref)
		return;
	for (auto& window : s_windows) {
		window->_wants_delete = true;
	}
}

//void window_manager::remove_window(window* window_reference) {
//	std::lock_guard<std::recursive_mutex> lock(s_window_modify_mutex);
//	remove_window_internal(window_reference);
//}

ImGuiWindow* get_leftmost_window(ImGuiDockNode* node, bool start = true) {
	ImGuiWindow* result = nullptr;
	if (start) {
		node = node->HostWindow->DockNodeAsHost;
	}
	if (!node)
		return nullptr;
	if (node->IsSplitNode()) {
		result = get_leftmost_window(node->ChildNodes[0], false);
		if (!result)
			result = get_leftmost_window(node->ChildNodes[1], false);
	}
	if (!result && node->TabBar) {
		result = node->TabBar->Tabs[0].Window;
	}
	return result;
}

ImGuiWindow* get_leftmost_window(window* wind) {
	auto window = ImGui::FindWindowByID(wind->im_id);
	if (!window || !window->DockIsActive || !window->DockNode)
		return nullptr;
	return get_leftmost_window(window->DockNode);
}

void message_kill_window(window* window) {
	imgui_helpers::messageBox(std::format("Window {:d} ({:x}) was killed due to too many exceptions.", window->window_id, window->window_id));
}

void window_manager::call_window_render(window* window) {
	__try {
		window->pre_render();
		window->render();
		window->_exception_counter = 0;
	} __except(EXCEPTION_EXECUTE_HANDLER) {
		printf("Window failed to render: %x\n", window->window_id);
		if (window->_exception_counter++ >= 10) {
			printf("Killing window %x due to exception\n", window->window_id);
			window->_wants_delete = true;
			message_kill_window(window);
		}
	}
}

void window_manager::render_error(const std::string& err) {
	auto renderer = ImguiRenderer::GetInstance();
	renderer->BeginScene();
	renderer->DrawString(ImGui::GetDefaultFont(), "Window manager failed to render!", ImVec2(50, 50), 18, IM_COL32(255, 0, 0, 255), false);
	renderer->DrawString(ImGui::GetDefaultFont(), err, ImVec2(50, 70), 18, IM_COL32(255, 0, 0, 255), false);
	renderer->EndScene();
}

void window_manager::render() {
	__try {
		render_ex();
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		render_error("C handler exception");
	}
}

void DECLSPEC_NOINLINE window_manager::render_ex() {
	try {
		for (int i = 0; i < s_windows.size(); i++) {
			auto window = s_windows[i];
			if (window) {
				if (window->_wants_delete) {
					i--;
					remove_window_internal(window.get());
					continue;
				}
				if (window->is_modal) {
					window->is_dependent = true;
					if (window->is_collapsed) {
						window->queue_deletion();
						continue;
					}
				}
				if (window->is_dependent) {
					auto dependant = window->created_by.lock();
					if (!dependant) {
						window->queue_deletion();
						continue;
					}
					else {
						dependant->_has_dependents = true;
					}
				}
				if (window->is_docked) {
					auto this_root = window->_root_dock.lock();
					if (this_root) {
						if (this_root->is_collapsed) {
							continue;
						}
						if (window->is_dependent) {
							auto creator = window->created_by.lock();
							if (creator) {
								if (this_root != creator->_root_dock.lock()) {
									ImGui::DockContextQueueUndockWindow(ImGui::GetCurrentContext(), ImGui::FindWindowByID(window->im_id));
								}
							}
						}
					}
				}

				if (window->_focus_next_frame && !window->is_collapsed && window->im_id) {
					window->_focus_next_frame = false;
					ImGui::FocusWindow(ImGui::FindWindowByID(window->im_id));
				}

				call_window_render(window.get());

				if (window->_is_focused)
					s_focused_window = window->window_id;

				if (window->_focus_next_frame) {
					window->set_collapsed(false);
				}
				else if (window->is_modal && !window->_is_focused) {
					window->queue_deletion();
				}

				auto imw = ImGui::GetCurrentWindowRead();
				if (window->im_id != 0) {
					if (imw->ID == window->im_id) {
						ImGui::End();
					}
					auto this_window = ImGui::FindWindowByID(window->im_id);
					if (this_window) {
						if (window->_dock_requests.size() > 0) {
							for (auto it = window->_dock_requests.begin(); it != window->_dock_requests.end();) {
								auto other = it->target.lock();
								if (other) {
									if (other->_im_id) {
										auto other_window = ImGui::FindWindowByID(other->im_id);
										if (other_window) {
											ImGui::DockContextQueueDock(ImGui::GetCurrentContext(), this_window, this_window->DockNodeAsHost, other_window, it->direction, it->size_ratio, true);
											window->_dock_requests.erase(it);
											continue;
										}
									}
								}
								it++;
							}
						}
						s_windows_by_im_id[window->im_id] = window;
						auto root = window->_root_dock.lock();
						//printf("visual root valid: %s\n", visual_root_window ? "Yes" : "No");
						if (!root || !root->is_collapsed) {
							auto visual_root_window = get_leftmost_window(window.get());
							window->_root_dock = visual_root_window ? s_windows_by_im_id[visual_root_window->ID] : window;
							root = window->_root_dock.lock();
							//window->_dock_uniqueid = this_window->RootWindowDockTree->ID;
							window->_is_docked = root != window;
						}
						window->_is_collapsed = root->is_collapsed;
						if (window->_wants_collapse) {
							root->_is_collapsed = true;
							window->_wants_collapse = false;
						}
						if (window->_wants_show) {
							root->_is_collapsed = false;
							window->_wants_show = false;
						}
						if (window->_first_render && !window->is_docked && !window->is_modal) {
							auto parent = window->created_by.lock();
							if (parent && parent->im_id) {
								auto parent_window = ImGui::FindWindowByID(parent->im_id);
								auto new_pos = parent_window->Pos;
								new_pos += parent_window->Size / 2;
								auto curr_window = ImGui::FindWindowByID(window->im_id);
								new_pos -= curr_window->Size / 2;
								if (new_pos.x < 20)
									new_pos.x = 20;
								if (new_pos.y < 20)
									new_pos.y = 20;
								curr_window->Pos = new_pos;
							}
						}
						window->_first_render = false;
					}
				}

				if (imw->LastFrameJustFocused) {
					s_latest_windows[window->get_window_type()] = window;
				}
			}
		}
		for (auto window : s_window_add_queue) {
			s_windows.push_back(std::move(window));
		}
		s_window_add_queue.clear();
	}
	catch (const std::exception& ex) {
		render_error("Failed to render because of error: " + std::string(ex.what()));
	}
	catch (...) {
		render_error("Failed to render (other error).");
	}
}

std::shared_ptr<window> window_manager::get_docked(window_type typ, window* from) {
	if (from && !ImGui::IsKeyDown(ImGuiKey_ModCtrl)) {
		for (auto& window : s_windows) {
			auto dock_window = window->get_root_dock().lock();
			if (dock_window) {
				if (window->get_window_type() == typ && from->_root_dock.lock() == dock_window)
					return window;
			}
		}
	}
	return {};
}

std::vector<std::shared_ptr<window>> window_manager::get_all_by_type(window_type typ) {
	std::vector<std::shared_ptr<window>> result;
	for (auto& window : s_windows) {
		if (window->get_window_type() == typ) {
			result.push_back(window);
		}
	}
	return result;
}

//template <typename T>
//std::shared_ptr<window> window_manager::get_latest_or_create(window* from) {
//	for (auto& window : *s_all_windows) {
//		if (dynamic_cast<T*>(window.second.helper_instance.get()) != nullptr) {
//			auto result = get_docked(window.second.helper_instance->get_window_type(), from);
//			if (!result)
//				result = get_latest_by_type(window.first);
//			if (!result) {
//				result = add_window(window.second.helper_instance->create_self(), from);
//			}
//			return result;
//		}
//	}
//	return {};
//}

std::shared_ptr<window> window_manager::get_latest_by_type(window_type typ) {
	auto result = s_latest_windows.find(typ);
	if (result != s_latest_windows.end()) {
		return result->second.lock();
	}
	return std::shared_ptr<window>{};
}

bool window::open_window(const char* title, int flags, ImVec2 size) {
	if (is_collapsed) {
		ImGui::Begin("###collapsed", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs);
		ImGui::SetWindowSize(ImVec2(0, 0));
		ImGui::SetWindowPos(ImVec2(5000, 5000));
		return false;
	}
	bool open = true;
	ImGui::SetNextWindowSize(size, ImGuiCond_Once);
	std::string name;
	if (ImGui::IsKeyDown(ImGuiKey_Menu) && im_id) {
		auto window = ImGui::FindWindowByID(im_id);
		if (window) {
			auto size = window->Size;
			int root_id = _root_dock.lock() ? _root_dock.lock()->window_id : -1;
			name = std::format("{:s} ({:d}x{:d}) {:d} dock: {:d}###{:x}", title == nullptr ? window_name() : title, (int)size.x, (int)size.y, window_id, root_id, window_id);
		}
	}
	if (name.empty()) {
		name = std::format("{:s}###{:x}", title == nullptr ? window_name() : title, window_id);
	}
	auto state = ImGui::Begin(name.c_str(), &open, flags | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
	if (!open) {
		this->queue_deletion();
	}
	if (state) {
		auto window = ImGui::GetCurrentWindowRead();
		this->_im_id = window->ID;
	}
	return state;
}

void window::display_addr(__int64 addr, const char* prepend) {
	ImGui::PushID(addr);
	if (addr > globals::gameBase && addr - globals::gameBase < globals::gameSize) {
		if (prepend)
			ImGui::Text("%s: (RVA) %x", prepend, addr - globals::gameBase);
		else
			ImGui::Text("RVA %x", addr - globals::gameBase);
	}
	else if (prepend)
		ImGui::Text("%s: %p", prepend, addr);
	else
		ImGui::Text("%p", addr);
	ImGui::SameLine();
	if (ImGui::Button(EMOJI_COPY)) {
		imgui_helpers::openCopyWindow(addr);
	}
	ImGui::PopID();
}

void window::display_text(char* text, const char* prepend) {
	ImGui::PushID(reinterpret_cast<int64>(text) + reinterpret_cast<int64>(prepend));

	if (prepend)
		ImGui::Text("%s: %s", prepend, text);
	else
		ImGui::Text("%s", text);

	if (text != nullptr && ImGui::Button("Copy")) {
		imgui_helpers::openCopyWindow(text);
	}

	ImGui::PopID();
}

//std::shared_ptr<window> window::get_root_dock() {
//	auto root_node = get_leftmost_window(this);
//	if (!root_node) {
//		return this_instance.lock();
//	}
//	auto all_windows = window_manager::get_all_windows();
//	if (all_windows)
//}

std::vector<std::shared_ptr<window>> window::get_docked() {
	std::vector<std::shared_ptr<window>> result;
	auto root = _root_dock.lock();
	auto this_inst = this_instance.lock();
	if (root && root == this_inst) {
		for (auto window : window_manager::s_windows) {
			if (window != this_inst && root == window->_root_dock.lock()) {
				result.push_back(window);
			}
		}
	}
	return result;
}

window* window::purge_docking_tree() {
	auto root = _root_dock.lock();
	if (root) {
		ImGui::DockContextClearNodes(ImGui::GetCurrentContext(), root->im_id, true);
	}
	return this;
}

window* window::dock_item_right(window* other, float size_ratio) {
	_dock_requests.push_back({ other->this_instance, ImGuiDir_Right, size_ratio });
	return this;
}

window* window::dock_item_down(window* other, float size_ratio) {
	_dock_requests.push_back({ other->this_instance, ImGuiDir_Down, size_ratio });
	return this;
}

window* window::dock_tab_here(window* other) {
	return this;
}

int argument_offset(STUInfo* stu, int name_hash) {
	while (stu) {
		for (int i = 0; i < stu->ArgsCount; i++) {
			auto arg = stu->Args[i];
			if (arg.Hash == name_hash)
				return arg.Offset;
		}
		stu = stu->Parent;
	}
	return 0;
}

int argument_offset(STUInfo* stu, const char* name) {
	return argument_offset(stu, stringHash(name));
}
