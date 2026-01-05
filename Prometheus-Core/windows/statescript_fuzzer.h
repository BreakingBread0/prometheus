#pragma once
#include "../window_manager/window_manager.h"

class statescript_fuzzer : public window {
	WINDOW_DEFINE(statescript_fuzzer, "Statescript", "FUZZZZZZZZZZZZZZ", true);

	std::vector<__int64> valid_resources{};

	inline void render() override {
		if (open_window()) {
			for (auto resid : valid_resources) {
				imgui_helpers::display_type(resid, true);
			}
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	inline void initialize() override {
		typedef __int64(__fastcall* resourceload_fn)(__int64 resource_id);
		resourceload_fn resourceload = (resourceload_fn)(globals::gameBase + 0x9c6470);//0x9c6470 0x9c5de0 0x9c8330
		for (__int64 i = 0; i < 0x10000000; i++) {
			__int64 load_stru = 0;
			__int64 resid = 0x580000000000000 | i;
			load_stru = resourceload(resid);
			bool valid = load_stru != 0;
			/*if (load_stru == 0xFFFFFFFFFFFFFFFFLL
				|| ((unsigned __int64)load_stru & 0xFFFFFFFFFFFFFFC0uLL) == 0
				|| (((unsigned __int8)load_stru ^ *(_BYTE*)(((unsigned __int64)load_stru & 0xFFFFFFFFFFFFFFC0uLL) + 8)) & 0x3F) != 0 ||
				*(__int64*)(load_stru & 0xFFFFFFFFFFFFFFC0uLL) == 0)
				valid = false;*/
			if (valid) {
				printf("%p: VALID\n", resid);
				valid_resources.push_back(resid);
			}
			//0x580000000000000
		}
		//for (__int64 i = 0; i < 0x10000000; i++) {
		//	__int64 load_stru = 0;
		//	__int64 resid = 0x0400000000000000 | i;
		//	load_stru = resourceload(resid);
		//	bool valid = load_stru != 0;
		//	/*if (load_stru == 0xFFFFFFFFFFFFFFFFLL
		//		|| ((unsigned __int64)load_stru & 0xFFFFFFFFFFFFFFC0uLL) == 0
		//		|| (((unsigned __int8)load_stru ^ *(_BYTE*)(((unsigned __int64)load_stru & 0xFFFFFFFFFFFFFFC0uLL) + 8)) & 0x3F) != 0 ||
		//		*(__int64*)(load_stru & 0xFFFFFFFFFFFFFFC0uLL) == 0)
		//		valid = false;*/
		//	if (valid) {
		//		printf("%p: VALID\n", resid);
		//		valid_resources.push_back(resid);
		//	}
		//	//0x580000000000000
		//}
	}
};

WINDOW_REGISTER(statescript_fuzzer);