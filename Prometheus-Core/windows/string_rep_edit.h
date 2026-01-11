#pragma once
#include "../window_manager/window_manager.h"
#include "../game.h"

class string_rep_edit : public window {
	WINDOW_DEFINE(string_rep_edit, "Tools", "string_rep edit", false);

	string_rep* _str;

	inline void render() override {
		if (open_window()) {
			if (IsBadReadPtr(_str, sizeof(string_rep))) {
				ImGui::Text("String became invalid.");
			}
			else {
				ImGui::Text("Size: %d/%d", _str->actual_size, _str->max_size);
				if (ImGui::InputText("", _str->get(), _str->max_size)) {
					_str->actual_size = strlen(_str->get());
				}
				if (ImGui::Button("Extend to 512")) {
				}
			}
			if (ImGui::Button("Close"))
				queue_deletion();
		}
		ImGui::End();
	}

public:
	string_rep_edit() { }
	string_rep_edit(string_rep* str) {
		_str = str;
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(string_rep_edit);
