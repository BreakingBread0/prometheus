#pragma once
#include "Statescript.h"

struct StatescriptVar_Base;
struct StatescriptVar_VarBag;
struct StatescriptVar_Primitive;
struct StatescriptVar_Dictionary;
struct StatescriptVar_InstanceReference;

enum StatescriptVar_SpecialType : __int64 {
	StatescriptVar_INSTANCE_VARBAG = 0xD80000000000003,
	StatescriptVar_ENTITY_VARBAG = 0xD80000000000004,
	StatescriptVar_CHILD_VARBAG = 0xD80000000000005,
	StatescriptVar_THIS_VARBAG = 0xD80000000000006,
	StatescriptVar_VEC3_X = 0xD80000000000008,
	StatescriptVar_VEC3_Y = 0xD80000000000009,
	StatescriptVar_VEC3_Z = 0xD8000000000000A,
	StatescriptVar_DICTSIZE = 0xD8000000000000B,
};

struct StatescriptVar_DictionaryArray // sizeof=0x18
{
    StatescriptPrimitive_Array cv_array;
    char field_10;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct StatescriptVar_Dictionary_ArrayCopy // sizeof=0x20
{
    StatescriptVar_DictionaryArray array;
    char was_unused;
    char needs_deallocate_afterwards;
	int field_1C;
};

struct RID_SearchStruct { char a; }; // Stub, since its not necessary, this is only used for internal search functions
// 00000000 struct RID_SearchStruct // sizeof=0x30
// 00000000 {                                       // XREF: RID_EmplaceSubscription:loc_7FF650F3E011/r
// 00000000                                         // RID_EmplaceSubscription+1F5/r ...
// 00000000     __int32 curr_idx;                   // XREF: RID_SIRef_GlobalVar_SearchForRightVarbag:loc_7FF650F3E517/r
// 00000000                                         // RID_SIRef_GlobalVar_SearchForRightVarbag+C7/w ...
// 00000004     // padding byte
// 00000005     // padding byte
// 00000006     // padding byte
// 00000007     // padding byte
// 00000008     STUConfigVarDynamic * stu_cv;        // XREF: RID_SIRef_GlobalVar_SearchForRightVarbag:loc_7FF650F3E501/r
// 00000008                                         // RID_Primitive_SetValueFromCVD+46/r ...
// 00000010     RID_VarBag * instance_ref;           // XREF: RID_VarBagGetter_SearchForRightRIDStart+1E/w
// 00000010                                         // RID_VarBagGetter_SearchForRightRIDFromCVD+8B/r ...
// 00000018     RID_VarBag * entity_varbag;          // XREF: RID_VarBagGetter_SearchForRightRIDStart+2C/w
// 00000018                                         // RID_VarBagGetter_SearchForRightRIDFromCVD+70/r ...
// 00000020     StatescriptInstance * ss_instance;   // XREF: RID_EmplaceSubscription+1F5/r
// 00000020                                         // RID_Primitive_SetValueFromCVD+65/r ...
// 00000028     StatescriptSyncMgr * ss_comp_inner;  // XREF: RID_EmplaceSubscription:loc_7FF650F3E011/r
// 00000028                                         // RID_SIRef_GlobalVar_SearchForRightVarbag+43/r ...
// 00000030 };

struct RID_Base_vt {
	union {
		STUBase_vt base;
		/*STRUCT_PLACE_CUSTOM(get_value, 0x110, __int64(__fastcall* GetValue)(StatescriptVar_Base*, StatescriptPrimitive*));
		STRUCT_PLACE_CUSTOM(set_value, 0x288, __int64 (*Primitive_SetValueNotify)(StatescriptVar_Base*, StatescriptSyncMgr*, StatescriptInstance*, StatescriptPrimitive*, RID_User*, char force));
		STRUCT_PLACE_CUSTOM(set_value_notify, 0x278, void(__fastcall* Primitive_SetValueNoNotify)(StatescriptVar_Base*, StatescriptPrimitive*));*/
		STRUCT_PLACE_CUSTOM(Reset, 0x000000D0, void(__fastcall* Reset)(__int64, StatescriptInstance*));
		STRUCT_PLACE_CUSTOM(GlobalVar_IsEmpty, 0x000000D8, __int64 (*GlobalVar_IsEmpty)(StatescriptVar_Primitive*));
		STRUCT_PLACE_CUSTOM(SetConfigVarValueFromSTUConfigVar, 0x000000E0, __int64(__fastcall* SetConfigVarValueFromSTUConfigVar)(StatescriptVar_VarBag*, STUConfigVarDynamic* out_var, STUConfigVar* m_value, RID_User*));
		STRUCT_PLACE_CUSTOM(VarBag_SearchForRIDForCVD_AndSetCV, 0x000000E8, __int64(__fastcall* VarBag_SearchForRIDForCVD_AndSetCV)(StatescriptVar_VarBag*, STUConfigVarDynamic*, StatescriptPrimitive*, RID_User*));
		STRUCT_PLACE_CUSTOM(VarBag_ReplaceRID, 0x000000F0, __int64(__fastcall* VarBag_ReplaceRID)(StatescriptVar_VarBag*, StatescriptVar_Base*));
		STRUCT_PLACE_CUSTOM(GetValueCVD_SearchForRightVarBag, 0x000000F8, StatescriptVar_Base*(__fastcall* GetValueCVD_SearchForRightVarBag)(StatescriptVar_Base*, STUConfigVarDynamic*, StatescriptPrimitive*));
		STRUCT_PLACE_CUSTOM(GetUser, 0x00000100, _QWORD* (__fastcall* GetUser)(__int64, RID_User*));
		STRUCT_PLACE_CUSTOM(GetValue_StackState, 0x00000108, int(__fastcall* GetValue_StackState)(StatescriptVar_Base*, StatescriptPrimitive*));
		STRUCT_PLACE_CUSTOM(GetValue, 0x00000110, __int64(__fastcall* GetValue)(StatescriptVar_Base*, StatescriptPrimitive*));
		STRUCT_PLACE_CUSTOM(VarBag_GetSubValue_StackState, 0x00000118, __int64(__fastcall* VarBag_GetSubValue_StackState)(StatescriptVar_VarBag*, __int64 rid, StatescriptPrimitive*));
		STRUCT_PLACE_CUSTOM(VarBag_GetRIDByID, 0x00000120, __int64(__fastcall* VarBag_GetRIDByID)(StatescriptVar_VarBag*, __int64 rid));
		STRUCT_PLACE_CUSTOM(VarBag_GetUserForRID, 0x00000128, __int64(__fastcall* VarBag_GetUserForRID)(StatescriptVar_VarBag*, RID_User* out, STUConfigVarDynamic*));
		STRUCT_PLACE_CUSTOM(VarBag_DictionaryAddSetItem, 0x00000130, char(__fastcall* VarBag_DictionaryAddSetItem)(StatescriptVar_VarBag*, STUConfigVarDynamic*, StatescriptPrimitive* m_key, StatescriptPrimitive* m_value, RID_User*, int));
		STRUCT_PLACE_CUSTOM(field_138, 0x00000138, __int64(__fastcall* field_138)(__int64, __int64, __int64, __int64, __int64, int));
		STRUCT_PLACE_CUSTOM(VarBag_ResetSubToDictVar, 0x00000140, __int64(__fastcall* VarBag_ResetSubToDictVar)(StatescriptVar_VarBag*, STUConfigVarDynamic, RID_User*));
		STRUCT_PLACE_CUSTOM(VarBag_SubDictVarFindKeyOrValue, 0x00000148, __int64(__fastcall* VarBag_SubDictVarFindKeyOrValue)(StatescriptVar_VarBag*, STUConfigVarDynamic*, StatescriptPrimitive*, StatescriptPrimitive*));
		STRUCT_PLACE_CUSTOM(Proxy_GetStateDefinedValueArray, 0x00000150, int (*Proxy_GetStateDefinedValueArray)(StatescriptVar_Base*, StatescriptPrimitive_Array**));
		STRUCT_PLACE_CUSTOM(VarBagArrayGetter_FromConfigVarDynamic, 0x00000158, __int64(__fastcall* VarBagArrayGetter_FromConfigVarDynamic)(StatescriptVar_Base*, STUConfigVarDynamic* m_value, StatescriptPrimitive_ArrayItem** array, DWORD* array_count));
		STRUCT_PLACE_CUSTOM(DictVar_GetArray, 0x00000160, __int64 (*DictVar_GetArray)(StatescriptVar_Dictionary*, StatescriptVar_Dictionary_ArrayCopy**));
		STRUCT_PLACE_CUSTOM(anonymous_14, 0x00000168, __int64 (*anonymous_14)());
		STRUCT_PLACE_CUSTOM(DictVar_CopyArrayToSyncMgr, 0x00000170, StatescriptVar_Dictionary* (__fastcall* DictVar_CopyArrayToSyncMgr)(StatescriptVar_Base*, STUConfigVarDynamic*, __int64, StatescriptVar_Dictionary_ArrayCopy*, RID_User*));
		STRUCT_PLACE_CUSTOM(DictVar_CompareArrayCopy_AndSetSimilarity, 0x00000178, void(__fastcall* DictVar_CompareArrayCopy_AndSetSimilarity)(StatescriptVar_Dictionary*, StatescriptVar_Dictionary_ArrayCopy*));
		STRUCT_PLACE_CUSTOM(anonymous_16, 0x00000180, __int64 (*anonymous_16)());
		STRUCT_PLACE_CUSTOM(SetFieldx90Value, 0x00000188, void(__fastcall* SetFieldx90Value)(__int64, int));
		STRUCT_PLACE_CUSTOM(GetCurrentStackIdx, 0x00000190, _DWORD* (__fastcall* GetCurrentStackIdx)(__int64, int* output));
		STRUCT_PLACE_CUSTOM(SetCurrentStackIdxTo0, 0x00000198, void(__fastcall* SetCurrentStackIdxTo0)(__int64));
		STRUCT_PLACE_CUSTOM(GetAndRemoveFlagx80, 0x000001A0, bool(__fastcall* GetAndRemoveFlagx80)(__int64, bool* hasFlag));
		STRUCT_PLACE_CUSTOM(AddFlagx80, 0x000001A8, void(__fastcall* AddFlagx80)(__int64, char addFlag));
		STRUCT_PLACE_CUSTOM(WriteStackState, 0x000001B0, void (*WriteStackState)(StatescriptVar_Base*, StatescriptSyncMgr*, StatescriptInstance*));
		STRUCT_PLACE_CUSTOM(RegisterSubscription, 0x000001B8, void(__fastcall* RegisterSubscription)(StatescriptVar_VarBag*, StatescriptState*, STUConfigVarDynamic*, char is_listx50));
		STRUCT_PLACE_CUSTOM(UnregisterSubscription, 0x000001C0, __int64(__fastcall* UnregisterSubscription)(StatescriptVar_VarBag* a1, StatescriptState* a2, STUConfigVarDynamic* a3, char a4));
		STRUCT_PLACE_CUSTOM(DeallocateSubscriptions, 0x000001C8, _QWORD* (__fastcall* DeallocateSubscriptions)(StatescriptVar_Base*));
		STRUCT_PLACE_CUSTOM(RID_VarBag_SetEntityVarBag, 0x000001D0, void(__fastcall* RID_VarBag_SetEntityVarBag)(StatescriptVar_VarBag*, StatescriptVar_VarBag*));
		STRUCT_PLACE_CUSTOM(RID_VarBag_IterateVarBag, 0x000001D8, _QWORD* (__fastcall* RID_VarBag_IterateVarBag)(StatescriptVar_VarBag*, void(__fastcall***)(__int64, StatescriptVar_Base*)));
		STRUCT_PLACE_CUSTOM(GetParentVarBag, 0x000001E0, StatescriptVar_VarBag* (__fastcall* GetParentVarBag)(StatescriptVar_Base*));
		STRUCT_PLACE_CUSTOM(GetInstanceResId, 0x000001E8, __int64(__fastcall* GetInstanceResId)(StatescriptVar_Base*));
		STRUCT_PLACE_CUSTOM(GetStatescriptComponentInner, 0x000001F0, StatescriptSyncMgr* (__fastcall* GetStatescriptComponentInner)(StatescriptVar_Base*));
		STRUCT_PLACE_CUSTOM(GetStatescriptInstance, 0x000001F8, StatescriptInstance* (__fastcall* GetStatescriptInstance)(StatescriptVar_Base*));
		STRUCT_PLACE_CUSTOM(VarBag_IsEmpty, 0x00000200, _BOOL8(__fastcall* VarBag_IsEmpty)(StatescriptVar_VarBag*));
		STRUCT_PLACE_CUSTOM(IsSameRIDuserForSubVarBagItem, 0x00000208, bool(__fastcall* IsSameRIDuserForSubVarBagItem)(StatescriptVar_Primitive* a1, STUConfigVarDynamic* config_var));
		STRUCT_PLACE_CUSTOM(SetFlag, 0x00000210, _BOOL8(__fastcall* SetFlag)(__int64, char));
		STRUCT_PLACE_CUSTOM(HasFlag1, 0x00000218, __int64(__fastcall* HasFlag1)(__int64));
		STRUCT_PLACE_CUSTOM(AppendMoreFlags, 0x00000220, void(__fastcall* AppendMoreFlags)(StatescriptVar_Base*, char));
		STRUCT_PLACE_CUSTOM(GetFlag2Or4State, 0x00000228, __int64(__fastcall* GetFlag2Or4State)(StatescriptVar_Base*, char));
		STRUCT_PLACE_CUSTOM(GetFlagState2, 0x00000230, bool(__fastcall* GetFlagState2)(__int64));
		STRUCT_PLACE_CUSTOM(GetFlagState3, 0x00000238, bool(__fastcall* GetFlagState3)(__int64));
		STRUCT_PLACE_CUSTOM(AddFlagx20, 0x00000240, void(__fastcall* AddFlagx20)(__int64));
		STRUCT_PLACE_CUSTOM(Write_bsDataStore, 0x00000248, __int64(__fastcall* is_on_movingplatform)(__int64, __int64, unsigned __int8, char));
		STRUCT_PLACE_CUSTOM(Read_bsDataStore, 0x00000250, __int64(__fastcall* field_250)(__int64, __int64));
		STRUCT_PLACE_CUSTOM(GetValueIterator, 0x00000258, __int64 (*GetValueIterator)(StatescriptVar_Base*, StatescriptPrimitive_ArrayItem**, DWORD* count));
		STRUCT_PLACE_CUSTOM(GetValueCVD_SearchForRightVarBag_Internal, 0x00000260, StatescriptVar_Base* (__fastcall* GetValueCVD_SearchForRightVarBag_Internal)(StatescriptVar_Base*, RID_SearchStruct*, StatescriptPrimitive*));
		STRUCT_PLACE_CUSTOM(VarBag_SearchForRIDForCVD_AndSetCV_Internal, 0x00000268, __int64(__fastcall* VarBag_SearchForRIDForCVD_AndSetCV_Internal)(StatescriptVar_VarBag*, RID_SearchStruct*, StatescriptPrimitive*, RID_User*));
		STRUCT_PLACE_CUSTOM(Primitive_SetValueFromCVD, 0x00000270, void(__fastcall* Primitive_SetValueFromCVD)(StatescriptVar_Base*, RID_SearchStruct*, StatescriptPrimitive*, RID_User*));
		STRUCT_PLACE_CUSTOM(Primitive_SetValueNoNotify, 0x00000278, void(__fastcall* Primitive_SetValueNoNotify)(StatescriptVar_Base*, StatescriptPrimitive*));
		STRUCT_PLACE_CUSTOM(Primitive_SetValueFromESetVar, 0x00000280, void(__fastcall* Primitive_SetValueFromESetVar)(StatescriptVar_Base*, StatescriptInstance*, StatescriptNotification_SetVar*));
		STRUCT_PLACE_CUSTOM(Primitive_SetValueFromRemote, 0x00000288, __int64 (*Primitive_SetValueFromRemote)(StatescriptVar_Base*, StatescriptSyncMgr*, StatescriptInstance*, StatescriptPrimitive*, RID_User*, char force));
	};
};

struct StatescriptVar_Base {
	union {
		STUBase<RID_Base_vt> stu_base;
		STRUCT_PLACE(teList<StatescriptState*>, subscriptions, 0x10);
		STRUCT_PLACE(teList<StatescriptState*>, remote_sync_subscriptions_mby, 0x50);
		STRUCT_PLACE(StatescriptVar_VarBag*, parent_rid, 0x80);
		STRUCT_PLACE(__int64, var_id, 0x98);
		STRUCT_PLACE(RID_User, user, 0x88);
		STRUCT_PLACE(int, cv_stack_idx, 0x90);
	};

