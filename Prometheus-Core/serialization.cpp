#include "serialization.h"

void to_json(nlohmann::json& j, const MovementState& s) {
	auto state = &s;
	j["0"] = state->field_0;
	j["10"] = state->field_10;
	j["20"] = state->field_20;
	j["30"] = state->field_30;
	j["34"] = state->field_34;
	j["38"] = state->field_38;
	j["40"] = state->field_40;
	j["50"] = state->field_50;
	j["60"] = state->field_60;
	j["70"] = state->field_70;
	j["80"] = state->field_80;
	j["84"] = state->field_84;
	j["88"] = state->field_88;
	j["8C"] = state->field_8C;
	j["90"] = state->field_90;
	//j["A0"] = state->field_A0;
	j["B0"] = state->field_B0;
	j["B8"] = state->field_B8;
	j["BC"] = state->field_BC;
	j["C0"] = state->field_C0;
	j["C4"] = state->field_C4;
	j["C8"] = state->field_C8;
	j["CC"] = state->field_CC;
	j["D0"] = state->field_D0;
	//ignoring lists for now
	j["198"] = state->movement_flags;
	j["19C"] = state->field_19C;
	j["1A0"] = state->command_frame;
	j["1A4"] = state->field_1A4;
	j["1A8"] = state->ticks_in_air;
	j["1AC"] = state->field_1AC;
	j["1B0"] = state->walk_direction;
	j["1B2"] = state->last_walk_direction;
	j["1B4"] = state->move_frame;
	j["1B8"] = state->bobbing_X;
	j["1BC"] = state->bobbing_Y;
	//ignoring matrix
	j["200"] = state->rotation_quaternion;
	j["210"] = state->rotation_Y;
	j["214"] = state->rotation_Z;
	j["218"] = state->rotation_X;
	j["21C"] = state->rotation_matrix_deserialized;
	j["21D"] = state->field_21D;
	j["220"] = state->absolute_position;
	j["230"] = state->position_delta;
	j["240"] = state->y_lookdir;
	j["244"] = state->movingplatform_entid;
	j["248"] = state->is_on_movingplatform;
	j["250"] = state->gravity_delta;
	//serialize the gameentityadmin ptr lolol
	j["268"] = state->field_268;
}
void from_json(const nlohmann::json& j, MovementState& s) {
	auto state = &s;
	state->field_0 = j["0"].get<Vector4>();
	state->field_10 = j["10"].get<Vector4>();
	state->field_20 = j["20"].get<float>();
	state->field_30 = j["30"].get<int>();
	state->field_34 = j["34"].get<int>();
	state->field_38 = j["38"].get<float>();
	state->field_40 = j["40"].get<int>();
	state->field_50 = j["50"].get<Vector4>();
	state->field_60 = j["60"].get<short>();
	state->field_70 = j["70"].get<Vector4>();
	state->field_80 = j["80"].get<int>();
	state->field_84 = j["84"].get<int>();
	state->field_88 = j["88"].get<int>();
	state->field_8C = j["8C"].get<char>();
	state->field_90 = j["90"].get<Vector4>();
	//state->field_A0 = j["A0"].get<>();
	state->field_B0 = j["B0"].get<char>();
	state->field_B8 = j["B8"].get<float>();
	state->field_BC = j["BC"].get<int>();
	state->field_C0 = j["C0"].get<float>();
	state->field_C4 = j["C4"].get<float>();
	state->field_C8 = j["C8"].get<float>();
	state->field_CC = j["CC"].get<int>();
	state->field_D0 = j["D0"].get<float>();
	//ignoring lists for now
	state->movement_flags = j["198"].get<int>();
	state->field_19C = j["19C"].get<int>();
	state->command_frame = j["1A0"].get<int>();
	state->field_1A4 = j["1A4"].get<int>();
	state->ticks_in_air = j["1A8"].get<int>();
	state->field_1AC = j["1AC"].get<float>();
	state->walk_direction = j["1B0"].get<WalkDirection>();
	state->last_walk_direction = j["1B2"].get<WalkDirection>();
	state->move_frame = j["1B4"].get<int>();
	state->bobbing_X = j["1B8"].get<float>();
	state->bobbing_Y = j["1BC"].get<float>();
	//ignoring matrix
	state->rotation_quaternion = j["200"].get<Vector4>();
	state->rotation_Y = j["210"].get<float>();
	state->rotation_Z = j["214"].get<float>();
	state->rotation_X = j["218"].get<float>();
	state->rotation_matrix_deserialized = j["21C"].get<char>();
	state->field_21D = j["21D"].get<char>();
	state->absolute_position = j["220"].get<Vector4>();
	state->position_delta = j["230"].get<Vector4>();
	state->y_lookdir = j["240"].get<float>();
	state->movingplatform_entid = j["244"].get<uint>();
	state->is_on_movingplatform = j["248"].get<int>();
	state->gravity_delta = j["250"].get<Vector4>();
	//serialize the gameentityadmin ptr lolol
	state->field_268 = j["268"].get<char>();
}

void to_json(nlohmann::json& j, const Vector4& state) {
	j["x"] = state.X;
	j["y"] = state.Y;
	j["z"] = state.Z;
	j["w"] = state.W;
}
void from_json(const nlohmann::json& j, Vector4& state) {
	state.X = j["x"].get<float>();
	state.Y = j["y"].get<float>();
	state.Z = j["z"].get<float>();
	state.W = j["w"].get<float>();
}

void to_json(nlohmann::json& j, const WalkDirection& state) {
	j["lr"] = (unsigned char)state.left_right;
	j["fb"] = (unsigned char)state.forward_backward;
}
void from_json(const nlohmann::json& j, WalkDirection& state) {
	state.left_right = (WalkDirectionLR)j["lr"].get<unsigned char>();
	state.forward_backward = (WalkDirectionFB)j["fb"].get<unsigned char>();
}