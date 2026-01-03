// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <stdio.h>
#include <MinHook.h>
#include <string>
#define LAZY_IMPORTER_CASE_INSENSITIVE
#include "lazy_importer.h"
#include <mutex>
#include <TlHelp32.h>
#include <thread>
#include <string>
#include <locale>
#include <codecvt>
#include "ExceptionFormatter.h"
#include <winternl.h>
#include <algorithm>
#include <vector>
#include <set>
#include <keystone/keystone.h>
#include <keystone/x86.h>
#include <capstone/x86.h>
#include <capstone/capstone.h>
#include <string>
#include <format>
#include <thread>
#include <nlohmann/json.hpp>
#include <d3d11.h>
#include "pe.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_freetype.h"
#include "idadefs.h"
#include "window_manager/window_manager.h"
#include "window_manager/management_window.h"
#include "globals.h"
#include "JAM.h"
#include "stringhash_library.h"
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "Crypt32.lib")
#include "Statescript.h"
#include "game.h"
#include "ResourceManager.h"
#include "STU.h"
#include "statescript_logger.h" 
#include <imnodes/imnodes.h>
#include "stu_resources.h"
#include "entity_admin.h"
#include <array>
#include "patches.h"

//Who needs a build system?
#include <imnodes/imnodes.cpp>

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
        printf("failed ow_dealloc for %p\n", address);
    }
}

//_insn decodeInsn(DWORD_PTR where);
//void patchJump(DWORD_PTR location, DWORD_PTR toAddr);
//void patchToJump(DWORD_PTR location);

const DWORD_PTR TlsCallback_Addr = 0x137f000;
const DWORD_PTR Start_Addr = 0x11fe3ac;
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

unsigned char TlsCallback_0[] =
{
  0x4C, 0x8B, 0xDC, 0x49, 0x89, 0x4B, 0x08, 0x55, 0x49, 0x8D,
  0xAB, 0xB8, 0xF9, 0xFF, 0xFF, 0x48, 0x81, 0xEC, 0x40, 0x07,
  0x00, 0x00, 0x53, 0x81, 0xCB, 0xF3, 0xA4, 0x27, 0x5A, 0x5B,
  0x71, 0x02, 0x49, 0xBE, 0x83, 0xFA, 0x01, 0x0F, 0x85, 0x92,
  0x20, 0x00, 0x00, 0x80, 0x3D, 0x0E, 0x5D, 0x49, 0x00, 0x00,
  0x0F, 0x85, 0x85, 0x20, 0x00, 0x00, 0x88, 0x15, 0x02, 0x5D,
  0x49, 0x00, 0x52, 0x81, 0xCA, 0x27, 0xB7, 0x61, 0x62, 0x5A,
  0x71, 0x02, 0x48, 0xBD, 0xC7, 0x85, 0x58, 0x06, 0x00, 0x00,
  0x60, 0x00, 0x00, 0x00, 0x8B, 0x85, 0x58, 0x06, 0x00, 0x00,
  0x65, 0x48, 0x8B, 0x00, 0x48, 0x85, 0xC0, 0x0F, 0x84, 0x56,
  0x20, 0x00, 0x00, 0x48, 0x8B, 0x40, 0x18, 0x48, 0x8B, 0x48,
  0x20, 0x48, 0x83, 0xC0, 0x20, 0x48, 0x3B, 0xC8, 0x0F, 0x84,
  0x41, 0x20, 0x00, 0x00, 0x4D, 0x89, 0x7B, 0xC8, 0x4C, 0x8D,
  0x79, 0xF0, 0x4D, 0x85, 0xFF, 0x0F, 0x84, 0x28, 0x20, 0x00,
  0x00, 0x52, 0x81, 0xE2, 0x29, 0x44, 0xB9, 0xB6, 0x5A, 0x73,
  0x02, 0x48, 0xBB, 0xC7, 0x45, 0x38, 0x6B, 0x65, 0x72, 0x6E,
  0xC7, 0x45, 0x3C, 0x65, 0x6C, 0x33, 0x32, 0xC7, 0x45, 0x40,
  0x2E, 0x64, 0x6C, 0x6C, 0x50, 0x81, 0xC8, 0xD7, 0x3A, 0xA9,
  0x6A, 0x58, 0x73, 0x02, 0x48, 0xBA, 0xC7, 0x85, 0x58, 0x06,
  0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x8B, 0x85, 0x58, 0x06,
  0x00, 0x00, 0x65, 0x48, 0x8B, 0x08, 0x48, 0x85, 0xC9, 0x0F,
  0x84, 0xDE, 0x1F, 0x00, 0x00, 0x49, 0x89, 0x5B, 0x18, 0x48,
  0x8B, 0x59, 0x18, 0x4C, 0x8B, 0x53, 0x20, 0x48, 0x83, 0xC3,
  0x20, 0x4C, 0x3B, 0xD3, 0x0F, 0x84, 0xBD, 0x1F, 0x00, 0x00,
  0x49, 0x89, 0x73, 0xF0, 0x49, 0x89, 0x7B, 0xE8, 0x4D, 0x89,
  0x6B, 0xD8, 0x45, 0x33, 0xED, 0x90, 0x45, 0x0F, 0xB7, 0x4A,
  0x38, 0x4D, 0x8B, 0x42, 0x40, 0x4D, 0x8D, 0x5A, 0xF0, 0x49,
  0x8B, 0xFD, 0x41, 0x8B, 0xF5, 0x45, 0x85, 0xC9, 0x0F, 0x8E,
  0xCB, 0x01, 0x00, 0x00, 0x0F, 0x1F, 0x40, 0x00, 0x41, 0x0F,
  0xB6, 0x08, 0x8D, 0x41, 0xBF, 0x3C, 0x19, 0x77, 0x03, 0x80,
  0xC1, 0x20, 0x0F, 0xB6, 0x54, 0x3D, 0x38, 0x8D, 0x42, 0xBF,
  0x3C, 0x19, 0x77, 0x03, 0x80, 0xC2, 0x20, 0x3A, 0xCA, 0x0F,
  0x85, 0x8F, 0x01, 0x00, 0x00, 0x48, 0xFF, 0xC7, 0x48, 0x83,
  0xFF, 0x0C, 0x0F, 0x85, 0x85, 0x01, 0x00, 0x00, 0x4D, 0x85,
  0xDB, 0x0F, 0x84, 0x3A, 0x1F, 0x00, 0x00, 0x4C, 0x89, 0xA4,
  0x24, 0x28, 0x07
};

