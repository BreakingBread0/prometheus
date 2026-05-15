// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <stdio.h>
#include <MinHook.h>
#include <string>
#define LAZY_IMPORTER_CASE_INSENSITIVE
#include "lazy_importer.h"
#include <mutex>
#include <tlhelp32.h>
#include <thread>
#include <locale>
#include <codecvt>
#include <winternl.h>
#include <algorithm>
#include <format>
#include <nlohmann/json.hpp>
#include <d3d11.h>
#include "pe.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_freetype.h"
#include "idadefs.h"
#include "UI/window_manager/window_manager.h"
#include "UI/window_manager/management_window.h"
#include "UI/imgui_helpers.h"
#include "globals.h"
#include "JAM.h"
#include "stringhash_library.h"
#include "Components/Component_54_LobbyMap.h"
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "ntdll.lib")
#ifdef MINGW
#pragma comment(lib, "crypt32.lib")
#else
#pragma comment(lib, "Crypt32.lib")
#endif
#include "Statescript.h"
#include "game.h"
#include "ResourceManager.h"
#include "STU.h"
#include "statescript_logger.h"
#include "../external/imnodes/imnodes.h"
#include "stu_resources.h"
#include "entity_admin.h"
#include <array>

//Who needs a build system?
#include "../external/imnodes/imnodes.cpp"

#include "Logs/Logs.h"
#include "Utility/Exception.h"
#include "AtlasExt/Utility/Modules.h"
#include "Probing/Probe_STU.h"
#include "Probing/Probing.h"
#include "displaytext_resources.h"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ImVec2, x, y);

struct memmgr;
struct memmgr_vt {
    union {
        STRUCT_PLACE_CUSTOM(a, 0x30, __int64(__fastcall* allocate)(memmgr*, __int64, int));
        STRUCT_PLACE_CUSTOM(b, 0x40, void(__fastcall* deallocate)(memmgr*, __int64));
    };
};

struct memmgr {
    memmgr_vt* vfptr;
};

memmgr* get_memmgr() {
    return *(memmgr**)(globals::gameBase + 0x181ee50);
}

__int64 ow_memalloc(int size) {
    auto memmgr = get_memmgr();
    return memmgr->vfptr->allocate(memmgr, size, 0x10);
}

void ow_dealloc(__int64 address) {
    auto memmgr = get_memmgr();
    __try {
        return memmgr->vfptr->deallocate(memmgr, address);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        LOG_CORE(Warn, "failed ow_dealloc for {:x}\n", address);
    }
}

//_insn decodeInsn(DWORD_PTR where);
//void patchJump(DWORD_PTR location, DWORD_PTR toAddr);
//void patchToJump(DWORD_PTR location);

const DWORD_PTR Start_Addr = 0x11fe3ac; //NOTE: Unused since usage of the launcher.
const DWORD_PTR TlsCallback_Addr = 0x137f000;
const DWORD_PTR TlsCallback2_Addr = 0x113cd10;
//const DWORD_PTR TlsCallback2_Addr = 0x113cd10;
//const DWORD_PTR owVEH_Addr = 0x13810d0;

DWORD mainThreadId;
__int64(__stdcall* AddVeh_orig)(__int64 old, __int64 func);
void(__fastcall* ExitProcess_orig)(int);
__int64(__fastcall* createwindow_orig)(__int64);

std::wstring s2ws(const std::string& str)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.from_bytes(str);
}

std::string ws2s(const std::wstring& wstr)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(wstr);
}

DWORD GetMainThreadId() {
    auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        LOG_CORE(Error, "Could not open handle to get main thread\n");
        return 0;
    }
    THREADENTRY32 tEntry;
    tEntry.dwSize = sizeof(THREADENTRY32);
    DWORD result = 0;
    DWORD currentPID = GetCurrentProcessId();
    for (BOOL success = Thread32First(snapshot, &tEntry);
        !result && success && GetLastError() != ERROR_NO_MORE_FILES;
        success = Thread32Next(snapshot, &tEntry))
    {
        if (tEntry.th32OwnerProcessID == currentPID) {
            result = tEntry.th32ThreadID;
            break;
        }
    }
    CloseHandle(snapshot);
    return result;
}

void exit_handler() {
    LOG_CORE(Info, "=============== shutdown ===============\n");
    if (!globals::exit_normal)
        system("pause");
}

void __fastcall ExitProcessHook(int exitCode) {
    LOG_CORE(Info, "Process exit initiazed: %d\n", exitCode);
    ExitProcess_orig(exitCode);
}

void decryptPage(__int64 page) {
    auto testHandler = (void(__fastcall*)(_EXCEPTION_POINTERS*))(globals::gameBase + 0x13810d0);
    //PVECTORED_EXCEPTION_HANDLER vehHandler = (PVECTORED_EXCEPTION_HANDLER)&;
    _EXCEPTION_POINTERS exc{};
    _EXCEPTION_RECORD record{};
    exc.ExceptionRecord = &record;
    exc.ExceptionRecord->ExceptionCode = 0xC0000005;
    exc.ExceptionRecord->ExceptionAddress = (PVOID)(globals::gameBase + page);
    exc.ExceptionRecord->ExceptionInformation[1] = globals::gameBase + page;
    testHandler(&exc);
}

int DebuggerTrapHook() {
    //printf("Debugger Trap from %p\n", _ReturnAddress());
    return false;
}

ID3D11Device* g_pd3dDevice = 0;
ID3D11DeviceContext* g_pd3dContext = 0;
typedef HRESULT(__stdcall* D3D11PresentHook) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
D3D11PresentHook phookD3D11Present = nullptr;
LONG_PTR m_ulOldWndProc;
ID3D11RenderTargetView* g_mainRenderTargetView = NULL;
std::once_flag imgui_init{};

void CreateRenderTarget(IDXGISwapChain* swap, bool firstTime)
{
    if (firstTime)
    {
        DXGI_SWAP_CHAIN_DESC sd;
        swap->GetDesc(&sd);
        // Create the render target
        ID3D11Texture2D* pBackBuffer;
        D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc;
        ZeroMemory(&render_target_view_desc, sizeof(render_target_view_desc));
        render_target_view_desc.Format = sd.BufferDesc.Format;
        render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, &render_target_view_desc, &g_mainRenderTargetView);
        g_pd3dContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        pBackBuffer->Release();
    }
    else
        g_pd3dContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui::GetCurrentContext() != NULL) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;
        else if (ImGui::GetIO().WantCaptureKeyboard) {
            switch (msg) {
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_CHAR:
                return false;
            }
        }
    }
    return CallWindowProcW((WNDPROC)m_ulOldWndProc, hWnd, msg, wParam, lParam);
}

typedef bool(*clipboard_fn)(__int64, string_rep* str_out);

const char* getClipboard(void*) {
    string_rep out{};
    clipboard_fn fn = (clipboard_fn)(globals::gameBase + 0x8ab990);
    if (fn(0, &out)) {
        LOG_CORE(Debug, "GetClipboard succeessful\n");
        size_t len = strlen(out.get());
        char* str = new char[len + 1];
        strcpy(str, out.get());
        return str;
    }
    LOG_CORE(Warn, "GetClipboard failed\n");
    return new char[0];
}

IDXGISwapChain* main_swapchain = nullptr;
bool ui_invisible = false;

ImFont* tryLoadFont(const char* file, const ImFontConfig* font_cfg = nullptr, const ImWchar* glyph_ranges = nullptr) {
    auto& io = ImGui::GetIO();
    if (std::filesystem::exists(file)) {
        auto font = io.Fonts->AddFontFromFileTTF(file, 13, font_cfg, glyph_ranges);
        if (font)
            return font;
    }
    LOG_CORE(Warn, "Failed to load font {}. Using default font.", file);
    return io.FontDefault;
}

HRESULT __stdcall PresentHook(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    if (!main_swapchain)
        main_swapchain = pSwapChain;
    if (main_swapchain == pSwapChain) {
        //fuck fullscreen, all my homies hate fullscreen in 0.8 - periodt
        *(int*)(globals::gameBase + 0x17a1123) = 0;
        std::call_once(imgui_init, [&]() {
            globals::gameWindow = (DWORD_PTR)FindWindowW(L"tankWindowClass", NULL);
            pSwapChain->GetDevice(__uuidof(g_pd3dDevice), (void**)&g_pd3dDevice);
            g_pd3dDevice->GetImmediateContext(&g_pd3dContext);

            ImGui::CreateContext();
            ImNodes::CreateContext();

            ImGui_ImplWin32_Init((void*)globals::gameWindow);
            ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dContext);

            static ImWchar ranges[] = { 0x1, 0x1FFFF, 0 };
            static ImFontConfig cfg;
            cfg.OversampleH = cfg.OversampleV = 1;
            cfg.MergeMode = true;
            cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;

            auto& io = ImGui::GetIO();
            imgui_helpers::BoldFont = tryLoadFont("MonaspaceXenon-ExtraBold.otf");
            tryLoadFont("MonaspaceXenon-Regular.otf");
            io.FontDefault = tryLoadFont("Font Awesome 6 Free-Solid-900.otf", &cfg, ranges);

            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable /*| ImGuiConfigFlags_ViewportsEnable*/; //TODO: Viewports
            io.ConfigDockingWithShift = true;

            //io.ConfigViewportsNoTaskBarIcon = true;
            //io.ConfigViewportsNoDefaultParent = true;
            ///*io.ConfigDockingAlwaysTabBar = true;*/
            //io.ConfigDockingTransparentPayload = true;

            CreateRenderTarget(pSwapChain, true);
            m_ulOldWndProc = SetWindowLongPtr((HWND)globals::gameWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);
            /*io.KeysData[ImGuiKey_Backspace]. = VK_BACK;
            io.KeysData[ImGuiKey_LeftArrow].Down = VK_LEFT;
            io.KeysData[ImGuiKey_RightArrow].Down = VK_RIGHT;
            io.KeysData[ImGuiKey_UpArrow].Down = VK_UP;
            io.KeysData[ImGuiKey_DownArrow].Down = VK_DOWN;
            io.KeysData[ImGuiKey_Escape].Down = VK_ESCAPE;*/
            //http://kbdedit.com/manual/low_level_vk_list.html
            io.GetClipboardTextFn = getClipboard;

            LOG_CORE(Debug, "Filling STU vfptr table\n");
            STURegistryData::initialize();
            statescript_logger::Initialize();
            window_manager::add_window(new management_window);

            Atlas::Utility::Hash::ParseStrings("./Prometheus/GameStrings.txt");
            Generation::Probing::ProbeToFile(Atlas::Utility::Modules::ProgramBounds(),
                "./Prometheus/ProbeData.json");
        });
        globals::pauseLogHook = true;
        CreateRenderTarget(pSwapChain, false);

        if (ImGui::IsKeyPressed(ImGuiKey_Insert, false)) {
            publish_game_msg(0x0240000000000164);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent, false)) {
            ui_invisible = !ui_invisible;
        }

        auto& io = ImGui::GetIO();
        bool menu_down = GetAsyncKeyState(VK_MENU) < 0;
        io.AddKeyEvent(ImGuiKey_LeftAlt, menu_down);
        io.AddKeyEvent(ImGuiKey_Menu, menu_down);
        io.AddKeyEvent(ImGuiKey_RightAlt, menu_down);

        /*ImGui::GetIO().DisplaySize.x = 500;
        ImGui::GetIO().DisplaySize.y = 500;*/
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (!ui_invisible)
            window_manager::render();

        ImGui::Render();

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        globals::pauseLogHook = false;

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }
    return phookD3D11Present(pSwapChain, SyncInterval, Flags);
}

