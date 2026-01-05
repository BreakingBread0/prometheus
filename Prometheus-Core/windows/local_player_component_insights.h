#pragma once
#include "../window_manager/window_manager.h"
#include "../entity_admin.h"
#include "entity_window.h"

class local_player_compoennt_insights : public window {
	WINDOW_DEFINE_ARG(local_player_compoennt_insights, "Game", "Local Player Component Insights", Component_2F_LocalPlayer*);

	void render_movcomm(const char* text, Component_2F_LocalPlayer::MovementCommand* mov) {
		ImGui::PushID(text);
		if (ImGui::CollapsingHeader(text)) {
			ImGui::Indent();
			ImGui::Text("field_0: %d (%x)", mov->field_0, mov->field_0);
			ImGui::Text("left_right_lookdir: %hd", mov->left_right_lookdir);
			ImGui::Text("up_down_lookdir: %hd", mov->up_down_lookdir);
			/*ImGui::Text("left_right_movement: %hhx", mov->left_right_movement);
			ImGui::Text("forward_backward_movement: %hhx", mov->forward_backward_movement);*/
			ImGui::Text("ability_button_states: %hx", mov->ability_button_states);
			ImGui::Checkbox("field_C", &mov->field_C);
			ImGui::Checkbox("field_D", &mov->field_D);
			ImGui::Unindent();
		}
		ImGui::PopID();
	}

	inline void render() override {
		if (open_window(nullptr, 0, ImVec2(400, 700))) {
			if (IsBadReadPtr(_arg, sizeof(_arg))) {
				ImGui::Text("Invalid Component");
				return;
			}
			display_addr((__int64)_arg);
			if (ImGui::Button(EMOJI_SHARE))
				entity_window::get_latest_or_create(this)->nav_to_ent(_arg->base.entity_backref);

			ImGui::NewLine();
			STRUCT_MODIFIABLE(_arg, field_40);
			STRUCT_MODIFIABLE(_arg, field_44);
			STRUCT_MODIFIABLE(_arg, mover_type);
			STRUCT_MODIFIABLE(_arg, is_thirdperson_model);
			STRUCT_MODIFIABLE(_arg, field_550);
			STRUCT_MODIFIABLE(_arg, field_558);

			ImGui::NewLine();
			STRUCT_MODIFIABLE(_arg, is_able_to_aim);
			STRUCT_MODIFIABLE(_arg, field_7D1);
			render_movcomm("field_1460", &_arg->field_1460);
			render_movcomm("curr_movcommand", &_arg->curr_movcommand);
			render_movcomm("copied_movcommand", &_arg->copied_movcommand);

			ImGui::NewLine();
			ImGui::Text("mouse_rel: %f %f", _arg->mouse_rel_x, _arg->mouse_rel_y);
			STRUCT_MODIFIABLE(_arg, field_1498);
			STRUCT_MODIFIABLE(_arg, field_149C);
			STRUCT_MODIFIABLE(_arg, crouch_toggle);

			ImGui::NewLine();
			//imgui_helpers::render_matrix4x4("proj_matrix", &_arg->viewproj_matrix_0);
			imgui_helpers::render_vec4("empty_vec", &_arg->empty_vec);
			imgui_helpers::render_vec4("current_position", &_arg->current_position);
			imgui_helpers::render_vec4("current_position_copied", &_arg->current_position_copied);
			ImGui::Text("entid: %x", _arg->some_entid);

			ImGui::NewLine();
			STRUCT_MODIFIABLE(_arg, list_1530.num);
			STRUCT_MODIFIABLE(_arg, field_15D1);
			STRUCT_MODIFIABLE(_arg, render_thirdperson);

			ImGui::NewLine();
			if (ImGui::CollapsingHeader("entids_array")) {
				for (int i = 0; i < 32; i++) {
					ImGui::BulletText("%d: %d (%x)", i, _arg->entids_array[i], _arg->entids_array[i]);
				}
			}
			STRUCT_MODIFIABLE(_arg, field_17E0);
			STRUCT_MODIFIABLE(_arg, field_17F0);
			STRUCT_MODIFIABLE(_arg, field_17F4);
			STRUCT_MODIFIABLE(_arg, list_1800.num);

			ImGui::NewLine();
			STRUCT_MODIFIABLE(_arg, field_18C0);
			STRUCT_MODIFIABLE(_arg, field_18C4);
			STRUCT_MODIFIABLE(_arg, field_18C8);
			STRUCT_MODIFIABLE(_arg, field_18D0);
			STRUCT_MODIFIABLE(_arg, field_18D4);
			STRUCT_MODIFIABLE(_arg, field_18D8);

			STRUCT_MODIFIABLE(_arg, add_to_angle_x);
			STRUCT_MODIFIABLE(_arg, add_to_angle_y);
			STRUCT_MODIFIABLE(_arg, field_18E8);
			STRUCT_MODIFIABLE(_arg, field_18EC);
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(local_player_compoennt_insights);
