#pragma once
#include "Statescript.h"
#include <MinHook.h>
#include <mutex>
#include <functional>

class statescript_logger {
public:
    struct LogStruct {
        std::vector<std::string> logs;
        std::vector<StatescriptNotification_Base*> e_log;
    };

    struct ScriptState {
        __int64 change_timestamp = 0;
        bool is_finish_state = true;
    };

    static void Initialize() {
        MH_CreateHook((PVOID)(globals::gameBase + 0xf4f600), TickScript, (PVOID*)&TickScript_orig);
        MH_EnableHook((PVOID)(globals::gameBase + 0xf4f600));

        MH_CreateHook((PVOID)(globals::gameBase + 0xd146a0), OnNodeStateEntry, (PVOID*)&StateEntry_orig);
        MH_EnableHook((PVOID)(globals::gameBase + 0xd146a0));

        MH_CreateHook((PVOID)(globals::gameBase + 0xd14850), OnNodeStateFinished, (PVOID*)&StateFinish_orig);
        MH_EnableHook((PVOID)(globals::gameBase + 0xd14850));

        MH_CreateHook((PVOID)(globals::gameBase + 0xd14a20), OnNodeActionEntry, (PVOID*)&ActionEntry_orig);
        MH_EnableHook((PVOID)(globals::gameBase + 0xd14a20));

        MH_CreateHook((PVOID)(globals::gameBase + 0xf47800), OnScriptEntry, (PVOID*)&ScriptEntry_orig);
        MH_EnableHook((PVOID)(globals::gameBase + 0xf47800));

        MH_CreateHook((PVOID)(globals::gameBase + 0xd73a80), Deallocate, (PVOID*)&Deallocate_orig);
        MH_EnableHook((PVOID)(globals::gameBase + 0xd73a80));
    }

    static const LogStruct& GetScriptLogs(StatescriptInstance* ss) {
        if (s_subscriptions.find(ss) != s_subscriptions.end())
            return s_subscriptions[ss];
        return EmptyLog;
    }
    static const std::map<int, ScriptState>& GetScriptState(StatescriptInstance* ss) {
        if (s_scriptStates.find(ss) != s_scriptStates.end())
            return s_scriptStates[ss];
        return EmptyScriptState;
    }
    static const ScriptState& GetScriptNodeState(StatescriptInstance* ss, int m_nodex_idx) {
        if (s_scriptStates.find(ss) != s_scriptStates.end())
            return s_scriptStates[ss][m_nodex_idx];
        return EmptyNodeState;
    }

    static void Subscribe(StatescriptInstance* ss) {
        s_subscriptions.emplace(ss, LogStruct{});
    }

    static void Unsubscribe(StatescriptInstance* ss) {
        s_subscriptions.erase(ss);
    }

    static void SubscribeDeallocation(std::weak_ptr<std::function<void(StatescriptInstance*)>> func) {
        s_deletionSubs.push_back(func);
    }

    static inline __int64(__fastcall* TickScript_orig)(StatescriptInstance*, StatescriptNotification_Base*);
    static __int64 __fastcall TickScript(StatescriptInstance* ss, StatescriptNotification_Base* ss_e) {
        //printf("Tick [%hd %p] ", ss->instance_id, ss->script_id);
        if (s_subscriptions.find(ss) != s_subscriptions.end()) {
            char buf[128];
            ss_e->vfptr->ToString(ss_e, buf, 128);
            StatescriptNotification_Base* cloned = nullptr;
            switch (ss_e->type) {
            case ETYPE_LIFETIME:
                cloned = (StatescriptNotification_Base*)new StatescriptNotification_Lifetime;
                break;
            case ETYPE_TIMER:
                cloned = (StatescriptNotification_Base*)new StatescriptNotification_Timer;
                break;
            case ETYPE_EVALEXITS:
                cloned = (StatescriptNotification_Base*)new StatescriptNotification_EvaluateExits;
                break;
            case ETYPE_TRAVERSEFROM:
                cloned = (StatescriptNotification_Base*)new StatescriptNotification_TraverseFrom;
                break;
            case ETYPE_SETVAR:
                cloned = (StatescriptNotification_Base*)new StatescriptNotification_SetVar;
                break;
            case ETYPE_SETVARARRAY:
                cloned = (StatescriptNotification_Base*)new StatescriptNotification_SetVarArray;
                break;
            case ETYPE_STOPINST:
                cloned = (StatescriptNotification_Base*)new StatescriptNotification_Base;
                break;
            case ETYPE_LOGICALBUTTON:
                cloned = (StatescriptNotification_Base*)new StatescriptNotification_LogicalButton;
                break;
            case ETYPE_ONATTRDEPCHANGE:
                cloned = (StatescriptNotification_Base*)new StatescriptNotification_AttrDependencyChanged;
                break;
            default:
                printf("Failed to clone statescript type %d.\n");
                break;
            }
            auto& sub = s_subscriptions[ss];
            sub.logs.push_back(buf);
            if (cloned) {
                cloned->vfptr = ss_e->vfptr;
                sub.e_log.push_back(cloned);
            }
            while (sub.logs.size() > 1000)
                sub.logs.erase(sub.logs.begin());
            while (sub.e_log.size() > 1000)
                sub.e_log.erase(sub.e_log.begin());
        }
        char result = TickScript_orig(ss, ss_e);
        return result;
    }
private:
    static inline void(__fastcall* StateEntry_orig)(StatescriptSyncMgr*, StatescriptInstance*, StatescriptState*);
    static inline void(__fastcall* StateFinish_orig)(StatescriptSyncMgr*, StatescriptInstance*, StatescriptState*);
    static inline void(__fastcall* ActionEntry_orig)(StatescriptSyncMgr*, StatescriptInstance*, StatescriptState*, int);
    static inline char(__fastcall* ScriptEntry_orig)(__int64, StatescriptInstance*, STUStatescriptBase*);
    static inline void(__fastcall* Deallocate_orig)(StatescriptInstance*, char);