	//struct StatescriptVar_Dictvar AsDictVar();

	bool is_varbag() {
		return (__int64)stu_base.vfptr - globals::gameBase == 0x1607a90;
	}

	bool is_dictvar() {
		return (__int64)stu_base.vfptr - globals::gameBase == 0x1607d20;
	}

	bool is_cvd() {
		return (__int64)stu_base.vfptr - globals::gameBase == 0x1608248;
	}

	bool is_siref() {
		return (__int64)stu_base.vfptr - globals::gameBase == 0x1607fb8;
	}
};

//0x1607a90
struct StatescriptVar_VarBag {
	struct VarBag_Item {
		__int64 rid_id;
		StatescriptVar_Base* ptr;
	};

	struct VarBag_ItemArray {
		VarBag_Item* arr_ptr;
		int item_count;
		int max;
		VarBag_Item first_instance;
		__int64 pad_1;
		__int64 pad_2;
	};

	union {
		StatescriptVar_Base base;
		STRUCT_PLACE_ARRAY(VarBag_ItemArray, sub_items, 16, 0xA0);
		STRUCT_PLACE(StatescriptVar_VarBag*, rid_entity_varbag, 0x3B0);
		STRUCT_PLACE(StatescriptInstance*, ss_instance, 0x3B8);
		STRUCT_PLACE(StatescriptSyncMgr*, ss_compo_inner, 0x3C0);
	};

