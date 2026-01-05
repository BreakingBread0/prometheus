#include "window_manager.h"
#include <string>
#include "memory.h"
#include <memory>
#include <mutex>
#include <windows.h>
#include <vector>
#include <map>
#include <regex>
#include <imgui_internal.h>
#include "../windows/message_window.h"
#include "../windows/copy_window.h"
#include "window_regs.cpp"
#include "../stringhash_library.h"
#include "../filetype_library.h"
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
	if (s_all_windows == nullptr)
		s_all_windows = new std::map<window_type, window_data>;

	if (s_window_categories == nullptr)
		s_window_categories = new std::set<std::string>;

	/*
	Dear Microsoft,
	
	Why does this code suddenly try to EXECUTE "instance" (literally jumps to the address of instance)
		_all_windows->emplace(instance->get_window_type(), window_data{ std::unique_ptr<window>(instance), instance->category_name(), instance->window_name() });
	???????????

	Kind Reagards,
	Someone who has had enough of this dumpster fire of a compiler and will switch to NixOS and clang.
	Update: after one month of pain while trying to switch i will do that laterTM
	*/

	const char* window_name = instance->window_name();
	//printf("Dont optimize me away please: %s\n", window_name);
	auto ptr = std::unique_ptr<window>(instance);
	auto data = window_data{ std::move(ptr), instance->category_name(), window_name};
	s_all_windows->emplace(instance->get_window_type(), std::move(data));
	s_window_categories->emplace(instance->category_name());
	//printf("Window manager: registered window '%s' (Category '%s').\n", instance->window_name(), instance->category_name());
}

void window_manager::call_preStartInitialize() {
	for (auto& window : *s_all_windows) {
		window.second.helper_instance->preStartInitialize();
	}
}

std::set<std::string>& window_manager::get_window_categories() {
	return *s_window_categories;
}

const std::map<window_type, window_manager::window_data>& window_manager::get_all_windows() {
	return *s_all_windows;
}