    static void __fastcall OnNodeStateEntry(StatescriptSyncMgr* inner, StatescriptInstance* ss, StatescriptState* istate) {
        //printf("State Became Active: %hhd:%d\n", ss->instance_id, istate->idx_in_m_states);
        if (!ss->graph) {
            printf("Invalid state update from %p/%p/%p\n", inner, ss, istate);
            return;
        }
        auto& state = s_scriptStates[ss][ss->graph->STUFromState(istate)->ss_base.idx_in_nodes];
        state.change_timestamp = inner->cf_timestamp;
        state.is_finish_state = false;
        StateEntry_orig(inner, ss, istate);
    }
    static void __fastcall OnNodeStateFinished(StatescriptSyncMgr* inner, StatescriptInstance* ss, StatescriptState* istate) {
        //printf("State Became Inactive: %hhd:%d\n", ss->instance_id, istate->idx_in_m_states);
        if (!ss->graph) {
            printf("Invalid state update from %p/%p/%p\n", inner, ss, istate);
            return;
        }
        auto& state = s_scriptStates[ss][ss->graph->STUFromState(istate)->ss_base.idx_in_nodes];
        state.change_timestamp = inner->cf_timestamp;
        state.is_finish_state = true;
        StateFinish_orig(inner, ss, istate);
    }
    static void __fastcall OnNodeActionEntry(StatescriptSyncMgr* inner, StatescriptInstance* ss, StatescriptState* istate, int idx_in_m_nodes) {
        //printf("Action Became Active: %hhd:%d\n", ss->instance_id, idx_in_m_nodes);
        auto& state = s_scriptStates[ss][idx_in_m_nodes];
        state.change_timestamp = inner->cf_timestamp;
        state.is_finish_state = true;
        ActionEntry_orig(inner, ss, istate, idx_in_m_nodes);
    }
    static char __fastcall OnScriptEntry(__int64 a1, StatescriptInstance* ss, STUStatescriptBase* stu) {
        //printf("Took Node Entry: %hhd:%d\n", ss->instance_id, stu->idx_in_nodes);
        auto& state = s_scriptStates[ss][stu->idx_in_nodes];
        state.change_timestamp = ss->ss_inner->cf_timestamp;
        state.is_finish_state = true;
        return ScriptEntry_orig(a1, ss, stu);
    }
    static void __fastcall Deallocate(StatescriptInstance* script, char a2) {
        //printf("Script %p (%d) deallocated\n", script->script_id, script->instance_id);
        s_subscriptions.erase(script);
        s_scriptStates.erase(script);
        for (auto it = s_deletionSubs.begin(); it != s_deletionSubs.end(); ) {
            auto func = it->lock();
            if (func) {
                (*func)(script);
                it++;
            }
            else {
                s_deletionSubs.erase(it);
            }
        }
        Deallocate_orig(script, a2);
    }

    static inline std::map<StatescriptInstance*, LogStruct> s_subscriptions{};
    static inline std::vector<std::weak_ptr<std::function<void(StatescriptInstance*)>>> s_deletionSubs{};
    static inline std::map<StatescriptInstance*, std::map<int, ScriptState>> s_scriptStates{};
    static inline const LogStruct const EmptyLog{};
    static inline const std::map<int, ScriptState> const EmptyScriptState{};
    // PARALIRA: Needed to initialise the variables here
    static inline const ScriptState const EmptyNodeState{0, true};
};