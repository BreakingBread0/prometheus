#ifndef PROMETHEUS_SYSTEM_22_NETWORKEVENTSYSTEM_H
#define PROMETHEUS_SYSTEM_22_NETWORKEVENTSYSTEM_H
#include "globals.h"

struct bsDataStore;
struct Entity;

struct NetworkReceiveContext
{
    union
    {
        Entity* subject;
        STRUCT_PLACE(int, field_8, 8);
        STRUCT_PLACE(__int64, field_10, 0x10);
        STRUCT_PLACE(int, field_18, 0x18);
        STRUCT_PLACE(int, recv_time_ticks, 0x1C);
        STRUCT_PLACE(int, field_20, 0x20);
        STRUCT_PLACE(int, subject_entid, 0x24);
        STRUCT_PLACE(char, field_28, 0x28);
    };
};

struct NetworkEventSystem_Callback_vt
{
    void (*field_0)();
    void (*field_1)();
    void (*field_2)();
    void (*field_3)();
    void (*field_4)();

    void (*SerializeForNetwork)(__int64, __int64, __int64*, unsigned int);
    void (*OnNewServerMessageReceived)(__int64, NetworkReceiveContext*, bsDataStore*);
    void (*MessageReceivedForEnt)(__int64, NetworkReceiveContext*, bsDataStore*);
};

#endif //PROMETHEUS_SYSTEM_22_NETWORKEVENTSYSTEM_H