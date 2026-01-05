#pragma once
#include "../window_manager/window_manager.h"

class entityadmin_window : public window {
	WINDOW_DEFINE(entityadmin_window, "ECS", "Mapfuncs", true);

	void print_enttiy_admin(__int64 entity_admin) {
		ImGui::PushID(entity_admin);
		int mfo_num = *(int*)(entity_admin + 0xe0);
		ImGui::Text("Mapfunc Outer Arr size: %d", mfo_num);
		ImGui::SameLine();
		bool print = ImGui::Button("Print");
		if (imgui_helpers::beginTable("mfo", { "Instance", "VTable" })) {
			for (int i = 0; i < mfo_num; i++) {
				__int64** mfo_list = *(__int64***)(entity_admin + 0xd8);
				ImGui::TableNextRow();

				ImGui::TableNextColumn();
				display_addr((__int64)mfo_list[i]);

				ImGui::TableNextColumn();
				display_addr(*mfo_list[i]);

				if (print) {
					printf("%p\n", *mfo_list[i] - globals::gameBase);
				}
			}
			ImGui::EndTable();
		}
		ImGui::PopID();
	}

	inline void render() override {
		if (open_window()) {
			/*__int64 entity_admin = *(__int64*)(globals::gameBase + 0x1833e18);
			print_enttiy_admin(entity_admin);*/
			auto entity_admin = *(__int64*)(globals::gameBase + 0x17b7f90);

			teList<__int64>* mapfunc_vt_table = (teList<__int64>*)(entity_admin + 0x180);
			ImGui::Text("Mapfuncs: %d", mapfunc_vt_table->num);
			bool print = ImGui::Button("Print");
			if (imgui_helpers::beginTable("mapfunc_vt", { "id", "PTR" })) {
				for (int i = 0; i < mapfunc_vt_table->num; i++) {
					__int64 vt = *(__int64*)mapfunc_vt_table->ptr[i];
					if (i != 0) {
						vt == *(__int64*)vt;
					}
					if (print)
						printf("%d: %p\n", i, vt - globals::gameBase);

					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text("%d", i);

					ImGui::TableNextColumn();
					display_addr(vt);
				}

				ImGui::EndTable();
			}

			//beginTable("ECS", { "i", "Type", "Address"});
			//__int64 ecs = *(__int64*)(globals::gameBase + 0x181ee10);
			//int i = 0;
			//while (ecs) {
			//	i++;
			//	ImGui::TableNextRow();
			//	
			//	ImGui::TableNextColumn();
			//	ImGui::Text("%d", i);

			//	ImGui::TableNextColumn();
			//	ImGui::TextUnformatted(*(char**)(ecs + 0x118));
			//	__int64 parent = *(__int64*)(ecs);
			//	while (parent) {
			//		ImGui::Text("-> %s", *(char**)(parent + 0x118));
			//		/*ImGui::SameLine();
			//		ImGui::Text("(%s)", *(char**)(ecs + 0x178));*/
			//		parent = *(__int64*)(parent);
			//	}

			//	ImGui::TableNextColumn();
			//	//ImGui::Text("%p", *(__int64*)(ecs + 0x120));
			//	display_addr(*(__int64*)(ecs + 0x120));
			//	ecs = *(__int64*)(ecs + 0x110);
			//}
			//ImGui::EndTable();
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(entityadmin_window);