#pragma once
#include "../window_manager/window_manager.h"

class imgui_demo_window : public window {
	WINDOW_DEFINE(imgui_demo_window, "Tools", "ImGui Demo Window", true);

	inline void render() override {
		bool open = true;
		ImGui::ShowDemoWindow(&open);
		if (!open)
			this->queue_deletion();
	}
};

WINDOW_REGISTER(imgui_demo_window)