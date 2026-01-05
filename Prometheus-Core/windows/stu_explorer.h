#pragma once
#include "../window_manager/window_manager.h"
#include "../STU.h"

class stu_explorer : public window {
	WINDOW_DEFINE(stu_explorer, "STU", "STU Explorer", true);

public:
	void navigate_to(STUInfo* info, __int64 instance, StatescriptInstance* ss_inst);

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

	struct Item {
		STUInfo* cls = 0;
		__int64 current_instance = 0;
		StatescriptInstance* ss = 0;
		uint followed = 0, followed_index = -1;
	} _current_item;

	bool _show_inheritance = false;
	std::vector<Item> _history{};
	std::vector<Item> _forward_history{};
};

WINDOW_REGISTER(stu_explorer);