__int64 __fastcall createwindow_hook(__int64 gameManager) {
    auto handle = createwindow_orig(gameManager);
    LOG_CORE(Info, "window handle: %x\n", handle);
    globals::gameWindow = handle;
    std::thread([]() {
        constexpr int max_attempts = 100;  // Timeout after ~10 seconds
        int attempts = 0;

        while (attempts++ < max_attempts) {
            Sleep(100);
            //teEngine instance
            DWORD_PTR render = *(DWORD_PTR*)(globals::gameBase + 0x181e3e0);
            if (!render)
                continue;
            render = *(DWORD_PTR*)(render + 0x900);
            if (!render)
                continue;
            render = *(DWORD_PTR*)(render + 0x2e0);
            if (!render)
                continue;
            render = *(DWORD_PTR*)(render);
            if (!render)
                continue;
            render = *(DWORD_PTR*)(render + 8);
            if (!render)
                continue;
            render = *(DWORD_PTR*)(render);
            if (!render)
                continue;
            render = *(DWORD_PTR*)(render + 0x40);
            if (!render)
                continue;
            MH_VERIFY(MH_CreateHook((DWORD_PTR*)render, (LPVOID)PresentHook, reinterpret_cast<void**>(&phookD3D11Present)));
            MH_VERIFY(MH_EnableHook((DWORD_PTR*)render));
            return;
        }
        LOG_CORE(Error, "Failed to hook Present! Will terminate application.");
        MessageBoxA(nullptr, "Failed to hook Present. This may indicate that rendering has not properly started within the timeout period or an external overlay prevented hooking.", "Prometheus", MB_OK | MB_ICONERROR);
        }).detach();
    return handle;
}

typedef __int64(__cdecl* regedit_fn)(JamType13String*, JamType13String*, __int64, __int64);
regedit_fn regedit_initialize_orig;
__int64 regedit_initialize(JamType13String* a1, JamType13String* a2, __int64 a3, __int64 a4) {
    auto str = std::string(readJamType13String(a2));
    //printf("regedit: %s\n", str.c_str());
    if (str == "REGION" || str == "LOCALE" || str == "LOCALE_AUDIO") {
        LOG_CORE(Debug, "patch regedit get-region.\n");
        return 1;
    }
    return regedit_initialize_orig(a1, a2, a3, a4);
}

struct CascLogEntry {
    const char* msg;
    char* buffer_begin;
    __int64 buffer_max_size;
    __int64 buffer_size;
    int four;
    int unk;
    const char* culprit;
};

typedef __int64(__cdecl* cascLogHook_fn)(int, const char*, const char*);
cascLogHook_fn cascLogHook_orig;
__int64 cascLogHook(int level, const char* culprit, const char* msg) {
    LOG_CORE(Debug, "CASC %d (%s): %s\n", level, culprit, msg);
    auto result = cascLogHook_orig(level, culprit, msg);
    return result;
}

int (*statescript_load_orig)(StatescriptInstance*);
int statescript_load_hook(StatescriptInstance* ss) {
    int result = statescript_load_orig(ss);
    auto entres = ss->ss_inner->component_ref->base.entity_backref->resload_entry;
    /*if (ss->script_id == 0x580000000000200) {
        printf("loadasd: \n");
        stacktrace();
    }*/
    //printf("---- Load Statescript %p - Entity %x: (entres: %p)\n", ss->script_id, ss->ss_inner->component_ref->base.entity_backref->entity_id, entres->valid() ? entres->align()->resource_id : 0);
    return result;
}

typedef __int64 (*stuux_string_hash_fn)(char* input);

stuux_string_hash_fn stuux_string_hash_orig;
__int64 last_stuux_hash;
__int64 stuux_string_hash_hook(char* input) {
    __int64 hash = stuux_string_hash_orig(input);
    if (last_stuux_hash != hash) {
        last_stuux_hash = hash;
        if (hash != 0)
            stringhash_library::add_comment(hash, input, true);
    }
    return hash;
}

//*chefs kiss* to casclib
#define CASC_KEY_LENGTH 0x10
struct CASC_ENCRYPTION_KEY
{
    ULONGLONG KeyName;                          // "Name" of the key
    BYTE Key[CASC_KEY_LENGTH];                  // The key itself
};

CASC_ENCRYPTION_KEY casc_keys[] = {
    { 0xFB680CB6A8BF81F3ULL, { 0x62, 0xD9, 0x0E, 0xFA, 0x7F, 0x36, 0xD7, 0x1C, 0x39, 0x8A, 0xE2, 0xF1, 0xFE, 0x37, 0xBD, 0xB9 } },   // 0.8.0.24919_retailx64 (hardcoded)
    { 0x402CD9D8D6BFED98ULL, { 0xAE, 0xB0, 0xEA, 0xDE, 0xA4, 0x76, 0x12, 0xFE, 0x6C, 0x04, 0x1A, 0x03, 0x95, 0x8D, 0xF2, 0x41 } },   // 0.8.0.24919_retailx64 (hardcoded)
    { 0xDBD3371554F60306ULL, { 0x34, 0xE3, 0x97, 0xAC, 0xE6, 0xDD, 0x30, 0xEE, 0xFD, 0xC9, 0x8A, 0x2A, 0xB0, 0x93, 0xCD, 0x3C } },   // 0.8.0.24919_retailx64 (streamed from server)
    { 0x11A9203C9881710AULL, { 0x2E, 0x2C, 0xB8, 0xC3, 0x97, 0xC2, 0xF2, 0x4E, 0xD0, 0xB5, 0xE4, 0x52, 0xF1, 0x8D, 0xC2, 0x67 } },   // 0.8.0.24919_retailx64 (streamed from server)
    { 0xA19C4F859F6EFA54ULL, { 0x01, 0x96, 0xCB, 0x6F, 0x5E, 0xCB, 0xAD, 0x7C, 0xB5, 0x28, 0x38, 0x91, 0xB9, 0x71, 0x2B, 0x4B } },   // 0.8.0.24919_retailx64 (streamed from server)
    { 0x87AEBBC9C4E6B601ULL, { 0x68, 0x5E, 0x86, 0xC6, 0x06, 0x3D, 0xFD, 0xA6, 0xC9, 0xE8, 0x52, 0x98, 0x07, 0x6B, 0x3D, 0x42 } },   // 0.8.0.24919_retailx64 (streamed from server)
    { 0xDEE3A0521EFF6F03ULL, { 0xAD, 0x74, 0x0C, 0xE3, 0xFF, 0xFF, 0x92, 0x31, 0x46, 0x81, 0x26, 0x98, 0x57, 0x08, 0xE1, 0xB9 } },   // 0.8.0.24919_retailx64 (streamed from server)
    { 0x8C9106108AA84F07ULL, { 0x53, 0xD8, 0x59, 0xDD, 0xA2, 0x63, 0x5A, 0x38, 0xDC, 0x32, 0xE7, 0x2B, 0x11, 0xB3, 0x2F, 0x29 } },   // 0.8.0.24919_retailx64 (streamed from server)
    { 0x49166D358A34D815ULL, { 0x66, 0x78, 0x68, 0xCD, 0x94, 0xEA, 0x01, 0x35, 0xB9, 0xB1, 0x6C, 0x93, 0xB1, 0x12, 0x4A, 0xBA } },   // 0.8.0.24919_retailx64 (streamed from server)
};
struct tact_key_struct {
    CASC_ENCRYPTION_KEY key;
    char what_is_this;
};

typedef __int64(__fastcall* manager_14_fn)(__int64);
manager_14_fn manager_14_orig;
__int64 __fastcall manager_14_init_hook(__int64 manager) {
    __int64 storageHandler = *(__int64*)(globals::gameBase + 0x182ca50);
    if (storageHandler && *(__int64*)(storageHandler + 0x50)) {
        __int64 magic_fairy_dust = *(__int64*)(storageHandler + 0x58);
        if (magic_fairy_dust) {
            auto emplace_keystruct_fn = (tact_key_struct * (__fastcall*)(__int64, ULONGLONG*))(globals::gameBase + 0x9d0470);
            for (auto& key : casc_keys) {
                tact_key_struct* key_struct = emplace_keystruct_fn(magic_fairy_dust + 8, &key.KeyName);
                LOG_CORE(Info, "Emplacing TACT key %llx\n", key.KeyName);
                memcpy(&key_struct->key, &key, sizeof(CASC_ENCRYPTION_KEY));
                key_struct->what_is_this = 1;
            }
        }
    }
    return manager_14_orig(manager);
}

typedef Entity* (__fastcall* load_entity_fn)(EntityAdminBase*, int* entid, EntityLoader*);
load_entity_fn emplace_entity_orig;
Entity* emplace_entity_hook(EntityAdminBase* ea, int* entid, EntityLoader* loader) {
    if (loader->stu_id == 0x0400000000000433 || //bigass boat on Gibraltar
        loader->stu_id == 0x04000000000001BE || //Spawn no entry grill
        loader->stu_id == 0x0400000000000254 || //Spawn no entry grill
        loader->stu_id == 0x0400000000000255) { //Spawn no entry grill
        for (auto& item : loader->loader_entries)
            item.component_id = 0;
    }
    //stacktrace();
    //printf("Load entity with ID: %p %x %p\n", loader->stu_id, *entid, loader);
    //for (int i = 1; i < 0xc0; i++) {
    //    EntityLoader_Entry* entry = &loader->loader_entries[i];
    //    if (entry->component_id == 0)
    //        continue;
    //    /*if (loader->stu_id == 0x0400000000000001 && (entry->component_id == 0xe || entry->component_id == 0x1c))
    //        entry->component_id = 0;*/
    //    if (stringhash_library::components.find(entry->component_id) != stringhash_library::components.end())
    //        printf("- Component %s (%x) (data %p %p %p )\n", stringhash_library::components[entry->component_id].name.c_str(), entry->component_id, entry->a1, entry->a2, entry->a3);
    //    else
    //        printf("- Component %x (data %p %p %p)\n", entry->component_id, entry->a1, entry->a2, entry->a3);
    //}
    /*if (loader->stu_id == 0x0400000000000169) {
        printf("Hook Load");
        auto entry = &loader->loader_entries[0x3F];
        entry->component_id = 0x3f;
        entry->a1 = 0;
        entry->a2 = 0;
        entry->a3 = 0;
    }*/
    /*if (loader->stu_id == 0x400000000000167) {
        printf("break here!");
        stacktrace();
    }*/
    /*if (loader->stu_id == 0x400000000000fc3) {
        loader->loader_entries[0x4b].component_id = 0x4b;
    }*/
    /*auto result = emplace_entity_orig(ea, entid, loader);;
    if (loader->stu_id == 0x0400000000000311) {
        printf("hook here!\n");
    }*/
    //return result;
    return emplace_entity_orig(ea, entid, loader);
}

