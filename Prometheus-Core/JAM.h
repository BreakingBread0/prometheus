#pragma once
#include <map>
#include "globals.h"

//TODO that may not be right. Never got around to look into it.
struct JamType13String {
	int length;
	int is_ptr;
	__int64 string_literal_or_ptr;
};

inline char* readJamType13String(JamType13String* str) {
	/*printf("len %d %x\n", str->length, str->length);
	printf("ptr %d %x\n", str->is_ptr, str->is_ptr);*/
	if ((str->is_ptr & 0x7FFFFFFF) >= 8) {
		return (char*)str->string_literal_or_ptr;
	}
	return (char*)&str->string_literal_or_ptr;
}

struct JamMessage;
struct JamMessageInfo;
struct JamMessageVTable { //0x30 pool item count
	__int64 destructor_1;
	__int64 destructor_outer;
	JamMessageInfo* (*get_message_info)();
	JamMessage* (*clone_instance)();
};

struct JamMessage {
	JamMessageVTable* vfptr;
};

struct JamMessageItemInfo {
	char* name_str;
	int message_type;
	int offset; //relative to JamMessage*
	int size;
	int has_extra_flag;
	JamMessageItemInfo* inner_data_ptr;
	__int64 decoding_func;
};

struct JamMessageInfo {
	char* message_name;
	char* message_protocol;
	JamMessageItemInfo* items;
	int crc;
	int message_id;
};


struct JamPoolVtable { //0x30 pool item count
	__int64 destructor_1;
	__int64 destructor_outer;
	char* (*pool_name)();
	int (*pool_crc)();
	int(*returns_zero)();
	short(*min_msgId)();
	short(*max_msgId)();
	JamMessage*(__fastcall* allocate_pool)(__int64 instance, short msgId);
	__int64(__fastcall* deallocate_pool)(__int64 instance, __int64 messageInstance);
};

static inline std::map<__int64, __int64> jamPoolInstances{};
static inline std::map<std::string, std::pair<__int64, __int64>> jamOrderedInstances{};
static inline std::map<int, __int64> poolAssociations{};