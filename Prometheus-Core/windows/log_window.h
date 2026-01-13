#pragma once
#include "../window_manager/window_manager.h"
#include <sstream>

class log_window : public window {
	WINDOW_DEFINE_2(log_window, "Game", "LOGS", true, true);

	inline void render() override {
		if (open_window()) {
			ImGui::Checkbox("Log into Console", &s_consoleLog);
			if (ImGui::BeginListBox("##log", ImVec2(-10, -10))) {
				for (int i = 0; i < s_logs.size(); i++) {
					ImGui::PushID(i);
					auto& item = s_logs[i];
					if (ImGui::Selectable(item.c_str(), _selected_id.find(i) != _selected_id.end())) {
						if (_selected_id.size() > 0) {
							if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) {
								int last_id = *_selected_id.rbegin();
								for (int emplace = last_id; emplace <= i; emplace++)
									_selected_id.emplace(emplace);
							}
							else if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl)) {
								if (_selected_id.find(i) != _selected_id.end())
									_selected_id.erase(i);
								else
									_selected_id.emplace(i);
							}
							else {
								_selected_id.clear();
								_selected_id.emplace(i);
							}
						}
						else {
							_selected_id.emplace(i);
						}
					}
					ImGui::PopID();
				}
				
				ImGui::EndListBox();
			}
			if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && _selected_id.size() > 0)
				ImGui::OpenPopup("copy");
			if (ImGui::BeginPopup("copy")) {
				char buf[32];
				sprintf_s(buf, "Copy %d item(s)", _selected_id.size());
				if (ImGui::Button(buf)) {
					std::stringstream str;
					for (auto item : _selected_id) {
						str << s_logs[item] << std::endl;
					}
					imgui_helpers::openCopyWindow(str.str());
				}
				ImGui::EndPopup();
			}
		}
		ImGui::End();
	}

	std::set<int> _selected_id{};
	static inline std::vector<std::string> s_logs{};
	static inline bool s_consoleLog = false;
	//const int MAX_SIZE = 2000;

	static inline void(__fastcall* log_orig_fn)(int prio, char* text);
	static void __fastcall log_fn(int prio, char* text) {
		std::string temp = "[" + std::to_string(prio) + "] " + text;
		if (s_consoleLog) {
			printf("%s\n", temp.c_str());
		}
		s_logs.push_back(std::move(temp));
		log_orig_fn(prio, text);
	}

	inline void preStartInitialize() override {
		MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0x7e5c90), log_fn, (PVOID*)&log_orig_fn));
		MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0x7e5c90)));
	}
	//inline void initialize() override {}
};

WINDOW_REGISTER(log_window);
