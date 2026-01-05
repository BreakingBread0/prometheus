#pragma once
#include "../window_manager/window_manager.h"

class statescript_log_window : public window {
	WINDOW_DEFINE(statescript_log_window, "Statescript", "Statescript Log Window", false);

	inline void render() override {
		if (open_window()) {
			
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(statescript_log_window);
