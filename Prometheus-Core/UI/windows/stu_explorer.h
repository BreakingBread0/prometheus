#pragma once
#include "Atlas/STU/RTTI/STUInfo.h"
#include "STU.h"
#include "../window_manager/window_manager.h"
#include "../imgui_helpers.h"
#include "UI/history_helper.h"

using namespace Atlas::STU::RTTI;

struct StatescriptInstance;
class stu_explorer : public window {
	WINDOW_DEFINE(stu_explorer, "STU", "STU Explorer", true);

public:
	void navigate_to(STUInfo* info, __int64 instance, StatescriptInstance* ss_inst);
	void navigate_to_resource(__int64 resource);

	inline void render() override;

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
private:
	static inline std::vector<std::string> s_columns{ "Name", "Value", "Type Flag", "Type", "Offset" };

	bool render_table_noheader() {
		return imgui_helpers::beginTable("##table", s_columns.size());
	}

	bool render_table() {
		return imgui_helpers::beginTable("##table", s_columns);
	}

	void render_resref(STUResourceReference* ref);

	void render_stu(STU_Object value);

	struct FollowedItem {
		STUArgumentInfo* item = nullptr;
		int index = -1;
	};

	void navigate_internal(STUInfo* stu_info, __int64 current_instance, StatescriptInstance* ss, bool remove_root, FollowedItem followItem);
	void navigate_with_history(STUInfo* stu_info, __int64 current_instance, FollowedItem followed_via);

	struct Item {
		STUInfo* stu_info = nullptr;
		__int64 current_instance = 0;
		StatescriptInstance* ss = 0;

		std::vector<FollowedItem> followed_path;
	} _root_item;
	__int64 _root_item_resource = 0;

	bool _show_inheritance = false;
	history_helper<Item> _history{};
};

WINDOW_REGISTER(stu_explorer);
