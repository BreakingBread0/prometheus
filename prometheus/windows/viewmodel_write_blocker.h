#pragma once
#include "../window_manager/window_manager.h"
#include "../Viewmodel.h"
#include <vector>
#include <intrin.h>
#include <MinHook.h>

class viewmodel_write_blocker : public window {
	WINDOW_DEFINE_2(viewmodel_write_blocker, "Viewmodel", "Write Blocker", true, true);

	void add_block(ViewModel* vm, __int64 key);
	void render() override;

// PARALIRA: Trial & Error removed ## until it compiled, please check this!
#define BlockHook(viewmodel_proptyp) \
static inline ViewModelProperty*(__fastcall* VMP_SetOrig_##viewmodel_proptyp)(ViewModel*, __int64, __int64 a3, __int64 a4); \
static ViewModelProperty* VMP_Set_##viewmodel_proptyp(ViewModel* vm, __int64 key, __int64 a3, __int64 a4) { \
	__int64 retaddr = (__int64)__builtin_return_address(0); \
	auto result = block_behaviour(vm, key, a3, a4, retaddr); \
	if (!result.second) return VMP_SetOrig_##viewmodel_proptyp(vm, key, a3, a4); \
	return result.first; \
}
#define DefineBlock(viewmodel_proptyp, addr) \
MH_VERIFY(MH_CreateHook((DWORD_PTR*)(globals::gameBase + addr), (LPVOID)VMP_Set_##viewmodel_proptyp, (PVOID*)&VMP_SetOrig_##viewmodel_proptyp)); \
MH_VERIFY(MH_EnableHook((DWORD_PTR*)(globals::gameBase + addr))); \
s_proptypes_declared.push_back(viewmodel_proptyp);

	BlockHook(ViewModelPropertyType_CHAR);
	BlockHook(ViewModelPropertyType_INT);
	BlockHook(ViewModelPropertyType_FLT);
	BlockHook(ViewModelPropertyType_STRING_REP);
	BlockHook(ViewModelPropertyType_LONG);

	inline void preStartInitialize() override {
		DefineBlock(ViewModelPropertyType_CHAR, 0xa76020);
		DefineBlock(ViewModelPropertyType_INT, 0xa760b0);
		DefineBlock(ViewModelPropertyType_FLT, 0xa76140);	
		DefineBlock(ViewModelPropertyType_STRING_REP, 0xa761e0);
		DefineBlock(ViewModelPropertyType_LONG, 0xa76370);
	}
	//inline void initialize() override {}

private:
	struct VM_Options {
		bool blocked;
		bool logged;
		bool logged_ultradeluxe;
	};

	//0 = block all
	std::map<std::pair<ViewModel*, __int64>, VM_Options> _blocked_viewmodels{};
	static inline std::map<std::pair<ViewModel*, __int64>, VM_Options> s_blocked_viewmodels[ViewModelPropertyType_Max]{};
	static inline std::vector<ViewModelPropertyType> s_proptypes_declared;
	bool _blocked_types[ViewModelPropertyType_Max];

	void redo_blocks();
	static bool isBlocked(ViewModel* vm, __int64 new_value, __int64 a4, ViewModelPropertyListItem* prop, __int64 return_addr, bool with_vm);
	static std::pair<ViewModelProperty*, bool> block_behaviour(ViewModel* vm, __int64 key, __int64 new_value, __int64 a4, __int64 return_addr);
};

WINDOW_REGISTER(viewmodel_write_blocker);