typedef __int64(__fastcall* afterdatapackload_)(__int64 a1, __int64 a2, __int64 a3, int a4, int a5);
afterdatapackload_ afterdatapackload_fn;//0xf3eb10
__int64 afterdatapackload_hook(__int64 a1, __int64 a2, __int64 a3, int a4, int a5) {
    __int64 result = afterdatapackload_fn(a1, a2, a3, a4, a5);
    if (a2 == 0x06E0000000000079) {
        LOG_CORE(Info, "Hook AfterDataPack\n");
        __int64 packs[] = {
            //Data Packs
            0x06e0000000000079,
            0x06e00000000000b5,
            0x06e0000000000008,
            0x06e000000000008d,
            0x06e0000000000017,
            0x06e0000000000029,
            0x06e0000000000065,
            0x06e0000000000066,

            //Heroes
            0x02e0000000000003,
            0x02e0000000000002,
            0x02e00000000000dd,
            0x02e000000000000a,
            0x02e0000000000079,
            0x02e0000000000068,
            0x02e0000000000065,
            0x02e0000000000042,
            0x02e0000000000040,
            0x02e0000000000029,
            0x02e0000000000020,
            0x02e0000000000016,
            0x02e0000000000015,
            0x02e0000000000009,
            0x02e0000000000008,
            0x02e000000000007a,
            0x02e0000000000007,
            0x02e000000000006e,
            0x02e0000000000006,
            0x02e0000000000005,
            0x02e0000000000004,

            //STUSkin
            0x0ae00000000003d4,
            0x0ae00000000003d6,
            0x0ae00000000003d7,
            0x0ae00000000003d8,
            0x0ae00000000003d9,
            0x0ae00000000003da,
            0x0ae00000000003dc,
            0x0ae00000000003dd,
            0x0ae00000000003de,
            0x0ae00000000003e0,
            0x0ae00000000003e1,
            0x0ae00000000003e2,
            0x0ae00000000003e3,
            0x0ae00000000003e4,
            0x0ae00000000003e5,
            0x0ae00000000003e6,
            0x0ae00000000003e7,
            0x0ae00000000003e8,
            0x0ae00000000003e9,
            0x0ae00000000003ea,
            0x0ae00000000003eb,

            //STUMapHeader
            0x0790000000000679,
            0x07900000000000d4,
            0x07900000000001c2,
            0x07900000000001d4,
            0x07900000000001db,
            0x07900000000002af,
            0x07900000000002c3,
            0x079000000000005b,
            0x079000000000067a,
            0x079000000000067b,
            0x0790000000000165,
            0x0790000000000184,
            0x0790000000000223,
            0x0790000000000661,
            0x0790000000000664,
            0x0790000000000675,
            0x0790000000000676,
            0x0790000000000677,
            0x0790000000000678,

            //STUCatalog
            0x046000000000002B,
            0x0460000000000068,
            0x046000000000007A,
            0x04600000000000E0,
            0x046000000000003E,
            0x046000000000002A,

            //Test
            0x0B90000000000001,
            0x0B90000000000002,
            0x0B90000000000003,
            0x0B90000000000004,
        };
        for (auto pack : packs) {
            afterdatapackload_fn(a1, pack, a3, 0, 3);
            afterdatapackload_fn(a1, pack, a3, 0, 5);
        }

        unsigned char ret_1[] = { 0xB0, 0x01, 0xC3 };
        memcpy((void*)(globals::gameBase + 0xc7c960), ret_1, sizeof(ret_1));
        LOG_CORE(Debug, "Force chat enabled\n"); //Must be done after data packs loaded
    }
    return result;
}
__int64(__fastcall* vmstu_stringhash_orig)(__int64);
__int64 vmstu_stringhash_hook(__int64 viewmodel) {
    const char* str = *(const char**)(viewmodel + 0x18);
    //printf("ViewModel: %s\n", str);
    return vmstu_stringhash_orig(viewmodel);
}

__int64(__fastcall* dataflow_bugfix_orig)(__int64 a1, __int64 a2, int a3, float* a4);
__int64 __fastcall dataflow_bugfix_fn(__int64 a1, __int64 a2, int a3, float* a4) {
    if (a3 == 0xF5) {
        auto ptr = *(_QWORD*)(a2 + 0x28);
        if (ptr) {
            ptr = *(_QWORD*)(ptr + 0x208);
            if (!ptr) {
                *a4 = 0;
                return 1;
            }
        }
    }
    return dataflow_bugfix_orig(a1, a2, a3, a4);
}

int ea_initcall = 0;
EntityAdminBase* (*Construct_GameEntityAdmin_orig)(EntityAdminBase* ea, EntityAdminCreationInfo* ci);
EntityAdminBase* Construct_GameEntityAdmin_fn(EntityAdminBase* ea, EntityAdminCreationInfo* ci) {
    ea = Construct_GameEntityAdmin_orig(ea, ci);

    switch (ea_initcall) {
        case 0: //Lobby Entity Admin
            break;
        case 1: {
            //Game Entity Admin
            auto sys = PrometheusSystem::create(ea);
            if (sys) //The second call is for GameEntityAdmin
                ea->systems_array.emplace_item((system_vt**)sys);
        }
            break;
        case 2:
        default:
            //...
            break;
    }
    ea_initcall++;
    return ea;
}

char (*selectiveResLoad_orig)(__int64, __int64);
char __fastcall selectiveResLoad_hook(__int64 a1, __int64 a2)
{
    char v2; // al
    int v3; // eax

    if ((*(_BYTE*)(a2 + 0x10) & 2) == 0) {
        //printf("& 2 == 0\n");
        //printf("aaa");
        return 1;
    }
    if ((*(_BYTE*)(a2 + 0x10) & 1) == 0)
    {
        v2 = *(_BYTE*)(a2 + 0x13);
        if (v2 == 8)
        {
            v3 = *(_DWORD*)(a2 + 0x5C);
        }
        else
        {
            if (v2 != 0xB)
                return 1;
            v3 = *(_DWORD*)(a2 + 0x78);
        }
        if (!v3)
            v3 = 2;
        if (v3 > *(int*)(globals::gameBase + 0x182b718)) {
            //printf("> LoadEntIndex\n");
            return 0;
        }
    }
    return 1;
}

char (*component_1_oncreate_orig)(Component_1_SceneRendering*, EntityLoader*);
char componen1_oncreate_hook(Component_1_SceneRendering* comp, EntityLoader* loader) {
    //0xa00450

    /*if (comp->base.entity_backref->resload_entry->valid() && comp->base.entity_backref->resload_entry->align()->resource_id == 0x0400000000000311) {
        printf("COMP1:!!\n");
        printf("STU COMPONENT DATA: %p\n", loader->loader_entries[1].stu_component_data);
        printf("INSTANTIATE DATA: %p\n", loader->loader_entries[1].init_data);
        printf("STU INSTANCE DATA: %p\n", loader->loader_entries[1].stu_instance_data);

        auto init_data = (Component1_InitData*)loader->loader_entries[1].init_data;
        printf("init_data field_0:  %f %f %f %f\n", init_data->field_1, init_data->field_2, init_data->field_3, init_data->field_4);
        printf("init_data field_10: %f %f %f %f\n", init_data->field_10.X, init_data->field_10.Y, init_data->field_10.Z, init_data->field_10.W);
        init_data->field_10.X = 3;
        init_data->field_10.Y = 3;
        init_data->field_10.Z = 3;
        init_data->field_10.W = 3;
        printf("init_data field_20: %f %f %f %f\n", init_data->field_20.X, init_data->field_20.Y, init_data->field_20.Z, init_data->field_20.W);
        printf("\n");
    }*/

    return component_1_oncreate_orig(comp, loader);
}

//void ConstructNewGameEAInPlace() {
//    memset((void*)(globals::gameBase + 0xbfe68a), 0x90, 5); //initialize random values
//    memset((void*)(globals::gameBase + 0xbfe696), 0x90, 5); //init entity admin creation info
//    memset((void*)(globals::gameBase + 0xbfe69b), 0x90, 5); //initialize lobby entity admin
//    memset((void*)(globals::gameBase + 0xbfe6ae), 0x90, 7); //set lobbyea EA_staticstru
//    memset((void*)(globals::gameBase + 0xbfe6c4), 0x90, 11); //lobbyea initialize
//    memset((void*)(globals::gameBase + 0xbfe6fb), 0x90, 5); //tankwindowmanager initialize, comp50 input stuff
//    memset((void*)(globals::gameBase + 0xbff1f8), 0x90, 5); //init ini settings
//    memset((void*)(globals::gameBase + 0xbff213), 0x90, 9); //init replay admin
//
//    ((char(*)(void))(globals::gameBase + 0xbfe670))();
//}
//
//void(*VigsTickOrig)(LARGE_INTEGER, LARGE_INTEGER);
//void testVIGSTickHook(LARGE_INTEGER a1, LARGE_INTEGER a2) {
//    if (globals::switchGameEA) {
//        globals::switchGameEA = false;
//        ConstructNewGameEAInPlace();
//
//    }
//    VigsTickOrig(a1, a2);
//}

__int64 (*System63_orig)(__int64);
__int64 System63_hook(__int64 sys) {
    if (!*(__int64*)(sys + 0x30)) { //should be component 2
        return ((__int64(*)(__int64, __int64))(globals::gameBase + 0xc43d00))(sys - 0x80, 0);
    }
    return System63_orig(sys);
}

__int64 (*CreateAllocator_orig)(int);
__int64 CreateAllocator_hook(int type) {
    //1: default
    //2: windows allocator
    //3: jemalloc
    //4: malloc
    return CreateAllocator_orig(1);
}

__int64 (*MapCreateSwitch_orig)(__int64, __int64);
__int64 MapCreateSwitch_hook(__int64 a1, __int64 a2) {
    auto sw = (char*)(a1 + 0x32);
    /* f 9 8 a d e */
    if (*sw == 0xa)
        return 2;
    return MapCreateSwitch_orig(a1, a2);
}

