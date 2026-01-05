#pragma once
#include "STU.h"
#include "ResourceManager.h"
#include <MinHook.h>
#include <map>

class stu_resources {
public:
	static inline void initialize() {
		MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xa4c3f0), (PVOID*)STUD_ConstructFromResource, (PVOID*)&stud_construct_orig));
		MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xa4c3f0)));

		MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xa4c620), (PVOID*)STUD_DestroyResource, (PVOID*)&stud_destroy_orig));
		MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xa4c620)));

		MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0x8553f0), (PVOID*)DisplayText_ConstructFromResource, (PVOID*)&DisplayText_construct_orig));
		MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0x8553f0)));
		memset((void*)(globals::gameBase + 0x8553fb), 0x90, 0x3BB);
	}

	static inline std::map<__int64, STUBase<>*> GetAll() {
		std::map<__int64, STUBase<>*> result{};
		for (auto& item : _constructed_stus) {
			if (item.second->valid()) {
				result.insert(item);
			}
		}
		return result;
	}

	static inline std::map<__int64, STUBase<>*> GetIfAssignableTo(uint hash) {
		std::map<__int64, STUBase<>*> result{};
		for (auto& item : _constructed_stus) {
			if (item.second->valid() && item.second->vfptr_stubase->GetSTUInfo()->assignable_to_hash(hash)) {
				result.insert(item);
			}
		}
		return result;
	}

	static inline STUBase<>* GetByID(__int64 id) {
		auto result = _constructed_stus.find(id);
		if (result != _constructed_stus.end())
			return result->second;
		return nullptr;
	}

	static inline int GetResourceLoadFrame() {
		return *(int*)(globals::gameBase + 0x18a1840);
	}
private:
	static inline std::map<__int64, STUBase<>*> _constructed_stus;

	static inline __int64(__fastcall* stud_construct_orig)(ResourceLoadEntry*);
	static __int64 STUD_ConstructFromResource(ResourceLoadEntry* entry);

	static inline __int64(__fastcall* DisplayText_construct_orig)(ResourceLoadEntry*);
	static __int64 DisplayText_ConstructFromResource(ResourceLoadEntry* entry);

	static inline __int64(__fastcall* stud_destroy_orig)(ResourceLoadEntry*);
	static inline __int64 STUD_DestroyResource(ResourceLoadEntry* entry) {
		__int64 result = stud_destroy_orig(entry);
		_constructed_stus.erase(entry->resource_id);
		return result;
	}
};