DWORD GetMainThreadId() {
    auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        printf("Could not open handle to get main thread\n");
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
        }
    }
    CloseHandle(snapshot);
    return result;
}

void exit_handler() {
    printf("=============== shutdown ===============\n");
    if (!globals::exit_normal)
        system("pause");
}

void __fastcall ExitProcessHook(int exitCode) {
    printf("Process exit initiazed: %d\n", exitCode);
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
        printf("GetClipboard succeessful\n");
        char* str = new char[strlen(out.get())];
        strcpy(str, out.get());
        return str;
    }
    else
        printf("GetClipboard failed\n");
    return new char[0];
}

IDXGISwapChain* main_swapchain = nullptr;
bool ui_invisible = false;
HRESULT __stdcall PresentHook(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    if (!main_swapchain)
        main_swapchain = pSwapChain;
    if (main_swapchain == pSwapChain) {
        //fuck fullscreen, all my homies hate fullscreen in 0.8
        *(int*)(globals::gameBase + 0x17a1123) = 0;
        std::call_once(imgui_init, [&]() {
            globals::gameWindow = (DWORD_PTR)FindWindow(L"tankWindowClass", NULL);
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

            imgui_helpers::BoldFont = io.Fonts->AddFontFromFileTTF("MonaspaceXenonNF-ExtraBold.otf", 13);
            io.Fonts->AddFontFromFileTTF("MonaspaceXenonNF-Regular.otf", 13/*, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesDefault()*/);
            io.FontDefault = ImGui::GetIO().Fonts->AddFontFromFileTTF("Font Awesome 6 Free-Solid-900.otf", 13, &cfg, ranges);

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

            printf("Filling STU vfptr table\n");
            STURegistryData::initialize();
            statescript_logger::Initialize();
            window_manager::add_window(new management_window);
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
    printf("window handle: %x\n", handle);
    globals::gameWindow = handle;
    std::thread([]() {
        while (true) {
            Sleep(100);
            //teEngine ínstance
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
            MH_VERIFY(MH_CreateHook((DWORD_PTR*)render, PresentHook, reinterpret_cast<void**>(&phookD3D11Present)));
            MH_VERIFY(MH_EnableHook((DWORD_PTR*)render));
            break;
        }
        }).detach();
    return handle;
}

typedef __int64(__cdecl* regedit_fn)(JamType13String*, JamType13String*, __int64, __int64);
regedit_fn regedit_initialize_orig;
__int64 regedit_initialize(JamType13String* a1, JamType13String* a2, __int64 a3, __int64 a4) {
    auto str = std::string(readJamType13String(a2));
    //printf("regedit: %s\n", str.c_str());
    if (str == "REGION" || str == "LOCALE" || str == "LOCALE_AUDIO") {
        printf("patch regedit get-region.\n");
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
    printf("CASC %d (%s): %s\n", level, culprit, msg);
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
            allmighty_hash_lib::add_comment(hash, input, true);
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
                printf("Emplacing TACT key %llx\n", key.KeyName);
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
    if (loader->stu_id == 0x400000000000433 || //bigass boat on Gibraltar
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
    //    if (allmighty_hash_lib::components.find(entry->component_id) != allmighty_hash_lib::components.end())
    //        printf("- Component %s (%x) (data %p %p %p )\n", allmighty_hash_lib::components[entry->component_id].name.c_str(), entry->component_id, entry->a1, entry->a2, entry->a3);
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
        printf("Hook AfterDataPack\n");
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

        char ret_1[] = { 0xB0, 0x01, 0xC3 };
        memcpy((void*)(globals::gameBase + 0xc7c960), ret_1, sizeof(ret_1));
        printf("Force chat enabled\n"); //Must be done after data packs loaded
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

bool ea_firstCall = true;
EntityAdminBase* (*Construct_GameEntityAdmin_orig)(EntityAdminBase* ea, EntityAdminCreationInfo* ci);
EntityAdminBase* Construct_GameEntityAdmin_fn(EntityAdminBase* ea, EntityAdminCreationInfo* ci) {
    ea = Construct_GameEntityAdmin_orig(ea, ci);

    //auto emplace_fn = (void(*)(EntityAdminBase*, mapfunc_outer_vt**, __int64 inheritance))(globals::gameBase + 0x80f450);
    //emplace_fn(ea, (mapfunc_outer_vt**)PrometheusSystem::create(ea), globals::gameBase + 0x80f450);

    if (ea_firstCall) {
        ea_firstCall = false;
        return ea;
    }

    auto sys = PrometheusSystem::create(ea);
    if (sys) //The second call is for GameEntityAdmin
        ea->systems_array.emplace_item((system_vt**)sys);
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

void __cdecl StartHook(void*) {
    //system("pause");
    printf("hello monsieur\n");
    atexit(exit_handler);

    char orig[] = { 0x7E, 0x41, 0xDB, 0xB6, 0x8F, 0x68, 0x93, 0x42, 0x09, 0xC8, 0x5F, 0x4A };
    memcpy((void*)(globals::gameBase + Start_Addr), orig, sizeof(orig));
    printf("restored game start.\n");

    typedef int(__stdcall* _TlsCallbackFunc)(DWORD_PTR, DWORD, PVOID);
    ((_TlsCallbackFunc)(globals::gameBase + 0x137f000))(globals::gameBase, 1, 0);
    printf("called iat decryptor (TlsCallback1)\n");

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
    printf("called text decryptor (veh)\n");

    char verify[] = { 0x48, 0x83, 0xEC, 0x28, 0xE8, 0xFF, 0xD8, 0x00, 0x00 };
    for (int i = 0; i < sizeof(verify); i++) {
        if (*(char*)(globals::gameBase + Start_Addr + i) != verify[i]) {
            printf(".text decryption failed!\n");
            system("pause");
        }
    }

    memset((void*)(globals::gameBase + TlsCallback_Addr), 0xc3, 0x10);
    printf("patched tls callbacks\n");

    memset((void*)(globals::gameBase + 0x1133D81), 0x90, 0x4); //illegal instruction at end of crash_me__.
    printf("patched debugger trap 1 (illegal isntruction)\n");

    //ud2
    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0x5abd00), DebuggerTrapHook, 0));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0x5abd00)));
    //basically same debugger trap here but __debugbreak()
    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0x5abd30), DebuggerTrapHook, 0));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0x5abd30)));
    //ame but ud2 again
    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0x808b90), DebuggerTrapHook, 0));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0x808b90)));
    //ud2
    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0x7e1710), DebuggerTrapHook, 0));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0x7e1710)));
    //crash_me__
    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0x1130960), DebuggerTrapHook, 0));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0x1130960)));

    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0x8fc240), createwindow_hook, (PVOID*)&createwindow_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0x8fc240)));

    char return_zero[] = { 0x31, 0xC0, 0xC3 };
    memcpy((void*)(globals::gameBase + 0x804740), return_zero, sizeof(return_zero));
    printf("patched debugger trap 2 (return value check)\n");

    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0x5ac130), regedit_initialize, (PVOID*)&regedit_initialize_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0x5ac130)));
    printf("patched regedit launch options\n");

    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0x80f7c0), emplace_entity_hook, (PVOID*)&emplace_entity_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0x80f7c0)));
    printf("emplace entity hook\n");

    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xf3eb10), afterdatapackload_hook, (PVOID*)&afterdatapackload_fn));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xf3eb10)));
    printf("after data pack load hook\n");

    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xa00450), componen1_oncreate_hook, (PVOID*)&component_1_oncreate_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xa00450)));

    /*MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xbff460), testVIGSTickHook, (PVOID*)&VigsTickOrig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xbff460)));*/

	patches::PatchDebugTraps();
    printf("patched all antidebug traps\n");

    memset((void*)(globals::gameBase + 0x805607), 0x85, 1);
    printf("patched veh set trap\n");

    printf("casc log hook\n");
    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xba00e0), cascLogHook, (LPVOID*)&cascLogHook_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xba00e0)));

    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xf4dff0), statescript_load_hook, (LPVOID*)&statescript_load_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xf4dff0)));

    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xb2ae70), stuux_string_hash_hook, (LPVOID*)&stuux_string_hash_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xb2ae70)));
    printf("stuux string hash uxlink\n");

    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0x9c3500), manager_14_init_hook, (LPVOID*)&manager_14_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0x9c3500)));
    printf("Manager 14 initialization hook\n");

    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xcd0590), dataflow_bugfix_fn, (LPVOID*)&dataflow_bugfix_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xcd0590)));
    printf("filterbits dataflow fix\n");

    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xc541c0), Construct_GameEntityAdmin_fn, (LPVOID*)&Construct_GameEntityAdmin_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xc541c0)));
    printf("gameEA initialize hook\n");

    char ret_1[] = { 0xB0, 0x01, 0xC3 };
    memcpy((void*)(globals::gameBase + 0xf4c920), ret_1, sizeof(ret_1)); //Debug Statescript log enabled
    memcpy((void*)(globals::gameBase + 0xcfd740), ret_1, sizeof(ret_1)); //Force Enable all heroes
    memset((void*)(globals::gameBase + 0x7e1f69), 0x90, 8);
    memset((void*)(globals::gameBase + 0xf44bba), 0x90, 0x13);

    //Return address checks from getHealth func
    memset((void*)(globals::gameBase + 0xcc6110), 0x90, 0xcc67f5 - 0xcc6110);
    memset((void*)(globals::gameBase + 0xcc57d3), 0x90, 0xcc5f92 - 0xcc57d3);

    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xc43a40), selectiveResLoad_hook, (PVOID*)&selectiveResLoad_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xc43a40)));
    printf("selectiveResLoad_hook\n");

    MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xc43ab0), System63_hook, (PVOID*)&System63_orig));
    MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xc43ab0)));
    printf("system63 bugfix\n"); 

    ((_TlsCallbackFunc)(globals::gameBase + TlsCallback2_Addr))(globals::gameBase, 3, 0);

    printf("Calling static prelaunch window initializers\n");
    window_manager::call_preStartInitialize();

    printf("Loading hashlib\n");
    allmighty_hash_lib::initialize();
    stu_resources::initialize();

    printf("Prepared to launch %p.\n", globals::gameBase + Start_Addr);
    //system("pause");
    void(*pFunc)() = (void(*)())(globals::gameBase + Start_Addr);
    pFunc();
}