//https://stackoverflow.com/questions/557081/how-do-i-get-the-hmodule-for-the-currently-executing-code
HMODULE GetCurrentModule()
{ // NB: XP+ solution!
    HMODULE hModule = NULL;
    GetModuleHandleEx(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        (LPCTSTR)GetCurrentModule,
        &hModule);

    return hModule;
}

__int64 AddVehHook(__int64 old, __int64 func) {
    if ((DWORD_PTR)_ReturnAddress() > globals::gameBase && (DWORD_PTR)_ReturnAddress() < globals::gameBase + globals::gameSize)
        return AddVeh_orig(old, func);
    LOG_CORE(Warn, "Caught RtlAddVectoredExceptionHandler %d %p ret: %p\n", old, func, _ReturnAddress());
    return 0;
}

bool CheckDebuggerPresentHook() {
    LOG_CORE(Warn, "CheckDebuggerPresent {:x}", reinterpret_cast<uintptr_t>(_ReturnAddress()));
    // stacktrace();
    return false;
}

void __cdecl StartHook() {
    atexit(exit_handler);

    globals::ensure_console_allocated();
    Atlas::Utility::Modules::Initialize();
    Logs::Initialize(); //soz i defo shouldnt be putting this inside dllmain but like idk where else to put it rn

    printf("Hello World!\n");

    LOG_CORE(Info, "Adding VEH: %p\n", AddVectoredExceptionHandler(true, Utility::Exception::GetExceptionHandler()));
    globals::gameBase = (DWORD_PTR)GetModuleHandleA(nullptr);
    globals::modBase = (DWORD_PTR)GetCurrentModule();
    const auto pe = Pe::PeNative::fromModule(GetModuleHandleA(NULL));
    globals::gameSize = pe.imageSize();
    const auto modpe = Pe::PeNative::fromModule(GetCurrentModule());
    globals::modSize = modpe.imageSize();
    mainThreadId = GetMainThreadId();

    if (GetCurrentThreadId() == mainThreadId)
    {
        LOG_CORE(Error, "Error: Prometheus loaded in an unsupported way. Not proceeding!\n");
        return;
    }
    LOG_CORE(Info, "Main Thread: {:x}\n", mainThreadId);

    MH_VERIFY(MH_Initialize());

    MH_VERIFY(MH_CreateHook((LPVOID)ExitProcess, (LPVOID)ExitProcessHook, (LPVOID*)&ExitProcess_orig));
    MH_VERIFY(MH_EnableHook((LPVOID)ExitProcess));

    auto addVeh = GetProcAddress(LoadLibraryA("ntdll.dll"), "RtlAddVectoredExceptionHandler");
    MH_VERIFY(MH_CreateHook((LPVOID)addVeh, (LPVOID)AddVehHook, (LPVOID*)&AddVeh_orig));
    MH_VERIFY(MH_EnableHook((LPVOID)addVeh));

    MH_VERIFY(MH_CreateHook((LPVOID)IsDebuggerPresent, (LPVOID)CheckDebuggerPresentHook, 0));
    MH_VERIFY(MH_EnableHook((LPVOID)IsDebuggerPresent));

    MH_VERIFY(MH_CreateHook((LPVOID)CheckRemoteDebuggerPresent, (LPVOID)CheckDebuggerPresentHook, 0));
    MH_VERIFY(MH_EnableHook((LPVOID)CheckRemoteDebuggerPresent));

    //The TlsCallbacks handle page decryption and import table decryption.
    LOG_CORE(Debug, "Calling original TlsCallbacks before hooking.");
    ((void(*)(DWORD_PTR, int, int))(globals::gameBase + TlsCallback_Addr))(globals::gameBase, 1, 0);
    ((void(*)(DWORD_PTR, int, int))(globals::gameBase + TlsCallback2_Addr))(globals::gameBase, 1, 0);

    LOG_CORE(Info, "Decrypting pages.");
    decryptPage(Start_Addr); //yes this is wanted
    decryptPage(Start_Addr); //yes this is wanted
    decryptPage(0x1000); //yes this is wanted
    //Encrypted pages are 0x1000 but sometimes it doesnt catch all of them
    //https://images.wikidexcdn.net/mwuploads/wikidex/thumb/4/46/latest/20130917005914/Pok%C3%A9mon_Gotta_catch_em_all_logo.png/600px-Pok%C3%A9mon_Gotta_catch_em_all_logo.png
    for (int i = 0x1000; i < /*0x137ec00 0x1382d58*/globals::gameSize; i += 0x100) {
        decryptPage(i);
        DWORD old;
        VirtualProtect((LPVOID)(globals::gameBase + i), 0x100, PAGE_EXECUTE_READWRITE, &old);
    }

    unsigned char verify[] = { 0x48, 0x83, 0xEC, 0x28, 0xE8, 0xFF, 0xD8, 0x00, 0x00 };
    for (int i = 0; i < sizeof(verify); i++) {
        if (*(unsigned char*)(globals::gameBase + Start_Addr + i) != verify[i]) {
            LOG_CORE(Error, ".text decryption failed! expected: %02x got: %02x at offset %x\n", verify[i], *(char*)(globals::gameBase + Start_Addr + i), i);
            return;
        }
    }

    memset((void*)(globals::gameBase + 0x1133D81), 0x90, 0x4); //illegal instruction at end of crash_me__.
    LOG_CORE(Debug, "patched debugger trap 1 (illegal isntruction)\n");

    //ud2
    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0x5abd00), (LPVOID)DebuggerTrapHook, 0));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0x5abd00)));
    //basically same debugger trap here but __debugbreak()
    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0x5abd30), (LPVOID)DebuggerTrapHook, 0));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0x5abd30)));
    //ame but ud2 again
    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0x808b90), (LPVOID)DebuggerTrapHook, 0));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0x808b90)));
    //ud2
    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0x7e1710), (LPVOID)DebuggerTrapHook, 0));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0x7e1710)));
    //crash_me__
    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0x1130960), (LPVOID)DebuggerTrapHook, 0));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0x1130960)));

    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0x8fc240), (LPVOID)createwindow_hook, (PVOID*)&createwindow_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0x8fc240)));

    unsigned char return_zero[] = { 0x31, 0xC0, 0xC3 };
    memcpy((void*)(globals::gameBase + 0x804740), return_zero, sizeof(return_zero));
    LOG_CORE(Debug, "patched debugger trap 2 (return value check)\n");

    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0x5ac130), (LPVOID)regedit_initialize, (PVOID*)&regedit_initialize_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0x5ac130)));
    LOG_CORE(Debug,"patched regedit launch options\n");

    // MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0x80f7c0), (LPVOID)emplace_entity_hook, (PVOID*)&emplace_entity_orig));
    // MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0x80f7c0)));
    // printf("emplace entity hook\n");

    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xf3eb10), (LPVOID)afterdatapackload_hook, (PVOID*)&afterdatapackload_fn));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xf3eb10)));
    LOG_CORE(Debug, "after data pack load hook\n");

    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xa485d0), (LPVOID)MapCreateSwitch_hook, (PVOID*)&MapCreateSwitch_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xa485d0)));
    LOG_CORE(Debug, "mapcreate hook\n");

    // MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xa00450), (LPVOID)componen1_oncreate_hook, (PVOID*)&component_1_oncreate_orig));
    // MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xa00450)));

    /*MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xbff460), testVIGSTickHook, (PVOID*)&VigsTickOrig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xbff460)));*/

