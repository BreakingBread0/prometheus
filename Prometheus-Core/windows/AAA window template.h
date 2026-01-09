#if false
#error Dont forget to add to window_regs.cpp
#pragma once
#include "../window_manager/window_manager.h"

class template_window : public window {
	WINDOW_DEFINE(json_test_window, "Game", "A Window", true);

	inline void render() override {
		if (open_window()) {
			
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(template_window);

#endif