#pragma once
#include "../window_manager/window_manager.h"
#include "../Statescript.h"
#include <set>
#include <string>

struct ComponentCreator;
struct ComponentCreator_vt {
	__int64 (*destruct)(ComponentCreator*, char);
};

struct ComponentCreator {
	union {
		ComponentCreator_vt* vfptr;
		STRUCT_PLACE(char, component_id, 8);
		STRUCT_PLACE(StructPageAllocator*, iclass, 0x28);
		STRUCT_PLACE(__int64, iwelche_flags, 0x30);
	};
};

struct ComponentRTTI
{
     ComponentRTTI* next;
     __int16 component_id;
     ComponentCreator*(__fastcall *create_componentCreator)();
     __int64 field_18;
 };

//aka a bulk memory allocator
class blizzbbqueueq_window : public window {
	WINDOW_DEFINE(blizzbbqueueq_window, "Game", "BlizzBBQueueQ", true);

	void print_queue(std::string name, StructPageAllocator* queue) {
		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(name.c_str());
		if (!queue) {
			ImGui::SameLine();
			ImGui::Text("(Empty)");
			return;
		}

		//ImGui::TableNextColumn();
		//ImGui::Text("%d (0x%x)", queue->instance_item_count, queue->instance_item_count);

		ImGui::TableNextColumn();
		ImGui::Text("%d (0x%x)", queue->item_size, queue->item_size);

		ImGui::TableNextColumn();
		ImGui::Text("%d (0x%x)", queue->thread_worker_id, queue->thread_worker_id);

		ImGui::TableNextColumn();
		ImGui::Text("%d (0x%x)", queue->flags, queue->flags);

		ImGui::TableNextColumn();
		ImGui::Text("%d (0x%x)", queue->structs_used, queue->structs_used);

		ImGui::TableNextColumn();
		ImGui::Text("%d (0x%x)", queue->instance_count, queue->instance_count);
	}

	std::vector<ComponentCreator*> _creators;
	inline void render() override {
		static std::vector<std::pair<std::string, __int64>> queues{
			{ "E_List 0", 0x17aafe0 },
			{ "E_List 1", 0x17ab070 },
			{ "E_List 2", 0x17ab100 },
			{ "E_List 3", 0x17ab190 },
			{ "E_List 4", 0x17ab220 },
			{ "E_List 5", 0x17ab2a0 },
			{ "E_List 6", 0x17ab320 },
			{ "E_List 7", 0x17ab3a0 },
			{ "E_List 8", 0x17acdb0 },
			{ "CV String", 0x17b9a20 },
			{ "VVariable Update Queue", 0x180a5f0 },
			{ "Viewmodel WTF", 0x1809048 },
			{ "Viewmodel 2.0 (gone wrong)", 0x180e450 },
			{ "mby_ux_root", 0x180f058 },
			{ "Job queue", 0x1892210 },
			{ "Resourceload queue", 0x17a56a0 },
			{ "Viewmodel shit", 0x18097b8 }
		};
		if (open_window()) {
			if (ImGui::Button("Load Component Creators")) {
				ComponentRTTI* rtti = *(ComponentRTTI**)(globals::gameBase + 0x18f74d0);
				while (rtti) {
					//printf("ID: %d\n", rtti->component_id);
					auto creator = rtti->create_componentCreator();
					if ((creator->iwelche_flags & 0x10) == 0) {
						printf("Component %d is static?\n", creator->component_id);
						creator->vfptr->destruct(creator, 1);
					} else {
						printf("Component %x creator at %p\n", creator->component_id, (__int64)creator->vfptr - globals::gameBase);
						_creators.push_back(creator);
					}
					rtti = rtti->next;
				}
			}
			if (imgui_helpers::beginTable("iowsuergniuernigu", {
				"Name",
				//"instance_item_count",
				"item_size",
				"thread_worker_id",
				"flags",
				"structs_used",
				"instance_count"
				})) {
				for (auto& ptr : queues) {
					StructPageAllocator* queue = *(StructPageAllocator**)(globals::gameBase + ptr.second);
					print_queue(ptr.first.c_str(), queue);
				}
				for (auto* comp_creator : _creators) {
					char buf[32];
					sprintf_s(buf, "Compo %hhx", comp_creator->component_id);
					print_queue(buf, comp_creator->iclass);
				}
				ImGui::EndTable();
			}
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(blizzbbqueueq_window);