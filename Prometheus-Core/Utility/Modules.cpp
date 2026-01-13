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

namespace Utility::Modules
{
    static std::vector<ModuleBounds> Modules{};

    static std::atomic dirty{true};
    static PVOID cookie = nullptr;

    static std::optional<ModuleBounds> GameBase{};
    static std::optional<ModuleBounds> CoreBase{};

    const ModuleBounds& GameBounds() { return *GameBase; }
    const ModuleBounds& CoreBounds() { return *CoreBase; }

    uint64 ModuleBounds::RVA(const uint64 address) const
    {
        if (address < base || address >= end) {
            return 0;
        }

        return address - base;
    }

    static void NTAPI DllNotify(ULONG reason, const void* data, void* ctx)
    {
        dirty.store(true, std::memory_order_relaxed);
    }

    void TryUpdateModules()
    {
        if (!dirty.exchange(false, std::memory_order_relaxed)) {
            return;
        }

        Modules.clear();

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
            Modules.emplace_back(me);
        } while (Module32Next(snap, &me));

        CloseHandle(snap);
        std::ranges::sort(Modules, [](const ModuleBounds& a, const ModuleBounds& b) { return a.Base() < b.Base(); });

        GameBase.emplace(*FindModuleForAddress(GetModuleHandleA(nullptr)));
        CoreBase.emplace(*FindModuleForAddress(reinterpret_cast<uint64>(&Initialize)));
    }

    void Initialize()
    {
        const HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
        const auto registerDll = reinterpret_cast<LdrRegisterDllNotification_t>(GetProcAddress(ntdll, "LdrRegisterDllNotification"));
        if (!registerDll) {
            printf("Failed to get LdrRegisterDllNotification\n");
            return;
        }

        if (registerDll(0, reinterpret_cast<PVOID>(&DllNotify), nullptr, &cookie) < 0) {
            printf("Failed to register DllNotify\n");
            return;
        }

        dirty.store(true, std::memory_order_relaxed);
        printf("Successfully registered DllNotify\n");

        TryUpdateModules();
    }

    void Uninitialize()
    {
        if (!cookie) {
            return;
        }

        const HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
        const auto unregisterDll = reinterpret_cast<LdrUnregisterDllNotification_t>(GetProcAddress(ntdll, "LdrUnregisterDllNotification"));
        if (!unregisterDll) {
            printf("Failed to get LdrUnregisterDllNotification\n");
            return;
        }

        if (unregisterDll(cookie)) {
            printf("Failed to unregister DllNotify\n");
            return;
        }

        cookie = nullptr;
    }

    const ModuleBounds* FindModuleForAddress(const uint64 addr)
    {
        TryUpdateModules();

        auto it = std::upper_bound(Modules.begin(), Modules.end(), addr,
                [](const uint64 value, const ModuleBounds& m) { return value < m.Base(); });

        if (it == Modules.begin()) {
            return nullptr;
        }

        --it;
        if (addr >= it->Base() && addr < it->End()) {
            return &*it;
        }

        return nullptr;
    }
}
