//
// Created by cereal on 19.04.26.
//

#include "simple_imgui.h"


#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <GL/gl.h>
#include <tchar.h>

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace simple_imgui
{
    // Data stored per platform window
    struct WGL_WindowData { HDC hDC; };

    // Data
    static HGLRC            g_hRC;
    static WGL_WindowData   g_MainWindow;
    static int              g_Width;
    static int              g_Height;

    // Forward declarations of helper functions
    bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data);
    void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data);
    void ResetDeviceWGL();
    WNDCLASSEXW wc;
    HWND hwnd;
    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Support function for multi-viewports
    // Unlike most other backend combination, we need specific hooks to combine Win32+OpenGL.
    // We could in theory decide to support Win32-specific code in OpenGL backend via e.g. an hypothetical ImGui_ImplOpenGL3_InitForRawWin32().
    static void Hook_Renderer_CreateWindow(ImGuiViewport* viewport)
    {
        assert(viewport->RendererUserData == NULL);

        WGL_WindowData* data = IM_NEW(WGL_WindowData);
        CreateDeviceWGL((HWND)viewport->PlatformHandle, data);
        viewport->RendererUserData = data;
    }

    static void Hook_Renderer_DestroyWindow(ImGuiViewport* viewport)
    {
        if (viewport->RendererUserData != NULL)
        {
            WGL_WindowData* data = (WGL_WindowData*)viewport->RendererUserData;
            CleanupDeviceWGL((HWND)viewport->PlatformHandle, data);
            IM_DELETE(data);
            viewport->RendererUserData = NULL;
        }
    }

    static void Hook_Platform_RenderWindow(ImGuiViewport* viewport, void*)
    {
        // Activate the platform window DC in the OpenGL rendering context
        if (WGL_WindowData* data = (WGL_WindowData*)viewport->RendererUserData)
            wglMakeCurrent(data->hDC, g_hRC);
    }

    static void Hook_Renderer_SwapBuffers(ImGuiViewport* viewport, void*)
    {
        if (WGL_WindowData* data = (WGL_WindowData*)viewport->RendererUserData)
            ::SwapBuffers(data->hDC);
    }

    bool first = true;
    bool imgui_loop()
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                return false;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (ImGui::GetCurrentContext() && ImGui::GetCurrentContext()->Windows.size() == 0 && !first)
            return false;
        first = false;
        return true;
    }

    void imgui_render()
    {
        ImGui::Render();

        auto main_viewport = ImGui::GetMainViewport();
        for (auto& window : ImGui::GetCurrentContext()->Windows)
        {
            if (window->ViewportId == main_viewport->ID)
            {
                window->ViewportId = window->ID;
            }
        }

        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        glViewport(0, 0, g_Width, g_Height);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            // TODO for OpenGL: restore current GL context.
        }

        // Present
        ::SwapBuffers(g_MainWindow.hDC);
    }

    //
    // // Main code
    bool imgui_init()
    {
        // Make process DPI aware and obtain main monitor scale
        ImGui_ImplWin32_EnableDpiAwareness();
        float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

        // Create application window
        wc = { sizeof(wc), CS_OWNDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
        ::RegisterClassExW(&wc);
        hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui Win32+OpenGL3 Example", WS_OVERLAPPEDWINDOW, 0, 0, 1, 1, nullptr, nullptr, wc.hInstance, nullptr);

        // Initialize OpenGL
        if (!CreateDeviceWGL(hwnd, &g_MainWindow))
        {
            CleanupDeviceWGL(hwnd, &g_MainWindow);
            ::DestroyWindow(hwnd);
            ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
            return false;
        }
        wglMakeCurrent(g_MainWindow.hDC, g_hRC);

        // Show the window
        //::ShowWindow(hwnd, SW_SHOWDEFAULT);
        ::UpdateWindow(hwnd);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // Enable Multi-Viewport / Platform Windows

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // Setup scaling
        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        // Setup Platform/Renderer backends
        ImGui_ImplWin32_InitForOpenGL(hwnd);
        ImGui_ImplOpenGL3_Init();

        // Win32+GL needs specific hooks for viewport, as there are specific things needed to tie Win32 and GL api.
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
            IM_ASSERT(platform_io.Renderer_CreateWindow == NULL);
            IM_ASSERT(platform_io.Renderer_DestroyWindow == NULL);
            IM_ASSERT(platform_io.Renderer_SwapBuffers == NULL);
            IM_ASSERT(platform_io.Platform_RenderWindow == NULL);
            platform_io.Renderer_CreateWindow = Hook_Renderer_CreateWindow;
            platform_io.Renderer_DestroyWindow = Hook_Renderer_DestroyWindow;
            platform_io.Renderer_SwapBuffers = Hook_Renderer_SwapBuffers;
            platform_io.Platform_RenderWindow = Hook_Platform_RenderWindow;
        }
        return true;
    }

    void imgui_cleanup()
    {
        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        CleanupDeviceWGL(hwnd, &g_MainWindow);
        wglDeleteContext(g_hRC);
        ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    }
    // Helper functions
    bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data)
    {
        HDC hDc = ::GetDC(hWnd);
        PIXELFORMATDESCRIPTOR pfd = { 0 };
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;

        const int pf = ::ChoosePixelFormat(hDc, &pfd);
        if (pf == 0)
            return false;
        if (::SetPixelFormat(hDc, pf, &pfd) == FALSE)
            return false;
        ::ReleaseDC(hWnd, hDc);

        data->hDC = ::GetDC(hWnd);
        if (!g_hRC)
            g_hRC = wglCreateContext(data->hDC);
        return true;
    }

    void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data)
    {
        wglMakeCurrent(nullptr, nullptr);
        ::ReleaseDC(hWnd, data->hDC);
    }

    // Win32 message handler
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        switch (msg)
        {
        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED)
            {
                g_Width = LOWORD(lParam);
                g_Height = HIWORD(lParam);
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
        }
        return ::DefWindowProcW(hWnd, msg, wParam, lParam);
    }
}