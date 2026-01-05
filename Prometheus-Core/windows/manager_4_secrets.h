#pragma once
#include "../window_manager/window_manager.h"

class manager_4_secrets : public window {
	WINDOW_DEFINE(manager_4_secrets, "Game", "Mgr4 (Logger) Secrets", true);

	struct Action_vt
	{
		__int64* (__fastcall* action_destruct)(__int64*, char);
		__int64(__fastcall* action_construct_class)(__int64, __int64*);
		const char* (*action_name)();
	};

	struct LinkedListQueueInner
	{
		LinkedListQueueInner* LL_next;
		LinkedListQueueInner* LL_prev;
		Action_vt** data;
	};




	inline void render() override {
		if (open_window()) {
			LinkedListQueueInner* start = *(LinkedListQueueInner**)(globals::gameBase + 0x1859ef0 + 0x38);
			if (imgui_helpers::beginTable("The deep dark secrets of manager 4", { "Location", "Name" })) {
				do {
					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					display_addr((__int64)start);

					ImGui::TableNextColumn();
					if (start) {
						ImGui::Text("%s", (*start->data)->action_name());
					}
				} while ((start = start->LL_prev) != nullptr);
				ImGui::EndTable();
			}
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(manager_4_secrets);