__int64 AddVehHook(__int64 old, __int64 func) {
    if ((DWORD_PTR)_ReturnAddress() > globals::gameBase && (DWORD_PTR)_ReturnAddress() < globals::gameBase + globals::gameSize)
        return AddVeh_orig(old, func);
    printf("Caught RtlAddVectoredExceptionHandler %d %p ret: %p\n", old, func, _ReturnAddress());
    return 0;
}

int __stdcall VectoredExceptionHandler(EXCEPTION_POINTERS* exception) {
    if (exception->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT ||
        exception->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP ||
        exception->ExceptionRecord->ExceptionCode == EXCEPTION_GUARD_PAGE)
        return EXCEPTION_CONTINUE_SEARCH;
    if (exception->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
        if (exception->ContextRecord->Rip > (DWORD64)std::addressof(IsBadReadPtr) && exception->ContextRecord->Rip - (DWORD64)std::addressof(IsBadReadPtr) < 0x100)
            return EXCEPTION_CONTINUE_SEARCH;
    }
    if (exception->ExceptionRecord->ExceptionCode == 0x40010006)
        return EXCEPTION_CONTINUE_SEARCH;
    if (exception->ExceptionRecord->ExceptionCode == 0xc0000008)
        return EXCEPTION_CONTINUE_EXECUTION;
    printf("New exception: %d 0x%x\n", GetCurrentThreadId(), GetCurrentThreadId());
    printf("%s\n", ExceptionFormatter::FormatException(exception).c_str());
    return EXCEPTION_CONTINUE_SEARCH;
}

