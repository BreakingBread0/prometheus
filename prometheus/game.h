#pragma once
#include <sstream>
#include <format>
#include "globals.h"
#include "idadefs.h"
#include "fmt/format.h"

struct LogicalButtonName // sizeof=0x18
{
	LogicalButtonName(__int32 key_id, __int32 _, char* name, __int64 zero) : 
		key_id(key_id), _(_), name(name), zero(zero) {}
public:
	__int32 key_id;
	__int32 _;
	char* name;
	__int64 zero;
}; //globals::gameBase + 0x17c4998

inline LogicalButtonName* LogicalButtonList() {
	return (LogicalButtonName*)(globals::gameBase + 0x17c4990);
}

inline LogicalButtonName* LogicalButtonById(int id) {
	LogicalButtonName* list = LogicalButtonList();
	for (int i = 0; i < 0xc3; i++) {
		if (list[i].key_id == id)
			return &list[i];
	}
	static LogicalButtonName invalid(-1, 0, (char*)"INVALID", 0);
	return &invalid;
}

inline LogicalButtonName* LogicalButtonByName(std::string name) {
	LogicalButtonName* list = LogicalButtonList();
	for (int i = 0; i < 0xc3; i++) {
		if (std::string(list[i].name) == name)
			return &list[i];
	}
	static LogicalButtonName invalid(-1, 0, (char*)"INVALID", 0);
	return &invalid;
}

/* 433 */
struct InheritanceInfo
{
	__int64 this_ptr;
	InheritanceInfo* next;
};

struct InheritanceInfo_vt {
	InheritanceInfo* (*get_inheritance)(__int64 instance);
	bool(__fastcall* rtti_assignable_to)(__int64, __int64);
	bool(__fastcall* rtti_instanceof)(__int64, __int64);
};

/* 493 */
struct InheritanceInfo_destructor_vt
{
	InheritanceInfo_vt base;
	__int64(__fastcall* destructor)(__int64 instance, char deallocate_too);
};

struct StructPageAllocator;
//TODO Most of the names dont make sense. Needs further reverse-engineering.
struct StructPageAllocator_vt // sizeof=0x48
{
	void* (*create_internal_instance)(StructPageAllocator*);
	__int64(__fastcall* deallocate_stuff)(StructPageAllocator*, __int64);
	void(__fastcall* nullsub_1)(StructPageAllocator*);
	_QWORD* (__fastcall* deallocate)(StructPageAllocator*, char); // deallocate in base class
	__int64(__fastcall* initialize_2)(StructPageAllocator*); // in base class
	_QWORD* (__fastcall* get_new_instance)(StructPageAllocator*); // maybe create_instance fn?
	unsigned __int64(__fastcall* instance_unlink)(StructPageAllocator*, void*);
	__int64(__fastcall* initialize_1)(StructPageAllocator*);
	__int64(__fastcall* field_40)(StructPageAllocator*, _QWORD*, _QWORD*, _QWORD*);
};

//BlizzBBQueueQ lol
//Allocates big pages and allocates the structure within to not allocate memory all the time.
struct StructPageAllocator {
	union {
		StructPageAllocator_vt* vfptr;
		STRUCT_PLACE(int, instance_item_count, 0x50);
		STRUCT_PLACE(int, item_size, 0x54);
		STRUCT_PLACE(int, thread_worker_id, 0x60);
		STRUCT_PLACE(int, flags, 0x38);
		STRUCT_PLACE(int, structs_used, 0x44);
		STRUCT_PLACE(int, instance_count, 0x48);
	};
};

/* 222 */
struct string_rep
{
	__int64 ptr_or_val;
	__int64 padding;
	int actual_size;
	int max_size;
	int zero;
	int hash;

	char* get() {
		if (max_size > 0x10) {
			return (char*)ptr_or_val;
		}
		return (char*)&ptr_or_val;
	}

	void extend_to(int newsize) {
		if (max_size >= newsize)
			return;
		char* newstr = (char*)ow_memalloc(newsize);
		//printf("memalloc: %p\n", newstr);
		if (actual_size > 0)
			memcpy(newstr, get(), actual_size);
		max_size = newsize;
		ptr_or_val = (__int64)newstr; // TODO: Who needs deallocation?
	}
};

enum GameState : char {
	VIGS_5_STARTUP = 0x5,  // Mode: Lobby
	VIGS_6_WORLDLOADED_LIVEGAME = 0x6,  // Mode: Live Game
	VIGS_7_REPLAYLOADED = 0x7,  // Mode: Replay
	VIGS_8_PLAYREEL = 0x8,  // Mode: Killcam
	VIGS_3_REQUESTEXIT = 0x3,
};

