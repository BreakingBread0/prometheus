#pragma once
#include "../window_manager/window_manager.h"

class stu_primitive_edit : public window {
	WINDOW_DEFINE_ARG(stu_primitive_edit, "STU", "Edit: Primitive", STU_Primitive);

	inline void render() override {
		if (open_window()) {
			imgui_helpers::editor_primitive(_arg);
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(stu_primitive_edit);
