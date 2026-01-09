#pragma once
#include "globals.h"
#include "MovementState.h"
#include <nlohmann/json.hpp>

//god doesnt exist because if he did i would have went straight to hell for this
void to_json(nlohmann::json& j, const MovementState& state);
void from_json(const nlohmann::json& j, MovementState& state);

void to_json(nlohmann::json& j, const Vector4& state);
void from_json(const nlohmann::json& j, Vector4& state);

void to_json(nlohmann::json& j, const WalkDirection& state);
void from_json(const nlohmann::json& j, WalkDirection& state);