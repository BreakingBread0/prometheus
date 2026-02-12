#include "entity_window.h"
#include "UI/window_manager/window_manager.h"
#include "entity_admin.h"
//#include "statescript_graph_window.h"
#include "statescript_list.h"
#include "local_player_component_insights.h"
#include "stringhash_library.h"
#include "pvpgame_explorer.h"
#include "entity_bounds_renderer.h"
#include "stu_explorer.h"
#include "movstate_visualizer.h"
#include "STU_Editable.h"
#include "Components/Component_4_Model.h"

struct MovementState;

entity_window* entity_window::set_entadmin(EntityAdminBase* ent_admin) {
	_entity_admin = ent_admin;
	return this;
}

entity_window* entity_window::nav_to_ent(Entity* ent) {
	_entity_admin = ent->entity_admin_backref;
	_nav_to_ent = ent;
	return this;
}

void entity_window::search_compo54(EntityAdminBase* ea) {
	if (!ea)
		return;
	for (int i = 0; i < ea->component_iterator[0x54].component_list.num; i++) {
		Component_54_Lobbymap* compo = (Component_54_Lobbymap*)ea->component_iterator[0x54].component_list.ptr[i];
		ImGui::PushID(compo);
		ImGui::Indent();
		if (ImGui::RadioButton("Embedded Compo54 Admin", _entity_admin == compo->embedded_game_ea)) {
			_entity_admin = compo->embedded_game_ea;
		}
		search_compo54(compo->embedded_game_ea);
		ImGui::Unindent();
		ImGui::PopID();
	}
}

