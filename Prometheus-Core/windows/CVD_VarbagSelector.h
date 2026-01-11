#pragma once
#include "../window_manager/window_manager.h"
#include "../stringhash_library.h"
#include "../game.h"
#include "CVD_List.h"

class CVD_VarbagSelector : public window {
    WINDOW_DEFINE_ARG(CVD_VarbagSelector, "Statescript", "CVD Varbag Selector", StatescriptSyncMgr*);

    int _selected_varbag = 0;

    void render_instance_varbag(StatescriptInstance* inst) {
        ImGui::PushID(inst);
        if (ImGui::RadioButton("##first", &_selected_varbag, inst->instance_id))
            CVD_List::get_latest_or_create(this)->set(inst->rid_instance_varbag);
        ImGui::SameLine();
        if (ImGui::RadioButton("##second", &_selected_varbag, inst->instance_id | 0xF000))
            CVD_List::get_latest_or_create(this)->set(inst->second_inst_varbag);
        ImGui::SameLine();
        imgui_helpers::display_type(inst->script_id, true, true, false);
        for (auto other_inst : _arg->g1_instanceArr) {
            if (other_inst->parent_instance_id == inst->instance_id) {
                ImGui::Indent();
                render_instance_varbag(other_inst);
                ImGui::Unindent();
            }
        }
        ImGui::PopID();
    }

    inline void navigate_to(StatescriptInstance* ss, STUConfigVarDynamic* cvd) {
        StatescriptPrimitive cv{};
        auto varbag = ss->rid_instance_varbag->base.stu_base.vfptr->GetValueCVD_SearchForRightVarBag(&ss->rid_instance_varbag->base, cvd, &cv);
        if (!varbag) {
            printf("CVD: Could not find varbag\n");
            return;
        }
        ss = varbag->stu_base.vfptr->GetStatescriptInstance(varbag);
        if (ss) {
            _arg = ss->ss_inner;
            _selected_varbag = ss->instance_id;
        }
        else {
            _arg = varbag->stu_base.vfptr->GetStatescriptComponentInner(varbag);
            _selected_varbag = 1; //entity varbag
        }
    }

	inline void render() override {
		if (open_window(nullptr, 0, ImVec2(1300, 650))) {
            //float size_right = ImGui::GetWindowSize().x - _size_left;
            //Splitter(true, 6, &_size_left, &size_right, 50.f, 50.f);
			if (IsBadReadPtr(_arg, 0x100)) {
				ImGui::Text("Invalid Statescript Component");
				return;
			}
			if (ImGui::BeginListBox("##varbags", ImVec2(-10, -10))) {
                if (ImGui::RadioButton("This", &_selected_varbag, 1)) {
                    CVD_List::get_latest_or_create(this)->set(_arg->rid_entity_varbag);
                }
                for (auto inst : _arg->g1_instanceArr) {
                    if (inst->parent_instance_id == 0) {
                        render_instance_varbag(inst);
                    }
                }
				ImGui::EndListBox();
			}
		}
        ImGui::End();
	}

	//inline void preStartInitialize() override {}
	inline void initialize() override {
        this->dock_item_right(CVD_List::create(this), 0.3f);
    }
};

WINDOW_REGISTER(CVD_VarbagSelector);
