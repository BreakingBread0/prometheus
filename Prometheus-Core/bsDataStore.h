#pragma once
#include "idadefs.h"
#include "globals.h"

struct bsDataStore;
struct bsDataStore_vt // sizeof=0x28
{
    __int64(__fastcall * deallocate)(bsDataStore*, char);
    __int64 (*ret_0)();
    void(__fastcall * ret_0_1)();
    __int64(__fastcall* dealloc_buffer)(bsDataStore*, __int64* dataStore_ptr, int* buffer_size_output);
    __int64(__fastcall* expand_buffer)(bsDataStore*, int wanted_min_size, __int64* dataStore_ptr, int* buffer_size_output);
};

struct bsDataStore {
    union {
        STRUCT_MIN_SIZE(0x30);
        bsDataStore_vt* vfptr;
        STRUCT_PLACE(__int64, dataStore_ptr, 8);
        STRUCT_PLACE(int, dataStore_bufferSize, 0x10);
        STRUCT_PLACE(int, curr_pos, 0x14);
        STRUCT_PLACE(int, unflushed_nextbyte_pos, 0x18);
        STRUCT_PLACE(int, round_up_num, 0x1C); //round up to this when expanding the byte buffer
        STRUCT_PLACE(char, curr_bits_used, 0x20); //Used to push bits instead of bytes.
        STRUCT_PLACE(char, curr_bits_data, 0x21);
        STRUCT_PLACE(int, field_28, 0x28);
    };

    static bsDataStore* create() {
        bsDataStore* result = (bsDataStore*)ow_memalloc(sizeof(bsDataStore));
        ZeroMemory(result, sizeof(bsDataStore));
        result->vfptr = (bsDataStore_vt*)(globals::gameBase + 0x15fb308);
        result->round_up_num = 0x100;
        result->unflushed_nextbyte_pos = -1;
        return result;
    }

    void clear() {
        if (dataStore_ptr)
            ZeroMemory((void*)dataStore_ptr, dataStore_bufferSize);
        curr_pos = 0;
        unflushed_nextbyte_pos = 0;
        curr_bits_used = 0;
    }

    void expandBuffer(int wanted_size) {
        vfptr->expand_buffer(this, wanted_size, &dataStore_ptr, &dataStore_bufferSize);
    }

    void deallocateBuffer() {
        vfptr->dealloc_buffer(this, &dataStore_ptr, &dataStore_bufferSize);
    }

    void deallocate() {
        vfptr->deallocate(this, 1);
    }
};