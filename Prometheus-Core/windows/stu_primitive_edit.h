#pragma once
#include "../window_manager/window_manager.h"

typedef std::pair<__int64, STUArgumentInfo*> primitive_edit_arg;
class stu_primitive_edit : public window {
	WINDOW_DEFINE_ARG(stu_primitive_edit, "STU", "Edit: Primitive", primitive_edit_arg);

	inline void render() override {
		if (open_window()) {

		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(stu_primitive_edit);
