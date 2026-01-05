#pragma once
#include "../window_manager/window_manager.h"
#include "../entity_admin.h"
#include <sstream>

class pachimari_creator : public window {
	WINDOW_DEFINE(pachimari_creator, "Tools", "Pachimari Creator", true);

	inline void render() override {
		if (open_window()) {
			if (ImGui::BeginListBox("", ImVec2(-5, 400))) {
				for (auto item : _created_ents) {
					if (!item) {
						ImGui::Text("Invalid Ent");
						continue;
					}
					ImGui::PushID(item);
					char buf[32];
					EntityIDToString(&item->entity_id, buf, sizeof(buf));
					if (ImGui::RadioButton(buf, _selected_ent == item)) {
						_selected_ent = item;
					}
					ImGui::PopID();
				}

				ImGui::EndListBox();
			}
			if (_selected_ent) {
				auto scenedata = _selected_ent->getById<Component_1_SceneRendering>(1);
				ImGui::Text("Position: %f %f %f", scenedata->position.X, scenedata->position.Y, scenedata->position.Z);
				ImGui::Text("Rotation: %f %f %f %f", scenedata->rotation.X, scenedata->rotation.Y, scenedata->rotation.Z);
				ImGui::Text("Scaling: %f %f %f", scenedata->scaling.X, scenedata->scaling.Y, scenedata->scaling.Z);
				ImGui::PushID("scale");
				if (ImGui::Button("++")) {
					Vector4 scaling(scenedata->scaling.X + 1.f);
					scenedata->SetScale(scaling);
				}
				ImGui::SameLine();
				if (ImGui::Button("+")) {
					Vector4 scaling(scenedata->scaling.X + 0.1f);
					scenedata->SetScale(scaling);
				}
				ImGui::SameLine();
				if (ImGui::Button("-")) {
					Vector4 scaling(scenedata->scaling.X - 0.1f);
					scenedata->SetScale(scaling);
				}
				ImGui::SameLine();
				if (ImGui::Button("--")) {
					Vector4 scaling(scenedata->scaling.X - 1.f);
					scenedata->SetScale(scaling);
				}
				ImGui::PopID();
			}

			ImGui::Checkbox("Create", &_create);
			if (_create && ImGui::IsKeyPressed(ImGuiKey_LeftShift, false)) {
				auto ea = GameEntityAdmin();
				auto local_ent = ea->getLocalEnt();
				auto model_ent = ea->getEntById(local_ent->getById<Component_20_ModelReference>(0x20)->cam_attach_entid);
				auto scenedata = model_ent->getById<Component_1_SceneRendering>(1);

				auto loader = EntityLoader::Create(0x04000000000006CA, try_load_resource(0x04000000000006CA), false, false);
				auto init_data = Component_1_SceneRendering::InitData{};

				init_data.position = scenedata->position;
				init_data.rotation = scenedata->rotation;
				init_data.scale = Vector4(1);

				loader->loader_entries[1].init_data = (__int64)&init_data;
				_created_ents.push_back(loader->Spawn(ea));
			}

			if (ImGui::Button("Copy")) {
				std::stringstream str{};

				for (auto ent : _created_ents) {
					char buf[256]; //laziness is my passion

					auto scenedata = ent->getById<Component_1_SceneRendering>(1);
					sprintf_s(buf, "pachipachi(Vector4(%f, %f, %f, %f), Vector4(%f, %f, %f, %f), Vector4(%f, %f, %f, %f));",
						scenedata->position.X, scenedata->position.Y, scenedata->position.Z, 0,
						scenedata->rotation.X, scenedata->rotation.Y, scenedata->rotation.Z, 0,
						scenedata->scaling.X, scenedata->scaling.Y, scenedata->scaling.Z, 0
					);
					str << buf << std::endl;
				}

				imgui_helpers::openCopyWindow(str.str());
			}
		}
		ImGui::End();
	}

	std::vector<Entity*> _created_ents{};
	Entity* _selected_ent = nullptr;
	bool _create = true;

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(pachimari_creator);
