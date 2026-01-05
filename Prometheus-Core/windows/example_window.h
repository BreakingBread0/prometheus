#pragma once
#include "../window_manager/window_manager.h"

class example_window : public window {
	WINDOW_DEFINE(example_window, "Tools", "Example Window", true);

	inline void render() override {
		if (open_window()) {
			ImGui::Text("Hello, World!");
		}
		ImGui::End();
	}
};

WINDOW_REGISTER(example_window);