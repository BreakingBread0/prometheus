// Hazno - 2025

#include "statescript_registry.h"

void statescript_registry::render() {
		static int count = 0;
		if (open_window()) {
			ImGui::Spacing();
			ImGui::Text("%d Entries -  Search RTTI", count);
			ImGui::SameLine(0, 15.0f);
			_search.search_box("");
			ImGui::Separator();
			ImGui::Spacing();

			StatescriptRTTI* rtti = *(StatescriptRTTI**)(globals::gameBase + 0x18f8e88);
			headerBase = *(STURegistry**)(globals::gameBase + 0x18f74e0);

			count = 0;
			const auto curStyle = ImGui::GetStyle();
			imgui_helpers::beginTable("SSRTTI", { "STUInfo->Hash" ,"field_8", "*field_8", "*(field_8+8)", "factory_func", "inherit", "field_20", "field_28", "*field_28"},
				ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersH | ImGuiTableFlags_HighlightHoveredColumn | ImGuiTableFlags_RowBg);

			while (rtti) {
				count++;
				const auto header = FindRegistryForRTTI(rtti);
				if (!header || !CanShowRTTI(rtti, header)) {
					rtti = rtti->next_instance;
					continue;
				}

				ImGui::TableNextRow();
				ImGui::PushID(rtti);
				ImGui::TableNextColumn();

				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, curStyle.Colors[ImGuiCol_Header]);
				ImGui::PushStyleColor(ImGuiCol_HeaderActive, curStyle.Colors[ImGuiCol_Header]);
				if (ImGui::Selectable("", selected == rtti, ImGuiSelectableFlags_NoAutoClosePopups | ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap, ImVec2(0, 20.0f))) {
					selected = rtti;
				}
				ImGui::PopStyleColor(2);

				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(6.0f, 6.0f));

				ImGui::SameLine();
				allmighty_hash_lib::display_hash(header->info->name_hash);

				ImGui::TableNextColumn();
				if (rtti->field_8 - (__int64)rtti != 0x10) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 0, 0, 255));
					ImGui::Text("%x", rtti->field_8 - (__int64)rtti);
					ImGui::PopStyleColor();
				}
				else {
					ImGui::Text("ok");
				}

				ImGui::TableNextColumn();
				display_addr(*(__int64*)rtti->field_8);

				ImGui::TableNextColumn();
				display_addr(*(__int64*)(rtti->field_8 + 8));

				ImGui::TableNextColumn();
				display_addr((__int64)rtti->factory_func);

				ImGui::TableNextColumn();
				display_addr((__int64)rtti->stuclass_inherit);

				ImGui::TableNextColumn();
				display_addr(rtti->field_20);

				ImGui::TableNextColumn();
				display_addr(rtti->field_28);

				ImGui::TableNextColumn();
				display_addr(*(int*)(rtti->field_28 + 4));

				ImGui::PopID();
				ImGui::PopStyleVar();
				rtti = rtti->next_instance;
			}

			ImGui::EndTable();
		}
	}


bool statescript_registry::CanShowRTTI(StatescriptRTTI* rtti, STURegistry* header)
{
	if (_search.needs_haystack()) {
		//will retrigger another hash-name lookup but its only when the lookup is dirty so... not a big deal
		_search.haystack_stringhash(header->info->name_hash);
	}

	return _search.found_needle(rtti);
}

STURegistry* statescript_registry::FindRegistryForRTTI(const StatescriptRTTI* rtti) const
{
	auto cur = headerBase;
	while (cur != nullptr) {
		if (cur->info->rtti_info == rtti->stuclass_inherit) {
			return cur;
		}
		cur = cur->next;
	}
	return nullptr;
}
