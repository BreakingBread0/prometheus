#pragma once
#include "../window_manager/window_manager.h"
#include "../entity_admin.h"
#include <format>

class freecam_movement_recorder : public window {
	WINDOW_DEFINE(freecam_movement_recorder, "Tools", "Freecam Movement Recorder", true);

	inline void render() override {
		auto camera = GameEntityAdmin()->getSingletonComponent<Component_4F_Camera>(0x4F);
		auto curr_view = camera->camera->current_view;
		Vector4* curr_pos = nullptr;
		Vector4* curr_rot = nullptr;
		if (curr_view) {
			curr_pos = &curr_view->view_position; 
			curr_rot = &curr_view->view_rotation;
		}
		else {
			printf("No current view found!\n");
		}

		if (open_window()) {
			display_addr((__int64)curr_view, "Curr View");
			if (curr_pos) {
				ImGui::Text("Pos: %f %f %f", curr_pos->X, curr_pos->Y, curr_pos->Z);
				ImGui::SameLine();
				if (ImGui::Button(EMOJI_COPY)) {
					imgui_helpers::openCopyWindow(fmt::format("cammove(Vector4({:f}, {:f}, {:f}, 0), Vector4({:f}, {:f}, {:f}, 0))", 
						curr_pos->X, curr_pos->Y, curr_pos->Z,
						curr_rot->X, curr_rot->Y, curr_rot->Z));
				}
				ImGui::Text("Rot: %f %f %f", curr_rot->X, curr_rot->Y, curr_rot->Z);
			}
			/*if (ImGui::Button("Clear")) {
				_movements.clear();
			}
			ImGui::SameLine();*/
			if (ImGui::Button("Copy")) {
				std::stringstream stream{};
				stream << "std::map<double, cammove> {" << std::endl;
				for (auto movement : _movements) {
					stream << "{ " << fmt::format("{:f}, cammove(Vector4({:f}, {:f}, {:f}, 0), Vector4({:f}, {:f}, {:f}, 0))", 
						movement.first, 
						movement.second.first.X, movement.second.first.Y, movement.second.first.Z,
						movement.second.second.X, movement.second.second.Y, movement.second.second.Z) << " }," << std::endl;
				}
				stream << "};";
				imgui_helpers::openCopyWindow(stream.str());
			}
			if (ImGui::Button("Play")) {
				_playback_start_sec = GetGameRuntimeSecs();
			}

			if (ImGui::BeginListBox("", ImVec2(-5, -5))) {
				for (auto it = _movements.begin(); it != _movements.end();) {
					ImGui::PushID(it->first);

					if (ImGui::Button(EMOJI_CROSS)) {
						_movements.erase(it);
						continue;
					}
					ImGui::SameLine();
					ImGui::Text("%lf -- (%f %f %f), (%f %f %f)", it->first, it->second.first.X, it->second.first.Y, it->second.first.Z, it->second.second.X, it->second.second.Y, it->second.second.Z);
					
					ImGui::PopID();
					it++;
				}
				ImGui::EndListBox();
			}
		}
		ImGui::End();

		if (curr_pos) {
			if (_playback_start_sec != 0) {
				auto last = _movements.rbegin();
				auto curr_move = _movements.lower_bound(GetGameRuntimeSecs() - _playback_start_sec);
				if (last != _movements.rend() && curr_move->first == last->first) { //good enough
					_playback_start_sec = 0;
				}
				*curr_pos = curr_move->second.first;
				*curr_rot = curr_move->second.second;
			}
			if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
				if (_start_sec == 0) {
					_start_sec = GetGameRuntimeSecs();
					_movements.clear();
				}
				_movements.emplace(GetGameRuntimeSecs() - _start_sec, std::pair<Vector4, Vector4>(*curr_pos, *curr_rot));
			}
			else {
				_start_sec = 0;
			}
		}
	}


private:
	double _playback_start_sec = 0;
	double _start_sec = 0;
	std::map<double, std::pair<Vector4, Vector4>> _movements{};
	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(freecam_movement_recorder);
