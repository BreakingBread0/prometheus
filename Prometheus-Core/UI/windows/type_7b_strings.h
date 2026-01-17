#pragma once
#include "../window_manager/window_manager.h"
#include "copy_window.h"
#include <sstream>

class type_7b_strings : public window {
	WINDOW_DEFINE(type_7b_strings, "Resource", "Type 7B Strings", true);

	struct string_resource {
		__int64 id;
		const char* string;
	};
	std::vector<string_resource> _strings{};
	__int64 to = 0x100000000;

	inline void render() override {
		if (open_window()) {
			try {
				static std::string toStr = "0x10000000";
				toStr.reserve(20);
				if (ImGui::InputText("To", toStr.data(), 20)) {
					to = std::stoll(toStr, nullptr, 16);
				}
				
				ImGui::Text("To: %x %d", to, to);
				if (ImGui::Button("Refresh")) {
					_strings.clear();
					for (int i = 0; i < to; i++) {
						//typedef const char** (*fn)(__int64);
						//fn string_from_resource = (fn)(globals::gameBase + 0x9c6470);
						auto resource_string = GetUXDisplayString(0xDE0000000000000 | i);
						if (resource_string)
							_strings.push_back({ 0xDE0000000000000 | i, resource_string });
					}
					/*for (auto id : ids) {
						typedef const char** (*fn)(__int64);
						fn string_from_resource = (fn)(globals::gameBase + 0x9c6470);
						auto resource_string = string_from_resource(id);
						if (resource_string)
							_strings.push_back({ id, *resource_string });
					}*/
				}
			}
			catch (...) {}

			if (ImGui::Button("Copy All")) {
				std::stringstream output;
				for (auto& str : _strings) {
					output << std::hex << str.id << "   " << str.string << std::endl;
				}
				window_manager::add_window(new copy_window(output.str()), this);
			}

			bool emplace = ImGui::Button("Emplace ALL");

			if (imgui_helpers::beginTable("Resource Strings", { "Type", "Bytes", "String" })) {
				for (auto& str : _strings) {
					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					imgui_helpers::display_type(str.id, true);

					ImGui::TableNextColumn();
					/*char* bytes = (char*)&str.id;
					char buf[128];
					sprintf_s(buf, "%02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX", bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7]);
					ImGui::Text("%s", buf);
					ImGui::SameLine();
					if (ImGui::Button(slug("Copy", str.id | 0x100000).c_str())) {
						window_manager::add_window(new copy_window(buf));
					}*/

					ImGui::TableNextColumn();
					ImGui::Text("%s", str.string);

					if (emplace) {
						stringhash_library::add_comment(str.id, str.string);
					}
				}
				ImGui::EndTable();
			}

		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(type_7b_strings);