void window_manager::add_default_window(window_type typ) {
	auto result = s_all_windows->find(typ);
	if (result != s_all_windows->end()) {
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

void window_manager::render() {
	for (int i = 0; i < s_windows.size(); i++) {
		auto window = s_windows[i];
		if (window) {
			if (window->_wants_delete) {
				i--;
				remove_window_internal(window.get());
				continue;
			}
			if (window->is_dependent) {
				auto dependant = window->created_by.lock();
				if (!dependant) {
					i--;
					remove_window_internal(window.get());
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

			if (window->_focus_next_frame) {
				window->set_collapsed(false);
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
					if (window->_first_render && !window->is_docked) {
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
		this->_is_latest = true;
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
	if (prepend)
		ImGui::Text("%s: %s", prepend, text);
	else
		ImGui::Text("%s", text);
	ImGui::SameLine();
	if (text != nullptr && ImGui::Button("Copy")) {
		imgui_helpers::openCopyWindow(text);
	}
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

}

STUArgumentInfo* argument_info(STUInfo* stu, int name_hash) {
	while (stu) {
		for (int i = 0; i < stu->argument_count; i++) {
			auto arg = &stu->arguments[i];
			if (arg->name_hash == name_hash)
				return arg;
		}
		stu = stu->base_stu;
	}
	return 0;
}
STUArgumentInfo* argument_info(STUInfo* stu, const char* name) {
	return argument_info(stu, stringHash(name));
}

int argument_offset(STUInfo* stu, int name_hash) {
	while (stu) {
		for (int i = 0; i < stu->argument_count; i++) {
			auto arg = stu->arguments[i];
			if (arg.name_hash == name_hash)
				return arg.offset;
		}
		stu = stu->base_stu;
	}
	return 0;
}
int argument_offset(STUInfo* stu, const char* name) {
	return argument_offset(stu, stringHash(name));
}

namespace imgui_helpers {
	void openCopyWindow(std::string value) {
		window_manager::add_window(new copy_window(value));
	}
	void openCopyWindow(__int64 value) {
		window_manager::add_window(new copy_window(value));
	}
	void messageBox(std::string data, std::string title, window* window) {
		window_manager::add_window(new message_window(data, title), window);
	}
	void messageBox(std::string data, window* window) {
		window_manager::add_window(new message_window(data, ""), window);
	}

	template <typename T>
	void modifiable_int(const char* text, T* value, window* window, const char* format_specifier, const char* format_specifier_2 = nullptr) {
		ImGui::PushID((__int64)value);
		if (TooltipButton(EMOJI_COPY, "Copy Address / Value")) {
			std::vector<std::string> choices = { "Copy Address", "Copy Value" };
			if (format_specifier_2) {
				choices.push_back("Copy Value (Hexadecimal)");
			}
			window_manager::add_window(new radio_selector_window("What to copy?", choices, [value, format_specifier, format_specifier_2](int sel) {
				if (sel == 0) {
					openCopyWindow((__int64)value);
				}
				else {
					char buf[32];
					sprintf_s(buf, sel == 1 ? format_specifier : format_specifier_2, *value);
					openCopyWindow(buf);
				}
			}));
		}

		bool alt = ImGui::IsKeyDown(ImGuiKey_Menu);
		if (alt) {
			ImGui::SameLine();
			if (ImGui::Button("- 0x100")) {
				*value -= 0x100;
			}
			ImGui::SameLine();
			if (ImGui::Button("- 0x10")) {
				*value -= 0x10;
			}
			ImGui::SameLine();
			if (ImGui::Button("- 1")) {
				*value -= 1;
			}
			ImGui::SameLine();
			if (ImGui::Button("0")) {
				*value = 0;
			}
			ImGui::SameLine();
			if (ImGui::Button("+ 1")) {
				*value += 1;
			}
			ImGui::SameLine();
			if (ImGui::Button("+ 0x10")) {
				*value += 0x10;
			}
			ImGui::SameLine();
			if (ImGui::Button("+ 0x100")) {
				*value += 0x100;
			}
		}

		ImGui::SameLine();
		char buf[32];
		if (format_specifier_2) {
			sprintf_s(buf, "%%s: %s (%s)", format_specifier, format_specifier_2);
			ImGui::Text(buf, text, *value, *value);
		} else {
			sprintf_s(buf, "%%s: %s", format_specifier);
			ImGui::Text(buf, text, *value);
		}

		ImGui::PopID();
	}

	void modifiable(const char* text, char* value, window* window) {
		modifiable_int<char>(text, value, window, "%hhd", "%hhx");
	}

	void modifiable(const char* text, short* value, window* window) {
		modifiable_int<short>(text, value, window, "%hd", "%hx");
	}

	void modifiable(const char* text, int* value, window* window) {
		modifiable_int<int>(text, value, window, "%d", "%x");
	}

	void modifiable(const char* text, uint* value, window* window) {
		modifiable_int<uint>(text, value, window, "%d", "%x");
	}

	void modifiable(const char* text, float* value, window* window) {
		modifiable_int<float>(text, value, window, "%f");
	}

	void modifiable(const char* text, __int64* value, window* window) {
		modifiable_int<__int64>(text, value, window, "%lld", "%p");
	}

	void modifiable(const char* text, double* value, window* window) {
		modifiable_int<double>(text, value, window, "%Lf");
	}

	void modifiable(const char* text, Vector4* value, window* window) {
		ImGui::Text("Vector4 <%s>:", text);
		modifiable_int<float>("X", &value->X, window, "%f");
		modifiable_int<float>("Y", &value->Y, window, "%f");
		modifiable_int<float>("Z", &value->Z, window, "%f");
		modifiable_int<float>("W", &value->W, window, "%f");
	}
	void modifiable(const char* text, Matrix4x4* value, window* window) {
		ImGui::Text("Matrix4x4 <%s>:", text);
		ImGui::Text("%f %f %f %f", value->row_1.X, value->row_1.Y, value->row_1.Z, value->row_1.W);
		ImGui::Text("%f %f %f %f", value->row_2.X, value->row_2.Y, value->row_2.Z, value->row_2.W);
		ImGui::Text("%f %f %f %f", value->row_3.X, value->row_3.Y, value->row_3.Z, value->row_3.W);
		ImGui::Text("%f %f %f %f", value->row_4.X, value->row_4.Y, value->row_4.Z, value->row_4.W);
	}

	template <typename T>
	void modifiable(const char* text, teList<T>* value, window* window) {
		ImGui::Text("%s array: (%d/%d items)", text, value->num, value->max);
		int i = 0;
		ImGui::Indent();
		for (auto& item : *value) {
			ImGui::BulletText("%d", i++);
			ImGui::SameLine();
			modifiable("", &item, window);
		}
		ImGui::Unindent();
	}

	bool display_cv(STUConfigVar* cv, StatescriptInstance* ss, STUArgumentInfo* arg_info, bool display_logicalButton) {
		ImGui::PushID(cv);
		if (arg_info) {
			display_type(arg_info->name_hash, false, true, false);
			ImGui::Indent();
		}
		if (cv->base.valid()) {
			if (IsBadReadPtr((void*)cv->base.vfptr, 8)) {
				ImGui::Text("Invalid");
				if (arg_info)
					ImGui::Unindent();
				ImGui::PopID();
				return false;
			}
			uint stu_hash = cv->base.vfptr->GetSTUInfo()->name_hash;
			display_type(stu_hash, true);
			StatescriptPrimitive value{};
			if (cv->get_value(ss, &value)) {
				ImGui::PushFont(imgui_helpers::BoldFont);
				ImGui::Text("value:");
				ImGui::PopFont();
				ImGui::SameLine();
				display_logicalButton |= stu_hash == STU_NAME::STUConfigVarLogicalButton;
				if (value.type == StatescriptPrimitive_INT64 || value.type == StatescriptPrimitive_INT) {
					if (display_logicalButton) {
						ImGui::Text("%d (%x): %s", value.value, value.value, LogicalButtonById(value.value)->name);
					}
					else {
						display_type(value.value, true, true, false);
					}
				}
				else {
					ImGui::TextUnformatted(value.get_value_str().c_str());
				}
			}
			if (cv->is_dynamic()) {
				ImGui::TextUnformatted("Path:");
				ImGui::SameLine();
				item_path_print((STUConfigVarDynamic*)cv);
			}
		}
		if (arg_info)
			ImGui::Unindent();
		ImGui::PopID();
		return false;
	}

	bool display_type(__int64 type, bool color, bool edit, bool hash_show, std::set<__int64> ptrs) {
		ImGui::PushID(type);
		if (ptrs.find(type) != ptrs.end()) {
			return false;
		}
		ptrs.emplace(type);
		bool hover = false;
		if (edit) {
			if (ImGui::Button(EMOJI_EDIT)) {
				window_manager::add_window(new comments_window(type));
			}
			ImGui::SameLine();
			if (ImGui::Button(EMOJI_COPY)) {
				imgui_helpers::openCopyWindow(type);
			}
			hover |= ImGui::IsItemHovered();
			ImGui::SameLine();
		}
		if (hash_show) {
			if (type > UINT_MAX)
				ImGui::Text("%llx", type);
			else
				ImGui::Text("%x", type);
			hover |= ImGui::IsItemHovered();
			ImGui::SameLine();
		}

		if (type < UINT_MAX) {
			auto found_dehash = allmighty_hash_lib::hashes.find((uint)type);
			if (found_dehash != allmighty_hash_lib::hashes.end()) {
				if (color) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(100, 255, 0, 255));
					ImGui::TextUnformatted(found_dehash->second.c_str());
					ImGui::PopStyleColor();
				}
				else {
					ImGui::Text("[%s]", found_dehash->second.c_str());
				}
				hover |= ImGui::IsItemHovered();
				ImGui::SameLine();
				hash_show = true;
			}
		}
		else {
			int file_type = bitswap(16 * (type & 0xFFFF000000000000uLL)) + 1;
			if (filetype_library::library.find(file_type) != filetype_library::library.end()) {
				auto type_str = filetype_library::library[file_type];
				if (color) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(100, 255, 0, 255));
					ImGui::TextUnformatted(type_str);
					ImGui::PopStyleColor();
				}
				else {
					ImGui::Text("[%s]", type_str);
				}
				hover |= ImGui::IsItemHovered();
				ImGui::SameLine();
			}
		}

		auto comment = allmighty_hash_lib::comments.find(type);
		if (comment != allmighty_hash_lib::comments.end()) {
			/*std::string cmt = comment->second;
			std::regex regex("0x([0-9a-f]{8,16})", std::regex_constants::ECMAScript | std::regex_constants::icase);
			auto beg = std::sregex_iterator(cmt.begin(), cmt.end(), regex);
			auto end = std::sregex_iterator();*/
			auto write_text = [&hover, color](std::string str, const char* pre = "[", const char* post = "]") {
				if (color) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 200, 0, 255));
					ImGui::TextUnformatted(str.c_str());
					ImGui::PopStyleColor();
				}
				else {
					ImGui::Text("%s%s%s", pre, str.c_str(), post);
				}
				hover |= ImGui::IsItemHovered();
			};

			/*if (beg != end) {
				int latest_pos = 0;
				for (std::sregex_iterator i = beg; i != end; ++i) {
					std::smatch match = *i;
					auto str = match.str();
					__int64 match_id = _strtoi64(str.data(), nullptr, 16);

					ImGui::PushID(i->position());

					if (ImGui::Button(EMOJI_COPY)) {
						imgui_helpers::openCopyWindow(match_id);
					}
					ImGui::SameLine();

					write_text(cmt.substr(latest_pos, match.position()), "[", "");
					latest_pos = match.position() + match.length();
					ImGui::SameLine();
					display_type(match_id, color, edit, hash_show, ptrs);
					ImGui::SameLine();

					ImGui::PopID();
				}
				write_text(cmt.substr(latest_pos), "");
			}
			else {
			}*/
			write_text(comment->second);
		}
		else if (!hash_show) {
			if (type > UINT_MAX)
				ImGui::Text("%p", type);
			else
				ImGui::Text("%llx", type);
			hover |= ImGui::IsItemHovered();
		}
		else {
			ImGui::NewLine();
		}
		ImGui::PopID();
		return hover;
	}

	bool display_type(__int64 type, bool color, bool edit, bool hash_show) {
		return display_type(type, color, edit, hash_show, {});
	}

	void item_path_print(STUConfigVarDynamic* cv) {
		bool first = true;
		for (int i = 0; i < cv->item_location.count(); i++) {
			if (!first) {
				ImGui::TextUnformatted("=>");
				ImGui::SameLine();
			}
			first = false;
			//printf("%p %p\n", item_location.list(), this);
			auto item = cv->item_location.list()[i];
			display_type(item.resource_id, true, true, false);
			ImGui::SameLine();
		}
		ImGui::NewLine();
	}

	void render_primitive(STU_Primitive value, uint32 hash) {
		__try {
			switch (hash) {
			case STU_NAME::Primitive::teMtx43A:
			case STU_NAME::Primitive::teVec3A:
			case STU_NAME::Primitive::teVec2:
			case STU_NAME::Primitive::teVec3:
			case STU_NAME::Primitive::teVec4:
			case STU_NAME::Primitive::teQuat:
			case STU_NAME::Primitive::teColorRGB:
			case STU_NAME::Primitive::teColorRGBA:
			case STU_NAME::Primitive::DBID:
			case STU_NAME::Primitive::teUUID:
			case STU_NAME::Primitive::teStructuredDataDateAndTime:
				ImGui::Text("ImplementMe!");
				break;
			case STU_NAME::Primitive::teString: {
				auto str = value.get_value<const char*>();
				ImGui::Text("%s", strlen(str) == 0 ? "(empty)" : str);
			}
											  break;
			case STU_NAME::Primitive::s16:
				ImGui::Text("%hd", value.get_value<short>());
				break;
			case STU_NAME::Primitive::s32:
				ImGui::Text("%d", value.get_value<int>());
				break;
			case STU_NAME::Primitive::s64:
				ImGui::Text("%lld", value.get_value<__int64>());
				break;
			case STU_NAME::Primitive::u8:
				ImGui::Text("%hhd (%hhx)", value.get_value<char>(), value.get_value<char>());
				break;
			case STU_NAME::Primitive::u16:
				ImGui::Text("%hd (%hx)", value.get_value<ushort>(), value.get_value<ushort>());
				break;
			case STU_NAME::Primitive::u32:
				ImGui::Text("%d (%x)", value.get_value<uint32>(), value.get_value<uint32>());
				break;
			case STU_NAME::Primitive::teEntityID:
				ImGui::Text("%x", value.get_value<uint32>());
				break;
			case STU_NAME::Primitive::u64:
				ImGui::Text("%lld (%p)", value.get_value<__int64>(), value.get_value<__int64>());
				break;
			case STU_NAME::Primitive::f32:
				ImGui::Text("%f", value.get_value<float>());
				break;
			case STU_NAME::Primitive::f64:
				ImGui::Text("%lf", value.get_value<double>());
				break;
			default:
				ImGui::TextUnformatted("Unknown Primitive");
				break;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			ImGui::Text("Exception while rendering primitive.");
		}
	}

	void editor_primitive(STU_Primitive value, uint32 hash) {
		ImGui::PushID(value.value);
		switch (hash) {
		case STU_NAME::Primitive::teMtx43A:
		case STU_NAME::Primitive::teVec3A:
		case STU_NAME::Primitive::teVec2:
		case STU_NAME::Primitive::teVec3:
		case STU_NAME::Primitive::teVec4:
		case STU_NAME::Primitive::teQuat:
		case STU_NAME::Primitive::teColorRGB:
		case STU_NAME::Primitive::teColorRGBA:
		case STU_NAME::Primitive::DBID:
		case STU_NAME::Primitive::teUUID:
		case STU_NAME::Primitive::teStructuredDataDateAndTime:
			ImGui::Text("ImplementMe!");
			ImGui::PopID();
			return;
		case STU_NAME::Primitive::teString: {
			char buf[256];
			auto str = value.get_value<const char*>();
			strcpy_s(buf, str);
			if (ImGui::InputText("", buf, sizeof(buf)))
				value.set_value(buf);
			ImGui::PopID();
			return;
		}
		case STU_NAME::Primitive::f32:
			ImGui::InputFloat("", (float*)value.value);
			ImGui::PopID();
			return;
		case STU_NAME::Primitive::f64:
			ImGui::InputDouble("", (double*)value.value);
			ImGui::PopID();
			return;
		case STU_NAME::Primitive::s64:
		case STU_NAME::Primitive::u64: {
			__int64 temp = value.get_value<uint64>();
			if (imgui_helpers::InputHex("", &temp))
				value.set_value<__int64>(temp);
			ImGui::PopID();
			return;
		}
		}
		auto storage = ImGui::GetStateStorage();
		ImGuiID id = ImGui::GetActiveID();
		auto hex = storage->GetBoolRef(id, false);
		if (hash != STU_NAME::Primitive::f32 && hash != STU_NAME::Primitive::f64) {
			ImGui::Checkbox("Hex", hex);
			ImGui::SameLine();
		}
		switch (hash) {
		case STU_NAME::Primitive::u8: {
			int temp = value.get_value<unsigned char>();
			if (hex ? imgui_helpers::InputHex("", &temp) : ImGui::InputInt("", &temp))
				value.set_value<unsigned char>(temp);
			break;
		}
		case STU_NAME::Primitive::s16: {
			int temp = value.get_value<short>();
			if (hex ? imgui_helpers::InputHex("", &temp) : ImGui::InputInt("", &temp))
				value.set_value<short>(temp);
			break;
		}
		case STU_NAME::Primitive::u16: {
			int temp = value.get_value<ushort>();
			if (hex ? imgui_helpers::InputHex("", &temp) : ImGui::InputInt("", &temp))
				value.set_value<ushort>(temp);
			break;
		}
		case STU_NAME::Primitive::s32: {
			int temp = value.get_value<int>();
			if (hex ? imgui_helpers::InputHex("", &temp) : ImGui::InputInt("", &temp))
				value.set_value<int>(temp);
			break;
		}
		case STU_NAME::Primitive::u32: {
			int temp = value.get_value<uint>();
			if (hex ? imgui_helpers::InputHex("", &temp) : ImGui::InputInt("", &temp))
				value.set_value<uint>(temp);
			break;
		}
		}
		ImGui::PopID();
	}

	ImFont* BoldFont = nullptr;
}