inline const char* GetGameStateStr(GameState state) {
	switch (state) {
	case VIGS_3_REQUESTEXIT:
		return "RequestExit";
	case VIGS_5_STARTUP:
		return "Lobby";
	case VIGS_6_WORLDLOADED_LIVEGAME:
		return "LiveGame";
	case VIGS_7_REPLAYLOADED:
		return "Replay";
	case VIGS_8_PLAYREEL:
		return "Killcam";
	}
	return "Unknown";
}

struct GameStateStru {
	GameState current_state;
	GameState wanted_state;
	const char* state_str;
};

inline GameStateStru* GetGameState() {
	return *(GameStateStru**)(globals::gameBase + 0x17b7fd0);
}

inline void copyToClipboard(std::string value) {
	typedef bool(__fastcall* setClipboard_fn)(int type, char* data, int size);
	setClipboard_fn setClipboard = (setClipboard_fn)(globals::gameBase + 0x8ab8a0);
	setClipboard(CF_TEXT, value.data(), value.size() + 1);
}

inline void copyAddress(__int64 addr) {
	copyToClipboard(fmt::format("{:p}", (void*)addr));
}

inline double GetGameRuntimeSecs() {
	return *(double*)(globals::gameBase + 0x181e320);
}

struct EntityAdminBase;
inline EntityAdminBase* LobbyEntityAdmin() {
	return *(EntityAdminBase**)(globals::gameBase + 0x1833e18);
}

inline EntityAdminBase* GameEntityAdmin() {
	return *(EntityAdminBase**)(globals::gameBase + 0x17b7f90);
}

inline float* globalTimeScale() {
	return (float*)(globals::gameBase + 0x17b7fe4);
}

inline uint stringHash(const char* input) {
	//this is an abomination hahahahahah
	return ((uint(__fastcall*)(const char*))(globals::gameBase + 0x7f88e0))(input);
}

//Gets resource without MisalignedResourceEntry
template <typename T>
T* GetLoadedResource(__int64 res) {
	auto fn = (T* (*)(__int64 resource_id))(globals::gameBase + 0x9c6470);
	return fn(res);
}

//0xDE00000000 strings / type 7B
inline const char* GetUXDisplayString(__int64 str_id) {
	auto result = GetLoadedResource<const char*>(str_id);
	return result && *result ? *result : "(invalid)";
}

inline void SetUXDisplayString(__int64 str_id, std::string str) {
	char* new_str = (char*)ow_memalloc(str.size());
	strcpy(new_str, str.c_str());
	auto res = GetLoadedResource<const char*>(str_id);
	*res = new_str;
}

//FLIP_BITS jesus fucking christ
inline __int64 bitswap(__int64 a1) {
	unsigned __int64 v1; // rdx
	unsigned __int64 v2; // rdx
	unsigned __int64 v3; // r8
	unsigned __int64 v4; // rcx

	v1 = (2 * a1) ^ ((2 * a1) ^ (a1 >> 1)) & 0x5555555555555555i64;
	v2 = (4 * v1) ^ ((4 * v1) ^ (v1 >> 2)) & 0x3333333333333333i64;
	v3 = (16 * v2) ^ ((16 * v2) ^ (v2 >> 4)) & 0xF0F0F0F0F0F0F0Fi64;
	v4 = (v3 << 8) ^ ((v3 << 8) ^ (v3 >> 8)) & 0xFF00FF00FF00FFi64;
	return __ROL8__((v4 << 16) ^ ((v4 << 16) ^ (v4 >> 16)) & 0xFFFF0000FFFFi64, 32);
}

inline __int64 crc64_round(__int64 start, void* data, int size) {
	__int64 v6 = start;
	do
	{
		char v11 = *(char*)data;
		data = (int*)((char*)data + 1);
		v6 = (v6 >> 8) ^ ((__int64*)(globals::gameBase + 0x1454c00))[(unsigned __int8)(v6 ^ v11)];
		--size;
	} while (size);
	return ~v6;
 }

inline void publish_game_msg(__int64 msg) {
	typedef void(__fastcall* publish_fn)(__int64, __int64 msgid);
	publish_fn publish = (publish_fn)(globals::gameBase + 0xc44dd0);
	publish(0, msg);
}

