#pragma once
#include "../window_manager/window_manager.h"
#include "../stringhash_library.h"
#include "../game.h"
#include "../StatescriptVar.h"

//TODO Not everything implemented!
class CVD_Edit : public window {
    WINDOW_DEFINE(CVD_Edit, "Statescript", "Config Var Edit", false);
	
    StatescriptVar_Base* _item;
    char _value[256]{};
    char _value2[256]{};
    char _value3[256]{};
    bool _is_hex = false;
    StatescriptPrimitive _out_type{ 0, 0, StatescriptPrimitive_Type::StatescriptPrimitive_BYTE };

	inline void render() override {
		if (open_window(nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            if (IsBadReadPtr(_item, 8)) {
                ImGui::Text("Invalid Address");
                return;
            }
            display_addr((__int64)_item);
            imgui_helpers::display_type(_item->var_id, true, true, false);
			ImGui::Text("RID Type:");
			ImGui::SameLine();

            if (_item->is_varbag()) {
                ImGui::TextUnformatted("VarBag in VarBag isnt supposed to be implemented");
            }
            else if (_item->is_cvd()) {
                ImGui::Text("Config Var");
                auto globalvar = (StatescriptVar_Primitive*)_item;
                ImGui::Text("Type: %s", globalvar->global_var.get_type_str().c_str());
                ImGui::Text("Current Value: %s", globalvar->global_var.get_value_str().c_str());

                auto parent = _item->parent_rid;
                if (!parent) {
                    ImGui::Text("Invalid parent");
                }
                else if (!parent->base.is_varbag()) {
                    ImGui::Text("Invalid parent type");
                }

                ImGui::Text("OUT: %s", _out_type.get_value_str().c_str());
                if (ImGui::BeginCombo("Type", _out_type.get_type_str().c_str())) {
                    for (int i = 1; i < 0xC; i++) {
                        StatescriptPrimitive temp{};
                        temp.type = (StatescriptPrimitive_Type)i;
                        if (ImGui::Selectable(temp.get_type_str().c_str()))
                            _out_type.type = (StatescriptPrimitive_Type)i;
                    }
                    ImGui::EndCombo();
                }
                if (ImGui::InputText("Value", _value, 256)) {
                    if (_out_type.type == StatescriptPrimitive_Type::StatescriptPrimitive_BYTE ||
                        _out_type.type == StatescriptPrimitive_Type::StatescriptPrimitive_INT ||
                        _out_type.type == StatescriptPrimitive_Type::StatescriptPrimitive_INT64) {
                        __int64 parsed = _strtoi64(_value, nullptr, _is_hex ? 16 : 10);
                        if (errno == EINVAL) {
                            return;
                        }
                        _out_type.value = parsed;
                    }
                    else if (_out_type.type == StatescriptPrimitive_Type::StatescriptPrimitive_FLT) {
                        float value = _strtof_l(_value, nullptr, nullptr);
                        if (errno == EINVAL) {
                            return;
                        }
                        *(float*)&_out_type.value = value;
                    }
                }
                if (_out_type.type == StatescriptPrimitive_Type::StatescriptPrimitive_VEC3) {
                    if (ImGui::InputText("Value2", _value2, 256)) {
                        float value = _strtof_l(_value2, nullptr, nullptr);
                        if (errno == EINVAL) {
                            return;
                        }
                        *(float*)((__int64)&_out_type.value + 4) = value;
                    }
                    if (ImGui::InputText("Value3", _value3, 256)) {
                        float value = _strtof_l(_value3, nullptr, nullptr);
                        if (errno == EINVAL) {
                            return;
                        }
                        *(float*)((__int64)&_out_type.value + 8) = value;
                    }
                }
                ImGui::Checkbox("Hexadecimal", &_is_hex);

                if (ImGui::Button("Set")) {
                    globalvar->base.stu_base.vfptr->Primitive_SetValueNoNotify(&globalvar->base, &_out_type);
                }
                ImGui::SameLine();
                if (ImGui::Button("Set & Notify")) {
                    globalvar->base.stu_base.vfptr->Primitive_SetValueFromRemote(&globalvar->base, globalvar->base.parent_rid->ss_compo_inner, globalvar->base.parent_rid->ss_instance, &_out_type, &globalvar->base.user, false);
                }
            }
            else if (_item->is_dictvar()) {
                ImGui::Text("DictVar");
            }
            else if (_item->is_siref()) {
                ImGui::Text("SI-Ref");
            }
		}
        ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
public:
    CVD_Edit() {}
    CVD_Edit(StatescriptVar_Base* item) : _item(item)  {
	}
};

WINDOW_REGISTER(CVD_Edit);
