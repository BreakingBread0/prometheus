#pragma once
#include "../window_manager/window_manager.h"
#include "../globals.h"
#include <MinHook.h>
#include "../ResourceManager.h"
#include <mutex>
#include "../filetype_library.h"

class loadlist_visualizer : public window {
	WINDOW_DEFINE(loadlist_visualizer, "Resource", "LoadList Visualizer", true);

	inline __int64 printHistory(PrioritizedLinkedList* list) {
		ImGui::Text("PrioritizedList Base: %d (%d/%d/%d/%d/%d/%d)", 
			list->total_item_count,
			list->lists[0].item_count,
			list->lists[1].item_count,
			list->lists[2].item_count,
			list->lists[3].item_count,
			list->lists[4].item_count,
			list->lists[5].item_count
		);
		//Now would probably be a good decision to re-evaluate all my life choices that have led me to write this.
		__int64 clickyclank = 0;

		for (int i = 0; i < 6; i++) {
			PriorityListQueueStart* start = &list->lists[i];
			ImGui::SeparatorText(("Index " + std::to_string(i)).c_str());
			ImGui::Text("Count %d", start->item_count);
			ClonedResourceLoadEntry* entry = (ClonedResourceLoadEntry*)start->front;
			while (entry) {
				ImGui::PushID(entry->old_location);
				display_addr(entry->old_location, "Old location");
				ImGui::SameLine();
				if (ImGui::Button("Track"))
					clickyclank = entry->old_location;
				ImGui::Text("ManagerInfo Index: %d - ", entry->loaded_entry.manager_info_index);
				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(0, 255, 0, 255)));
				manager_info* manager = ((manager_info**)(globals::gameBase + 0x182c940))[entry->loaded_entry.manager_info_index];
				ImGui::Text("%s", manager->name);
				ImGui::PopStyleColor();
				ImGui::Text("Resource State: %d", entry->loaded_entry.resource_state);
				ImGui::Text("Requested Term: %s", entry->loaded_entry.requested_termination ? "TRUE" : "FALSE");
				ImGui::Text("Prio Override: %d", entry->loaded_entry.prio_override);
				ImGui::Text("_two: %d - ffff: %x", entry->loaded_entry._two, entry->loaded_entry.xFFFFFFFF);
				ImGui::Text("State Changes: %d %x", entry->loaded_entry.state_changes, entry->loaded_entry.state_changes);
				display_addr(entry->loaded_entry.manager_negator, "Negator");
				ImGui::Text("Ressouce ID: ");
				ImGui::SameLine();
				filetype_library::display_filetype(entry->loaded_entry.resource_id);

				ImGui::Separator();
				entry = (ClonedResourceLoadEntry*)entry->loaded_entry.next;
				ImGui::PopID();
			}
		}
		return clickyclank;
	}

	inline void render() override {
		if (open_window()) {
			std::unique_lock<std::mutex> mutex(s_mutex);
			std::vector<const char*> listitems{};

			for (int i = 0; i < s_history.size(); i++) {
				auto& item = s_history[i];
				char* buf = new char[256];
				sprintf_s(buf, 256, "%d (%d/%d)", i, item.first->total_item_count, item.second->total_item_count);
				listitems.push_back(buf);
			}

			static int current_entry = 0;
			static __int64 entry_trace = 0;
			if (ImGui::BeginChild("history_list", ImVec2(150, 0))) {
				ImGui::ListBox("##History", &current_entry, (const char**)&listitems[0], listitems.size());
			}
			ImGui::EndChild();

			for (auto& inst : listitems)
				delete[] inst;
			ImGui::SameLine();
			if (ImGui::BeginChild("history_outer", ImVec2(0, 0))) {
				int remaining = ImGui::GetWindowContentRegionMax().x - ImGui::GetCursorPos().x;
				PrioritizedLinkedList* entry;

				if (ImGui::BeginChild("history_details1", ImVec2(remaining / 2, entry_trace ? -210 : 0))) {
					__int64 bruh = printHistory(s_history[current_entry].first);
					if (bruh)
						entry_trace = bruh;
				}
				ImGui::EndChild();
				ImGui::SameLine();

				if (ImGui::BeginChild("history_details2", ImVec2(remaining / 2, entry_trace ? -210 : 0))) {
					__int64 bruh = printHistory(s_history[current_entry].second);
					if (bruh)
						entry_trace = bruh;
				}
				ImGui::EndChild();
				
				if (entry_trace) {
					if (ImGui::BeginChild("history_tracer", ImVec2(0, 200))) {
						ImGui::Text("Location: %p", entry_trace);
						ImGui::SameLine();
						if (ImGui::Button("Close##history_tracer")) {
							entry_trace = 0;
						}
						int counter = 0;
						int history_counter = 0;
						int prio = 0;
						int current_state = 0;
						__int64 DATA = 0;
						int last_found = -1;
						if (imgui_helpers::beginTable("history_table", { "i", "idx", "typ", "from", "to" })) {
							for (auto& item : s_history) {
								for (int i = 0; i < 6; i++) {
									ClonedResourceLoadEntry* start = (ClonedResourceLoadEntry*)item.second->lists[i].front;
									bool found = false;
									while (start) {
										if (start->old_location == entry_trace) {
											if (last_found != -1 && history_counter - 1 != last_found) {
												for (int i = last_found; i < history_counter; i++) {
													ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(255, 0, 0, 255)));
													ImGui::TableNextRow();

													ImGui::TableNextColumn();
													ImGui::Text("%d", counter);

													ImGui::TableNextColumn();
													ImGui::Text("%d", i);

													ImGui::TableNextColumn();
													ImGui::Text("MISSING");

													ImGui::TableNextColumn();
													ImGui::TableNextColumn();
													ImGui::PopStyleColor();
													counter++;
												}
											}
											last_found = history_counter;
											found = true;
											if (prio != i) {
												ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(255, 0, 255, 255)));
												ImGui::TableNextRow();

												ImGui::TableNextColumn();
												ImGui::Text("%d", counter);

												ImGui::TableNextColumn();
												ImGui::Text("%d", history_counter);

												ImGui::TableNextColumn();
												ImGui::Text("PRIO");

												ImGui::TableNextColumn();
												ImGui::Text("%d", prio);

												ImGui::TableNextColumn();
												ImGui::Text("%d", i);

												ImGui::PopStyleColor();
												prio = i;
												counter++;
											}
											if (current_state != start->loaded_entry.resource_state) {
												ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(0, 255, 255, 255)));
												ImGui::TableNextRow();

												ImGui::TableNextColumn();
												ImGui::Text("%d", counter);

												ImGui::TableNextColumn();
												ImGui::Text("%d", history_counter);

												ImGui::TableNextColumn();
												ImGui::Text("STATE");

												ImGui::TableNextColumn();
												ImGui::Text("%d", current_state);

												ImGui::TableNextColumn();
												ImGui::Text("%d", start->loaded_entry.resource_state);

												ImGui::PopStyleColor();
												current_state = start->loaded_entry.resource_state;
												counter++;
											}
											if (DATA != start->loaded_entry.DATA) {
												ImGui::TableNextRow();

												ImGui::TableNextColumn();
												ImGui::Text("%d", counter);

												ImGui::TableNextColumn();
												ImGui::Text("%d", history_counter);

												ImGui::TableNextColumn();
												ImGui::Text("DATA");

												ImGui::TableNextColumn();
												display_addr(DATA);

												ImGui::TableNextColumn();
												display_addr(start->loaded_entry.DATA);
												DATA = start->loaded_entry.DATA;
												counter++;
											}
											break;
										}
										start = (ClonedResourceLoadEntry*)start->loaded_entry.next;
									}
								}
								history_counter++;
							}
							ImGui::EndTable();
						}
					}
					ImGui::EndChild();
				}
			}
			ImGui::EndChild();
		}
		ImGui::End();
	}

	static inline std::vector<std::pair<PrioritizedLinkedList*, PrioritizedLinkedList*>> s_history{};
	static inline std::mutex s_mutex;

	typedef __int64(*__WorkLoadList_fn)(__int64, __int64);
	static inline __WorkLoadList_fn __WorkLoadList_orig;
	
	static inline PrioritizedLinkedList* cloneEntries() {
		PrioritizedLinkedList* ll = (PrioritizedLinkedList*)(globals::gameBase + 0x18a1850); //loadList_mutex
		PrioritizedLinkedList* clone = new PrioritizedLinkedList;
		memcpy(clone, ll, sizeof(PrioritizedLinkedList));
		if (ll->total_item_count > 0) {
			for (int i = 0; i < 6; i++) {
				clone->lists[i].back = nullptr;
				ResourceLoadEntry* entry = ll->lists[i].front;
				clone->lists[i].front = nullptr;
				ResourceLoadEntry* prev = nullptr;
				if (entry) {
					while (entry) {
						ClonedResourceLoadEntry* clone_inner = new ClonedResourceLoadEntry;
						memcpy(clone_inner, entry, sizeof(ResourceLoadEntry));
						if (!clone->lists[i].front)
							clone->lists[i].front = &clone_inner->loaded_entry;
						if (prev != nullptr)
							prev->next = &clone_inner->loaded_entry;
						clone_inner->loaded_entry.prev = prev;
						clone_inner->old_location = (__int64)entry;
						prev = &clone_inner->loaded_entry;
						
						if (entry->resource_id == 0x06e0000000000008)
							entry->resource_id = 0x06e0000000000066;

						entry = entry->next;
					}
				}
				clone->lists[i].back = prev;
			}
		}
		return clone;
	}

	static inline void acquire_mutex() {
		if (_InterlockedCompareExchange((volatile unsigned __int32*)(globals::gameBase + 0x18a1850), 1024, 0))
		{
			auto v5 = 1;
			do
			{
				if (v5 > 0x10)
				{
					v5 = 1;
					Sleep(0);
				}
				else
				{
					v5 *= 2;
				}
			} while (_InterlockedCompareExchange((volatile unsigned __int32*)(globals::gameBase + 0x18a1850), 1024, 0));
		}
	}

	static inline void release_mutex() {
		*(volatile unsigned __int32*)(globals::gameBase + 0x18a1850) = 0;
	}
	
	static inline __int64 __WorkLoadList_hook(__int64 a1, __int64 a2) {
		acquire_mutex();
		auto before = cloneEntries();
		release_mutex();
		auto result = __WorkLoadList_orig(a1, a2);
		acquire_mutex();
		auto after = cloneEntries();
		release_mutex();
		std::unique_lock<std::mutex> mutex(s_mutex);
		s_history.push_back({ before, after });
		return result;
	}

	inline void preStartInitialize() override {
		MH_CreateHook((LPVOID)(globals::gameBase + 0x9d2e10), __WorkLoadList_hook, (void**)&__WorkLoadList_orig);
		MH_EnableHook((LPVOID)(globals::gameBase + 0x9d2e10));
	}
	//inline void initialize() override {}
};

WINDOW_REGISTER(loadlist_visualizer);
