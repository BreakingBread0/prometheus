//
// Created by cereal on 18.04.26.
//

#include <cstdio>
#include <direct.h>
#include <imgui.h>
#include <iostream>
#include <windows.h>
#include <nfd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <nlohmann/json.hpp>
#include "simple_imgui.h"
#include "signature.h"


PROCESS_INFORMATION start_process_suspended(const std::string& process)
{
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = {};

    BOOL success = CreateProcessA(
        process.c_str(),
        nullptr,           // command line
        nullptr,           // process security attrs
        nullptr,           // thread security attrs
        FALSE,             // inherit handles
        CREATE_SUSPENDED,  // <-- suspend before entry point
        nullptr,           // environment
        nullptr,           // current directory
        &si,
        &pi
    );

    if (!success) {
        printf("CreateProcess failed: %x\n", GetLastError());
        return {};
    }

    printf("Process PID: %x\n", pi.dwProcessId);
    printf("Main thread ID: %x\n", pi.dwThreadId);

    return pi;
}

bool InjectDLL(HANDLE hProcess, const std::string& dllPath) {
    SIZE_T pathLen = dllPath.size() + 1;
    LPVOID remoteMem = VirtualAllocEx(
        hProcess,
        nullptr,
        pathLen,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    );
    if (!remoteMem) {
        printf("VirtualAllocEx failed: %x\n", GetLastError());
        return false;
    }

    if (!WriteProcessMemory(hProcess, remoteMem, dllPath.c_str(), pathLen, nullptr)) {
        printf("WriteProcessMemory failed: %x\n", GetLastError());
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        return false;
    }

    LPVOID loadLibAddr = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (!loadLibAddr) {
        printf("GetProcAddress failed: %x\n", GetLastError());
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        return false;
    }

    HANDLE hThread = CreateRemoteThread(
        hProcess,
        nullptr,
        0,
        (LPTHREAD_START_ROUTINE)loadLibAddr,
        remoteMem,
        0,
        nullptr
    );
    if (!hThread) {
        printf("CreateRemoteThread failed: %x\n", GetLastError());
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeThread(hThread, &exitCode);
    if (exitCode == 0)
        printf("LoadLibrary in remote process returned NULL: %x\n", GetLastError());

    CloseHandle(hThread);
    VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
    return exitCode != 0;
}

//https://stackoverflow.com/questions/2831841/how-to-get-the-time-in-milliseconds-in-c
__int64 time_in_ms()
{
    namespace sc = std::chrono;

    auto time = sc::system_clock::now(); // get the current time

    auto since_epoch = time.time_since_epoch(); // get the duration since epoch

    // I don't know what system_clock returns
    // I think it's uint64_t nanoseconds since epoch
    // Either way this duration_cast will do the right thing
    auto millis = sc::duration_cast<sc::milliseconds>(since_epoch);

    return millis.count(); // just like java (new Date()).getTime();
}

std::string open_file_dialog() {
    NFD_Init();

    nfdu8char_t *outPath;
    nfdu8filteritem_t filters[1] = { { "Overwatch (GameClientApp.exe)", "exe" }};
    nfdopendialogu8args_t args = {0};
    args.filterList = filters;
    args.filterCount = 1;

    char fileName[256];
    _getcwd(fileName, sizeof(fileName));

    args.defaultPath = fileName;
    nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
    if (result == NFD_OKAY)
    {
        std::string result = outPath;
        NFD_FreePathU8(outPath);
        NFD_Quit();
        return result;
    }

    NFD_Quit();
    return "";
}

struct launch_config
{
    std::string game_location = "";
    std::string working_dir = "";
    std::string extra_args = "";

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(launch_config, game_location, working_dir, extra_args);
} s_launch_config{};

const char* launch_config_name = "launch_config.json";
const char* overwatch_beta_md5 = "fe3ad8a77eef77b383df4929aed816fd";

//Context
std::string _current_file_signature = "";

static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 50);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void InputText(const std::string& label, std::string& inout)
{
    char buf[256];
    strcpy_s(buf, inout.c_str());
    if (ImGui::InputText(label.c_str(), buf, sizeof(buf)))
    {
        inout = buf;
    }
}

void open_link(const std::string& str)
{
    ShellExecute(0, 0, str.c_str(), 0, 0 , SW_SHOW );
}

int main()
{
    printf("Hello, World!\n");

    if (!simple_imgui::imgui_init())
    {
        printf("Failed to initialize imgui!");
        return 1;
    }

    while (simple_imgui::imgui_loop())
    {
        bool open = true;
        ImGui::SetNextWindowSize(ImVec2(800, 400));
        if (ImGui::Begin("Prometheus Launcher", &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
        {
            ImGui::Text("Welcome to the Prometheus Launcher.");

            ImGui::Text("Game Executable");
            ImGui::SameLine();
            HelpMarker("Select the GameClientApp.exe to execute.\n\nUnfortunately, you need to find the Overwatch 0.8 game files yourself, although this launcher ensures that the file you have downloaded is legitemate.");
            ImGui::SameLine();
            InputText("##executable", s_launch_config.game_location);
            ImGui::SameLine();
            if (ImGui::Button("Select..."))
            {
                auto str = open_file_dialog();;
                if (!str.empty())
                    s_launch_config.game_location = str;
                _current_file_signature = "";
            }
            if (!s_launch_config.game_location.empty())
            {
                if (_current_file_signature.empty())
                {
                    _current_file_signature = md5_file(s_launch_config.game_location);
                    printf("MD5 of file '%s': %s\n", s_launch_config.game_location.c_str(), _current_file_signature.c_str());
                } else if (_current_file_signature != std::string(overwatch_beta_md5))
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
                    ImGui::Text("Failed to verify game client signature.");
                    ImGui::Text("Got: %s", _current_file_signature.c_str());
                    ImGui::Text("Expected: %s", overwatch_beta_md5);
                    ImGui::SameLine();
                    HelpMarker("The file you selected is not GameClientApp.exe.\n\nThis may mean that the game executable was modified. Ensure that you have a legitemate game copy of Beta 0.8(24919).\n\nFor older users: Please select the unpatched executable, as the patch is not necessary anymore.");
                    ImGui::PopStyleColor();
                } else
                {
                    ImGui::TextDisabled("Successfully verified the game executable's integrity.");
                }
            }

            if (ImGui::Button("Trust?"))
            {
                printf("MD5: %s\n", md5_file(s_launch_config.game_location).c_str());
            }

            char buf[256];
            GetCurrentDirectoryA(sizeof(buf), buf);
            std::filesystem::path dll_path(buf);
            dll_path.append("PrometheusCore.dll");
            ImGui::TextUnformatted(dll_path.string().c_str());
            if (ImGui::Button("Test Launch"))
            {
                auto process = start_process_suspended(s_launch_config.game_location);
                if (!process.hProcess)
                {
                    printf("Failed to launch process");
                    continue;
                }
                bool dll = InjectDLL(process.hProcess, dll_path.string());
                printf("Launch result: %d", dll);

            }
        }
        if (!open)
            break;
        ImGui::End();

        simple_imgui::imgui_render();
    }
    simple_imgui::imgui_cleanup();
    return 0;
}
