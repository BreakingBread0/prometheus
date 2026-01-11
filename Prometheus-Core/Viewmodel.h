#pragma once
#include "game.h"

struct ViewModelTypeSpecifier {
	__int64 vfptr;
	int flags;
	__int16 field_C;
	__int64 field_E;
};

struct ViewModel_vt {
	InheritanceInfo_destructor_vt rtti;
	char* (__fastcall* GetName)();
	char* (__fastcall* returns_zero)();
};

struct ViewModelArray_vt {
	__int64(__fastcall* deallocate)(__int64, char);
	void** (*get_rtti)();
	__int64(__fastcall * allocate_new)(__int64);
	__int64(__fastcall* allocate_at_pos)(__int64, int);
	__int64(__fastcall * get_at_pos)(__int64, int);
	__int64(__fastcall* deallocate_at_pos)(__int64, int);
	__int64(__fastcall * get_vm_count)(__int64);
	char(__fastcall* is_empty)(__int64);
	__int64(__fastcall * clear_mby)(__int64);
};

enum ViewModelPropertyType : char {
	ViewModelPropertyType_CHAR = 0x1,
	ViewModelPropertyType_INT = 0x2,
	ViewModelPropertyType_UINT = 0x3,
	ViewModelPropertyType_FLT = 0x4,
	ViewModelPropertyType_LONG = 0x5,
	ViewModelPropertyType_ULONG = 0x6,
	ViewModelPropertyType_STRING_REP = 0x7,
	ViewModelPropertyType_VIEWMODEL = 0x8,
	ViewModelPropertyType_ARRAY = 0xA,
	ViewModelPropertyType_Max = 0xB
};

inline std::string VMPropTypeToString(ViewModelPropertyType typ) {
	std::string result;
	switch (typ) {
		case ViewModelPropertyType_CHAR:
			result = "CHAR";
			break;
		case ViewModelPropertyType_INT:
			result = "INT";
			break;
		case ViewModelPropertyType_UINT:
			result = "UINT";
			break;
		case ViewModelPropertyType_FLT:
			result = "FLT";
			break;
		case ViewModelPropertyType_LONG:
			result = "LONG";
			break;
		case ViewModelPropertyType_ULONG:
			result = "ULONG";
			break;
		case ViewModelPropertyType_STRING_REP:
			result = "STRING_REP";
			break;
		case ViewModelPropertyType_VIEWMODEL:
			result = "VIEWMODEL";
			break;
		case ViewModelPropertyType_ARRAY:
			result = "SUB_VM";
			break;
		default:
			result = "unk";
	}
	return result + " (" + std::to_string_hex(typ) + ")";
}

struct ViewModelProperty {
	__int64 value;
	ViewModelPropertyType type;
	char has_value;

	inline std::string type_str() {
		return VMPropTypeToString(type);
	}
};

struct ViewModelPropertyListItem {
	__int64 key;
	ViewModelProperty property;
};

struct ViewModel {
	union {
		ViewModel_vt* vfptr;
		STRUCT_PLACE(__int64, id, 0x48);
		STRUCT_PLACE(ViewModel*, this_or_parent, 0x40);
		STRUCT_PLACE(__int64, viewmodel_type, 0x68);
		STRUCT_PLACE(__int64, field_70, 0x70); //has_viewmodel_id
		STRUCT_PLACE(__int64, field_74, 0x74);
		STRUCT_PLACE(teList<ViewModelPropertyListItem>, properties, 0x50);
	};

	ViewModelPropertyListItem* getByKey(__int64 key) {
		for (auto& item : properties) {
			if (item.key == key) {
				return &item;
			}
		}
		return nullptr;
	}
};

struct ViewModelArray {
	union {
		ViewModelArray_vt* vfptr;
		STRUCT_PLACE(teList<ViewModel*>, viewmodel_list, 0x28);
		STRUCT_PLACE(ViewModel*, viewmodel_backref, 0x8);
	};
};

struct ViewModelRoot {
	union {
		ViewModelTypeSpecifier type_specifier;
		STRUCT_PLACE(teList<ViewModel*>, viewmodel_list, 0x1A0);
	};

	ViewModel* vmById(int id) {
		for (int i = 0; i < viewmodel_list.num; i++)
			if (viewmodel_list.ptr[i]->id == id)
				return viewmodel_list.ptr[i];
		return nullptr;
	}
};

inline ViewModelRoot* GetViewModelRoot() {
	return *(ViewModelRoot**)(globals::gameBase + 0x18a7070);
}

inline void SendViewModelPropUpdate(ViewModel* vm, ViewModelPropertyListItem* item) {
	auto func1 = (void(__fastcall*)(ViewModel*, __int64, ViewModelProperty*, char))(globals::gameBase + 0xa74710);
	auto func2 = (void(__fastcall*)(ViewModel*, __int64))(globals::gameBase + 0xa75530);

	func1(vm, item->key, &item->property, 0);
	func2(vm, item->key);
}