//Has a common iherited class, but I do not know what structure members come from the parent, so this should suffice for now.
struct View_vt;
struct View {
	union {
		View_vt* vfptr;
		STRUCT_PLACE(Vector4, view_position, 0x00000010);              // XREF: sub_7FF6508294B0/o
		STRUCT_PLACE(Vector4, view_rotation, 0x00000020);              // XREF: sub_7FF650F20420+56F/w
		STRUCT_PLACE(Vector4, view_roll, 0x00000030);                  // XREF: sub_7FF650F20420+573/w TODO wrong name
		STRUCT_PLACE(float, field_40, 0x00000040);                     // XREF: sub_7FF650F20420+577/o
		STRUCT_PLACE(int, field_44, 0x00000044);                       // XREF: sub_7FF650F20420+57C/w
		STRUCT_PLACE(float, field_48, 0x00000048);                     // XREF: sub_7FF650F20420+581/w
		STRUCT_PLACE(float, field_50, 0x00000050);                     // XREF: sub_7FF650F22B80+1B/w
		STRUCT_PLACE(float, field_54, 0x00000054);                     // XREF: sub_7FF650CDE7C0/r
		STRUCT_PLACE(float, field_58, 0x00000058);                     // XREF: sub_7FF650CDE7D0/r
		STRUCT_PLACE(float, field_5C, 0x0000005C);                     // XREF: CameraSystem_EmplaceViewFromList+C4/r
		STRUCT_PLACE(float, field_60, 0x00000060);                     // XREF: CameraSystem_EmplaceViewFromList+99/r
		STRUCT_PLACE(float, field_64, 0x00000064);                     // XREF: sub_7FF650F22B80+2/w
		STRUCT_PLACE(char, is_view_active, 0x00000068);                // XREF: CameraSystem_EmplaceViewFromList+DC/w
	};
};

struct teFreeLookView {
	union {
		View base;
		STRUCT_PLACE(Vector4, position_delta, 0x70);
	};

	static teFreeLookView* create() {
		__int64 view_ptr = ow_memalloc(0xC0);
		auto init_freelook_view = (teFreeLookView * (*)(__int64))(globals::gameBase + 0xf42b80);
		printf("FreeLookView allocated: %p\n", view_ptr);
		return init_freelook_view(view_ptr);
	}

	void deallocate() {
		ow_dealloc((__int64)this);
	}
};

struct View_vt // sizeof=0xA0
{
	union {
		__int64(__fastcall* deallocate)(teFreeLookView*, char);
		STRUCT_PLACE_CUSTOM_2(0x00000008, char(__fastcall* field_8)(View*, __int64, __int64, double));
		STRUCT_PLACE_CUSTOM_2(0x00000010, char(__fastcall* Tick)(View*, __int64, __int64));
		STRUCT_PLACE_CUSTOM_2(0x00000018, void(__fastcall* CopyPositionFromCamera)(View*, __int64 camera));
		STRUCT_PLACE_CUSTOM_2(0x00000020, void(__fastcall* field_0)(View*));
		STRUCT_PLACE_CUSTOM_2(0x00000028, __int64 (*ChangeViewBasedOnKeypress)(View*, __int64 camera_mby));
		STRUCT_PLACE_CUSTOM_2(0x00000030, void(__fastcall* field_30)(View*, float, float));
		STRUCT_PLACE_CUSTOM_2(0x00000038, __m128 (__fastcall* field_38)(View*, double));
		STRUCT_PLACE_CUSTOM_2(0x00000040, Vector4* (__fastcall* GetAbsolutePos)(View*));
		STRUCT_PLACE_CUSTOM_2(0x00000048, __int64 (*field_1)(View*));
		STRUCT_PLACE_CUSTOM_2(0x00000050, __int64 (*field_2)(View*));
		STRUCT_PLACE_CUSTOM_2(0x00000058, float(__fastcall* field_58)(View*));
		STRUCT_PLACE_CUSTOM_2(0x00000060, float(__fastcall* field_60)(View*));
		STRUCT_PLACE_CUSTOM_2(0x00000068, void(__fastcall* CopyPositionAndRotationFromOtherInst)(View*, teFreeLookView*));
		STRUCT_PLACE_CUSTOM_2(0x00000070, void(__fastcall* field_70)(View*, float));
		STRUCT_PLACE_CUSTOM_2(0x00000078, __int64 (*field_78)(View*));
		STRUCT_PLACE_CUSTOM_2(0x00000080, __int64 (*field_3)(View*));
		STRUCT_PLACE_CUSTOM_2(0x00000088, __int64 (*field_4)(View*));
		STRUCT_PLACE_CUSTOM_2(0x00000090, const char* (*strFirstPersonView)(View*));
		STRUCT_PLACE_CUSTOM_2(0x00000098, float (*field_98)(View*));
	};
};
