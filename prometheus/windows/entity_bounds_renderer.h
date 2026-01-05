#pragma once
#include "../window_manager/window_manager.h"
#include "../ImguiRenderer.h"
#include "../entity_admin.h"
#include "../Statescript.h"
#include "entity_window.h"
#include "../STUConfigVar_Custom.h"

//TODO: Apparently does not take the camera FOV into account. But who cares.
class entity_bounds_renderer : public window {
	WINDOW_DEFINE_ARG(entity_bounds_renderer, "Tools", "Entity Bounds Renderer", Entity*);

	const int color = 0xFF00FFFF;

	/*void drawLine(Vector3 from, Vector3 to) {
		auto pos1 = _camera->WorldToScreen(from);
		auto pos2 = _camera->WorldToScreen(to);
		_renderer->DrawLine(ImVec2(pos1.X, pos1.Y), ImVec2(pos2.X, pos2.Y), color);
	}*/

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
			ImGui::Text("Pos: %f %f %f", _pos.X, _pos.Y, _pos.Z);
			ImGui::Text("Bounds: %f %f %f", _bounds_size.X, _bounds_size.Y, _bounds_size.Z);
		}
		ImGui::End();
		_renderer = ImguiRenderer::GetInstance();
		_renderer->BeginScene();
		
		//get_ent_cv.initialize_configVar();
		//get_ent_cv.get_argument_primitive("m_value").set_value(_arg->entity_id);
		StatescriptPrimitive ent;
		ent.type = StatescriptPrimitive_ENTITY;
		ent.value = _arg->entity_id;
		auto get_ent_cv = STUConfigVar_Custom(ent);

		{
			auto get_pos_cv = STU_Object::create(GetSTUInfoByHash(0xd7aa244a)); //Get entity root pos
			get_pos_cv.initialize_configVar();
			get_pos_cv.set_object("m_entity", get_ent_cv.get()->base.to_editable());
			auto cv = (STUConfigVar*)get_pos_cv.value;
			StatescriptPrimitive output{};
			cv->cv_impl->vfptr->GetConfigVarValue(cv->cv_impl, _ss, cv, &output);
			_pos = output.get_vec3();
			get_pos_cv.deallocate();
		}
		{
			auto get_pos_cv = STU_Object::create(GetSTUInfoByHash(0x8d4869d1)); //Get entity bounds size
			get_pos_cv.initialize_configVar();
			get_pos_cv.set_object("m_entity", get_ent_cv.get()->base.to_editable());
			auto cv = (STUConfigVar*)get_pos_cv.value;
			StatescriptPrimitive output{};
			cv->cv_impl->vfptr->GetConfigVarValue(cv->cv_impl, _ss, cv, &output);
			_bounds_size = output.get_vec3();
			get_pos_cv.deallocate();
		}

		/*drawLine(Vector3(_pos.X + _bounds_size.X, _pos.Y + _bounds_size.Y, _pos.Z + _bounds_size.Z), Vector3(_pos.X + _bounds_size.X, _pos.Y - _bounds_size.Y, _pos.Z + _bounds_size.Z));
		drawLine(Vector3(_pos.X - _bounds_size.X, _pos.Y + _bounds_size.Y, _pos.Z + _bounds_size.Z), Vector3(_pos.X - _bounds_size.X, _pos.Y - _bounds_size.Y, _pos.Z + _bounds_size.Z));
		drawLine(Vector3(_pos.X + _bounds_size.X, _pos.Y + _bounds_size.Y, _pos.Z - _bounds_size.Z), Vector3(_pos.X + _bounds_size.X, _pos.Y - _bounds_size.Y, _pos.Z - _bounds_size.Z));
		drawLine(Vector3(_pos.X - _bounds_size.X, _pos.Y + _bounds_size.Y, _pos.Z - _bounds_size.Z), Vector3(_pos.X - _bounds_size.X, _pos.Y - _bounds_size.Y, _pos.Z - _bounds_size.Z));*/

		if (_bounds_size.X == 0 && _bounds_size.Y == 0) {
			_renderer->DrawCircleFilled(ImVec2(_pos.X, _pos.Y), 20, color);
		}
		else {
			_renderer->DrawBox(ImVec2(_pos.X - _bounds_size.X / 2, _pos.Y - _bounds_size.Y / 2), ImVec2(_pos.X + _bounds_size.X / 2, _pos.Y + _bounds_size.Y / 2), color);
		}
		_renderer->EndScene();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
private:
	StatescriptInstance* _ss = nullptr;
	Component_4F_Camera* _camera = nullptr;
	ImguiRenderer* _renderer = nullptr;
	Vector3 _pos;
	Vector3 _bounds_size;
};

WINDOW_REGISTER(entity_bounds_renderer);
