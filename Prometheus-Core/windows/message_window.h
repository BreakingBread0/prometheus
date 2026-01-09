#pragma once
#include "../window_manager/window_manager.h"

class message_window : public window {
private:
	std::string title{};
public:
	WINDOW_DEFINE_ARG(message_window, "Tools", "Message Window", std::string);

	message_window(std::string value, std::string title = "") {
		set(value);
		this->title = title;
	}
	message_window() {}

	inline void render() override {
		if (open_window(title.empty() ? window_name() : title.c_str()), ImGuiWindowFlags_AlwaysAutoResize) {
			ImGui::TextWrapped("%s", _arg.c_str());
			if (imgui_helpers::CenteredButton("Close")) {
				queue_deletion();
			}
		}
		ImGui::End();
	}
};

WINDOW_REGISTER(message_window)