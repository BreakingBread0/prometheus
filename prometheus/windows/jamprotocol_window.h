#pragma once
#include "../window_manager/window_manager.h"
#include "jam_messagedetail_window.h"
#include <MinHook.h>
#include <map>
#include "../JAM.h"

class jamprotocol_window : public window {
public:
	WINDOW_DEFINE(jamprotocol_window, "Jam", "Jam Protocol", true);
	
	typedef __int64(__fastcall* addJamProtocolPool_fn)(__int64 linkedList, __int64 dispatchInfo);
	static inline addJamProtocolPool_fn addJamProtocolPool_orig;
	static inline addJamProtocolPool_fn associatePool_orig;
	static __int64 __fastcall addJamProtocolHook(__int64 linkedList, __int64 dispatchInfo) {
		__int64 result = addJamProtocolPool_orig(linkedList, dispatchInfo);
		//printf("ADD JAM: list %p dispatchInfo %p result %p\n", linkedList, dispatchInfo, result);
		if (jamPoolInstances.contains(dispatchInfo)) {
			//printf("DISPATCH INFO ALREADY EXISTS!\n"); never happens
		}
		jamPoolInstances[result] = dispatchInfo;
		return result;
	}

	//associationInfo:
	//0: crc
	//8: name
	static __int64 __fastcall associatePoolHook(__int64 associationInfo, __int64 receiverInstance) {
		__int64 receiverFunc = *(__int64*)(receiverInstance + 0x20);
		//printf("ASSOCIATE JAM: crc %x (%s) with %p\n", *(int*)(associationInfo), (char*)*(__int64*)(associationInfo + 8), receiverFunc);
		poolAssociations[*(int*)(associationInfo)] = receiverFunc;
		__int64 result = associatePool_orig(associationInfo, receiverInstance);
		return result;
	}

	inline void preStartInitialize() override {
		MH_CreateHook((void*)(globals::gameBase + 0x8a27d0), (PVOID*)addJamProtocolHook, (void**)(&addJamProtocolPool_orig));
		MH_EnableHook((void*)(globals::gameBase + 0x8a27d0));
		MH_CreateHook((void*)(globals::gameBase + 0xf25ba0), (PVOID*)associatePoolHook, (void**)(&associatePool_orig));
		MH_EnableHook((void*)(globals::gameBase + 0xf25ba0));
	}

	inline void initialize() override {

	}

	inline void render() override {
		for (auto& item : jamPoolInstances) {
			__int64 instance = *(__int64*)item.first;
			JamPoolVtable* vtable = *(JamPoolVtable**)(instance);
			char* static_name = vtable->pool_name();
			jamOrderedInstances.emplace(std::string(static_name), std::pair<__int64, __int64> { item.first, item.second });
		}
		jamPoolInstances.clear();
		if (open_window()) {
			ImGui::BeginTable("JAMMING TABLE", 10, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersH);
			ImGui::TableSetupColumn("Instance");
			ImGui::TableSetupColumn("Dispatch Info");
			ImGui::TableSetupColumn("Pool Name");
			ImGui::TableSetupColumn("Pool static name");
			ImGui::TableSetupColumn("Min ID");
			ImGui::TableSetupColumn("Max ID");
			ImGui::TableSetupColumn("Zero?");
			ImGui::TableSetupColumn("Count");
			ImGui::TableSetupColumn("Actions");
			ImGui::TableSetupColumn("ASSOCIATE");
			ImGui::TableHeadersRow();
			for (auto& item1 : jamOrderedInstances) {

				auto& item = item1.second;
				ImGui::PushID(*(__int64*)item.first);
				ImGui::TableNextRow();
				__int64 instance = *(__int64*)item.first;
				JamPoolVtable* vtable = *(JamPoolVtable**)(instance);

				ImGui::TableNextColumn();
				display_addr(item.first);
				
				ImGui::TableNextColumn();
				display_addr(item.second);

				ImGui::TableNextColumn();
				ImGui::Text("%s", *(char**)(instance + 0x18));

				ImGui::TableNextColumn();
				char* static_name = vtable->pool_name();
				ImGui::Text("%s", static_name == nullptr ? "NULL" : static_name);

				ImGui::TableNextColumn();
				short min = vtable->min_msgId();
				ImGui::Text("%hx %hd", min, min);

				ImGui::TableNextColumn();
				short max = vtable->max_msgId();
				ImGui::Text("%hx %hd", max, max);

				ImGui::TableNextColumn();
				int zero = vtable->returns_zero();
				ImGui::Text("%d", zero);

				ImGui::TableNextColumn();
				ImGui::Text("%d", *(int*)(instance + 0x30));

				ImGui::TableNextColumn();
				char buf[32];
				sprintf(buf, "Open %d messages", max - min);
				if (ImGui::Button(buf)) {
					std::vector<__int64> instances{};
					for (short i = min; i < max; i++) {
						instances.push_back((__int64)vtable->allocate_pool(instance, i));
					}
					window_manager::add_window(new jam_messagedetail_window(std::move(instances), static_name), this);
				}

				ImGui::TableNextColumn();
				if (poolAssociations.contains(vtable->pool_crc())) {
					display_addr(poolAssociations[vtable->pool_crc()]);
				}
				ImGui::PopID();
			}
			ImGui::EndTable();

			if (imgui_helpers::CenteredButton("Close")) {
				queue_deletion();
			}
		}
		ImGui::End();
	}
};

WINDOW_REGISTER(jamprotocol_window);