#pragma region Anti-Anti-Debug
    memcpy((void*)(globals::gameBase + 0x000000000137F061), std::array<unsigned char, 5>{0xe9, 0x57, 0x20, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x000000000137F0D1), std::array<unsigned char, 5>{0xe9, 0xdf, 0x1f, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x000000000138119B), std::array<unsigned char, 2>{0xeb, 0x42}.data(), 0x2);
    memcpy((void*)(globals::gameBase + 0x0000000001380408), std::array<unsigned char, 5>{0xe9, 0x78, 0xc, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000005A50F7), std::array<unsigned char, 5>{0xe9, 0x92, 0x4, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007CEE55), std::array<unsigned char, 5>{0xe9, 0x2d, 0x4, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000001130A17), std::array<unsigned char, 2>{0xeb, 0x70}.data(), 0x2);
    memcpy((void*)(globals::gameBase + 0x00000000005A59F1), std::array<unsigned char, 5>{0xe9, 0x7f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000001380E4D), std::array<unsigned char, 5>{0xe9, 0x33, 0x2, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007CF3CA), std::array<unsigned char, 5>{0xe9, 0xdf, 0x4, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000001130F81), std::array<unsigned char, 5>{0xe9, 0x56, 0x2, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000005A640A), std::array<unsigned char, 5>{0xe9, 0xf1, 0x41, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007E7C25), std::array<unsigned char, 5>{0xe9, 0x12, 0x2, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007E1BA9), std::array<unsigned char, 2>{0xeb, 0x78}.data(), 0x2);
    memcpy((void*)(globals::gameBase + 0x000000000113215B), std::array<unsigned char, 5>{0xe9, 0x15, 0x1c, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000001131D1E), std::array<unsigned char, 5>{0xe9, 0xe3, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000005AA74D), std::array<unsigned char, 5>{0xe9, 0xd2, 0xf9, 0xff, 0xff}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000005A6F3C), std::array<unsigned char, 5>{0xe9, 0x8d, 0x4, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007E7EBC), std::array<unsigned char, 5>{0xe9, 0xd4, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000005AA2E6), std::array<unsigned char, 5>{0xe9, 0x2, 0xa, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007E80C3), std::array<unsigned char, 5>{0xe9, 0x7e, 0x4, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007E2163), std::array<unsigned char, 5>{0xe9, 0x5a, 0x2, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000005AAE10), std::array<unsigned char, 5>{0xe9, 0x76, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000001132579), std::array<unsigned char, 2>{0xeb, 0x7b}.data(), 0x2);
    memcpy((void*)(globals::gameBase + 0x00000000007D0C32), std::array<unsigned char, 5>{0xe9, 0x7f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000005AB1D8), std::array<unsigned char, 5>{0xe9, 0xea, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000005A9357), std::array<unsigned char, 5>{0xe9, 0x41, 0xd0, 0xff, 0xff}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000005AB3C7), std::array<unsigned char, 5>{0xe9, 0x91, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007E8EB6), std::array<unsigned char, 5>{0xe9, 0x7e, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007E902F), std::array<unsigned char, 5>{0xe9, 0xcc, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C3E713), std::array<unsigned char, 5>{0xe9, 0xcd, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007D9B64), std::array<unsigned char, 5>{0xe9, 0xda, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007D945F), std::array<unsigned char, 5>{0xe9, 0xd0, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007DADAB), std::array<unsigned char, 2>{0xeb, 0x7a}.data(), 0x2);
    memcpy((void*)(globals::gameBase + 0x00000000007E951F), std::array<unsigned char, 5>{0xe9, 0xa8, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007E2CB7), std::array<unsigned char, 5>{0xe9, 0xa6, 0xf2, 0xff, 0xff}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000001132AF1), std::array<unsigned char, 5>{0xe9, 0x56, 0x2, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C3E970), std::array<unsigned char, 5>{0xe9, 0x7, 0x4, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007D9DF2), std::array<unsigned char, 5>{0xe9, 0xf, 0x7, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007D95FC), std::array<unsigned char, 5>{0xe9, 0x93, 0x2, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007D15D5), std::array<unsigned char, 5>{0xe9, 0x15, 0x37, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000001133995), std::array<unsigned char, 5>{0xe9, 0xdb, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000009C2E08), std::array<unsigned char, 2>{0xeb, 0x1b}.data(), 0x2);
    memcpy((void*)(globals::gameBase + 0x00000000007D2192), std::array<unsigned char, 5>{0xe9, 0xe9, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000009C2EEE), std::array<unsigned char, 5>{0xe9, 0xb3, 0x2, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000805177), std::array<unsigned char, 2>{0xeb, 0x7b}.data(), 0x2);
    memcpy((void*)(globals::gameBase + 0x00000000007DB658), std::array<unsigned char, 5>{0xe9, 0xe6, 0x2e, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007FAA1B), std::array<unsigned char, 5>{0xe9, 0xef, 0x5, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000005A144D), std::array<unsigned char, 2>{0xeb, 0x77}.data(), 0x2);
    memcpy((void*)(globals::gameBase + 0x0000000000805213), std::array<unsigned char, 5>{0xe9, 0xc8, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000008086E8), std::array<unsigned char, 5>{0xe9, 0xce, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007D3FA1), std::array<unsigned char, 5>{0xe9, 0xc5, 0xd4, 0xff, 0xff}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x000000000080577A), std::array<unsigned char, 5>{0xe9, 0x4c, 0x6, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000808883), std::array<unsigned char, 5>{0xe9, 0xc1, 0x2, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007DC0C2), std::array<unsigned char, 5>{0xe9, 0x79, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C38F24), std::array<unsigned char, 5>{0xe9, 0x52, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C3B58D), std::array<unsigned char, 5>{0xe9, 0x21, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000808E07), std::array<unsigned char, 2>{0xeb, 0x7d}.data(), 0x2);
    memcpy((void*)(globals::gameBase + 0x00000000007F7F13), std::array<unsigned char, 5>{0xe9, 0x1, 0x6, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C3D417), std::array<unsigned char, 2>{0xeb, 0x22}.data(), 0x2);
    memcpy((void*)(globals::gameBase + 0x00000000007DDB21), std::array<unsigned char, 5>{0xe9, 0xd5, 0xd9, 0xff, 0xff}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000005A1914), std::array<unsigned char, 5>{0xe9, 0xaa, 0x16, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C3BEEA), std::array<unsigned char, 5>{0xe9, 0xd9, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C3D594), std::array<unsigned char, 5>{0xe9, 0x2e, 0x5, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C3C174), std::array<unsigned char, 5>{0xe9, 0xc4, 0x6, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007EBE76), std::array<unsigned char, 2>{0xeb, 0x7c}.data(), 0x2);
    memcpy((void*)(globals::gameBase + 0x00000000005A1DA6), std::array<unsigned char, 5>{0xe9, 0x4, 0x2, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C5203D), std::array<unsigned char, 5>{0xe9, 0x59, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000809770), std::array<unsigned char, 5>{0xe9, 0x35, 0x39, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007EBFEA), std::array<unsigned char, 5>{0xe9, 0xd1, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C24AAE), std::array<unsigned char, 5>{0xe9, 0xd3, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C3CF5B), std::array<unsigned char, 5>{0xe9, 0xf0, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C51A0E), std::array<unsigned char, 5>{0xe9, 0xac, 0x2, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x000000000080D19D), std::array<unsigned char, 5>{0xe9, 0x63, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x000000000080A1C0), std::array<unsigned char, 5>{0xe9, 0x5b, 0x4, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007EC593), std::array<unsigned char, 5>{0xe9, 0x2d, 0x7, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C24CC3), std::array<unsigned char, 5>{0xe9, 0x93, 0x4, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000005A2942), std::array<unsigned char, 5>{0xe9, 0xb8, 0xee, 0xff, 0xff}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C26431), std::array<unsigned char, 5>{0xe9, 0xcf, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C83B5A), std::array<unsigned char, 5>{0xe9, 0xb7, 0x2, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C39CFC), std::array<unsigned char, 5>{0xe9, 0x5a, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C265DB), std::array<unsigned char, 5>{0xe9, 0xc1, 0xfd, 0xff, 0xff}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C52C7D), std::array<unsigned char, 5>{0xe9, 0x6d, 0x5, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007FD6A4), std::array<unsigned char, 5>{0xe9, 0x7d, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x000000000080C41D), std::array<unsigned char, 5>{0xe9, 0x5, 0xd3, 0xff, 0xff}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000008305AA), std::array<unsigned char, 5>{0xe9, 0xd6, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000008110F1), std::array<unsigned char, 5>{0xe9, 0x87, 0x4, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000008307E8), std::array<unsigned char, 5>{0xe9, 0xd4, 0x5, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C1EFD8), std::array<unsigned char, 5>{0xe9, 0xd1, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C3A1D6), std::array<unsigned char, 2>{0xeb, 0x1b}.data(), 0x2);
    memcpy((void*)(globals::gameBase + 0x0000000000C1F1DA), std::array<unsigned char, 5>{0xe9, 0x8e, 0x4, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C80EC6), std::array<unsigned char, 5>{0xe9, 0x21, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C3A2FE), std::array<unsigned char, 5>{0xe9, 0xdf, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000F3EE5A), std::array<unsigned char, 5>{0xe9, 0xd6, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C1F9B3), std::array<unsigned char, 5>{0xe9, 0xcd, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000F3F064), std::array<unsigned char, 5>{0xe9, 0x2b, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C05679), std::array<unsigned char, 5>{0xe9, 0x27, 0x9, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C1FB4C), std::array<unsigned char, 5>{0xe9, 0xcf, 0xfd, 0xff, 0xff}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C3ADD9), std::array<unsigned char, 5>{0xe9, 0x5d, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000009D260A), std::array<unsigned char, 2>{0xeb, 0x22}.data(), 0x2);
    memcpy((void*)(globals::gameBase + 0x0000000000C383B1), std::array<unsigned char, 5>{0xe9, 0x1a, 0x4, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007FEF06), std::array<unsigned char, 5>{0xe9, 0xd7, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007FDE07), std::array<unsigned char, 5>{0xe9, 0x3, 0x2, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000817C0A), std::array<unsigned char, 5>{0xe9, 0xfe, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000009D275B), std::array<unsigned char, 5>{0xe9, 0x5e, 0x4, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000CB17C2), std::array<unsigned char, 5>{0xe9, 0x6b, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C05A69), std::array<unsigned char, 5>{0xe9, 0xf2, 0xf7, 0xff, 0xff}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007FF0D6), std::array<unsigned char, 5>{0xe9, 0x77, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000818CEC), std::array<unsigned char, 5>{0xe9, 0x25, 0x6, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000008180DC), std::array<unsigned char, 5>{0xe9, 0x9e, 0x2, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000CB6B30), std::array<unsigned char, 5>{0xe9, 0xfa, 0x4, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000E630AB), std::array<unsigned char, 5>{0xe9, 0xd3, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000E62596), std::array<unsigned char, 5>{0xe9, 0xd8, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007FEA60), std::array<unsigned char, 5>{0xe9, 0xfa, 0xef, 0xff, 0xff}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000E63C58), std::array<unsigned char, 5>{0xe9, 0xe8, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000E632FA), std::array<unsigned char, 5>{0xe9, 0x26, 0xfc, 0xff, 0xff}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000E62779), std::array<unsigned char, 5>{0xe9, 0x98, 0x2, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x000000000101258C), std::array<unsigned char, 5>{0xe9, 0xaf, 0x2, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C06C22), std::array<unsigned char, 5>{0xe9, 0xd4, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C75113), std::array<unsigned char, 5>{0xe9, 0xc7, 0xfe, 0xff, 0xff}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000E63F5C), std::array<unsigned char, 5>{0xe9, 0x76, 0xfb, 0xff, 0xff}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000CA2F7E), std::array<unsigned char, 5>{0xe9, 0x58, 0x5, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C9CD8A), std::array<unsigned char, 5>{0xe9, 0x38, 0xfe, 0xff, 0xff}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C9C381), std::array<unsigned char, 5>{0xe9, 0xd8, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000A4B54D), std::array<unsigned char, 5>{0xe9, 0xf3, 0x4, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C9C570), std::array<unsigned char, 5>{0xe9, 0xfa, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000A4BB85), std::array<unsigned char, 5>{0xe9, 0xd9, 0x4, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000CB5108), std::array<unsigned char, 5>{0xe9, 0xfe, 0xc, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000CA3CC3), std::array<unsigned char, 5>{0xe9, 0xf7, 0x8, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C9D8C4), std::array<unsigned char, 5>{0xe9, 0x1a, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C75B53), std::array<unsigned char, 5>{0xe9, 0xce, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000CB5493), std::array<unsigned char, 5>{0xe9, 0xd6, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C76593), std::array<unsigned char, 5>{0xe9, 0xce, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C75D6B), std::array<unsigned char, 5>{0xe9, 0x1c, 0x5, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000CB56E6), std::array<unsigned char, 5>{0xe9, 0x1d, 0xf8, 0xff, 0xff}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C767AB), std::array<unsigned char, 5>{0xe9, 0x1c, 0x5, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C94624), std::array<unsigned char, 2>{0xeb, 0x1b}.data(), 0x2);
    memcpy((void*)(globals::gameBase + 0x0000000000C1BE66), std::array<unsigned char, 5>{0xe9, 0xca, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C94710), std::array<unsigned char, 5>{0xe9, 0xc1, 0x2, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C1C0A8), std::array<unsigned char, 5>{0xe9, 0xfc, 0x5, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C082B7), std::array<unsigned char, 5>{0xe9, 0xd8, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C312C7), std::array<unsigned char, 5>{0xe9, 0xc9, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C084E9), std::array<unsigned char, 5>{0xe9, 0x71, 0x5, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C3145F), std::array<unsigned char, 5>{0xe9, 0xc4, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C093A5), std::array<unsigned char, 5>{0xe9, 0xce, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C095D3), std::array<unsigned char, 5>{0xe9, 0x87, 0x5, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C9FE42), std::array<unsigned char, 5>{0xe9, 0xde, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000CA009F), std::array<unsigned char, 5>{0xe9, 0xf4, 0x5, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000D4D705), std::array<unsigned char, 5>{0xe9, 0xcb, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000D4D8C3), std::array<unsigned char, 5>{0xe9, 0xfc, 0xfb, 0xff, 0xff}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C2D9B4), std::array<unsigned char, 5>{0xe9, 0xcc, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C2DB4F), std::array<unsigned char, 5>{0xe9, 0xfd, 0xfd, 0xff, 0xff}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C739C6), std::array<unsigned char, 5>{0xe9, 0x26, 0xfe, 0xff, 0xff}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C35A3F), std::array<unsigned char, 2>{0xeb, 0x22}.data(), 0x2);
    memcpy((void*)(globals::gameBase + 0x0000000000C35B33), std::array<unsigned char, 5>{0xe9, 0x93, 0x2, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C2E015), std::array<unsigned char, 5>{0xe9, 0xc3, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C33CB7), std::array<unsigned char, 5>{0xe9, 0xce, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C2E1EF), std::array<unsigned char, 5>{0xe9, 0xfe, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C33E84), std::array<unsigned char, 5>{0xe9, 0x4c, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C362D0), std::array<unsigned char, 5>{0xe9, 0xc8, 0xfe, 0xff, 0xff}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C2E998), std::array<unsigned char, 5>{0xe9, 0xcb, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C2EC08), std::array<unsigned char, 5>{0xe9, 0x86, 0xfc, 0xff, 0xff}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000CBC913), std::array<unsigned char, 5>{0xe9, 0xd6, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000CBCBE8), std::array<unsigned char, 5>{0xe9, 0x23, 0x8, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C78165), std::array<unsigned char, 5>{0xe9, 0xbd, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C783AA), std::array<unsigned char, 5>{0xe9, 0xee, 0x5, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C78EDA), std::array<unsigned char, 5>{0xe9, 0xbd, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000C790DE), std::array<unsigned char, 5>{0xe9, 0x48, 0xfd, 0xff, 0xff}.data(), 0x5);

    memcpy((void*)(globals::gameBase + 0x000000000113CD10), std::array<unsigned char, 2>{0xeb, 0x1e}.data(), 0x2);
    memcpy((void*)(globals::gameBase + 0x0000000001130A8C), std::array<unsigned char, 5>{0xe9, 0xd4, 0xc, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000005A5A78), std::array<unsigned char, 5>{0xe9, 0xcd, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007E1C26), std::array<unsigned char, 5>{0xe9, 0x5c, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000011325F9), std::array<unsigned char, 5>{0xe9, 0xd0, 0xc, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007D0CC1), std::array<unsigned char, 5>{0xe9, 0x36, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007DAE32), std::array<unsigned char, 5>{0xe9, 0xd5, 0x2, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000005A14C9), std::array<unsigned char, 5>{0xe9, 0x4b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x0000000000808E91), std::array<unsigned char, 5>{0xe9, 0x96, 0x3, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x00000000007FD729), std::array<unsigned char, 5>{0xe9, 0x48, 0x1, 0x0, 0x0}.data(), 0x5);
    LOG_CORE(Debug,"patched all antidebug traps\n");

    memcpy((void*)(globals::gameBase + 0x7ed74a), std::array<unsigned char, 5>{0xe9, 0x5b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x7edf96), std::array<unsigned char, 5>{0xe9, 0x7d, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x7ee604), std::array<unsigned char, 5>{0xe9, 0xf8, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x7eedca), std::array<unsigned char, 5>{0xe9, 0x5b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x7ef616), std::array<unsigned char, 5>{0xe9, 0x7d, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x7efe1a), std::array<unsigned char, 5>{0xe9, 0x5b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x7f0666), std::array<unsigned char, 5>{0xe9, 0x7d, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x7f0cd4), std::array<unsigned char, 5>{0xe9, 0xf8, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x7f149a), std::array<unsigned char, 5>{0xe9, 0x5b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x7f1cd8), std::array<unsigned char, 5>{0xe9, 0x7d, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x7f24da), std::array<unsigned char, 5>{0xe9, 0x5b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x7f2c9a), std::array<unsigned char, 5>{0xe9, 0x5b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x7f34e6), std::array<unsigned char, 5>{0xe9, 0x7d, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x80e424), std::array<unsigned char, 5>{0xe9, 0x11, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x80e968), std::array<unsigned char, 5>{0xe9, 0xb0, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x8284b0), std::array<unsigned char, 5>{0xe9, 0x6c, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x828952), std::array<unsigned char, 5>{0xe9, 0xcf, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x828da8), std::array<unsigned char, 5>{0xe9, 0xe0, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x8293d0), std::array<unsigned char, 5>{0xe9, 0x4b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x829914), std::array<unsigned char, 5>{0xe9, 0x2, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x852d7a), std::array<unsigned char, 5>{0xe9, 0x2b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x853596), std::array<unsigned char, 5>{0xe9, 0xad, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x853bce), std::array<unsigned char, 5>{0xe9, 0x9, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x854440), std::array<unsigned char, 5>{0xe9, 0x20, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x854fa5), std::array<unsigned char, 5>{0xe9, 0x11, 0x2, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x8556e7), std::array<unsigned char, 5>{0xe9, 0xca, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x85a543), std::array<unsigned char, 5>{0xe9, 0x5d, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x85a9be), std::array<unsigned char, 5>{0xe9, 0xb8, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x86403e), std::array<unsigned char, 5>{0xe9, 0xf1, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x8911ce), std::array<unsigned char, 5>{0xe9, 0x9, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x8ccc58), std::array<unsigned char, 5>{0xe9, 0xd9, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x8cd0a8), std::array<unsigned char, 5>{0xe9, 0xd9, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x8cd788), std::array<unsigned char, 5>{0xe9, 0x7d, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x8cdc74), std::array<unsigned char, 5>{0xe9, 0xc0, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x8ce2d0), std::array<unsigned char, 5>{0xe9, 0xff, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x907fc2), std::array<unsigned char, 5>{0xe9, 0x52, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x90875a), std::array<unsigned char, 5>{0xe9, 0x5b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x908ea9), std::array<unsigned char, 5>{0xe9, 0x52, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x90a9cf), std::array<unsigned char, 5>{0xe9, 0x5b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x90b422), std::array<unsigned char, 5>{0xe9, 0xa8, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x90c30e), std::array<unsigned char, 5>{0xe9, 0x45, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x90d18e), std::array<unsigned char, 5>{0xe9, 0x8, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x90d9c0), std::array<unsigned char, 5>{0xe9, 0x34, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x90e217), std::array<unsigned char, 5>{0xe9, 0xab, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x90f0c0), std::array<unsigned char, 5>{0xe9, 0x73, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9284b8), std::array<unsigned char, 5>{0xe9, 0xbf, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9350be), std::array<unsigned char, 5>{0xe9, 0xd9, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x93e9dc), std::array<unsigned char, 5>{0xe9, 0x85, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x93ef84), std::array<unsigned char, 5>{0xe9, 0x2, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x93f4b8), std::array<unsigned char, 5>{0xe9, 0x9, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x93fa68), std::array<unsigned char, 5>{0xe9, 0x9, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x93feea), std::array<unsigned char, 5>{0xe9, 0xb9, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x940582), std::array<unsigned char, 5>{0xe9, 0x52, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x940aba), std::array<unsigned char, 5>{0xe9, 0xd0, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x940f34), std::array<unsigned char, 5>{0xe9, 0xc8, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x94155c), std::array<unsigned char, 5>{0xe9, 0x32, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x941cd4), std::array<unsigned char, 5>{0xe9, 0x6c, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x942578), std::array<unsigned char, 5>{0xe9, 0xad, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9429c0), std::array<unsigned char, 5>{0xe9, 0x87, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9430d4), std::array<unsigned char, 5>{0xe9, 0x84, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x943708), std::array<unsigned char, 5>{0xe9, 0x9, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x943b10), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x943ec4), std::array<unsigned char, 5>{0xe9, 0xb0, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x94443c), std::array<unsigned char, 5>{0xe9, 0x1a, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x944c46), std::array<unsigned char, 5>{0xe9, 0xad, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9451ea), std::array<unsigned char, 5>{0xe9, 0xf2, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x94556a), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x945b70), std::array<unsigned char, 5>{0xe9, 0x54, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x946336), std::array<unsigned char, 5>{0xe9, 0x8c, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x946aa0), std::array<unsigned char, 5>{0xe9, 0x73, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x947009), std::array<unsigned char, 5>{0xe9, 0xf1, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x94d7c4), std::array<unsigned char, 5>{0xe9, 0xe1, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x94f10a), std::array<unsigned char, 5>{0xe9, 0x59, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x94f74c), std::array<unsigned char, 5>{0xe9, 0x1a, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x95040a), std::array<unsigned char, 5>{0xe9, 0x5d, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x950866), std::array<unsigned char, 5>{0xe9, 0xa8, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x950d28), std::array<unsigned char, 5>{0xe9, 0xf1, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x951298), std::array<unsigned char, 5>{0xe9, 0x9, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9516b4), std::array<unsigned char, 5>{0xe9, 0xb0, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x951c2e), std::array<unsigned char, 5>{0xe9, 0x11, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9524e0), std::array<unsigned char, 5>{0xe9, 0x9c, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x952c84), std::array<unsigned char, 5>{0xe9, 0x6c, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9533a4), std::array<unsigned char, 5>{0xe9, 0x54, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x953ad0), std::array<unsigned char, 5>{0xe9, 0x5b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x953f54), std::array<unsigned char, 5>{0xe9, 0xb0, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x954504), std::array<unsigned char, 5>{0xe9, 0x28, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x954d20), std::array<unsigned char, 5>{0xe9, 0xb7, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x955530), std::array<unsigned char, 5>{0xe9, 0x4b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x955b08), std::array<unsigned char, 5>{0xe9, 0xff, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x956a15), std::array<unsigned char, 5>{0xe9, 0x58, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x957358), std::array<unsigned char, 5>{0xe9, 0xbf, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x957d7e), std::array<unsigned char, 5>{0xe9, 0xd2, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x958852), std::array<unsigned char, 5>{0xe9, 0x5, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x959797), std::array<unsigned char, 5>{0xe9, 0xfc, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x95a542), std::array<unsigned char, 5>{0xe9, 0xd1, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x95ac62), std::array<unsigned char, 5>{0xe9, 0x88, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x95b430), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x95b98a), std::array<unsigned char, 5>{0xe9, 0xbf, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x95c4f1), std::array<unsigned char, 5>{0xe9, 0xd1, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x95cee2), std::array<unsigned char, 5>{0xe9, 0xe1, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9d51ca), std::array<unsigned char, 5>{0xe9, 0x43, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9d577e), std::array<unsigned char, 5>{0xe9, 0x21, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9d5b19), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9d5f1e), std::array<unsigned char, 5>{0xe9, 0xd9, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9d624d), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9d6511), std::array<unsigned char, 5>{0xe9, 0x88, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9d6b6a), std::array<unsigned char, 5>{0xe9, 0x73, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9d7f3a), std::array<unsigned char, 5>{0xe9, 0x5b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9d83ee), std::array<unsigned char, 5>{0xe9, 0xd9, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9db0f6), std::array<unsigned char, 5>{0xe9, 0xad, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9db800), std::array<unsigned char, 5>{0xe9, 0x54, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9dbdb4), std::array<unsigned char, 5>{0xe9, 0x1a, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9dc2c4), std::array<unsigned char, 5>{0xe9, 0x2, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9f7da1), std::array<unsigned char, 5>{0xe9, 0xbe, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9f8985), std::array<unsigned char, 5>{0xe9, 0x11, 0x2, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9f920a), std::array<unsigned char, 5>{0xe9, 0x44, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9f9764), std::array<unsigned char, 5>{0xe9, 0xc8, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9fc95e), std::array<unsigned char, 5>{0xe9, 0x9, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x9fcee4), std::array<unsigned char, 5>{0xe9, 0x1a, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xa2e238), std::array<unsigned char, 5>{0xe9, 0xb0, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xbfd04a), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xbffeda), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc02fda), std::array<unsigned char, 5>{0xe9, 0x8b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc0347c), std::array<unsigned char, 5>{0xe9, 0xbe, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc040ba), std::array<unsigned char, 5>{0xe9, 0x3a, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc046c4), std::array<unsigned char, 5>{0xe9, 0x1a, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc04daa), std::array<unsigned char, 5>{0xe9, 0x73, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc1b75e), std::array<unsigned char, 5>{0xe9, 0xd1, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc1d120), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc1d68e), std::array<unsigned char, 5>{0xe9, 0xf1, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc1e4e0), std::array<unsigned char, 5>{0xe9, 0x6c, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc23d1e), std::array<unsigned char, 5>{0xe9, 0xf1, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc36f24), std::array<unsigned char, 5>{0xe9, 0x7a, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc37640), std::array<unsigned char, 5>{0xe9, 0x63, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc37afe), std::array<unsigned char, 5>{0xe9, 0xd9, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc42b18), std::array<unsigned char, 5>{0xe9, 0xc8, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc431ca), std::array<unsigned char, 5>{0xe9, 0x43, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc452e8), std::array<unsigned char, 5>{0xe9, 0xe0, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc459c6), std::array<unsigned char, 5>{0xe9, 0x74, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc46024), std::array<unsigned char, 5>{0xe9, 0x32, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc46574), std::array<unsigned char, 5>{0xe9, 0xf9, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc46d4e), std::array<unsigned char, 5>{0xe9, 0x9, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc4738a), std::array<unsigned char, 5>{0xe9, 0x43, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc47a64), std::array<unsigned char, 5>{0xe9, 0x62, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc48054), std::array<unsigned char, 5>{0xe9, 0x1a, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc484de), std::array<unsigned char, 5>{0xe9, 0xd0, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc48bfc), std::array<unsigned char, 5>{0xe9, 0x85, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc49482), std::array<unsigned char, 5>{0xe9, 0xae, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc499be), std::array<unsigned char, 5>{0xe9, 0xd9, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc49fa0), std::array<unsigned char, 5>{0xe9, 0x33, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc4a302), std::array<unsigned char, 5>{0xe9, 0x8c, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc4a5bf), std::array<unsigned char, 5>{0xe9, 0x8d, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc4ac6c), std::array<unsigned char, 5>{0xe9, 0x85, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc4b3ea), std::array<unsigned char, 5>{0xe9, 0x73, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc4bc7c), std::array<unsigned char, 5>{0xe9, 0xbe, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc4c2ce), std::array<unsigned char, 5>{0xe9, 0x21, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc4c8aa), std::array<unsigned char, 5>{0xe9, 0x2b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc4cfca), std::array<unsigned char, 5>{0xe9, 0x73, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc4d3b8), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc4db36), std::array<unsigned char, 5>{0xe9, 0xad, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc4e1f4), std::array<unsigned char, 5>{0xe9, 0x1a, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc4e8c0), std::array<unsigned char, 5>{0xe9, 0x63, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc4eeda), std::array<unsigned char, 5>{0xe9, 0x2b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc53868), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc5401a), std::array<unsigned char, 5>{0xe9, 0x8b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc549c6), std::array<unsigned char, 5>{0xe9, 0x7d, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc551e0), std::array<unsigned char, 5>{0xe9, 0x9c, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc5595a), std::array<unsigned char, 5>{0xe9, 0x73, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc56196), std::array<unsigned char, 5>{0xe9, 0xad, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc569e0), std::array<unsigned char, 5>{0xe9, 0x9c, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc56dec), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc5752c), std::array<unsigned char, 5>{0xe9, 0x9d, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc57c40), std::array<unsigned char, 5>{0xe9, 0x54, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc5800c), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc58584), std::array<unsigned char, 5>{0xe9, 0x32, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc5892c), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc590a6), std::array<unsigned char, 5>{0xe9, 0xad, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc59528), std::array<unsigned char, 5>{0xe9, 0xb0, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc59b5a), std::array<unsigned char, 5>{0xe9, 0x5b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc59fe2), std::array<unsigned char, 5>{0xe9, 0xcf, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc5a478), std::array<unsigned char, 5>{0xe9, 0xf1, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc5ac96), std::array<unsigned char, 5>{0xe9, 0xad, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc5b5f1), std::array<unsigned char, 5>{0xe9, 0x43, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc5bc44), std::array<unsigned char, 5>{0xe9, 0x2, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc5c20e), std::array<unsigned char, 5>{0xe9, 0x9, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc5c8da), std::array<unsigned char, 5>{0xe9, 0x43, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc5cd90), std::array<unsigned char, 5>{0xe9, 0xb7, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc5d4f6), std::array<unsigned char, 5>{0xe9, 0x7d, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc5d9fe), std::array<unsigned char, 5>{0xe9, 0xd9, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc5ddc2), std::array<unsigned char, 5>{0xe9, 0xb7, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc5e1d8), std::array<unsigned char, 5>{0xe9, 0xd7, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc5e72a), std::array<unsigned char, 5>{0xe9, 0x18, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc5ee10), std::array<unsigned char, 5>{0xe9, 0x4b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc5f1a2), std::array<unsigned char, 5>{0xe9, 0x88, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc5f70c), std::array<unsigned char, 5>{0xe9, 0x32, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc5fcb8), std::array<unsigned char, 5>{0xe9, 0x9, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc6031a), std::array<unsigned char, 5>{0xe9, 0x43, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc60aa6), std::array<unsigned char, 5>{0xe9, 0x95, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc6115a), std::array<unsigned char, 5>{0xe9, 0x43, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc61840), std::array<unsigned char, 5>{0xe9, 0x6c, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc61e04), std::array<unsigned char, 5>{0xe9, 0x1a, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc6242a), std::array<unsigned char, 5>{0xe9, 0x43, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc62832), std::array<unsigned char, 5>{0xe9, 0xb7, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc62dfa), std::array<unsigned char, 5>{0xe9, 0x43, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc63464), std::array<unsigned char, 5>{0xe9, 0x41, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc639de), std::array<unsigned char, 5>{0xe9, 0x9, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc6425c), std::array<unsigned char, 5>{0xe9, 0x85, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc6475a), std::array<unsigned char, 5>{0xe9, 0xd1, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc756e0), std::array<unsigned char, 5>{0xe9, 0x85, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc84734), std::array<unsigned char, 5>{0xe9, 0x32, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc84d4e), std::array<unsigned char, 5>{0xe9, 0xf1, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc85168), std::array<unsigned char, 5>{0xe9, 0xc8, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc866da), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc9a7d8), std::array<unsigned char, 5>{0xe9, 0xb0, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc9bb68), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc9be60), std::array<unsigned char, 5>{0xe9, 0x87, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc9e446), std::array<unsigned char, 5>{0xe9, 0xe7, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xc9ea30), std::array<unsigned char, 5>{0xe9, 0x85, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xca9de4), std::array<unsigned char, 5>{0xe9, 0x1a, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcaa192), std::array<unsigned char, 5>{0xe9, 0xa8, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcaebe0), std::array<unsigned char, 5>{0xe9, 0x87, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcaee60), std::array<unsigned char, 5>{0xe9, 0x85, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcb09e4), std::array<unsigned char, 5>{0xe9, 0x1a, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcb425a), std::array<unsigned char, 5>{0xe9, 0x8b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcbe82a), std::array<unsigned char, 5>{0xe9, 0x8b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcc45b0), std::array<unsigned char, 5>{0xe9, 0xde, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcc4b13), std::array<unsigned char, 5>{0xe9, 0xa, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcc4f5b), std::array<unsigned char, 5>{0xe9, 0xb8, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcc54a8), std::array<unsigned char, 5>{0xe9, 0x9, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcc5df8), std::array<unsigned char, 5>{0xe9, 0x95, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcc6684), std::array<unsigned char, 5>{0xe9, 0x6c, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcc6c04), std::array<unsigned char, 5>{0xe9, 0xb0, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcc6fc6), std::array<unsigned char, 5>{0xe9, 0xa7, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcc7a88), std::array<unsigned char, 5>{0xe9, 0xad, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcde0b0), std::array<unsigned char, 5>{0xe9, 0x87, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcde351), std::array<unsigned char, 5>{0xe9, 0x87, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcde600), std::array<unsigned char, 5>{0xe9, 0x87, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcde880), std::array<unsigned char, 5>{0xe9, 0x87, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcdeb31), std::array<unsigned char, 5>{0xe9, 0x85, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xceca38), std::array<unsigned char, 5>{0xe9, 0xbf, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xced1ce), std::array<unsigned char, 5>{0xe9, 0x69, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xced77e), std::array<unsigned char, 5>{0xe9, 0x9, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcedbe9), std::array<unsigned char, 5>{0xe9, 0xcc, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcee294), std::array<unsigned char, 5>{0xe9, 0x32, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcee778), std::array<unsigned char, 5>{0xe9, 0xb0, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcf11ca), std::array<unsigned char, 5>{0xe9, 0x8b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcf1b9a), std::array<unsigned char, 5>{0xe9, 0x8b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcf3b39), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcf4238), std::array<unsigned char, 5>{0xe9, 0xc8, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcf471e), std::array<unsigned char, 5>{0xe9, 0x9, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcf4d2a), std::array<unsigned char, 5>{0xe9, 0x43, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcf53e4), std::array<unsigned char, 5>{0xe9, 0x54, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcf5caa), std::array<unsigned char, 5>{0xe9, 0x67, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcf6689), std::array<unsigned char, 5>{0xe9, 0xd9, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcf6ab4), std::array<unsigned char, 5>{0xe9, 0xb0, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcf6df0), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcf74ce), std::array<unsigned char, 5>{0xe9, 0x85, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcf79c9), std::array<unsigned char, 5>{0xe9, 0xd9, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcf8110), std::array<unsigned char, 5>{0xe9, 0x84, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcf8598), std::array<unsigned char, 5>{0xe9, 0xb0, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcf89b4), std::array<unsigned char, 5>{0xe9, 0xb0, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcf909a), std::array<unsigned char, 5>{0xe9, 0x44, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcf9fca), std::array<unsigned char, 5>{0xe9, 0x8b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcfb052), std::array<unsigned char, 5>{0xe9, 0xae, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcfb54a), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcfb88d), std::array<unsigned char, 5>{0xe9, 0xab, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcfc086), std::array<unsigned char, 5>{0xe9, 0x95, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcfc4b0), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcfcc56), std::array<unsigned char, 5>{0xe9, 0x95, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcfd2d6), std::array<unsigned char, 5>{0xe9, 0x34, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xcfdc2c), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xd19de8), std::array<unsigned char, 5>{0xe9, 0x10, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xd1a14a), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xd4dc0d), std::array<unsigned char, 5>{0xe9, 0x84, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xd4dea4), std::array<unsigned char, 5>{0xe9, 0x84, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xd4e164), std::array<unsigned char, 5>{0xe9, 0x84, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xd4e444), std::array<unsigned char, 5>{0xe9, 0x84, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xd4e6d4), std::array<unsigned char, 5>{0xe9, 0x84, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xd4e964), std::array<unsigned char, 5>{0xe9, 0x84, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xd59f1c), std::array<unsigned char, 5>{0xe9, 0x8d, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xd5aa4a), std::array<unsigned char, 5>{0xe9, 0xa1, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xd5b276), std::array<unsigned char, 5>{0xe9, 0x8c, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xd5b7ae), std::array<unsigned char, 5>{0xe9, 0xd9, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xf0e586), std::array<unsigned char, 5>{0xe9, 0xbc, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xf0ebea), std::array<unsigned char, 5>{0xe9, 0x22, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xf0ef68), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0xf1b85e), std::array<unsigned char, 5>{0xe9, 0xad, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x1019bba), std::array<unsigned char, 5>{0xe9, 0x8b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x101a7fa), std::array<unsigned char, 5>{0xe9, 0x8b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x101cd9a), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x101d774), std::array<unsigned char, 5>{0xe9, 0x32, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x101dc68), std::array<unsigned char, 5>{0xe9, 0xc8, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x101e2fa), std::array<unsigned char, 5>{0xe9, 0x5b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x101e8ed), std::array<unsigned char, 5>{0xe9, 0x2, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x101f24c), std::array<unsigned char, 5>{0xe9, 0xbe, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x101f78e), std::array<unsigned char, 5>{0xe9, 0xd9, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x101fa8c), std::array<unsigned char, 5>{0xe9, 0x8d, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x10237c0), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x1023d3a), std::array<unsigned char, 5>{0xe9, 0x2b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x102447a), std::array<unsigned char, 5>{0xe9, 0x5c, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x1024b30), std::array<unsigned char, 5>{0xe9, 0x43, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x10250d4), std::array<unsigned char, 5>{0xe9, 0xf9, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x1025598), std::array<unsigned char, 5>{0xe9, 0xf1, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x102597e), std::array<unsigned char, 5>{0xe9, 0xa0, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x1025dae), std::array<unsigned char, 5>{0xe9, 0xe2, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x1026642), std::array<unsigned char, 5>{0xe9, 0xcf, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x1026c1c), std::array<unsigned char, 5>{0xe9, 0x2, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x1027044), std::array<unsigned char, 5>{0xe9, 0xb0, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x10275ea), std::array<unsigned char, 5>{0xe9, 0x22, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x102e7a0), std::array<unsigned char, 5>{0xe9, 0x2b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x102ee28), std::array<unsigned char, 5>{0xe9, 0x9, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x103c92a), std::array<unsigned char, 5>{0xe9, 0x8b, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x103d0f7), std::array<unsigned char, 5>{0xe9, 0x34, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x103da1e), std::array<unsigned char, 5>{0xe9, 0x9d, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x103e0e6), std::array<unsigned char, 5>{0xe9, 0x34, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x103e4e8), std::array<unsigned char, 5>{0xe9, 0x9f, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x103e948), std::array<unsigned char, 5>{0xe9, 0xd9, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x103f2b9), std::array<unsigned char, 5>{0xe9, 0xcb, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x1040388), std::array<unsigned char, 5>{0xe9, 0xb0, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x1041bc8), std::array<unsigned char, 5>{0xe9, 0x18, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x104252c), std::array<unsigned char, 5>{0xe9, 0xa6, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x1042cda), std::array<unsigned char, 5>{0xe9, 0x45, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x10433b4), std::array<unsigned char, 5>{0xe9, 0xf8, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x1043780), std::array<unsigned char, 5>{0xe9, 0x8d, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x10527ae), std::array<unsigned char, 5>{0xe9, 0xb8, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x1077458), std::array<unsigned char, 5>{0xe9, 0xb0, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x10811ac), std::array<unsigned char, 5>{0xe9, 0xa6, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x1081b35), std::array<unsigned char, 5>{0xe9, 0xd1, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x10824c8), std::array<unsigned char, 5>{0xe9, 0x9, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x1082a54), std::array<unsigned char, 5>{0xe9, 0xe0, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x10838a1), std::array<unsigned char, 5>{0xe9, 0xfe, 0x1, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x1083fa6), std::array<unsigned char, 5>{0xe9, 0x90, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x1084684), std::array<unsigned char, 5>{0xe9, 0xc8, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x1084dee), std::array<unsigned char, 5>{0xe9, 0xf1, 0x0, 0x0, 0x0}.data(), 0x5);
    memcpy((void*)(globals::gameBase + 0x108516a), std::array<unsigned char, 5>{0xe9, 0xa0, 0x0, 0x0, 0x0}.data(), 0x5);
    LOG_CORE(Debug, "patched retaddr checks\n"); //i think those are like 335/342?
