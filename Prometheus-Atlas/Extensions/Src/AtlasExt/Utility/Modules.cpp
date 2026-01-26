// Hazno - 2026

#include <vector>
#include <algorithm>
#include <atomic>
#include <windows.h>
#include <winternl.h>
#include <optional>

#include "Modules.h"

using LdrRegisterDllNotification_t = NTSTATUS(NTAPI*)(_In_ ULONG Flags, _In_ PVOID NotificationFunction, _In_opt_ PVOID Context, _Out_ PVOID* Cookie);
using LdrUnregisterDllNotification_t = NTSTATUS(NTAPI*)(_In_ PVOID Cookie);

namespace Atlas::Utility::Modules
{
    static std::vector<ModuleBounds> s_modules{};

    static std::atomic s_dirty{true};
    static PVOID s_cookie = nullptr;

    static std::optional<ModuleBounds> s_programBounds{};
    static std::optional<ModuleBounds> s_runtimeBounds{};

    const ModuleBounds& ProgramBounds() { return *s_programBounds; }
    const ModuleBounds& RuntimeBounds() { return *s_runtimeBounds; }

    uint64 ModuleBounds::RVA(const uint64 absolute) const
    {
        if (absolute < base || absolute >= end) {
            return 0;
        }

        return absolute - base;
    }

    uint64 ModuleBounds::VA(const uint64 relative) const
    {
        const auto res = base + relative;
        if (res < base || res >= end) {
            return 0;
        }

        return res;
    }

    static void NTAPI DllNotify(ULONG reason, const void* data, void* ctx)
    {
        s_dirty.store(true, std::memory_order_relaxed);
    }

    void TryUpdateModules()
    {
        if (!s_dirty.exchange(false, std::memory_order_relaxed)) {
            return;
        }

        s_modules.clear();

        const DWORD pid = GetCurrentProcessId();
        const auto snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
        if (snap == INVALID_HANDLE_VALUE) {
            printf("Invalid handle returned from snapshot in TryUpdateModules\n");
            return;
        }

        MODULEENTRY32 me{};
        me.dwSize = sizeof(me);

        if (!Module32First(snap, &me)) {
            CloseHandle(snap);
            printf("Failed to get first module in TryUpdateModules\n");
            return;
        }

        do {
            s_modules.emplace_back(me);
        } while (Module32Next(snap, &me));

        CloseHandle(snap);
        std::ranges::sort(s_modules, [](const ModuleBounds& a, const ModuleBounds& b) { return a.Base() < b.Base(); });

        s_programBounds.emplace(*FindModuleForAddress(GetModuleHandleA(nullptr)));
        s_runtimeBounds.emplace(*FindModuleForAddress(reinterpret_cast<uint64>(&Initialize)));
    }

    void Initialize()
    {
        if (s_cookie) {
            printf("Init failed. Modules Utility is already initialized!!\n");
            return;
        }

        const HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
        const auto registerDll = reinterpret_cast<LdrRegisterDllNotification_t>(GetProcAddress(ntdll, "LdrRegisterDllNotification"));
        if (!registerDll) {
            printf("Failed to get LdrRegisterDllNotification\n");
            return;
        }

        if (registerDll(0, reinterpret_cast<PVOID>(&DllNotify), nullptr, &s_cookie) < 0) {
            printf("Failed to register DllNotify\n");
            return;
        }

        s_dirty.store(true, std::memory_order_relaxed);
        printf("Successfully registered DllNotify\n");

        TryUpdateModules();
    }

    void Uninitialize()
    {
        if (!s_cookie) {
            return;
        }

        const HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
        const auto unregisterDll = reinterpret_cast<LdrUnregisterDllNotification_t>(GetProcAddress(ntdll, "LdrUnregisterDllNotification"));
        if (!unregisterDll) {
            printf("Failed to get LdrUnregisterDllNotification\n");
            return;
        }

        if (unregisterDll(s_cookie)) {
            printf("Failed to unregister DllNotify\n");
            return;
        }

        s_cookie = nullptr;
    }

    const ModuleBounds* FindModuleForAddress(const uint64 addr)
    {
        TryUpdateModules();

        auto it = std::upper_bound(s_modules.begin(), s_modules.end(), addr,
                [](const uint64 value, const ModuleBounds& m) { return value < m.Base(); });

        if (it == s_modules.begin()) {
            return nullptr;
        }

        --it;
        if (addr >= it->Base() && addr < it->End()) {
            return &*it;
        }

        return nullptr;
    }
}
