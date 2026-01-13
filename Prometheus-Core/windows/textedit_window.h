#pragma once
#include "../window_manager/window_manager.h"
#include <functional>

class textedit_window : public window {
	WINDOW_DEFINE(textedit_window, "Tools", "TextEdit Window", false);
	const int buffer = 512;

	inline void render() override {
		if (open_window(nullptr, _collapsed_textbox ? ImGuiWindowFlags_AlwaysAutoResize : 0)) {
			if (ImGui::Button(_collapsed_textbox ? EMOJI_FORWARD : EMOJI_BACK)) {
				_collapsed_textbox = !_collapsed_textbox;
				if (!_collapsed_textbox)
					ImGui::SetWindowSize(ImVec2(500, 200));
			}
			ImGui::SameLine();
			if (ImGui::Button(EMOJI_CHECK)) {
				_save_lambda(std::string(*_text_buffer.get()));
				this->queue_deletion();
			}
			ImGui::SameLine();
			if (ImGui::Button(EMOJI_CROSS)) {
				this->queue_deletion();
			}
			ImGui::SameLine();
			int len = strlen(*_text_buffer.get());
			ImGui::Text("%d/%d", len, _text_buffer_size);
			bool edited = _collapsed_textbox ? ImGui::InputText("##input", *_text_buffer.get(), 999999) :
				ImGui::InputTextMultiline("##input2", *_text_buffer.get(), 999999, ImVec2(-10, -10));
			if (edited) {
				if (_text_buffer_size - len < 10) {
					char* new_buf = new char[len + buffer];
					_text_buffer_size = len + buffer;
					strcpy(new_buf, *_text_buffer.get());
					_text_buffer = std::make_unique<char*>(new_buf);
				}
			}
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}

public:
	textedit_window() { }
	textedit_window(std::string input, std::function<void(std::string)> save_lambda) {
		_text_buffer = std::make_unique<char*>(new char[_text_buffer_size = input.size() + buffer]);
		strcpy(*_text_buffer.get(), input.c_str());
		_save_lambda = save_lambda;
		if (icontains(input, "\n"))
			_collapsed_textbox = false;
	}

private:
	std::unique_ptr<char*> _text_buffer;
	int _text_buffer_size;
	bool _collapsed_textbox = true;
	std::function<void(std::string)> _save_lambda;
};

WINDOW_REGISTER(textedit_window);