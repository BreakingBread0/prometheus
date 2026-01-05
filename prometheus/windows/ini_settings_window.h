#pragma once
#include "../window_manager/window_manager.h"
#include "../globals.h"
#include <format>
#include <string>
#include "../JAM.h"

class ini_settings_window : public window {
	WINDOW_DEFINE(ini_settings_window, "Game", "Ini Settings", true);

	std::string ini_type_to_string(int type) {
		switch (type) {
		case INI_TYPE_BOOL:
			return "INI_TYPE_BOOL";
		case INI_TYPE_INT:
			return "INI_TYPE_INT";
		case INI_TYPE_CHAR:
			return "INI_TYPE_CHAR";
		case INI_TYPE_QWORD:
			return "INI_TYPE_QWORD";
		case INI_TYPE_FLOAT:
			return "INI_TYPE_FLOAT";
		case INI_TYPE_STRING:
			return "INI_TYPE_STRING";
		}
		return "UNKNOWN (" + std::to_string(type) + ")";
	}

	enum ini_setting_type {
		INI_TYPE_BOOL = 1,
		INI_TYPE_INT = 2,
		INI_TYPE_CHAR = 3,
		INI_TYPE_QWORD = 4,
		INI_TYPE_FLOAT = 5,
		INI_TYPE_STRING = 9
	};

	struct iniSetting {
		__int64 vtable;
		string_rep setting_name;
		string_rep setting_description;
		string_rep parsed_value;
		int ini_type;
		char pad[12];

		__int64 on_update_func;
		char pad_1[10];

		char has_setting_name;
		char sanity_check_enable;
		char is_value_set;
		char pad_2[2];

		union {
			__int64* qwordVal;
			int* intVal;
			char* charVal;
			float* floatVal;
		};
		//__int64 current_value;
		int default_value;
		int min_value;
		int max_value;
		char pad_3[4];
	};

	inline void render() override {
		if (open_window(nullptr, ImGuiWindowFlags_HorizontalScrollbar)) {
			DWORD_PTR ini_base = *(DWORD_PTR*)(globals::gameBase + 0x1859ed0);
			int iniSettSize = *(int*)(globals::gameBase + 0x1859ed0 + 8);
			ImGui::Text("Ini Settings Size: %d (%x)", iniSettSize, iniSettSize);

			ImGui::BeginTable("Ini Settings Table", 14, ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_NoKeepColumnsVisible | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable);
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Description");
			ImGui::TableSetupColumn("String Value");
			ImGui::TableSetupColumn("Ini Type");
			ImGui::TableSetupColumn("VALUE");
			ImGui::TableSetupColumn("DEF");
			ImGui::TableSetupColumn("MIN");
			ImGui::TableSetupColumn("MAX");
			ImGui::TableSetupColumn("hasSettingName");
			ImGui::TableSetupColumn("hasSanityCheck");
			ImGui::TableSetupColumn("hasValueSet");
			ImGui::TableSetupColumn("On Update Func");
			ImGui::TableSetupColumn("VTable");
			ImGui::TableSetupColumn("Actions");
			ImGui::TableHeadersRow();

			for (int i = 0; i < iniSettSize; i++) {
				ImGui::TableNextRow();

				iniSetting* ini = (iniSetting*)*(DWORD_PTR*)(ini_base + 8 * i);
				ImGui::PushID(ini);
				DWORD_PTR vtable = ini->vtable;

				auto str = (string_rep*)(ini + 8);
				ImGui::TableNextColumn();
				ImGui::Text("%s", ini->setting_name.get());

				ImGui::TableNextColumn();
				ImGui::Text("%s", ini->setting_description.get());
				
				ImGui::TableNextColumn();
				ImGui::Text("%s", ini->parsed_value.get());

				ImGui::TableNextColumn();
				ImGui::Text("%s", ini_type_to_string(ini->ini_type).c_str());

				ImGui::TableNextColumn();
				switch (ini->ini_type) {
				case INI_TYPE_BOOL:
					ImGui::Text("%s (%d)", *ini->charVal != 0 ? "YES" : "NO", *ini->charVal);
					break;
				case INI_TYPE_INT:
					ImGui::Text("%d %x", *ini->intVal, *ini->intVal);
					break;
				case INI_TYPE_CHAR:
					ImGui::Text("%d %x", *ini->charVal, *ini->charVal);
					break;
				case INI_TYPE_QWORD:
					ImGui::Text("%llu %p", *ini->qwordVal, *ini->qwordVal);
					break;
				case INI_TYPE_FLOAT:
					ImGui::Text("%f", *ini->floatVal);
					break;
				case INI_TYPE_STRING:
					ImGui::Text("%s", ini->qwordVal);
					break;
				}
				ImGui::Text("%p", ini->qwordVal);
				if (ImGui::Button("Copy")) {
					imgui_helpers::openCopyWindow((__int64)ini->qwordVal);
				}
				ImGui::SameLine();
				if (ImGui::Button("+++")) {
					switch (ini->ini_type) {
					case INI_TYPE_BOOL:
					case INI_TYPE_CHAR:
						*ini->charVal = *ini->charVal + 1;
						break;
					case INI_TYPE_INT:
						*ini->intVal = *ini->intVal + 1;
						break;
					case INI_TYPE_QWORD:
						*ini->qwordVal = *ini->qwordVal + 1;
						break;
					case INI_TYPE_FLOAT:
						*ini->floatVal = *ini->floatVal + 1.f;
						break;
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("---")) {
					switch (ini->ini_type) {
					case INI_TYPE_BOOL:
					case INI_TYPE_CHAR:
						*ini->charVal = *ini->charVal - 1;
						break;
					case INI_TYPE_INT:
						*ini->intVal = *ini->intVal - 1;
						break;
					case INI_TYPE_QWORD:
						*ini->qwordVal = *ini->qwordVal - 1;
						break;
					case INI_TYPE_FLOAT:
						*ini->floatVal = *ini->floatVal - 1.f;
						break;
					}
				}

				ImGui::TableNextColumn();
				ImGui::Text("%d", ini->default_value);

				ImGui::TableNextColumn();
				ImGui::Text("%d", ini->min_value);

				ImGui::TableNextColumn();
				ImGui::Text("%d", ini->max_value);

				ImGui::TableNextColumn();
				ImGui::Text("%s", ini->has_setting_name ? "YES" : "NO");

				ImGui::TableNextColumn();
				ImGui::Text("%s", ini->sanity_check_enable ? "YES" : "NO");

				ImGui::TableNextColumn();
				ImGui::Text("%s", ini->is_value_set ? "YES" : "NO");

				ImGui::TableNextColumn();
				ImGui::Text("%p", ini->on_update_func);

				ImGui::TableNextColumn();
				ImGui::Text("%p %p", vtable, vtable - globals::gameBase);

				ImGui::TableNextColumn();
				ImGui::Text("<todo>");

				ImGui::PopID();

				//printf("Option (%d) %p / %p: %s ---- %s\n", i, ini, vtable - globals::gameBase, str, ini + 0x48);
			}

			ImGui::EndTable();
		}
		ImGui::End();
	}
};


WINDOW_REGISTER(ini_settings_window)