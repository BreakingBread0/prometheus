//
// Created by cereal on 18.04.26.
//

#include <cstdio>
#include <direct.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>
#include <windows.h>
#include <nfd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fmt/base.h>
#include <nlohmann/json.hpp>
#include "simple_imgui.h"
#include "signature.h"


PROCESS_INFORMATION start_process_suspended(const std::string& process, const std::string& arguments = "", const std::string& working_dir = "")
{
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = {};

    BOOL success = CreateProcessA(
        process.c_str(),
        arguments.empty() ? nullptr : (char*)arguments.c_str(),
        nullptr,
        nullptr,
        FALSE,
        CREATE_SUSPENDED,
        nullptr,
        working_dir.empty() ? std::filesystem::path(process).remove_filename().string().c_str() : working_dir.c_str(),
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

bool inject_dll(HANDLE hProcess, const std::string& dllPath) {
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

    bool launch_console = true;
    bool launch_showmode = false;
    int posX, posY, resX, resY;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(launch_config, game_location, working_dir, extra_args, launch_console, launch_showmode, posX, posY, resX, resY);
} s_launch_config{};

const char* launch_config_filename = "launch_config.json";
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

std::string get_arguments()
{
    std::vector<std::string> options;
    if (s_launch_config.launch_console)
        options.push_back("--console");
    if (s_launch_config.launch_showmode)
        options.push_back("--tank_ShowMode");

    if (s_launch_config.posX > 0)
        options.push_back("--tank_windowPosX=" + std::to_string(s_launch_config.posX));
    if (s_launch_config.posY > 0)
        options.push_back("--tank_windowPosY=" + std::to_string(s_launch_config.posY));

    if (s_launch_config.resX > 0)
        options.push_back("--tank_windowResX=" + std::to_string(s_launch_config.resX));
    if (s_launch_config.resY > 0)
        options.push_back("--tank_windowResY=" + std::to_string(s_launch_config.resY));

    if (!s_launch_config.extra_args.empty())
        options.push_back(s_launch_config.extra_args);

    std::stringstream result{};
    for (auto& opt : options)
        result << opt + " ";
    return result.str();
}

bool launch_game(const std::string& dll_to_load)
{
    auto process = start_process_suspended(s_launch_config.game_location, get_arguments(), s_launch_config.working_dir);
    if (!process.hProcess)
    {
        printf("Failed to launch process");
        return false;
    }
    return inject_dll(process.hProcess, dll_to_load);
}

void save_config()
{
    try
    {
        std::ofstream fstream(launch_config_filename, std::ios::out | std::ios::trunc);
        if (!fstream.is_open()) {
            printf("Failed to open %s for saving config.\n", launch_config_filename);
            return;
        }
        nlohmann::json json = s_launch_config;
        fstream << json.dump(4);
        fstream.flush();
    } catch (nlohmann::json::exception& ex)
    {
        printf("Failed to save config %s: %s\n", launch_config_filename, ex.what());
    }
}

int main()
{
    printf("Hello, World!\n");
    printf("Commit: %s (%s)\n", GIT_HASH, GIT_BRANCH);

    if (!simple_imgui::imgui_init())
    {
        printf("Failed to initialize imgui!\n");
        return 1;
    }

    if (std::filesystem::exists(launch_config_filename))
    {
        printf("Loading saved config.\n");
        try
        {
            std::ifstream file(launch_config_filename);
            if (!file.is_open())
            {
                printf("Failed to load existing %s: File failed to open.\n", launch_config_filename);
            }
            auto json = nlohmann::json::parse(file);
            s_launch_config = json;
        } catch (nlohmann::json::exception& ex)
        {
            printf("Failed to load existing %s because of error: %s\n", launch_config_filename, ex.what());
        }
    }

    char working_dir[256];
    GetCurrentDirectoryA(sizeof(working_dir), working_dir);
    std::filesystem::path dll_path(working_dir);
    dll_path.append("PrometheusCore.dll");

    while (simple_imgui::imgui_loop())
    {
        bool open = true;
        ImGui::SetNextWindowSize(ImVec2(800, 400));
        if (ImGui::Begin("Prometheus Launcher", &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
        {
            ImGui::TextUnformatted("Welcome to the Prometheus Launcher.");
            bool launch_possible = true;
            bool exe_valid = false;

            if (ImGui::BeginChild("##config", ImVec2(-10, -30)))
            {
                ImGui::TextUnformatted("Game Executable");
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
                    }
                    if (_current_file_signature != std::string(overwatch_beta_md5))
                    {
                        launch_possible = false;
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
                        ImGui::TextUnformatted("Failed to verify game client signature.");
                        ImGui::Text("Got: %s", _current_file_signature.c_str());
                        ImGui::Text("Expected: %s", overwatch_beta_md5);
                        ImGui::SameLine();
                        HelpMarker("The file you selected is not GameClientApp.exe.\n\nThis may mean that the game executable was modified. Ensure that you have a legitemate game copy of Beta 0.8(24919).\n\nFor older users: Please select the unpatched executable, as the patch is not necessary anymore.");
                        ImGui::PopStyleColor();
                    } else
                    {
                        ImGui::TextDisabled("Successfully verified the game executable's integrity.");
                        exe_valid = true;
                    }
                } else
                {
                    launch_possible = false;
                }

                if (ImGui::TreeNode("Advanced configuration"))
                {
                    ImGui::TextUnformatted("Working Directory");
                    ImGui::SameLine();
                    HelpMarker("Working directory of the game. Leave empty to use the default working directory (the directory of the game's executable).");
                    ImGui::SameLine();
                    InputText("##wd", s_launch_config.working_dir);

                    ImGui::NewLine();
                    ImGui::SeparatorText("Launch Options");
                    if (ImGui::TextLink("See more..."))
                    {
                        open_link("https://owdev.wiki/wiki/index.php/Game/Launch_Options");
                    }
                    ImGui::Checkbox("Console", &s_launch_config.launch_console);
                    ImGui::SameLine();
                    ImGui::Checkbox("Show Mode", &s_launch_config.launch_showmode);
                    ImGui::SameLine();
                    HelpMarker("Show Mode is the mode Blizzard uses when hosting local LAN tournaments (ex.: Blizzcon).");

                    const int item_size = 80;
                    ImGui::TextUnformatted("Resolution:");
                    ImGui::TextUnformatted("X:");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(80);
                    ImGui::InputInt("##x", &s_launch_config.posX, 0, 0);
                    ImGui::SameLine();
                    ImGui::TextUnformatted("Y:");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(80);
                    ImGui::InputInt("##y", &s_launch_config.posY, 0, 0);
                    ImGui::SameLine();
                    ImGui::TextUnformatted("W:");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(80);
                    ImGui::InputInt("##w", &s_launch_config.resX, 0, 0);
                    ImGui::SameLine();
                    ImGui::TextUnformatted("H:");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(80);
                    ImGui::InputInt("##h", &s_launch_config.resY, 0, 0);

                    ImGui::TextUnformatted("Extra Options");
                    ImGui::SameLine();
                    InputText("##lopt", s_launch_config.extra_args);

                    ImGui::NewLine();
                    ImGui::TextDisabled("%s", get_arguments().c_str());

                    ImGui::NewLine();
                    ImGui::SeparatorText("Misc");

                    if (ImGui::TextLink("Don't click me"))
                        open_link("https://www.youtube.com/watch?v=dQw4w9WgXcQ");

                    ImGui::TreePop();
                }
                if (!std::filesystem::exists(dll_path))
                {
                    launch_possible = false;
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
                    ImGui::Text("DLL not found: %s", dll_path.string().c_str());
                    ImGui::SameLine();
                    HelpMarker("Make sure PrometheusCore.dll is in the path that is displayed. If you downloaded a release build, make sure to extract every file.");
                    ImGui::PopStyleColor();
                }
            }
            ImGui::EndChild();

            if (!launch_possible)
                ImGui::BeginDisabled();

            if (ImGui::Button("Launch"))
            {
                save_config();
                launch_game(dll_path.string());
                break;
            }

            if (!launch_possible)
            {
                if (ImGui::BeginItemTooltip())
                {
                    ImGui::Text("Cannot launch game due to an invalid configuration.");
                    ImGui::EndTooltip();
                }
                ImGui::EndDisabled();
            }

            // if (!exe_valid)
            //     ImGui::BeginDisabled();
            //
            // ImGui::SameLine();
            // if (ImGui::Button("Launch (without mods)"))
            // {
            //     auto process = start_process_suspended(s_launch_config.game_location);
            //     if (ResumeThread(process.hThread) == -1)
            //         printf("Failed to resume thread: %x\n", GetLastError());
            // }
            //
            // if (!exe_valid)
            //     ImGui::EndDisabled();

            ImGui::SameLine();
            if (ImGui::Button("Exit"))
                break;

            ImGui::SameLine();
            ImGui::TextDisabled("%s", dll_path.string().c_str());
        }
        if (!open)
            break;
        ImGui::End();

        simple_imgui::imgui_render();
    }
    simple_imgui::imgui_cleanup();
    return 0;
}
