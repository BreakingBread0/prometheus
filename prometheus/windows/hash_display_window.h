#pragma once
#include "../window_manager/window_manager.h"
#include "../stringhash_library.h"
#include <format>

class hash_display_window : public window {
	WINDOW_DEFINE(hash_display_window, "Game", "Hash Display", true);

	inline void render() override {
		if (open_window()) {
			static char buf[256];
			ImGui::InputText("String Hash", buf, 256);
			display_addr(stringHash(buf) & 0xFFFFFFFF, "Result");
			ImGui::SameLine();
			if (ImGui::Button("Add to Library")) {
				allmighty_hash_lib::add_hash(buf);
			}
			ImGui::NewLine();
			ImGui::Separator();

			ImGui::Text("The allmighty string hashing library has a size of %d. ALL PRAISE OUR LORD AND SAVIOUR STRINGHASH", allmighty_hash_lib::hashes.size());

			static char filter[256];
			ImGui::InputText("Filter: ", filter, 256);

			if (imgui_helpers::beginTable("Hashes", { "Hash", "Meaning" })) {
				for (auto& hash : allmighty_hash_lib::hashes) {
					std::string searchFor = hash.second + fmt::format("{:x}", (uint)hash.first);
					if (strlen(filter) > 0 && !icontains(searchFor, filter))
						continue;

					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					display_addr(hash.first);

					ImGui::TableNextColumn();
					display_text((char*)hash.second.c_str());
				}
				ImGui::EndTable();
			}
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(hash_display_window);