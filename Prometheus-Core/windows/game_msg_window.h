#pragma once
#include "../window_manager/window_manager.h"

class game_msg_window : public window {
	WINDOW_DEFINE(game_msg_window, "Statescript", "Send Game Message", true);

	char gamemsg_str[32]{};
	__int64 gamemsg;
	inline void render() override {
		if (open_window(nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			if (ImGui::InputText("msgid", gamemsg_str, 32)) {
				gamemsg = _strtoi64(gamemsg_str, nullptr, 16) | 0x240000000000000;
			}
			ImGui::Text("Message to publish: %p", gamemsg);
			if (ImGui::Button("Send")) {
				publish_game_msg(gamemsg);
			}
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(game_msg_window);