#pragma once
#include "../window_manager/window_manager.h"
#include "../ResourceManager.h"


class asset_pack_manager : public window {
	WINDOW_DEFINE(asset_pack_manager, "Resource", "Asset Pack Manager", true);

	void print_asset_pack_list(__int64 addr) {
		auto arr = *(AssetPackListItem**)addr;
		int count = *(int*)(addr + 8);
		
		ImGui::Text("Size: %d", count);
		if (imgui_helpers::beginTable("asset pack", { "i", "field_0", "tact_key", "assetPack", "field_2c", "field_38", "field_40" })) {
			for (int i = 0; i < count; i++) {
				auto item = &arr[i];

				ImGui::TableNextRow();

				ImGui::TableNextColumn();
				ImGui::Text("%d", i);

				ImGui::TableNextColumn();
				display_addr(item->field_0);

				ImGui::TableNextColumn();
				display_addr(item->tact_key_used);

				ImGui::TableNextColumn();
				display_addr(item->assetPackId);

				ImGui::TableNextColumn();
				ImGui::Text("%d", item->field_2C);

				ImGui::TableNextColumn();
				display_addr(item->field_38);

				ImGui::TableNextColumn();
				display_addr(item->field_40);
			}
			ImGui::EndTable();
		}
	}

	inline void render() override {
		if (open_window()) {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     
			__int64 apm = globals::gameBase + 0x17a98c0;
			ImGui::Text("Asset Pack List 1:");
			print_asset_pack_list(apm + 0x38);
			ImGui::Text("Asset Pack List 2:");
			print_asset_pack_list(apm + 0x50);
		}

		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(asset_pack_manager);