void entity_window::render() {
	if (open_window(nullptr, 0, ImVec2(1300, 700))) {
		if (ImGui::BeginChild("radios", ImVec2(400, 100), ImGuiChildFlags_ResizeY | ImGuiChildFlags_AutoResizeY)) {
			if (ImGui::RadioButton("lobby_entity_admin", _entity_admin == LobbyEntityAdmin())) {
				_entity_admin = LobbyEntityAdmin();
			}
			search_compo54(LobbyEntityAdmin());
			if (ImGui::RadioButton("game_entity_admin", _entity_admin == GameEntityAdmin())) {
				_entity_admin = GameEntityAdmin();
			}
		}
		_is_gameEA = _entity_admin == GameEntityAdmin();
		ImGui::EndChild();
		ImGui::SameLine();
		if (ImGui::BeginChild("search", ImVec2(-10, 100), ImGuiChildFlags_ResizeY | ImGuiChildFlags_AutoResizeY)) {
			display_addr((__int64)_entity_admin);
			_search.search_box("Search Entity");
			ImGui::SameLine();
			if (ImGui::Checkbox("Compo IDs", &_search_components)) {
				_search.set_needs_haystack();
			}
			if (_is_gameEA) {
				ImGui::Text("Local Entity: %x", _entity_admin->local_entid);
				ImGui::SameLine();
				if (ImGui::Button("Nav")) {
					_nav_to_ent = _entity_admin->getLocalEnt();
				}
				// if (ImGui::Button("Unload World (broken)")) {
				// 	auto sys = get_system27_WorldEngineSystem(GameEntityAdmin());
				// 	sys->wanted_map_id = 0;
				// 	for (auto ent : *_entity_admin) {
				// 		if (ent->resload_entry->valid() && ent->resload_entry->align()->resource_id > 0) {
				// 			_entity_admin->delEnt(ent);
				// 		}
				// 	}
				// 	sys->field_50 = 1;
				// 	sys->worldloaded_mby = 0;
				// 	sys->gamemodeloaded_mby = 0;
				// 	if (sys->world_state >= 2) {
				// 		if ((sys->world_state - 4) <= 1) {
				// 			while (sys->world_state != 2) {
				// 				((void(*)(System_27_WorldEngineSystem*))(globals::gameBase + 0xc38b00))(sys);
				// 			}
				// 		}
				// 		else {
				// 			((void(*)(System_27_WorldEngineSystem*))(globals::gameBase + 0xc3b160))(sys);
				// 		}
				// 	}
				// 	GetGameState()->current_state = GameState::VIGS_5_STARTUP;
				// }
			}
		}
		ImGui::EndChild();
		//search_compo54(*(EntityAdminBase**)(globals::gameBase + 0x17b7f90)); //Never in GameEntityAdmin
		/*if (ImGui::RadioButton("entity_admin_embai", &_ent_admin, 2)) {
			auto temp = *(__int64*)(globals::gameBase + 0x17b7fc0);
			if (temp)
				entity_admin = *(EntityAdminBase**)(entity_admin + 0xB0);
		}*/
		if (ImGui::BeginChild("ents", ImVec2(-10, -10), ImGuiChildFlags_Borders)) {
			if (_entity_admin && !IsBadReadPtr((LPVOID)_entity_admin, 8)) {
				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10.0f, 10.0f));
				if (imgui_helpers::beginTable("Entities", { "ID", "Addr", "Components" })) {
					for (auto entity : *_entity_admin) {
						if (entity->parent == nullptr)
							render_entity(entity);
					}
					ImGui::EndTable();
				}
				ImGui::PopStyleVar();
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void entity_window::render_resload(MisalignedResourceLoadEntry* res, int item) {
	ImGui::PushID("resload");
	ImGui::PushID(item);
	if (res->valid()) {
		auto resload_ent = res->align();
		if (resload_ent->valid()) {
			ImGui::Text("%d:", item);
			ImGui::SameLine();
			if (imgui_helpers::TooltipButton(EMOJI_SHARE, "Open in stu explorer")) {
				stu_explorer::get_latest_or_create(this)->navigate_to_resource(resload_ent->resource_id);
			}
			ImGui::SameLine();
			imgui_helpers::display_type(resload_ent->resource_id, true, true, false);
		}
	}
	ImGui::PopID();
	ImGui::PopID();
}

void entity_window::render_entity(Entity* ent, int depth) {
	if (_search.needs_haystack()) {
		if (_search_components) {
			for (int i = 0; i < ent->component_list.num - 1; i++) {
				_search.haystack_hex(ent->component_list.ptr[i]->component_id);
			}
		}
		else {
			_search.haystack_hex(ent->entity_id);
			if (ent->resload_entry->valid())
				_search.haystack_stringhash(ent->resload_entry->align()->resource_id);
			if (ent->resload_entry2->valid())
				_search.haystack_stringhash(ent->resload_entry2->align()->resource_id);
			if (ent->resload_entry3->valid())
				_search.haystack_stringhash(ent->resload_entry3->align()->resource_id);
		}
	}

	if (!_search.found_needle(ent))
		return;
	ImGui::PushID(ent);

	auto sceneRendering = ent->getById<Component_1_SceneRendering>(1);
	ImGui::TableNextRow(0, sceneRendering ? 220.0f : 50.0f);

	ImGui::TableNextColumn();
	for (int i = 0; i < depth; i++) {
		ImGui::Bullet();
		ImGui::SameLine();
	}
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,188,255,255));
	ImGui::Text("%x", ent->entity_id);
	ImGui::SameLine();
	if (ImGui::Button(EMOJI_COPY)) {
		imgui_helpers::openCopyWindow(ent->entity_id);
	}
	ImGui::PopStyleColor();

	if (ImGui::Button("Del")) {
		_entity_admin->delEnt(ent);
	}

	if (_is_gameEA) {
		ImGui::SameLine();
		if (ImGui::Button("Render")) {
			entity_bounds_renderer::create(this)->set(ent);
		}
		ImGui::SameLine();
		if (ImGui::Button("Set Local")) {
			_entity_admin->local_entid = ent->entity_id;
		}
	}

	if (sceneRendering) {
		bool is_visible = sceneRendering->IsVisible();

		ImGui::SameLine();
		if (ImGui::Button("A")) {
			imgui_helpers::openCopyWindow((__int64)&sceneRendering->rendering_flags);
		}

		ImGui::Separator();
		ImGui::Dummy(ImVec2(0.0f, 5.0f));

		if (ImGui::Checkbox("Is Visible", &is_visible)) {
			if (is_visible) {
				sceneRendering->SetInvisible();
			}
			else {
				sceneRendering->SetVisible();
			}
		}

		if (ent->hasComponent(4)) {
			auto model = ent->getById<Component_4_Model>(4);
			bool enabled = ((ushort)model->model_rendering_flags & (ushort)Component_4_Model::ModelRenderingFlags::OUTLINE_MASK) != 0;
			if (ImGui::Checkbox("Outline", &enabled)) {
				if (enabled) {
					model->EnableOutline(Component_4_Model::ModelRenderingFlags::OUTLINE_INVISIBLE, 0xFF00FF00);
				} else {
					model->DisableOutline();
				}
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Will not work on player entities, since outlines are set by the game every frame.");
			}
			if (enabled) {
				ImGui::SameLine();
				float color[] = {
					(float)(model->outline_color & 0xFF) / 255.0f,
					(float)(BYTE1(model->outline_color)) / 255.f,
					(float)(BYTE2(model->outline_color)) / 255.f,
					(float)(BYTE3(model->outline_color)) / 255.f,
				};
				if (ImGui::ColorEdit4("Color", color, ImGuiColorEditFlags_NoInputs)) {
					auto new_col = (uint32)(color[0] * 255.0f);
					new_col |= (uint32)(color[1] * 255.0f) << 8;
					new_col |= (uint32)(color[2] * 255.0f) << 16;
					new_col |= (uint32)(color[3] * 255.0f) << 24;
					model->outline_color = new_col;
				}
			}
		}
		/*if (ent->hasComponent(4)) {
			if (ImGui::Button("Outlines 0x40 :)")) {
				struct outlineStru {
					unsigned short outline_type = 0x40;
					unsigned char r = 0xFF;
					unsigned char g = 0xFF;
					unsigned char b = 0xFF;
					unsigned char a = 0xFF;
					int unk = 3;
				} outline;
				((void(*)(Entity*, outlineStru*))(globals::gameBase + 0xd5a050))(ent, &outline);
			}
			if (ImGui::Button("Outlines 0x80 :)")) {
				struct outlineStru {
					unsigned short outline_type = 0x80;
					unsigned char r = 0xFF;
					unsigned char g = 0xFF;
					unsigned char b = 0xFF;
					unsigned char a = 0xFF;
					int unk = 3;
				} outline;
				((void(*)(Entity*, outlineStru*))(globals::gameBase + 0xd5a050))(ent, &outline);
			}
			if (ImGui::Button("Outlines 0x100 :)")) {
				struct outlineStru {
					unsigned short outline_type = 0x100;
					unsigned char r = 0xFF;
					unsigned char g = 0xFF;
					unsigned char b = 0xFF;
					unsigned char a = 0xFF;
					int unk = 3;
				} outline;
				((void(*)(Entity*, outlineStru*))(globals::gameBase + 0xd5a050))(ent, &outline);
			}
			if (ImGui::Button("Outlines :(")) {
				((void(*)(Entity*))(globals::gameBase + 0xd5a430))(ent);
			}
		}*/

		if (sceneRendering->scaling.X == sceneRendering->scaling.Y == sceneRendering->scaling.Z) {
			ImGui::Text("scale: %f", sceneRendering->scaling.X);
		}
		else {
			ImGui::Text("scale:\n%f %f %f", sceneRendering->scaling.X, sceneRendering->scaling.Y, sceneRendering->scaling.Z);
		}
		if (ImGui::Button("+")) {
			sceneRendering->SetScale(Vector4(sceneRendering->scaling.X + 1));
		}
		ImGui::SameLine();
		if (ImGui::Button("-")) {
			sceneRendering->SetScale(Vector4(sceneRendering->scaling.X - 1));
		}

		ImGui::Text("Pos:\n%f %f %f", sceneRendering->position.X, sceneRendering->position.Y, sceneRendering->position.Z);

		if (ImGui::Button(EMOJI_COPY " pos")) {
			char buf[128];
			sprintf_s(buf, "Vector4(%f, %f, %f, 0)", sceneRendering->position.X, sceneRendering->position.Y, sceneRendering->position.Z);
			imgui_helpers::openCopyWindow(buf);
		}

		ImGui::SameLine();
		if (ImGui::Button(EMOJI_COPY " rot")) {
			char buf[128];
			sprintf_s(buf, "Vector4(%f, %f, %f, 0)", sceneRendering->rotation.X, sceneRendering->rotation.Y, sceneRendering->rotation.Z);
			imgui_helpers::openCopyWindow(buf);
		}
	}

	if (_nav_to_ent == ent) {
		ImGui::ScrollToItem();
		_nav_to_ent = 0;
	}

	ImGui::TableNextColumn();
	display_addr((__int64)ent);

	ImGui::NewLine();

	render_resload(ent->resload_entry, 1);
	render_resload(ent->resload_entry2, 2);
	render_resload(ent->resload_entry3, 3);

	ImGui::TableNextColumn();
	for (int i = 0; i < ent->component_list.num - 1; i++) {
		ComponentBase* compo = ent->component_list.ptr[i];
		if (compo) {
			ImGui::PushID(compo);
			auto mirror_data = (STUBase<>*)compo->GetMirrorData();
			if (mirror_data) {
				if (ImGui::Button("MD")) {
					printf("mirror_data: %p\n", mirror_data);
					stu_explorer::get_latest_or_create(this)->navigate_to(mirror_data->to_editable().struct_info, (__int64)mirror_data, nullptr);
				}
				ImGui::SameLine();
			}
			ImGui::PushFont(imgui_helpers::BoldFont);
			ImGui::Text(compo->is_mirrored ? "(ext):" : ":");
			ImGui::PopFont();
			ImGui::SameLine();
			stringhash_library::display_component(compo->component_id);
			ImGui::SameLine();
			display_addr((__int64)compo);
			if (compo->component_id == 0x23) {
				ImGui::SameLine();
				if (ImGui::Button("Open")) {
					//window_manager::add_window(new statescript_window());
					statescript_list::get_latest_or_create(this)->display_compo((Component_23_Statescript*)compo);
				}
			}
			else if (compo->component_id == 0x2F) {
				ImGui::SameLine();
				if (ImGui::Button("Open")) {
					local_player_compoennt_insights::get_latest_or_create(this)->set((Component_2F_LocalPlayer*)compo);
				}
				ImGui::SameLine();
				if (ImGui::Button("MS 1")) {
					movstate_visualizer::get_latest_or_create(this)->set({ "Comp2F MS 1", (MovementState*)((__int64)compo + 0x60) });
				}
				ImGui::SameLine();
				if (ImGui::Button("MS 2")) {
					movstate_visualizer::get_latest_or_create(this)->set({ "Comp2F MS 2", (MovementState*)((__int64)compo + 0x2e0) });
				}
				ImGui::SameLine();
				if (ImGui::Button("MS 3")) {
					movstate_visualizer::get_latest_or_create(this)->set({ "Comp2F MS 2", (MovementState*)((__int64)compo + 0x560) });
				}
			}
			else if (compo->component_id == 0x24) {
				ImGui::SameLine();
				if (ImGui::Button("Open")) {
					pvpgame_explorer::get_latest_or_create(this)->set((Component_24_PvPGame*)compo);
				}
			}
			else if (compo->component_id == 0x15) {
				ImGui::SameLine();
				if (ImGui::Button("MS 1")) {
					movstate_visualizer::get_latest_or_create(this)->set({ "Comp15 MS 1", (MovementState*)((__int64)compo + 0x7E0) });
				}
				ImGui::SameLine();
				if (ImGui::Button("MS 2")) {
					movstate_visualizer::get_latest_or_create(this)->set({ "Comp15 MS 2", (MovementState*)((__int64)compo + 0xA50) });
				}
			}
			else if (compo->component_id == 0x12) {
				ImGui::SameLine();
				if (ImGui::Button("MS")) {
					movstate_visualizer::get_latest_or_create(this)->set({ "Comp12 MS", (MovementState*)((__int64)compo + 0xA060) });
				}
			}
			ImGui::PopID();
		}
	}
	if (ent->child) {
		render_entity(ent->child, depth + 1);
	}
	if (ent->next_child) {
		render_entity(ent->next_child, depth);
	}
	ImGui::PopID();
}
