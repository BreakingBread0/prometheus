#pragma once
#include "globals.h"
#include "idadefs.h"

struct ResourceLoadEntry;

/* 505 */
struct LinkedListQueueInner
{
	LinkedListQueueInner* prev;
	LinkedListQueueInner* next;
	__int64 data;
};

/* 504 */
struct ResourceLoadEntry
{
	ResourceLoadEntry* prev;
	ResourceLoadEntry* next;
	__int32 manager_info_index;
	__int32 resource_state;
	__int32 requested_termination;
	__int32 prio_override;
	__int32 _two;
	__int64 field_28;
	__int64 field_30;
	__int64 field_38;
	__int64 DATA;
	__int32 xFFFFFFFF;
	__int64 resource_id;
	__int64 field_58;
	__int64 field_60;
	__int64 field_68;
	__int64 field_70;
	__int32 manager_negator;
	__int32 state_changes;
};

/* 511 */
struct manager_info
{
	char* name;
	__int32 manager_id;
	_BYTE gapC[4];
	__int64 crc_continued;
	__int32 handles_filetype_ent;
	__int32 manager_flags;
	__int64 field_20;
	void(__fastcall* BeforeDataDeletion)(ResourceLoadEntry*);
	__int64 field_30;
	__int64 field_38;
	void(__fastcall* stu_construct_fn)(ResourceLoadEntry*);
	void(__fastcall* OnResourceLoaded)(ResourceLoadEntry*);
	__int64 field_50;
	void(__fastcall* BeforeDeallocateError)(ResourceLoadEntry*);
	__int64 field_60;
	__int64 manager_negated;
};

/* 512 */
struct PriorityListQueueStart
{
	ResourceLoadEntry* front;
	ResourceLoadEntry* back;
	__int32 item_count;
};

/* 507 */
struct PrioritizedLinkedList
{
	__int32 mutex;
	_BYTE gap4[4];
	PriorityListQueueStart lists[6];
	__int64 field_98;
	__int64 field_A0;
	__int64 field_A8;
	__int32 total_item_count;
	__int32 field_B4;
};

struct ClonedResourceLoadEntry {
	ResourceLoadEntry loaded_entry;
	__int64 old_location;
};


struct AssetPackListItem {
	union {
		__int64 field_0;
		STRUCT_PLACE(__int64, tact_key_used, 0x18);
		STRUCT_PLACE(__int64, assetPackId, 0x20);
		STRUCT_PLACE(int, field_2C, 0x2c);
		STRUCT_PLACE(__int64, field_38, 0x38);
		STRUCT_PLACE(__int64, field_40, 0x40);
	};
};