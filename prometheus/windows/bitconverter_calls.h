#pragma once
#include "../window_manager/window_manager.h"
#include <MinHook.h>

class bitconverter_calls : public window {
	WINDOW_DEFINE(bitconverter_calls, "Game", "Bitconv calls", true);

	struct call {
		__int64 returnaddress;
		__int64 input;
		__int64 output;
		int occurences = 1;
	};
	static inline std::vector<call> flipbitCalls;

	inline void render() override {
		if (open_window()) {
			ImGui::Text("Dont forget to uncomment MH_EnableHook");
			ImGui::Text("Calls total: %d", flipbitCalls.size());
			if (imgui_helpers::beginTable("Bitconverter Table wohooo", { "Return Address", "Input", "Output", "Occurences" })) {
				for (auto& call : flipbitCalls) {
					ImGui::TableNextRow();
					ImGui::TableNextColumn();

					display_addr(call.returnaddress);
					ImGui::TableNextColumn();

					display_addr(call.input);
					ImGui::TableNextColumn();

					display_addr(call.output);
					ImGui::TableNextColumn();

					ImGui::Text("%d", call.occurences);
				}

				ImGui::EndTable();
			}
		}
		ImGui::End();
	}

	typedef __int64(*flipBits_fn)(__int64);
	static inline flipBits_fn flipBits_orig;
	static inline __int64 flipBits_hook(__int64 input) {
		auto result = flipBits_orig(input);
		call newCall{ (__int64)_ReturnAddress(), input, result, 1 };
		//printf("Bitconverter: Ret %p Input %p Output %p\n", newCall.returnaddress, newCall.input, newCall.output);
		if (flipbitCalls.size() > 0 && 
			flipbitCalls.back().returnaddress == newCall.returnaddress && 
			flipbitCalls.back().output == newCall.output) {
			flipbitCalls.back().occurences++;
		}
		else {
			flipbitCalls.push_back(newCall);
		}
		return result;
	}

	inline void preStartInitialize() override {
		MH_CreateHook((LPVOID)(globals::gameBase + 0x848720), (PVOID*)flipBits_hook, (void**)&flipBits_orig);
		//MH_EnableHook((LPVOID)(globals::gameBase + 0x848720));
	}
	//inline void initialize() override {}
};

WINDOW_REGISTER(bitconverter_calls);