#pragma endregion

    memset((void*)(globals::gameBase + 0x805607), 0x85, 1);
    LOG_CORE(Debug, "patched veh set trap\n");

    LOG_CORE(Debug, "casc log hook\n");
    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xba00e0), (LPVOID)cascLogHook, (LPVOID*)&cascLogHook_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xba00e0)));

    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xf4dff0), (LPVOID)statescript_load_hook, (LPVOID*)&statescript_load_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xf4dff0)));

    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xb2ae70), (LPVOID)stuux_string_hash_hook, (LPVOID*)&stuux_string_hash_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xb2ae70)));
    LOG_CORE(Debug, "stuux string hash uxlink\n");

    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0x9c3500), (LPVOID)manager_14_init_hook, (LPVOID*)&manager_14_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0x9c3500)));
    LOG_CORE(Debug, "Manager 14 initialization hook\n");

    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xcd0590), (LPVOID)dataflow_bugfix_fn, (LPVOID*)&dataflow_bugfix_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xcd0590)));
    LOG_CORE(Debug, "filterbits dataflow fix\n");

    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xc541c0), (LPVOID)Construct_GameEntityAdmin_fn, (LPVOID*)&Construct_GameEntityAdmin_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xc541c0)));
    LOG_CORE(Debug, "gameEA initialize hook\n");

    unsigned char ret_1[] = { 0xB0, 0x01, 0xC3 };
    memcpy((void*)(globals::gameBase + 0xf4c920), ret_1, sizeof(ret_1)); //Debug Statescript log enabled
    memcpy((void*)(globals::gameBase + 0xcfd740), ret_1, sizeof(ret_1)); //Force Enable all heroes
    memset((void*)(globals::gameBase + 0x7e1f69), 0x90, 8);
    memset((void*)(globals::gameBase + 0xf44bba), 0x90, 0x13);

    // //Return address checks from getHealth func, should be handled by Anti-Anti-Debug now.
    // memset((void*)(globals::gameBase + 0xcc6110), 0x90, 0xcc67f5 - 0xcc6110);
    // memset((void*)(globals::gameBase + 0xcc57d3), 0x90, 0xcc5f92 - 0xcc57d3);

    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xc43a40), (LPVOID)selectiveResLoad_hook, (PVOID*)&selectiveResLoad_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xc43a40)));
    LOG_CORE(Debug, "selectiveResLoad_hook\n");

    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xc43ab0), (LPVOID)System63_hook, (PVOID*)&System63_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xc43ab0)));
    LOG_CORE(Debug,"system63 bugfix\n");

    //0x7fc6b0
    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0x7fc6b0), (LPVOID)CreateAllocator_hook, (PVOID*)&CreateAllocator_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0x7fc6b0)));
    LOG_CORE(Debug, "memallocator hook\n");

    ((void(*)(DWORD_PTR, int, int))(globals::gameBase + TlsCallback2_Addr))(globals::gameBase, 3, 0);
    memset((void*)(globals::gameBase + TlsCallback_Addr), 0xc3, 0x10);
    memset((void*)(globals::gameBase + TlsCallback2_Addr), 0xc3, 0x10);
    LOG_CORE(Debug, "patched tls callbacks\n");

    LOG_CORE(Debug, "Calling static prelaunch window initializers\n");
    window_manager::call_preStartInitialize();

    LOG_CORE(Debug, "Loading hashlib\n");
    stringhash_library::initialize();
    resource_handler_helper::initialize();
    stu_resources::initialize();
    displaytext_resources::initialize();
    Component_54_Lobbymap::initialize();

    //force all retaddr checks to fail to check if any remain
    //const auto pe = Pe::PeNative::fromModule(GetModuleHandleA(NULL));
    //WORD* retaddr_bypasser_9000 = (WORD*)&pe.headers().nt()->FileHeader.NumberOfSections;
    //DWORD old;
    //VirtualProtect(retaddr_bypasser_9000, 2, PAGE_READWRITE, &old);
    //*retaddr_bypasser_9000 = 0;

    LOG_CORE(Info, "Prepared to launch {:x}.\n", globals::gameBase + Start_Addr);

    HANDLE mainThread = OpenThread(THREAD_ALL_ACCESS, false, mainThreadId);
    ResumeThread(mainThread);
    CloseHandle(mainThread);
}

//Initial entry for the executable.
//Making hooks / calling stuff here isnt really a good idea, since this is called before TlsCallback_0 which is very restrictive.
//Place your hooks after .text decryption
BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    static std::once_flag entrypoint_mutex;
    static std::once_flag exitpoint_mutex;
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            std::call_once(entrypoint_mutex, [] {
                StartHook();
            });
            break;
        case DLL_PROCESS_DETACH:
            std::call_once(exitpoint_mutex, [] {
                LOG_CORE(Info, "Goodbye World (unloading DLL)!\n");
                MH_VERIFY(MH_Uninitialize());
                Logs::Uninitialize();
                Atlas::Utility::Modules::Uninitialize();
            });
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        default:
            break;
    }


    return TRUE;
}


