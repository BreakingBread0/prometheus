#pragma once
#include "../window_manager/window_manager.h"
#include "../entity_admin.h"
#include "../search_helper.h"

class entity_window : public window {
	WINDOW_DEFINE(entity_window, "ECS", "Entities", true);

	entity_window* set_entadmin(EntityAdminBase* ent_admin);
	entity_window* nav_to_ent(Entity* ent);

	void render() override;

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}

private:
	EntityAdminBase* _entity_admin = 0;
	bool _is_gameEA;
	Entity* _nav_to_ent = 0;
	search_helper_imgui _search;
	bool _search_components = false;

	void search_compo54(EntityAdminBase* ea);
	void render_entity(Entity* ent, int depth = 0);
};

WINDOW_REGISTER(entity_window);