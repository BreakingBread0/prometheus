#pragma once
#include "game.h"
#include "../window_manager/window_manager.h"
#include "UI/imgui_helpers.h"

class gamestate_window : public window {
	WINDOW_DEFINE(gamestate_window, "Game", "Game State Info", true);

	inline void render() override {
		if (open_window()) {
			if (imgui_helpers::beginTable("asdf", {"subscriber", "arg"}))
			{
				for (auto& item : GetGameState()->subscribers)
				{
					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					display_addr(item.callback);

					ImGui::TableNextColumn();
					display_addr(item.arg);
				}

				ImGui::EndTable();
			}
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(gamestate_window);
