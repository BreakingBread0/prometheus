#ifndef PROMETHEUS_EFFECT_TESTER_H
#define PROMETHEUS_EFFECT_TESTER_H

#include "../window_manager/window_manager.h"

class effect_tester : public window {
    WINDOW_DEFINE(effect_tester, "Game", "Effect Tester", true);

    inline void render() override {
        if (open_window()) {
            ImGui::Text("Tests effects");
            if (ImGui::Button("Refresh available effects"))
                refresh_effects();
            if (ImGui::BeginListBox("##Effects")) {
                for (auto effect : _avail_effects) {
                    ImGui::PushID(effect);
                    if (ImGui::RadioButton("", _chosen_effect == effect))
                        _chosen_effect = effect;
                    ImGui::SameLine();
                    imgui_helpers::display_type(effect, true, true, false);
                    ImGui::PopID();
                }
                ImGui::EndListBox();
            }
            imgui_helpers::InputHex("Parent Entity ID", &_entity_id);
            Entity* parent = GameEntityAdmin()->getEntById(_entity_id);
            if (!parent) {
                ImGui::BeginDisabled();
                ImGui::Text("Invalid Parent Entity");
            }
            if (ImGui::Button("Spawn")) {

            }
            if (!parent)
                ImGui::EndDisabled();
        }
        ImGui::End();
    }

private:
    void refresh_effects() {
        const __int64 effect_type = 0x300000000000000;
        _avail_effects.clear();
        for (int i = 0; i < 0x10000; i++) {
            __int64 effect = effect_type | i;
            if (try_load_resource(effect)) {
                _avail_effects.push_back(effect);
            }
        }
    }

    __int64 _entity_id;
    __int64 _chosen_effect;
    __int64 _spawned_entity_id;
    std::vector<__int64> _avail_effects;
    //inline void preStartInitialize() override {}
    inline void initialize() override {
        refresh_effects();
    }
};

WINDOW_REGISTER(effect_tester);

#endif //PROMETHEUS_EFFECT_TESTER_H