#pragma once
#include "../window_manager/window_manager.h"
#include "../filetype_library.h"

class filetype_window : public window {
	WINDOW_DEFINE(filetype_window, "Resource", "Filetype Window", true);

	char search_buf[256]{};

	inline void render() override {
		if (open_window()) {
			ImGui::InputText("Search buf", search_buf, 256);

			if (ImGui::Button("Reload")) {
				for (auto item : filetype_library::library) {
					delete[] item.second;
				}
				filetype_library::library.clear();
				filetype_library::init();
			}
			if (imgui_helpers::beginTable("filetypelib", { "Type", "Type u64", "Meaning" })) {
				for (auto& item : filetype_library::library) {
					char search_value[300];
					sprintf_s(search_value, "%p %p %s", item.first, bitswap(item.first - 1), item.second);
					ImGui::PushID(item.first);

					if (strlen(search_buf) > 0 && !icontains(search_value, search_buf)) {
						ImGui::PopID();
						continue;
					}
					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					ImGui::Text("0x%x", item.first);

					ImGui::TableNextColumn();
					ImGui::Text("0x%p", bitswap(item.first - 1));

					ImGui::TableNextColumn();
					ImGui::InputText("", item.second, 256);
					ImGui::PopID();
				}
				ImGui::EndTable();
			}
		}
		ImGui::End();
	}

	inline void preStartInitialize() override {
		filetype_library::init();
	}
	//inline void initialize() override {}
};

WINDOW_REGISTER(filetype_window);
