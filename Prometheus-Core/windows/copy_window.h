#pragma once
#include "../window_manager/window_manager.h"
#include "../game.h"

class copy_window : public window {
private:
	bool isInt = false;
	std::string value;
	__int64 valueInt;
public:
	copy_window() {}
	copy_window(std::string data) {
		value = data;
		isInt = false;
	}
	copy_window(__int64 data) {
		valueInt = data;
		isInt = true;
	}

	WINDOW_DEFINE(copy_window, "Tools", "Copy Window", false);

	inline void render() override {
		if (open_window(nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			if (isInt) {
				__int64 rva = valueInt - globals::gameBase;
				ImGui::Text("DEC: %lld", valueInt);
				ImGui::Text("HEX: %p", valueInt);
				if (rva < globals::gameSize) {
					ImGui::Text("RVA: %p", rva);
				}
				if (ImGui::Button("Copy dec")) {
					copyToClipboard(std::to_string(valueInt));
					queue_deletion();
				}
				ImGui::SameLine();
				if (ImGui::Button("Copy hex")) {
					copyAddress(valueInt);
					queue_deletion();
				}
				if (rva < globals::gameSize) {
					ImGui::SameLine();
					if (ImGui::Button("Copy RVA")) {
						copyAddress(rva);
						queue_deletion();
					}
				}
			}
			else {
				ImGui::Text("Value:");
				ImGui::Text("%s", value.c_str());
				if (ImGui::Button("Copy")) {
					copyToClipboard(value);
					queue_deletion();
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Close"))
				queue_deletion();
		}
		ImGui::End();
	}
};

WINDOW_REGISTER(copy_window)