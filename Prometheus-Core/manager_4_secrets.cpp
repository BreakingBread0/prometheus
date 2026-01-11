#pragma once
#include "window_manager/window_manager.h"

class template_window : public window {
	WINDOW_ALLOW_AUTO(json_test_window);
	//WINDOW_NO_AUTO(cls);

	WINDOW_NAME("A Widnow");
	WINDOW_CATEGORY("Game");

	inline void render() override {
		if (open_window()) {
			if (ImGui::Button("JSON Constructor test")) {
				char* buffer = new char[0x190];
				typedef __int64(__cdecl* JsonParserC)(__int64*);
				JsonParserC JsonParser = (JsonParserC)(globals::gameBase + 0xe4b510);
				JsonParser((__int64*)buffer);
				imgui_helpers::openCopyWindow((__int64)buffer);
			}
		}
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(template_window);