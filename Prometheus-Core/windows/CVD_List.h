#pragma once
#include "../window_manager/window_manager.h"
#include "../stringhash_library.h"
#include "../game.h"
#include "CVD_Edit.h"
#include "statescript_window.h"

class CVD_List : public window {
    WINDOW_DEFINE_ARG(CVD_List, "Statescript", "CVD List", StatescriptVar_VarBag*);

    int _selected_varbag = 0;
    const int table_flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersH | ImGuiTableFlags_HighlightHoveredColumn | ImGuiTableFlags_NoSavedSettings;
    void print_varbag_item(StatescriptVar_Base* rid, bool is_this) {
        ImGui::PushID(rid);
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        imgui_helpers::display_type(rid->var_id, true, true, false);
        if (is_this) {
            ImGui::SameLine();
            ImGui::TextUnformatted("(this)");
        }

        ImGui::TableNextColumn();
        ImGui::PushID("subs");
        for (int i = 0; i < rid->subscriptions.num; i++) {
            ImGui::PushID(i);
            StatescriptState* state = rid->subscriptions.ptr[i];
            StatescriptInstance* state_inst = state->ss_instance;
            if (state_inst->graph) {
                int nodes_idx = state_inst->graph->m_nodes.indexof(&state_inst->graph->m_states.list()[state->idx_in_m_states]->ss_base);
                ImGui::Text("%hhx: <%d, %d>", state_inst->instance_id, nodes_idx, state->idx_in_m_states);
                ImGui::SameLine();
                if (ImGui::Button(EMOJI_SHARE)) {
                    statescript_window::get_latest_or_create(this)->display_instance(state_inst, state_inst->graph->m_nodes.list()[nodes_idx]);
                }
            }
            ImGui::PopID();
        }
        ImGui::PopID();

        ImGui::TableNextColumn();
        if (rid->is_varbag()) {
            if (!is_this) {
                StatescriptVar_VarBag* vb = (StatescriptVar_VarBag*)rid;
                ImGui::EndTable();
                ImGui::Indent();
                print_varbag(vb);
                ImGui::Unindent();
                imgui_helpers::beginTable("varbag", 4/*, table_flags*/);
            }
        }
        else if (rid->is_cvd()) {
            StatescriptVar_Primitive* glob_var = (StatescriptVar_Primitive*)rid;
            ImGui::TextUnformatted(glob_var->global_var.get_value_str().c_str());
        }
        else if (rid->is_dictvar()) {
            StatescriptVar_Dictionary* dv = (StatescriptVar_Dictionary*)rid;
            ImGui::Text("Dictionary (Count/Max %d/%d)", dv->items_array.item_count, dv->items_array.max);
            ImGui::Text("Missing Implementation.");
        }
        else if (rid->is_siref()) {
            StatescriptVar_InstanceReference* si_ref = (StatescriptVar_InstanceReference*)rid;
            ImGui::Text("Ref to: %x", si_ref->ss_inst_id);
        }
        else {
            ImGui::Text("Unknown RID Type");
        }

        ImGui::TableNextColumn();
        char buf[32];
        sprintf_s(buf, "Copy##%d");
        if (ImGui::Button(buf)) {
            imgui_helpers::openCopyWindow((__int64)rid);
        }
        ImGui::SameLine();
        if (rid->is_cvd() && ImGui::Button("Edit")) {
            window_manager::add_window(new CVD_Edit(rid), this);
        }
        ImGui::PopID();
    }

	void print_varbag(StatescriptVar_VarBag* vb) {
        ImGui::PushID(vb);
        if (imgui_helpers::beginTable("varbag", { "ID", "Loc", "Value", "Edit" }/*, table_flags*/)) {
            print_varbag_item(&vb->base, true);
            for (int i = 0; i < 16; i++) {
                auto item = &vb->sub_items[i];
                for (int j = 0; j < item->item_count; j++) {
                    ImGui::Indent();
                    print_varbag_item(item->arr_ptr[j].ptr, false);
                    ImGui::Unindent();
                    ImGui::Separator();
                }
            }
            ImGui::EndTable();
        }
        ImGui::PopID();
	}

	inline void render() override {
		if (open_window(nullptr, 0, ImVec2(800, 500))) {
            //float size_right = ImGui::GetWindowSize().x - _size_left;
            //Splitter(true, 6, &_size_left, &size_right, 50.f, 50.f);
			if (IsBadReadPtr(_arg, 0x100)) {
				ImGui::Text("Invalid Varbag");
				return;
			}
            print_varbag(_arg);
		}
        ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(CVD_List);
