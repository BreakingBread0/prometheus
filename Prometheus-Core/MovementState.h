#pragma once
#include "game.h"
#include "entity_admin.h"

//Component_15_1 size 0x270
struct MovementState {
	union {
		Vector4 field_0;
		STRUCT_PLACE(Vector4, field_10, 0x10);
		STRUCT_PLACE(float, field_20, 0x20);

		STRUCT_PLACE(int, field_30, 0x30);
		STRUCT_PLACE(int, field_34, 0x34);
		STRUCT_PLACE(float, field_38, 0x38);

		STRUCT_PLACE(int, field_40, 0x40);

		STRUCT_PLACE(Vector4, field_50, 0x50);
		STRUCT_PLACE(short, field_60, 0x60); //TODO 2. word danach?

		STRUCT_PLACE(Vector4, field_70, 0x70);
		STRUCT_PLACE(int, field_80, 0x80);
		STRUCT_PLACE(int, field_84, 0x84);

		STRUCT_PLACE(int, field_88, 0x88);
		STRUCT_PLACE(char, field_8C, 0x8C);

		STRUCT_PLACE(Vector4, field_90, 0x90);
		STRUCT_PLACE(teList<Vector4>, field_A0, 0xA0); //location vectors
		STRUCT_PLACE(char, field_B0, 0xB0);

		STRUCT_PLACE(float, field_B8, 0xB8);
		STRUCT_PLACE(int, field_BC, 0xBC);
		STRUCT_PLACE(float, field_C0, 0xC0);
		STRUCT_PLACE(float, field_C4, 0xC4);
		STRUCT_PLACE(float, field_C8, 0xC8);
		STRUCT_PLACE(int, field_CC, 0xCC);
		STRUCT_PLACE(float, field_D0, 0xD0);

		STRUCT_PLACE(teList<Vector4>, field_D8, 0xD8);
		STRUCT_PLACE(teList<Vector4>, field_F0, 0xF0);
		STRUCT_PLACE(teList<int>, field_108, 0x108);

		STRUCT_PLACE_CUSTOM(mirrored_anim_data, 0x120, int mirrored_anim_data[30]);

		//0x200: Default
		//0x2: sneaking
		//0x5: jumping
		//0x8: falling (when jumping off, 0x5 is set instead)
		//0x20000: moving against wall
		STRUCT_PLACE(int, movement_flags, 0x198);
		STRUCT_PLACE(int, field_19C, 0x19C);
		STRUCT_PLACE(int, command_frame, 0x1A0);
		STRUCT_PLACE(int, field_1A4, 0x1A4);
		STRUCT_PLACE(int, ticks_in_air, 0x1A8);
		STRUCT_PLACE(float, field_1AC, 0x1AC);

		/*STRUCT_PLACE(char, field_1B0, 0x1B0);
		STRUCT_PLACE(char, field_1B1, 0x1B1);
		STRUCT_PLACE(char, field_1B2, 0x1B2);
		STRUCT_PLACE(char, field_1B3, 0x1B3);*/
		STRUCT_PLACE(WalkDirection, walk_direction, 0x1B0);
		STRUCT_PLACE(WalkDirection, last_walk_direction, 0x1B2); //last walk direction
		STRUCT_PLACE(int, move_frame, 0x1B4); //Frame changes when moving

		STRUCT_PLACE(float, bobbing_X, 0x1B8); //value that changes when hitting ground
		STRUCT_PLACE(float, bobbing_Y, 0x1BC); 

		STRUCT_PLACE(Matrix4x4, rotation_matrix, 0x1C0); //Immer Einheitsmatrix?
		STRUCT_PLACE(Vector4, rotation_quaternion, 0x200);

		STRUCT_PLACE(float, rotation_Y, 0x210); //is this true? bildet ein rechtssystem tho
		STRUCT_PLACE(float, rotation_Z, 0x214);
		STRUCT_PLACE(float, rotation_X, 0x218);
		STRUCT_PLACE(char, rotation_matrix_deserialized, 0x21C);
		STRUCT_PLACE(char, field_21D, 0x21D);

		STRUCT_PLACE(Vector4, absolute_position, 0x220);
		STRUCT_PLACE(Vector4, position_delta, 0x230);
		
		STRUCT_PLACE(float, y_lookdir, 0x240); //-1.5 is UP, 1.5 is DOWN
		STRUCT_PLACE(uint, movingplatform_entid, 0x244); //Entity ID of which moving platform we are on
		STRUCT_PLACE(int, is_on_movingplatform, 0x248);

		STRUCT_PLACE(Vector4, gravity_delta, 0x250); //Only applies -Y direction
		STRUCT_PLACE(EntityAdminBase*, entity_admin, 0x260);
		STRUCT_PLACE(char, field_268, 0x268);
	};

	MovementState() {
		((void(*)(MovementState*))(globals::gameBase + 0xc1de50))(this); //God doesnt exist
	}

	static MovementState* create() {
		auto result = new MovementState{};
		((void(*)(MovementState*))(globals::gameBase + 0xc1de50))(result); //constructor
		return result;
	}