bool CheckDebuggerPresentHook() {
    printf("CheckDebuggerPresent %p\n", _ReturnAddress());
    return false;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    static std::once_flag entrypoint_mutex;
    std::call_once(entrypoint_mutex, [] {
        globals::ensure_console_allocated();
        printf("Hello World!\n");

        printf("Adding VEH: %p\n", AddVectoredExceptionHandler(true, (PVECTORED_EXCEPTION_HANDLER)VectoredExceptionHandler));
        globals::gameBase = (DWORD_PTR)GetModuleHandleA(nullptr);
        const auto pe = Pe::PeNative::fromModule(GetModuleHandleA(NULL));
        globals::gameSize = pe.imageSize();
        printf("Game Base: %p %p\n", globals::gameBase, globals::gameSize);
        mainThreadId = GetCurrentThreadId();
        printf("Main Thread: %d (%x)\n", mainThreadId, mainThreadId);
        printf("Patching back TLSCallback0 at %p!\n");
        DWORD_PTR tlsCallback = TlsCallback_Addr + globals::gameBase;
        DWORD old;
        VirtualProtect((void*)(tlsCallback), sizeof(TlsCallback_0), PAGE_EXECUTE_READWRITE, &old);
        memcpy((void*)tlsCallback, TlsCallback_0, sizeof(TlsCallback_0));

        MH_VERIFY(MH_Initialize());

        MH_VERIFY(MH_CreateHook(ExitProcess, ExitProcessHook, (LPVOID*)&ExitProcess_orig));
        MH_VERIFY(MH_EnableHook(ExitProcess));

        auto addVeh = GetProcAddress(LoadLibraryA("ntdll.dll"), "RtlAddVectoredExceptionHandler");
        MH_VERIFY(MH_CreateHook(addVeh, AddVehHook, (LPVOID*)&AddVeh_orig));
        MH_VERIFY(MH_EnableHook(addVeh));

        MH_VERIFY(MH_CreateHook(IsDebuggerPresent, CheckDebuggerPresentHook, 0));
        MH_VERIFY(MH_EnableHook(IsDebuggerPresent));

        MH_VERIFY(MH_CreateHook(CheckRemoteDebuggerPresent, CheckDebuggerPresentHook, 0));
        MH_VERIFY(MH_EnableHook(CheckRemoteDebuggerPresent));

        printf("Creating WinMain Hook...\n");
        char starthook[] = {
            0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rax,0
            0xFF, 0xE0 //jmp rax
        };
        VirtualProtect((void*)(globals::gameBase + Start_Addr), sizeof(starthook), PAGE_EXECUTE_READWRITE, &old);
        memcpy((void*)(globals::gameBase + Start_Addr), starthook, sizeof(starthook));
        DWORD_PTR thatAddr = (DWORD_PTR)&StartHook;
        memcpy((void*)(globals::gameBase + Start_Addr + 2), (void*)&thatAddr, 8);
        });
    return TRUE;
}


