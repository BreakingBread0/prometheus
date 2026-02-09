#ifndef PROMETHEUS_COMPONENT_WINDOW_H
#define PROMETHEUS_COMPONENT_WINDOW_H

#include "STU.h"
#include "../window_manager/window_manager.h"
#include "Components/Component.h"
#include "UI/imgui_helpers.h"
#include "UI/windows/stu_explorer.h"

struct strange_compostru {
    __int64 field_0;
    __int64 field_8;
    __int64 field_10;
    int field_18;
};

class component_window : public window {
    WINDOW_DEFINE(component_window, "ECS", "Component Infos", true);

    inline void render() override {
        if (open_window()) {
            bool save_mirrordata_hashes = ImGui::Button("Save MirrorData hashes");
            if (imgui_helpers::beginTable("components", {"ID", "Size", "VTable", "MirrorData", "Creator VT", "CC Flags", "field_18"})) {
                for (auto& component : s_compos) {
                    ImGui::TableNextRow();
                    ImGui::PushID(component.first); //should be unique

                    ImGui::TableNextColumn();
                    ImGui::Text("%x", component.first);

                    ImGui::TableNextColumn();
                    ImGui::Text("%x", component.second.size);

                    ImGui::TableNextColumn();
                    display_addr(component.second.vtable);

                    ImGui::TableNextColumn();
                    if (component.second.mirrordata) {
                        imgui_helpers::display_type(component.second.mirrordata->Hash, true, true, false);
                        ImGui::SameLine();
                        if (ImGui::Button("Show")) {
                            stu_explorer::get_latest_or_create(this)->navigate_to(component.second.mirrordata, 0, nullptr);
                        }
                        if (save_mirrordata_hashes) {
                            stringhash_library::add_comment(component.second.mirrordata->Hash, std::format("MirrorData for 0x{:x}", component.first), true);
                        }
                    }

                    ImGui::TableNextColumn();
                    display_addr(component.second.compo_creator);

                    ImGui::TableNextColumn();
                    ImGui::Text("%p", component.second.cc_flags);

                    ImGui::TableNextColumn();
                    display_addr(component.second.field_18);

                    ImGui::PopID();
                }

                ImGui::EndTable();
            }
            if (imgui_helpers::beginTable("compostru", {"index", "field_0", "field_8", "field_10", "field_18"})) {
                auto strange = (strange_compostru*)(globals::gameBase + 0x1828eb0);
                for (int i = 0; i < GameEntityAdmin()->creation_info->MAX_COMPONENT_ID; i++) {
                    auto stru = strange[i];
                    if (!stru.field_0 && !stru.field_8 && !stru.field_10 && !stru.field_18)
                        continue;

                    ImGui::TableNextRow();
                    ImGui::PushID(i);

                    ImGui::TableNextColumn();
                    ImGui::Text("%x", i);

                    ImGui::TableNextColumn();
                    display_addr(stru.field_0);

                    ImGui::TableNextColumn();
                    display_addr(stru.field_8);

                    ImGui::TableNextColumn();
                    display_addr(stru.field_10);

                    ImGui::TableNextColumn();
                    ImGui::Text("%x (%d)", stru.field_18, stru.field_18);

                    ImGui::PopID();
                }

                ImGui::EndTable();
            }
        }
        ImGui::End();
    }

private:
    struct compoinfo {
        int size;
        __int64 vtable;
        __int64 compo_creator;
        unsigned __int64 cc_flags;
        STUInfo* mirrordata;
        __int64 field_18;
    };

    static inline std::map<int, compoinfo> s_compos;
    static inline std::once_flag s_compoinfo_init;

    //inline void preStartInitialize() override {}
    inline void initialize() override {
        std::call_once(s_compoinfo_init, []() {
            auto rtti = ComponentRTTI::Get();
            while (rtti) {
                compoinfo comp{};
                auto creator = rtti->create_componentCreator();
                owassert(creator);
                comp.cc_flags = creator->iwelche_flags;
                comp.field_18 = rtti->field_18;

                //printf("%hhx %hhx\n", creator->component_id, rtti->component_id);
                owassert(creator->component_id == (unsigned char)rtti->component_id);
                comp.compo_creator = (__int64)creator->vfptr;
                comp.size = creator->iclass->item_size;
                auto test_compo = creator->vfptr->create_component(creator);
                owassert(test_compo);
                comp.vtable = (__int64)test_compo->vfptr;
                auto test_mirrordata = test_compo->GetMirrorData();
                if (test_mirrordata) {
                    comp.mirrordata = ((STUBase<>*)test_mirrordata)->vfptr->GetSTUInfo();
                } else {
                    comp.mirrordata = nullptr;
                }
                creator->iclass->vfptr->instance_unlink(creator->iclass, (void*)test_compo);
                creator->vfptr->destruct(creator, 1);

                s_compos.emplace(rtti->component_id, std::move(comp));

                rtti = rtti->next;
            }
        });
    }
};

WINDOW_REGISTER(component_window);

#endif //PROMETHEUS_COMPONENT_WINDOW_H