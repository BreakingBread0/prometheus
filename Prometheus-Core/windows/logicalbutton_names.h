#pragma once
#include "../window_manager/window_manager.h"
#include "../game.h"

class logicalbutton_names : public window {
	WINDOW_DEFINE(logicalbutton_names, "Resource", "Logical Buttons", true);

	//size 0xc3
	LogicalButtonName* logical_buttons = (LogicalButtonName*)(globals::gameBase + 0x17c4990);

	inline void render() override {
		if (open_window()) {
			if (imgui_helpers::beginTable("Logic", { "Name", "ID", "Zero" })) {
				for (int i = 0; i < 0xc3; i++) {
					auto& btn = logical_buttons[i];
					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					ImGui::TextUnformatted(btn.name);

					ImGui::TableNextColumn();
					ImGui::Text("%d (0x%x)", btn.key_id, btn.key_id);

					ImGui::TableNextColumn();
					ImGui::Text("%p", btn.zero);
				}
				ImGui::EndTable();
			}
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(logicalbutton_names);
