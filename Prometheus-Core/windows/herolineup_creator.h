#pragma once
#include "../window_manager/window_manager.h"
#include "../STU.h"
#include "../STU_Editable.h"
#include "../stu_resources.h"
#include "entity_window.h"

class herolineup_creator : public window {
	WINDOW_DEFINE(herolineup_creator, "Tools", "HeroLineup Creator", true);

	inline void render() override {
		if (open_window()) {
			bool copy = ImGui::Button("Copy");
			std::stringstream str{};
			str << "std::vector<lineup> {" << std::endl;
			if (ImGui::BeginListBox("", ImVec2(-5, -5))) {
				int hero_no = 0;
				for (auto& hero : _heroes) {
					ImGui::PushID(hero.hero.value);
					hero_no++;
					//auto hero_no = hero.hero.get_argument_primitive(0xacadfb85).get_value<int>(); //hero no
					ImGui::RadioButton(hero.hero.get_argument_resource(0xcd16baa)->get_UXDisplayString(), &_selected_hero, hero_no); //hero name
					if (hero.ent) {
						if (copy) {
							auto res = hero.ent->resload_entry->align()->resource_id;
							auto scenerendering = hero.ent->getById<Component_1_SceneRendering>(1);
							str << std::format("lineup(0x{:x}, Vector4({:f}, {:f}, {:f}, 0), Vector4({:f}, {:f}, {:f}, 0)),",
								res,
								scenerendering->position.X, scenerendering->position.Y, scenerendering->position.Z,
								scenerendering->rotation.X, scenerendering->rotation.Y, scenerendering->rotation.Z) << std::endl;
						}
						ImGui::SameLine();
						ImGui::Text("%x", hero.ent->entity_id);
						ImGui::SameLine();
						if (ImGui::Button(EMOJI_SHARE)) {
							entity_window::get_latest_or_create(this)->nav_to_ent(hero.ent);
						}
					}
					if (hero_no == _selected_hero && ImGui::IsKeyPressed(ImGuiKey_LeftCtrl, false)) {
						if (hero.ent)
							hero.ent->entity_admin_backref->delEnt(hero.ent);

						auto potg_res = hero.hero.get_argument_resource(0xdf68b9e6)->resource_id;
						auto simple_res = try_load_resource(potg_res);
						if (!simple_res) {
							imgui_helpers::messageBox("Failed to load entity " + std::format("{:x}", potg_res), this);
							return;
						}
						auto loader = EntityLoader::Create(potg_res, simple_res, false, false);
						Component_1_SceneRendering::InitData data{};
						Entity* controller = GameEntityAdmin()->getLocalEnt();
						Entity* model = GameEntityAdmin()->getEntById(controller->getById<Component_20_ModelReference>(0x20)->cam_attach_entid);
						auto comp1 = model->getById<Component_1_SceneRendering>(1);
						if (comp1) {
							data.position = comp1->position;
							data.rotation = comp1->rotation;
							data.scale = comp1->scaling;
							loader->loader_entries[1].init_data = (__int64)&data;
						}
						hero.ent = loader->Spawn(GameEntityAdmin());
						hero_no++;
					}
					ImGui::PopID();
				}
				ImGui::EndListBox();
			}
			str << "}";
			if (copy) {
				imgui_helpers::openCopyWindow(str.str());
			}
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	
	
	inline void initialize() override {
		for (auto item : stu_resources::GetIfAssignableTo(stringHash("STUHero"))) {
			_heroes.push_back(hero{ item.second->to_editable(), nullptr });
		}
	}

private:
	int _selected_hero = 0;
	struct hero {
		STU_Object hero;
		Entity* ent;
	};
	std::vector<hero> _heroes;
};

WINDOW_REGISTER(herolineup_creator);
