#pragma once
#include "../window_manager/window_manager.h"
#include "../entity_admin.h"

class statescript_list : public window {
	WINDOW_DEFINE(statescript_list, "Statescript", "Statescript List", true);

	void render() override;

	void display_compo(Component_23_Statescript* ss);

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
	Component_23_Statescript* _ss = nullptr;
private:
	int _curr_inst_id = 0;
	__int64 _load_script = 0;

	void list_script(StatescriptInstance* script);
};

WINDOW_REGISTER(statescript_list);