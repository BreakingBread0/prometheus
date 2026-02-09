#ifndef PROMETHEUS_STU_ENUM_WINDOW_H
#define PROMETHEUS_STU_ENUM_WINDOW_H

#include "STU.h"
#include "../window_manager/window_manager.h"
#include "UI/imgui_helpers.h"

class stu_enum_window : public window {
    WINDOW_DEFINE_ARG(stu_enum_window, "STU", "Enum Display Window", STUEnumDefinition*);

    inline void render() override {
        if (open_window()) {
            if (_arg == nullptr) {
                ImGui::Text("Invalid STU Enum");
                return;
            }

            ImGui::TextUnformatted("Displaying Enum: ");
            ImGui::SameLine();
            imgui_helpers::display_type(_arg->enum_hash, true, true, false);

            if (imgui_helpers::beginTable("enum", {"Index", "Hash"})) {
                for (int i = 0; i < _arg->values_count; i++) {
                    ImGui::TableNextRow();
                    ImGui::PushID(i);
                    auto value = &_arg->values[i];

                    ImGui::TableNextColumn();
                    ImGui::Text("%d", value->value);

                    ImGui::TableNextColumn();
                    imgui_helpers::display_type(value->hash, true, true, false);

                    ImGui::PopID();
                }

                ImGui::EndTable();
            }
        }
        ImGui::End();
    }

    //inline void preStartInitialize() override {}
    //inline void initialize() override {}
};

WINDOW_REGISTER(stu_enum_window);

#endif //PROMETHEUS_STU_ENUM_WINDOW_H