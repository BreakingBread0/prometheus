#pragma once
#include <memory>
#include "imgui.h"
#include "window_manager.h"

class management_window : public window {
public:
	WINDOW_DEFINE(management_window, "Tools", "Management Window", false);
	management_window() {}

	void render() override;
};

WINDOW_REGISTER(management_window)