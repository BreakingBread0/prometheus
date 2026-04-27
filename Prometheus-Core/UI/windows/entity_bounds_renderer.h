#pragma once
#include "../window_manager/window_manager.h"
#include "ImguiRenderer.h"
#include "entity_admin.h"
#include "Statescript.h"
#include "entity_window.h"
#include "STU_Editable.h"
#include "STUConfigVar_Custom.h"

class entity_bounds_renderer : public window {
	WINDOW_DEFINE_ARG(entity_bounds_renderer, "Tools", "Entity Bounds Renderer", Entity*);

	const int color = 0xFF00FFFF;

	void drawLine(Vector3 from, Vector3 to) {
		_renderer->DrawLine(ImVec2(from.X, from.Y), ImVec2(to.X, to.Y), IM_COL32(0, 200, 0, 255));
	}

	inline void render() override {
		if (open_window(nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			if (IsBadReadPtr(_arg, sizeof(Entity))) {
				queue_deletion();
				return;
			}
			if (!_ss || IsBadReadPtr(_ss, sizeof(StatescriptInstance))) {
				auto ss_comp = _arg->getById<Component_23_Statescript>(0x23);
				if (ss_comp) {
					for (auto script : ss_comp->ss_inner.g1_instanceArr) {
						_ss = script;
						break;
					}
				}
				//Fallback
				if (!_ss) {
					auto ea = GameEntityAdmin();
					auto local_ent = ea->getEntById(ea->local_entid);
					if (local_ent) {
						auto model_ent = local_ent->getById<Component_20_ModelReference>(0x20)->cam_attach_entid;
						ss_comp = ea->getEntById(model_ent)->getById<Component_23_Statescript>(0x23);
						if (ss_comp) {
							for (auto script : ss_comp->ss_inner.g1_instanceArr) {
								_ss = script;
								break;
							}
						}
					}
				}
				if (!_camera) {
					_camera = GameEntityAdmin()->getSingletonComponent<Component_4F_Camera>(0x4F);
				}
			}
			ImGui::Text("Entity: %x", _arg->entity_id);
			ImGui::SameLine();
			if (ImGui::Button(EMOJI_SHARE)) {
				entity_window::get_latest_or_create(this)->nav_to_ent(_arg);
			}
			// ImGui::Text("Pos: %f %f %f %f", _pos.X, _pos.Y, _pos.Z, _pos.Z);
			// ImGui::Text("Pos: %f %f %f", _pos.X, _pos.Y, _pos.Z);
			// ImGui::Text("Bounds: %f %f %f", _bounds_size.X, _bounds_size.Y, _bounds_size.Z);
		}
		ImGui::End();
		_renderer = ImguiRenderer::GetInstance();
		_renderer->BeginScene();

		auto comp_1 = _arg->getById<Component_1_SceneRendering>(1);

		auto camera = GameEntityAdmin()->vfptr->GetCameraComponent(GameEntityAdmin());
		_pos = camera->WorldToScreen(comp_1->position);

		_renderer->DrawCircle(ImVec2(_pos.X, _pos.Y), 5, color);

		Vector4 bb[8]{};
		comp_1->GetBoundingBoxes(bb);
		for (int i = 0; i < 8; i++)
		{
			bb[i] = camera->WorldToScreen(bb[i]);
			// _renderer->DrawOutlinedText(imgui_helpers::BoldFont, std::format("{:d}: {:f} {:f} {:f} {:f}", i, bb[i].X, bb[i].Y, bb[i].Z, bb[i].W), ImVec2(200, 200 + i * 20), 18, color, false);
		}

		drawLine(bb[0], bb[1]);
		drawLine(bb[1], bb[2]);
		drawLine(bb[2], bb[3]);
		drawLine(bb[3], bb[0]);

		drawLine(bb[4], bb[5]);
		drawLine(bb[5], bb[6]);
		drawLine(bb[6], bb[7]);
		drawLine(bb[7], bb[4]);

		drawLine(bb[0], bb[4]);
		drawLine(bb[1], bb[5]);
		drawLine(bb[2], bb[6]);
		drawLine(bb[3], bb[7]);

		//{
		//	auto get_pos_cv = STU_Object::create(STURegistry::Get()->GetSTUInfoByHash(0x8d4869d1)); //Get entity bounds size
		//	get_pos_cv.initialize_configVar();
		//	get_pos_cv.set_object("m_entity", get_ent_cv.get_editable());
		//	auto cv = (STUConfigVar*)get_pos_cv.value;
		//	StatescriptPrimitive output{};
		//	cv->cv_impl->vfptr->GetConfigVarValue(cv->cv_impl, _ss, cv, &output);
		//	_bounds_size = output.get_vec3();
		//	get_pos_cv.deallocate();
		//}

		// auto w2s_pos_fn = (void(*)(Component_4F_Camera*, Entity*, Vector4*))(globals::gameBase + 0xcf0520);
		// auto camera = GameEntityAdmin()->vfptr->GetCameraComponent(GameEntityAdmin());
		// _pos = Vector4();
		// w2s_pos_fn(camera, _arg, &_pos);
		// _bounds_size = Vector3();

		// if (_bounds_size.X == 0 && _bounds_size.Y == 0) {
		// 	//Function 0xcf0520 returns some WEEEEEEEEEEEEEIRD values
		// 	//x,y and z,w are screen-space positions but if the entity is not directly in the center its just... off
		// 	//Also the statescript config var calculates the "screen space position" in some weeeeeeeeeeeeird way (see below)
		// 	// _renderer->DrawCircleFilled(ImVec2(_pos.X, _pos.Y), 10, color);
		// 	// _renderer->DrawCircle(ImVec2(_pos.Y, _pos.W), 10, color);
		// 	// _renderer->DrawCircleFilled(
		// 	// 	ImVec2(
		// 	// 	(_pos.X * 0.5f) + _pos.Z,
		// 	// 	(_pos.Y * 0.5f) + _pos.W),
		// 	// 	10, 0xFFFF00FF);
		// 	_renderer->DrawCircleFilled(ImVec2(_pos.X, _pos.Y), 10, color);
		// }
		// else {
		// 	_renderer->DrawBox(ImVec2(_pos.X - _bounds_size.X / 2, _pos.Y - _bounds_size.Y / 2), ImVec2(_pos.X + _bounds_size.X / 2, _pos.Y + _bounds_size.Y / 2), color);
		// }
		_renderer->EndScene();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
private:
	StatescriptInstance* _ss = nullptr;
	Component_4F_Camera* _camera = nullptr;
	ImguiRenderer* _renderer = nullptr;
	Vector3 _pos{};
	Vector3 _bounds_size{};
};

WINDOW_REGISTER(entity_bounds_renderer);