	void SetVar(std::vector<__int64> cv_path, StatescriptPrimitive value) {
		STUConfigVarDynamic_Editable set_health_cv(cv_path); //current health value
		STUConfigVar value_cv{};
		GetSTUInfoByHash(stringHash("STUConfigVar"))->clear_instance_fn((__int64)&value_cv);
		STUConfigVar_impl_Custom value_cv_impl(value);
		value_cv.cv_impl = (STUConfigVar_impl*)&value_cv_impl;
		base.stu_base.vfptr->SetConfigVarValueFromSTUConfigVar(this, set_health_cv.get(), (STUConfigVar*)&value_cv, 0);
	}

	void SetArray(std::vector<__int64> cv_path, StatescriptPrimitive m_key, StatescriptPrimitive m_value, int m_index) {
		STUConfigVarDynamic_Editable cvd(cv_path);
		base.stu_base.vfptr->VarBag_DictionaryAddSetItem(this, cvd.get(), &m_key, &m_value, nullptr, m_index);
	}

	void AddArray(std::vector<__int64> cv_path, StatescriptPrimitive m_key, StatescriptPrimitive m_value) {
		SetArray(cv_path, m_key, m_value, -1);
	}
};

//0x1608248
struct StatescriptVar_Primitive {
	union {
		StatescriptVar_Base base;
		STRUCT_PLACE(StatescriptPrimitive, global_var, 0xA0);
	};
};

//0x1607d20
struct StatescriptVar_Dictionary {
	union {
		StatescriptVar_Base base;
		STRUCT_PLACE(StatescriptPrimitive_Array, items_array, 0xA0);
		STRUCT_PLACE(int, field_B8, 0xB8);
	};
};

//0x1607fb8
struct StatescriptVar_InstanceReference {
	union {
		StatescriptVar_Base base;
		STRUCT_PLACE(int, ss_inst_id, 0xa0);
	};
};