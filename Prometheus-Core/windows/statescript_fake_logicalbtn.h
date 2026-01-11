#pragma once
#include "../window_manager/window_manager.h"
#include "../Statescript.h"
#include "statescript_list.h"
#include "../Statescript.h"

class statescript_fake_logicalbtn : public window {
	WINDOW_DEFINE(statescript_fake_logicalbtn, "Game", "A Window", false);

	inline void render_inputstru(Component_50_Input* comp, Component_50_Input::InputStruct* stru) {
		ImGui::PushID(stru);
		
		char* curr_list = comp->tick10 ? stru->tick0 : stru->tick1;
		for (int i = 0; i < 0xC3; i++) {
			if (curr_list[i])
				ImGui::BulletText("%d [%s]", i, LogicalButtonById(i)->name);
		}

		ImGui::PopID();
	}

	char prev_buff[0xC3];

	inline void render() override {
		auto parent = get_creator_as<statescript_list>();
		if (open_window()) {
			auto ss = parent->_ss;
			if (IsBadReadPtr(ss, sizeof(ss))) {
				ImGui::Text("Invalid SS Comp");
				return;
			}
			auto key_comp = LobbyEntityAdmin()->getSingletonComponent<Component_50_Input>(0x50);
			if (!key_comp) {
				ImGui::Text("Key Comp Invalid!");
				return;
			}

			char* curr = key_comp->tick10 ? key_comp->stru1.tick0 : key_comp->stru1.tick1;
			char* prev = key_comp->tick10 ? key_comp->stru1.tick1 : key_comp->stru1.tick0;
			ImGui::Checkbox("Fake it till you make it", &_update_states);
			if (_update_states) {
				for (int i = 0; i < 0xc3; i++) {
					if (curr[i] != prev[i]) {
						printf("button state changed: %x [%s] - %d\n", i, LogicalButtonById(i)->name, curr[i]);
						for (auto& script : ss->ss_inner) {
							auto event = (StatescriptNotification_LogicalButton*)ss->ss_inner.vfptr->Allocate_StatescriptE(&ss->ss_inner, ETYPE_LOGICALBUTTON);
							event->base.m_timestamp = ss->ss_inner.cf_timestamp;
							event->base.m_instanceId = script->instance_id;
							event->m_logicalButton = i;
							event->m_bButtonGoingDown = curr[i];
							script->vfptr->EnqueueE(script, (StatescriptNotification_Base*)event);
						}
					}
				}
			}

			memcpy(prev_buff, curr, sizeof(prev_buff));

			/*STRUCT_PLACE(int, tick10, 0x40);
			STRUCT_PLACE(char, field_4C, 0x4C);
			STRUCT_PLACE(char, not_ingame, 0x4D);
			STRUCT_PLACE(float, pause_input_for, 0x144);
			STRUCT_PLACE(InputStruct, stru1, 0x148);
			STRUCT_PLACE(InputStruct, stru2, 0x2CE);
			STRUCT_PLACE(InputStruct, stru3, 0x454);
			STRUCT_PLACE_CUSTOM(stru4, 0x5E6, char stru4[0xc3]);*/
			ImGui::Text("tick: %d", key_comp->tick10);
			STRUCT_MODIFIABLE(key_comp, field_4C);
			STRUCT_MODIFIABLE(key_comp, not_ingame);
			STRUCT_MODIFIABLE(key_comp, pause_input_for);
			ImGui::Text("Stru1");
			render_inputstru(key_comp, &key_comp->stru1);
			ImGui::Text("Stru2");
			render_inputstru(key_comp, &key_comp->stru2);
			ImGui::Text("Stru3");
			render_inputstru(key_comp, &key_comp->stru3);
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}


	statescript_fake_logicalbtn() {
		is_dependent = true;
	}
private:
	int state_to_use;
	bool _button_states[256]{};
	bool _update_states = false;

	inline void update_button_state(const char* logical_button, int vkey) {
		
	}
};

WINDOW_REGISTER(statescript_fake_logicalbtn);