#pragma once
#include "../window_manager/window_manager.h"
#include "statescript_window.h"

class statescript_varbag_window : public window {
	WINDOW_DEFINE(statescript_varbag_window, "Statescript", "Varbag Window", false);

	inline void render() override {
		if (open_window()) {
			auto script = get_creator_as<statescript_window>();
			if (script->get_instance_invalid())
				return;

			
		}
		ImGui::End();
	}

	statescript_varbag_window() {
		is_dependent = true;
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(statescript_varbag_window);