#pragma once
#include "../window_manager/window_manager.h"
#include "../state_replicator.h"

class awful_demo_server : public window {
	WINDOW_DEFINE(awful_demo_server, "Tools", "Awful Demo Server", true);

	inline void render() override {
		if (open_window()) {
			if (ImGui::Button("Start Server")) {
				state_replicator::start_server();
			}
			ImGui::Text("Is Connected / Server Connected: %s", state_replicator::is_connected ? "Oh god i should have never opened up pandoras box" : "No");
			ImGui::InputText("IP", _connect_to, sizeof(_connect_to));
			if (ImGui::Button("Connect Server")) {
				state_replicator::connect(_connect_to);
			}
		}
		ImGui::End();
	}

private:
	char _connect_to[32]{};
	//inline void preStartInitialize() override {}
	inline void initialize() override {
		strcpy(_connect_to, "127.0.0.1");
	}
};

WINDOW_REGISTER(awful_demo_server);
