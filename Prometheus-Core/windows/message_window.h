#pragma once
#include "../window_manager/window_manager.h"

class message_window : public window {
private:
	std::string value, title;
public:
	WINDOW_DEFINE(message_window, "Tools", "Message Window", false);

	message_window(std::string value, std::string title = "") {
		this->value = value;
		this->title = title;
	}
	message_window() {}

	inline void render() override {
		if (open_window(title.empty() ? window_name() : title.c_str(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
			ImGui::TextWrapped("%s", value.c_str());
			if (imgui_helpers::CenteredButton("Close")) {
				queue_deletion();
			}
		}
		ImGui::End();
	}
};

WINDOW_REGISTER(message_window)