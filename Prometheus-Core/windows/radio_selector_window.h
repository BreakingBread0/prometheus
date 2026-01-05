#pragma once
#include "../window_manager/window_manager.h"
#include <vector>
#include <functional>

class radio_selector_window : public window {
	WINDOW_DEFINE(radio_selector_window, "Tools", "Radio Selector Window", false);

	inline void render() override {
		if (open_window(_title.c_str(), ImGuiWindowFlags_AlwaysAutoResize)) {
			for (int i = 0; i < _choices.size(); i++) {
				auto& choice = _choices[i];
				ImGui::PushID(choice.c_str());
				
				ImGui::RadioButton(choice.c_str(), &_choice, i);

				ImGui::PopID();
			}
			if (ImGui::Button(EMOJI_CHECK)) {
				_callback(_choice);
				this->queue_deletion();
			}
			ImGui::SameLine();
			if (ImGui::Button(EMOJI_CROSS)) {
				this->queue_deletion();
			}
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}

public:
	radio_selector_window() {

	}

	radio_selector_window(std::string title, std::vector<std::string> choices, std::function<void(int)> callback) :
		_choices(choices),
		_title(title),
		_callback(callback) { }

private:
	std::string _title;
	int _choice;
	std::vector<std::string> _choices;
	std::function<void(int)> _callback;
};

WINDOW_REGISTER(radio_selector_window);
