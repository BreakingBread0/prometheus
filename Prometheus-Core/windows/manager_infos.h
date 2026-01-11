#pragma once
#include "../window_manager/window_manager.h"

class manager_infos : public window {
	WINDOW_DEFINE(manager_infos, "Resource", "Manager Infos", true);

	inline void render() override {
		if (open_window()) {
			if (imgui_helpers::beginTable("Managers", { "Name", "Supported", "id", "negated", "crc_continued", "field_18", "manager_flags", "field_20", "field_28", "field_30", "field_38", "stu_construct_fn", "field_48", "field_50", "field_58", "field_60" })) {

				for (int i = 0; i < 34; i++) {
					manager_info* manager = ((manager_info**)(globals::gameBase + 0x182c940))[i];
					if (!manager)
						continue;
					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					ImGui::Text("%s", manager->name);

					ImGui::TableNextColumn();
					int* resource_to_manager_map = (int*)(globals::gameBase + 0x146c5f0);
					for (int j = 1; j < 0xB3; j++) {
						if (resource_to_manager_map[j] == i) {
							__int64 ptr = (bitswap(j - 1) / 0x10) & 0x0FFF000000000000;
							if (ptr == 0)
								continue;
							//__int64 type, bool color, bool edit, bool hash_show
							imgui_helpers::display_type(ptr, true, true, true);
						}
					}

					ImGui::TableNextColumn();
					ImGui::Text("%d", manager->manager_id);

					ImGui::TableNextColumn();
					ImGui::Text("%d", manager->manager_negated);

					ImGui::TableNextColumn();
					display_addr(manager->crc_continued);

					ImGui::TableNextColumn();
					display_addr(manager->handles_filetype_ent);

					ImGui::TableNextColumn();
					display_addr(manager->manager_flags);

					ImGui::TableNextColumn();
					display_addr(manager->field_20);

					ImGui::TableNextColumn();
					display_addr((__int64)manager->BeforeDataDeletion);

					ImGui::TableNextColumn();
					display_addr(manager->field_30);

					ImGui::TableNextColumn();
					display_addr(manager->field_38);

					ImGui::TableNextColumn();
					display_addr((__int64)manager->stu_construct_fn);

					ImGui::TableNextColumn();
					display_addr((__int64)manager->OnResourceLoaded);

					ImGui::TableNextColumn();
					display_addr(manager->field_50);

					ImGui::TableNextColumn();
					display_addr((__int64)manager->BeforeDeallocateError);

					ImGui::TableNextColumn();
					display_addr(manager->field_60);
				}

				ImGui::EndTable();
			}
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(manager_infos);