	void copyTo(MovementState* destination) {
		((void(*)(MovementState*, MovementState*))(globals::gameBase + 0xc235b0))(destination, this);
	}

	void dealloc() {
		field_A0.dealloc();
		field_D8.dealloc();
		field_F0.dealloc();
		field_108.dealloc();
		ow_dealloc((__int64)this);
	}
};

struct Component_12_STUMovementStateComponent {
	struct RecvFrame {
		double command_frame;
		int cf_from_movstate;
	};

	struct MovementHistory {
		union {
			STRUCT_MIN_SIZE(0xA040);

			RecvFrame recv_frames[0x40];

			STRUCT_PLACE(int, recv_frames_max, 0x400);
			STRUCT_PLACE(int, latest_recv_frame, 0x410);
			STRUCT_PLACE(int, field_414, 0x414); //used array cnt?

			STRUCT_PLACE_CUSTOM(movstate_arr, 0x420, MovementState movstates[0x40]);
			STRUCT_PLACE(int, movstate_arr_max, 0xA020);
			STRUCT_PLACE(int, field_A024, 0xA024);
			STRUCT_PLACE(int, data_1_arr_cnt, 0xA030);
			STRUCT_PLACE(int, movstate_arr_used, 0xA034);
		};
	};

	union {
		ComponentBase base;

		STRUCT_PLACE(MovementHistory, history_1, 0x20);
		STRUCT_PLACE(MovementState, mov_state, 0xA060);

		STRUCT_PLACE(int, field_A2D0, 0xA2D0);
		STRUCT_PLACE(int, field_A2D4, 0xA2D4);
		STRUCT_PLACE(__int64, field_A2D8, 0xA2D8);
		STRUCT_PLACE(int, field_A2E0, 0xA2E0);
		STRUCT_PLACE(int, field_A2E4, 0xA2E4);
		STRUCT_PLACE(int, field_A2E8, 0xA2E8);
		STRUCT_PLACE(int, field_A2EC, 0xA2EC);
		STRUCT_PLACE(int, field_A2F0, 0xA2F0);
		STRUCT_PLACE(__int64, field_A2F8, 0xA2F8);

		STRUCT_PLACE(MovementHistory, history_2, 0x20);
		STRUCT_PLACE(Vector4, field_14340, 0x14340);
		STRUCT_PLACE(Vector4, field_14350, 0x14350);
		STRUCT_PLACE(Vector4, field_14360, 0x14360);
		STRUCT_PLACE(Vector4, field_14370, 0x14370);
		STRUCT_PLACE(Vector4, field_14380, 0x14380);

		STRUCT_PLACE(int, latest_recv_cf, 0x14390);
		STRUCT_PLACE(int, field_14394, 0x14394);
		STRUCT_PLACE(double, latest_recv_secs, 0x14398);
		STRUCT_PLACE(int, field_143A0, 0x143A0);
		STRUCT_PLACE(char, field_143A4, 0x143A4);
	};

	int PasteMovementState(MovementState* state) {
		auto get_last_movstate_fn = (MovementState * (*)(Component_12_STUMovementStateComponent*))(globals::gameBase + 0xc23580);
		//auto get_update_hz = (double (*)(int))(globals::gameBase + 0x102ea30);
		auto CreateRotMatrix = (void(*)(MovementState*, Matrix4x4*))(globals::gameBase + 0x102a610);
		auto EmplaceMovStateData = (__int64(*)(Component_12_STUMovementStateComponent::MovementHistory*, MovementState*, double))(globals::gameBase + 0x102d7c0);
		auto movement_stuff = (char(__fastcall*)(Component_12_STUMovementStateComponent*, float, double, int, double))(globals::gameBase + 0xd06600); //Pain

		//0xc26fb0
		int v32 = history_1.field_414;
		if (v32 >= 0)
			latest_recv_cf = state->command_frame;
		auto command_frame = state->command_frame;
		//if (v32 > 0 || get_last_movstate_fn(comp12)->command_frame >= command_frame) {
		//	printf("FixMe!\n");
		//}
		latest_recv_cf = state->command_frame;
		double v35 = 16000 * state->command_frame * (double)0.000001; // * 0,016 (62,5hz? is that static in the beta?)
		//CreateRotMatrix(&state, &state.rotation_matrix);
		double result = EmplaceMovStateData(&history_1, state, v35);
		if (result) {
			double curr_match_time_secs = *(double*)(globals::gameBase + 0xc217a4);
			latest_recv_secs = curr_match_time_secs;
			field_143A0 = result;
		}
		//auto timing = GameEntityAdmin()->GameEA_timing;
		//movement_stuff(comp12, 0.f, timing->field_50 + 0x3E80 * timing->field_60 * 0.000001f, timing->tick_count, 0.f);
		field_14394 = command_frame;
		//skip_some_stuff:
		return result;
	}
};

struct Component_15_STUCharacterMoverComponent {
	union {
		ComponentBase base;
		STRUCT_PLACE(MovementState, movement_state_1, 0x7E0);
		STRUCT_PLACE(MovementState, movement_state_2, 0xA50);
	};
};