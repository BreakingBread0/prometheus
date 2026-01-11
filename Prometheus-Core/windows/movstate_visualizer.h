#pragma once
#include "../window_manager/window_manager.h"
#include "../MovementState.h"
#include <utility>
#include "../serialization.h"
#include "../player_spawner.h"

class movstate_visualizer : public window {
	typedef typename std::pair<std::string, MovementState*> movstate_argtyp;
	WINDOW_DEFINE_ARG(movstate_visualizer, "Game", "MovementState Visualizer", movstate_argtyp);

	inline void render() override {
		if (open_window(_arg.first.c_str())) {
			if (IsBadReadPtr(_arg.second, sizeof(_arg))) {
				ImGui::Text("Invalid!");
				return;
			}

			ImGui::Checkbox("Replicate States", &_replicate_states);

			int index = _state_index;
			if (++_state_index >= STATES_MAX) {
				_state_index = 0;
			}

			if (_replicate_states) {
				if (!_replicate_to) {
					player_spawner spawner(0x400000000000001);
					spawner.controller_info.set_localent = false;
					spawner.controller_info.load_2f_33 = false;
					_replicate_to = spawner.spawn().second; //replicate to model obv me stoobid
				}

				_state_history[index] = *_arg.second;

				auto get_last_movstate_fn = (MovementState * (*)(Component_12_STUMovementStateComponent*))(globals::gameBase + 0xc23580);
				//auto get_update_hz = (double (*)(int))(globals::gameBase + 0x102ea30);
				auto CreateRotMatrix = (void(*)(MovementState*, Matrix4x4*))(globals::gameBase + 0x102a610);
				auto EmplaceMovStateData = (__int64(*)(Component_12_STUMovementStateComponent::MovementHistory*, MovementState*, double))(globals::gameBase + 0x102d7c0);
				auto movement_stuff = (char(__fastcall*)(Component_12_STUMovementStateComponent*, float, double, int, double))(globals::gameBase + 0xd06600); //Pain

				if (!_state_history[_state_index].is_null()) {
					auto state = _state_history[_state_index].get<MovementState>();
					auto comp12 = _replicate_to->getById<Component_12_STUMovementStateComponent>(0x12);
					
					comp12->PasteMovementState(&state);
				}
			}

			if (ImGui::Button(EMOJI_COPY)) {
				imgui_helpers::openCopyWindow((__int64)_arg.second);
			}

			if (ImGui::Button("Copy State")) {
				_state = *_arg.second;
			}
			ImGui::SameLine();
			if (ImGui::Button("Copy JSON")) {
				try {
					std::string json = _state.dump();
					imgui_helpers::openCopyWindow(json);
				}
				catch (nlohmann::json::exception& ex) {
					imgui_helpers::messageBox(ex.what(), this);
				}
			}
			if (ImGui::Button("Paste State")) {
				MovementState state = _state.get<MovementState>();
				((void(*)(MovementState*, Matrix4x4*))(globals::gameBase + 0x102a610))(&state, &state.rotation_matrix);
				state.copyTo(_arg.second);
			}
			ImGui::SameLine();
			if (ImGui::Button("Paste into Comp12 (Local)")) {
				auto controller = GameEntityAdmin()->getLocalEnt();
				if (controller) {
					auto model = GameEntityAdmin()->getEntById(controller->getById<Component_20_ModelReference>(0x20)->aim_entid);
					if (model) {
						auto comp12 = model->getById<Component_12_STUMovementStateComponent>(0x12);
						if (comp12) {
							MovementState state = _state.get<MovementState>();
							comp12->PasteMovementState(&state);
						}
					}
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Paste into Comp2F")) {
				auto controller = GameEntityAdmin()->getLocalEnt();
				if (controller) {
					auto comp2f = controller->getById<Component_2F_LocalPlayer>(0x2F);
					if (comp2f) {
						MovementState state = _state.get<MovementState>();
						state.copyTo((MovementState*)((__int64)comp2f + 0x60));
						comp2f->is_thirdperson_model = 1;
					}
				}
			}

			STRUCT_MODIFIABLE(_arg.second, field_0);
			STRUCT_MODIFIABLE(_arg.second, field_10);
			STRUCT_MODIFIABLE(_arg.second, field_20);
			STRUCT_MODIFIABLE(_arg.second, field_30);
			STRUCT_MODIFIABLE(_arg.second, field_34);
			STRUCT_MODIFIABLE(_arg.second, field_38);
			STRUCT_MODIFIABLE(_arg.second, field_40);
			STRUCT_MODIFIABLE(_arg.second, field_50);
			STRUCT_MODIFIABLE(_arg.second, field_60); //TODO 2. word danach?
			STRUCT_MODIFIABLE(_arg.second, field_70);
			STRUCT_MODIFIABLE(_arg.second, field_80);
			STRUCT_MODIFIABLE(_arg.second, field_84);
			STRUCT_MODIFIABLE(_arg.second, field_88);
			STRUCT_MODIFIABLE(_arg.second, field_8C);
			STRUCT_MODIFIABLE(_arg.second, field_90);
			STRUCT_MODIFIABLE(_arg.second, field_A0); //location vectors
			STRUCT_MODIFIABLE(_arg.second, field_B0);
			STRUCT_MODIFIABLE(_arg.second, field_B8);
			STRUCT_MODIFIABLE(_arg.second, field_BC);
			STRUCT_MODIFIABLE(_arg.second, field_C0);
			STRUCT_MODIFIABLE(_arg.second, field_C4);
			STRUCT_MODIFIABLE(_arg.second, field_C8);
			STRUCT_MODIFIABLE(_arg.second, field_CC);
			STRUCT_MODIFIABLE(_arg.second, field_D0);
			STRUCT_MODIFIABLE(_arg.second, field_D8);
			STRUCT_MODIFIABLE(_arg.second, field_F0);
			STRUCT_MODIFIABLE(_arg.second, field_108);
			STRUCT_MODIFIABLE(_arg.second, movement_flags);
			STRUCT_MODIFIABLE(_arg.second, field_19C);
			STRUCT_MODIFIABLE(_arg.second, command_frame);
			STRUCT_MODIFIABLE(_arg.second, field_1A4);
			STRUCT_MODIFIABLE(_arg.second, ticks_in_air);
			STRUCT_MODIFIABLE(_arg.second, field_1AC);
			/*STRUCT_MODIFIABLE(_arg.second, field_1B0);
			STRUCT_MODIFIABLE(_arg.second, field_1B1);
			STRUCT_MODIFIABLE(_arg.second, field_1B2);
			STRUCT_MODIFIABLE(_arg.second, field_1B3);*/
			ImGui::Text("Walk Direction: %s", _arg.second->walk_direction.to_string().c_str());
			ImGui::Text("Walk Direction (Last): %s", _arg.second->last_walk_direction.to_string().c_str());
			STRUCT_MODIFIABLE(_arg.second, move_frame);
			STRUCT_MODIFIABLE(_arg.second, bobbing_X);
			STRUCT_MODIFIABLE(_arg.second, bobbing_Y);
			STRUCT_MODIFIABLE(_arg.second, rotation_matrix);
			STRUCT_MODIFIABLE(_arg.second, rotation_quaternion);
			STRUCT_MODIFIABLE(_arg.second, rotation_Y); //is this true?
			STRUCT_MODIFIABLE(_arg.second, rotation_Z);
			STRUCT_MODIFIABLE(_arg.second, rotation_X);
			STRUCT_MODIFIABLE(_arg.second, rotation_matrix_deserialized);
			STRUCT_MODIFIABLE(_arg.second, field_21D);
			STRUCT_MODIFIABLE(_arg.second, absolute_position);
			STRUCT_MODIFIABLE(_arg.second, position_delta);
			STRUCT_MODIFIABLE(_arg.second, y_lookdir);
			STRUCT_MODIFIABLE(_arg.second, movingplatform_entid);
			STRUCT_MODIFIABLE(_arg.second, is_on_movingplatform);
			STRUCT_MODIFIABLE(_arg.second, gravity_delta);
			STRUCT_MODIFIABLE(_arg.second, field_268);
		}
		ImGui::End();
	}

private:
	nlohmann::json _state{};
	int _state_index = 0;
	static const int STATES_MAX = 300;
	nlohmann::json _state_history[STATES_MAX]{};
	bool _replicate_states = 0;
	Entity* _replicate_to = 0;

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(movstate_visualizer);
