/*#define NOMINMAX
#include "renderer.h"
#include "memory.h"
#include <dwmapi.h>
#include <d3d11.h>
#include <string>
#include <math.h>
#include <sstream>
#define STB_IMAGE_IMPLEMENTATION //ONLY DEFINE ONCE IN THE PROJECT! @Nusty
#define STB_IMAGE_RESIZE_IMPLEMENTATION

#include "imgui_internal.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "imfont.h"
#include "imgui.h"
#include "stb_image.h"
#include "stb_image_resize.h"
#include "renderer.h"

ID3D11Device* d3d_device{};
ID3D11DeviceContext* d3d_context{};
IDXGISwapChain* dxgi_chain{};
ID3D11RenderTargetView* d3d_rtv{};
//#include <Windows.h>
bool renderer::LoadTextureFromFile(const char* filename, void** out_srv, int wanted_width, int wanted_height)
{
	// Load from disk into a raw RGBA buffer
	int image_width = 0;
	int image_height = 0;
	int channels = 0;
	/*char buf[256];
	GetCurrentDirectoryA(256, buf);
	printf("%s\n", buf);#1#
	unsigned char* image_unresized_data = stbi_load(filename, &image_width, &image_height, &channels, 4);
	if (image_unresized_data == NULL) {
		printf("Image not found: %s. Cannot continue!\n", filename);
		__fastfail(0);
		return false;
	}
	unsigned char* image_data = new unsigned char[wanted_width * wanted_height * channels];
	stbir_resize_uint8(image_unresized_data, image_width, image_height, 0, image_data, wanted_width, wanted_height, 0, channels);

	// Create texture
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = wanted_width;
	desc.Height = wanted_height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;

	ID3D11Texture2D* pTexture = NULL;
	D3D11_SUBRESOURCE_DATA subResource;
	subResource.pSysMem = image_data;
	subResource.SysMemPitch = desc.Width * 4;
	subResource.SysMemSlicePitch = 0;
	d3d_device->CreateTexture2D(&desc, &subResource, &pTexture);

	// Create texture view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	d3d_device->CreateShaderResourceView(pTexture, &srvDesc, (ID3D11ShaderResourceView**)out_srv);
	pTexture->Release();

	stbi_image_free(image_unresized_data);
	delete[] image_data;

	return true;
}
void renderer::ChangeClickability(bool canclick)
{
	long style = GetWindowLong(render_window, GWL_EXSTYLE);
	if (canclick) {
		style &= ~WS_EX_LAYERED;
		SetWindowLong(render_window, GWL_EXSTYLE, style);
		SetForegroundWindow(render_window);
		//windowstate = 1;
	}
	else {
		style |= WS_EX_LAYERED;
		SetWindowLong(render_window, GWL_EXSTYLE, style);
		//windowstate = 0;
	}
}
HWND renderer::render_window;
ImFont* renderer::render_font;
ImFont* renderer::header_1_font;
ImFont* renderer::header_2_font;
ImFont* renderer::header_3_font;
int renderer::window_width;
int renderer::window_height;
float renderer::current_fov;

//void message_loop()
//{
//	MSG msg;
//	ZeroMemory(&msg, sizeof(msg));
//	while (PeekMessage(&msg, renderer::render_window, 0, 0, PM_REMOVE))
//	{
//		if (msg.message == WM_CLOSE)
//			continue;
//		TranslateMessage(&msg);
//		DispatchMessageA(&msg);
//	}
//}

bool initialize_directx(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 144;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &dxgi_chain, &d3d_device, &featureLevel, &d3d_context) != S_OK)
		return false;

	ID3D11Texture2D* pBackBuffer;
	dxgi_chain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	d3d_device->CreateRenderTargetView(pBackBuffer, NULL, &d3d_rtv);
	pBackBuffer->Release();


	return true;
}

void renderer::begin_frame() {
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void renderer::quit() {
	DWORD procID;
	GetWindowThreadProcessId(render_window, &procID);
	TerminateProcess(OpenProcess(PROCESS_TERMINATE, false, procID), 0);

	TerminateProcess(GetCurrentProcess(), 0);
}

void renderer::end_frame() {
	//ImGui::End();
	ImGui::EndFrame();
	ImGui::Render();

	static ImVec4 c{};
	d3d_context->OMSetRenderTargets(1, &d3d_rtv, NULL);
	d3d_context->ClearRenderTargetView(d3d_rtv, (float*)&c);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	dxgi_chain->Present(1, 0);
}

void renderer::refresh_window_properties() {
	int i = (int)GetWindowLong(render_window, -20);
	SetWindowLongPtr(render_window, -20, (LONG_PTR)(i | 0x20));
	GetWindowLong(render_window, GWL_EXSTYLE);

	MARGINS margin;
	UINT opacity, opacityFlag, colorKey;

	margin.cyBottomHeight = -1;
	margin.cxLeftWidth = -1;
	margin.cxRightWidth = -1;
	margin.cyTopHeight = -1;

	SetWindowLongA(render_window, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED);
	DwmExtendFrameIntoClientArea(render_window, &margin);
	SetLayeredWindowAttributes(render_window, 0, 0xFF, 0x02);
	SetWindowPos(render_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	ShowWindow(render_window, SW_SHOW);
}

void renderer::initialize() {
#ifdef INCLUDE_DEBUG
	char name[21];
	WNDCLASSA window_class;
	if (global::use_overlay) {
		memcpy(name, "aaaaaaaaaaaaaaaaaa", 18);
		window_class = WNDCLASSA
		{
			0,
			[](const HWND window, const UINT message, const WPARAM wparam, const LPARAM lparam) -> LRESULT
			{
				return DefWindowProcA(window, message, wparam, lparam);
			},
			0,
			0,
			GetModuleHandleW(nullptr),
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			name
		};

		RegisterClassA(&window_class);
	}
#endif
	if (global::dry_run) {
		window_width = GetSystemMetrics(SM_CXSCREEN);
		window_height = GetSystemMetrics(SM_CYSCREEN);
	}
	else {
		RECT overwatch_window;
		GetWindowRect(memory::process_window, &overwatch_window);
		window_width = overwatch_window.right - overwatch_window.left;
		window_height = overwatch_window.bottom - overwatch_window.top;
	}
	if (!global::use_overlay) {
		//printf("Size: %d %d\n", width, height);
		while (!render_window) {
			printf("Waiting for Nvidia\n");
			render_window = FindWindowA("CEF-OSC-WIDGET", nullptr);
			/*DWORD proc;
			GetWindowThreadProcessId(render_window, &proc);
			printf("%d\n", proc);#1#
			Sleep(500);
		}
	}
	else {
#ifdef INCLUDE_DEBUG
		render_window = CreateWindowExA(0, name, "", WS_POPUP, 0, 0, window_width, window_height, nullptr, nullptr, window_class.hInstance, nullptr);

		HRGN rgn;
		DWM_BLURBEHIND blur;
		rgn = CreateRectRgn(0, 0, 1, 1);
		blur.dwFlags = 0x1 | 0x2;
		blur.fEnable = true;
		blur.fTransitionOnMaximized = true;
		blur.hRgnBlur = rgn;

		DwmEnableBlurBehindWindow(render_window, &blur);

		ShowWindow(render_window, SW_SHOW);
		UpdateWindow(render_window);

		SetWindowLongA(render_window, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED);
		SetWindowPos(render_window, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

		SetWindowPos(render_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
#endif
	}

	refresh_window_properties();

	if (!initialize_directx(render_window))
	{
		DestroyWindow(render_window);
#ifdef INCLUDE_DEBUG
		if (global::use_overlay)
			UnregisterClassA(name, GetModuleHandleW(nullptr));
#endif

		//who needs cleanups when you can just yeet the process
		//cleanup_device_d3d();

		__fastfail(0); //YEET
		return;
	}

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.IniSavingRate = 0;
	io.IniFilename = nullptr;

	//io.Fonts->AddFontFromMemoryTTF(fontData, 24848, 16);
	render_font = io.Fonts->AddFontFromMemoryTTF(fontData, 24848, 16, nullptr, io.Fonts->GetGlyphRangesDefault());

	header_1_font = io.Fonts->AddFontFromMemoryTTF(fontData, 24848, 18, nullptr, io.Fonts->GetGlyphRangesDefault());
	header_2_font = io.Fonts->AddFontFromMemoryTTF(fontData, 24848, 22, nullptr, io.Fonts->GetGlyphRangesDefault());
	header_3_font = io.Fonts->AddFontFromMemoryTTF(fontData, 24848, 26, nullptr, io.Fonts->GetGlyphRangesDefault());
	//render_font = io.Fonts->AddFontDefault();
	//AddFontFromMemoryCompressedTTF(verdanab_compressed_data, verdanab_compressed_size, 16.f, NULL, io.Fonts->GetGlyphRangesCyrillic());

	ImGui_ImplWin32_Init(render_window);
	ImGui_ImplDX11_Init(d3d_device, d3d_context);


	ImGuiStyle* style = &ImGui::GetStyle();
	ImVec4* colors = ImGui::GetStyle().Colors;
	style->WindowPadding = ImVec2(15, 15);
	//nugget's design (bad)
	//:shut:
	style->Alpha = 0.900000f;
	style->WindowRounding = 5.100000f;
	style->WindowBorderSize = 0.000000f;
	style->ChildRounding = 0.000000f;
	style->ChildBorderSize = 1.000000f;
	style->PopupRounding = 0.000000f;
	style->PopupBorderSize = 0.000000f;
	style->FrameRounding = 5.100000f;
	style->FrameBorderSize = 0.000000f;
	style->IndentSpacing = 25.000000f;
	style->ColumnsMinSpacing = 6.000000f;
	style->ScrollbarSize = 15.000000f;
	style->ScrollbarRounding = 3.700000f;
	style->GrabMinSize = 5.000000f;
	style->GrabRounding = 3.000000f;
	style->TabRounding = 4.000000f;
	style->TabBorderSize = 0.000000f;
	style->MouseCursorScale = 1.000000f;
	style->CurveTessellationTol = 1.250000f;

	style->Colors[ImGuiCol_Text] = ImVec4(1.000000, 1.000000, 1.000000, 1.000000);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.680000, 0.710000, 0.740000, 0.800000);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.129412, 0.141176, 0.168627, 1.000000);
	style->Colors[ImGuiCol_ChildBg] = ImVec4(0.000000, 0.000000, 0.000000, 0.000000);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.070000, 0.070000, 0.090000, 1.000000);
	style->Colors[ImGuiCol_Border] = ImVec4(0.550379, 0.550387, 0.743682, 0.880000);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.844765, 0.785296, 0.606889, 0.000000);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.060000, 0.080000, 0.110000, 1.000000);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.074510, 0.094118, 0.125490, 1.000000);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.560000, 0.560000, 0.580000, 1.000000);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.074510, 0.094118, 0.125490, 1.000000);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.074510, 0.094118, 0.125490, 1.000000);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.000000, 0.980000, 0.950000, 0.150000);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.100000, 0.090000, 0.120000, 0.800000);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.039711, 0.034514, 0.023081, 0.000000);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.949020, 0.309804, 0.000000, 1.000000);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.949020, 0.309804, 0.000000, 1.000000);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.949020, 0.309804, 0.000000, 1.000000);
	style->Colors[ImGuiCol_CheckMark] = ImVec4(0.949020, 0.309804, 0.000000, 1.000000);
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.949020, 0.309804, 0.000000, 1.000000);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.949020, 0.309804, 0.000000, 1.000000);
	style->Colors[ImGuiCol_Button] = ImVec4(0.019305, 0.019305, 0.019305, 0.263000);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.382239, 0.124781, 0.000000, 1.000000);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.505792, 0.169060, 0.005859, 1.000000);
	style->Colors[ImGuiCol_Header] = ImVec4(0.141750, 0.091686, 0.241877, 1.000000);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.225069, 0.145056, 0.382671, 1.000000);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.283178, 0.216176, 0.350181, 1.000000);
	style->Colors[ImGuiCol_Separator] = ImVec4(0.410014, 0.410015, 0.516245, 0.500000);
	style->Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.100000, 0.400000, 0.750000, 0.780000);
	style->Colors[ImGuiCol_SeparatorActive] = ImVec4(0.100000, 0.400000, 0.750000, 1.000000);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.000000, 0.000000, 0.000000, 0.000000);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.560000, 0.560000, 0.580000, 1.000000);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.060000, 0.050000, 0.070000, 1.000000);
	style->Colors[ImGuiCol_Tab] = ImVec4(0.173285, 0.173285, 0.173285, 0.862000);
	style->Colors[ImGuiCol_TabHovered] = ImVec4(0.317690, 0.317690, 0.317690, 0.800000);
	style->Colors[ImGuiCol_TabActive] = ImVec4(0.317690, 0.317690, 0.317690, 1.000000);
	style->Colors[ImGuiCol_TabUnfocused] = ImVec4(0.068000, 0.102000, 0.148000, 0.972400);
	style->Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.136000, 0.262000, 0.424000, 1.000000);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.400000, 0.390000, 0.380000, 0.630000);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.250000, 1.000000, 0.000000, 1.000000);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.400000, 0.390000, 0.380000, 0.630000);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.250000, 1.000000, 0.000000, 1.000000);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.250000, 1.000000, 0.000000, 0.430000);
	style->Colors[ImGuiCol_DragDropTarget] = ImVec4(1.000000, 1.000000, 0.000000, 0.900000);
	style->Colors[ImGuiCol_NavHighlight] = ImVec4(0.260000, 0.590000, 0.980000, 1.000000);
	style->Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.000000, 1.000000, 1.000000, 0.700000);
	style->Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000, 0.800000, 0.800000, 0.200000);
	style->Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000, 0.800000, 0.800000, 0.350000);
}

namespace renderer {
	namespace drawing {
		ImGuiWindow* window;
		void begin(char* window_name)
		{
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			ImGui::Begin(window_name, 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav);

			ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
			ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y), ImGuiCond_Always);
			window = ImGui::GetCurrentWindow();
		}

		void end()
		{
			window->DrawList->PushClipRectFullScreen();

			ImGui::End();
			ImGui::PopStyleColor();
		}

		//old code :)
		//
		//Vector3 WorldToScreen(Vector3 vec) {
		//	float w = (Viewmatrix.M14 * vec.X) + (Viewmatrix.M24 * vec.Y) + (Viewmatrix.M34 * vec.Z + Viewmatrix.M44);
		//	/*if (w < 0.1f)
		//		return Vector3{};#1#
		//	float y = (Viewmatrix.M12 * vec.X) + (Viewmatrix.M22 * vec.Y) + (Viewmatrix.M32 * vec.Z + Viewmatrix.M42);
		//	float x = (Viewmatrix.M11 * vec.X) + (Viewmatrix.M21 * vec.Y) + (Viewmatrix.M31 * vec.Z + Viewmatrix.M41);
		//	Vector3 ret{};
		//	ret.X = (screenSize.x / 2) + (screenSize.x / 2) * x / w;
		//	ret.Y = (screenSize.y / 2) - (screenSize.y / 2) * y / w;
		//	return ret;
		//}

		float DrawString(std::string text, const ImVec2& pos, float size, uint32_t color, bool center)
		{
			ImGuiWindow* window = ImGui::GetCurrentWindow();

			float a = (color >> 24) & 0xff; //#AARRGGBB
			float r = (color >> 16) & 0xff;
			float g = (color >> 8) & 0xff;
			float b = (color) & 0xff;

			std::stringstream steam(text);
			std::string line;
			float y = 0.0f;
			int i = 0;

			while (std::getline(steam, line))
			{
				ImVec2 textSize = renderer::render_font->CalcTextSizeA(size, FLT_MAX, 0.0f, line.c_str());
				if (center)
				{
					window->DrawList->AddText(renderer::render_font, size, ImVec2(pos.x - textSize.x / 2.0f, pos.y + textSize.y * i), ImGui::GetColorU32(ImVec4(r / 255, g / 255, b / 255, a / 255)), line.c_str());
				}
				else
				{
					window->DrawList->AddText(renderer::render_font, size, ImVec2(pos.x, pos.y + textSize.y * i), ImGui::GetColorU32(ImVec4(r / 255, g / 255, b / 255, a / 255)), line.c_str());
				}

				y = pos.y + textSize.y * (i + 1);
				i++;
			}

			return y;
		}

		const int HealthColor = 2;

		void DrawHealth(const ImVec2& scalepos, float width, float health, float maxHealth) {
			float thickness = 5;
			/*float health = health + armor + shields;
			float maxHealth = maxHealth + maxarmor + maxshields;#1#
			float oneSegment = width / maxHealth;

			float segmentHealth = oneSegment * health;
			/*float segmentArmor = oneSegment * armor;
			float segmentShield = oneSegment * shields;#1#

			int progress = (int)((float)health / ((float)maxHealth / 100));
			uint32_t backColor = 0xFF303030;

			//uint32_t color = 0xFF0000FF;
			////ABGR?
			//if (progress >= 20) color = 0xFF00A5FF;
			//if (progress >= 40) color = 0xFF00FFFF;
			//if (progress >= 60) color = 0xFF2FFFAD;
			//if (progress >= 80) color = 0xFF00FF00;

			int Health = (int)((health / maxHealth) * 100);

			uint32_t color = 0xFF000000;
			color += std::min((int)(510 * Health) / 100, 255) << 8;
			if (HealthColor == 3)
				color += std::min((int)(255 * (100 - Health) / 100), 255) << 16;
			else if (HealthColor == 2)
				color += std::min((int)(510 * (100 - Health) / 100), 255);
			else if (HealthColor == 1)
				color = 0xFFFF00FF;

			DrawLine(scalepos, ImVec2(scalepos.x + width, scalepos.y), backColor, thickness);
			DrawLine(scalepos, ImVec2(scalepos.x + segmentHealth, scalepos.y), color, thickness);
		}
		void DrawFOV(float radious) {
			/*static float test = 70.f;
			static float test2 = 80.f;
			ImGui::Begin("asfaedsfgawerg");
			ImGui::SliderFloat("asdasd", &test, 30, 150);
			ImGui::SliderFloat("asdasd123", &test2, 30, 150);
			ImGui::End();
			float asd = test;#1#
			//I have no idea why 33.f is correct and at this point I am too afraid to ask
			float pxR = tanf(DEGTORAD(radious) / 2) / (2 * tanf(DEGTORAD((float)(renderer::current_fov - 33.f)) / 2)) * (renderer::window_height);
			DrawCircle(ImVec2(renderer::window_width / 2, renderer::window_height / 2), pxR, 0xFF000000);
		}

		void DrawHealthVertical(const ImVec2& scalepos, float width, float health, float maxHealth, int HealthColor) {
			float thickness = 4;
			/*float actualHealth = health + armor + shields;
			float actualMaxHealth = maxHealth + maxarmor + maxshields;#1#
			float oneSegment = width / maxHealth;

			float segmentHealth = oneSegment * health;
			/*float segmentArmor = oneSegment * armor;
			float segmentShield = oneSegment * shields;
			float segments = segmentHealth + segmentArmor + segmentShield;#1#

			int progress = (int)((float)health / ((float)maxHealth / 100));
			uint32_t backColor = 0xFF303030;

			int Health = (int)((health / maxHealth) * 100);

			uint32_t color = 0xFF000000;
			color += std::min((int)(510 * Health) / 100, 255) << 8;
			if (HealthColor == 3)
				color += std::min((int)(255 * (100 - Health) / 100), 255) << 16;
			else if (HealthColor == 2)
				color += std::min((int)(510 * (100 - Health) / 100), 255);
			else if (HealthColor == 1)
				color = 0xFFFF00FF;



			DrawLine(scalepos, ImVec2(scalepos.x, scalepos.y + width), backColor, thickness);
			DrawLine(ImVec2(scalepos.x, scalepos.y + (width - segmentHealth)), ImVec2(scalepos.x, scalepos.y + width), color, thickness);
		}

		void DrawLine(const ImVec2& from, const ImVec2& to, ImU32 color, float thickness)
		{
			window->DrawList->AddLine(from, to, color, thickness);
		}

		void DrawRect(const ImVec2& from, const ImVec2& to, ImU32 color, float thickness)
		{
			auto drawList = window->DrawList;
			float percentage = 0.2;
			float width = abs(from.x - to.x) * percentage;
			float height = abs(from.y - to.y) * percentage;

			drawList->AddLine(from, ImVec2(from.x + width, from.y), color, thickness);
			drawList->AddLine(ImVec2(to.x, from.y), ImVec2(to.x - width, from.y), color, thickness);

			drawList->AddLine(from, ImVec2(from.x, from.y + height), color, thickness);
			drawList->AddLine(ImVec2(to.x, from.y), ImVec2(to.x, from.y + height), color, thickness);

			drawList->AddLine(to, ImVec2(to.x, to.y - height), color, thickness);
			drawList->AddLine(ImVec2(from.x, to.y), ImVec2(from.x, to.y - height), color, thickness);

			drawList->AddLine(to, ImVec2(to.x - width, to.y), color, thickness);
			drawList->AddLine(ImVec2(from.x, to.y), ImVec2(from.x + width, to.y), color, thickness);


			/*ImGuiWindow* window = ImGui::GetCurrentWindow();
			window->DrawList->AddRect(from, to, color, 0.0F, -1, thickness);#1#
		}


		void DrawRect(const ImVec2& from, const ImVec2& to, ImU32 color, float thickness, float rounding)
		{
			window->DrawList->AddRect(from, to, color, rounding, 0xF, thickness);
		}

		void DrawRectFilled(const ImVec2& from, const ImVec2& to, ImU32 color, float thickness, float rounding)
		{
			window->DrawList->AddRectFilled(from, to, color, rounding, 0xF);
		}

		void DrawTextRaw(const std::string& text, const ImVec2& pos, float size, const ImVec4& color)
		{
			return DrawTextRaw(renderer::render_font, text, pos, size, color);
		}

		void DrawTextRaw(ImFont* pFont, const std::string& text, const ImVec2& pos, float size, const ImVec4& color)
		{
			pFont = ImGui::GetFont();
			if (!pFont)
				return;
			window->DrawList->AddText(pFont, size, pos, ImGui::GetColorU32(color), text.c_str());
		}

		/*inline float Renderer::DrawTextX(const string& text, const ImVec2& pos, float size, const ImVec4& color, bool center)
		{
			return Renderer::DrawTextX(m_pFont, text, pos, size, color, center);
		}

		inline float Renderer::DrawTextX(ImFont* pFont, const string& text, const ImVec2& pos, float size, const ImVec4& color, bool center)
		{

			pFont = ImGui::GetFont();

			if (!pFont)
				return 0.0f;

			ImU32 col1 = ImGui::GetColorU32(ImVec4(0, 0, 0, color.w / 1.0f));
			ImU32 col2 = ImGui::GetColorU32(ImVec4(color.x / 1.0f, color.y / 1.0f, color.z / 1.0f, color.w / 1.0f));

			stringstream steam(text);
			string line;

			float y = 0.0f;
			int i = 0;
			while (getline(steam, line))
			{
				ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, line.c_str());

				if (center)
				{
					window->DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) + 1, (pos.y + textSize.y * i) + 1), col1, line.c_str());
					window->DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) - 1, (pos.y + textSize.y * i) - 1), col1, line.c_str());
					window->DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) + 1, (pos.y + textSize.y * i) - 1), col1, line.c_str());
					window->DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) - 1, (pos.y + textSize.y * i) + 1), col1, line.c_str());

					window->DrawList->AddText(pFont, size, ImVec2(pos.x - textSize.x / 2.0f, pos.y + textSize.y * i), col2, line.c_str());
				}
				else
				{
					window->DrawList->AddText(pFont, size, ImVec2((pos.x) + 1, (pos.y + textSize.y * i) + 1), col1, line.c_str());
					window->DrawList->AddText(pFont, size, ImVec2((pos.x) - 1, (pos.y + textSize.y * i) - 1), col1, line.c_str());
					window->DrawList->AddText(pFont, size, ImVec2((pos.x) + 1, (pos.y + textSize.y * i) - 1), col1, line.c_str());
					window->DrawList->AddText(pFont, size, ImVec2((pos.x) - 1, (pos.y + textSize.y * i) + 1), col1, line.c_str());

					window->DrawList->AddText(pFont, size, ImVec2(pos.x, pos.y + textSize.y * i), col2, line.c_str());
				}

				y = pos.y + textSize.y * (i + 1);
				i++;
			}

			return y;
		}#1#

		void DrawHealthBar(int width, const ImVec2& from, int currentHealth, int maxHealth, int currentArmor, int maxArmor, int currentBarrier, int maxBarrier)
		{
			int height = width * 0.1;
			int indent = width * 0.02;
			int blockSize = width * 0.06;
			int space = width * 0.01;
			int nbBlock = width / blockSize;
			int nbBlockArmor = 0;
			if (maxArmor != 0 && currentArmor != 0) {
				int nbBlockArmorMax = nbBlock * 0.25;
				nbBlockArmor = (currentArmor * nbBlockArmorMax) / maxArmor;
			}
			int nbBlockBarrier = 0;
			if (maxBarrier != 0 && currentBarrier != 0) {
				int nbBlockBarrierMax = nbBlock * 0.25;
				nbBlockBarrier = (currentBarrier * nbBlockBarrierMax) / maxBarrier;
			}
			int nbBlockHealthFull = (currentHealth * (nbBlock - nbBlockArmor - nbBlockBarrier)) / maxHealth;

			for (int i = 0; i < nbBlock; i++) {
				int pos1X = from.x + (i * (blockSize + space));
				int pos1Y = from.y;
				ImVec2 pos1(pos1X, pos1Y);

				int pos2X = from.x + blockSize + (i * (blockSize + space));
				int pos2Y = from.y;
				ImVec2 pos2(pos2X, pos2Y);

				int pos3X = from.x + indent + blockSize + (i * (blockSize + space));
				int pos3Y = from.y - height;
				ImVec2 pos3(pos3X, pos3Y);

				int pos4X = from.x + indent + (i * (blockSize + space));
				int pos4Y = from.y - height;
				ImVec2 pos4(pos4X, pos4Y);

				if (i < nbBlockHealthFull) {
					DrawQuadFilled(pos4, pos3, pos2, pos1, ImColor(255, 0, 0, 255), 0, 0);
				}
				else {
					if (nbBlockArmor > 0 && i < (nbBlockHealthFull + nbBlockArmor)) {
						DrawQuadFilled(pos4, pos3, pos2, pos1, ImColor(255, 220, 49, 255), 0, 0);
					}
					else {
						if (nbBlockBarrier > 0 && i < (nbBlockHealthFull + nbBlockArmor + nbBlockBarrier)) {
							DrawQuadFilled(pos4, pos3, pos2, pos1, ImColor(114, 189, 234, 255), 0, 0);
						}
						else {
							DrawQuadFilled(pos4, pos3, pos2, pos1, ImColor(80, 80, 80, 125), 0, 0);
						}
					}

				}
			}
		}

		void DrawQuad(const ImVec2& pos1, const ImVec2& pos2, const ImVec2& pos3, const ImVec2& pos4, ImU32 color, float thickness, float rounding)
		{
			window->DrawList->AddQuad(pos1, pos2, pos3, pos4, color, thickness);
		}

		void DrawQuadFilled(const ImVec2& pos1, const ImVec2& pos2, const ImVec2& pos3, const ImVec2& pos4, ImU32 color, float thickness, float rounding)
		{
			window->DrawList->AddQuadFilled(pos1, pos2, pos3, pos4, color);
		}

		void DrawCircle(const ImVec2& position, float radius, ImU32 color, float thickness)
		{
			int number = radius / 2 + 6;
			window->DrawList->AddCircle(position, radius, color, (radius / 2 + 6), thickness);
		}

		void DrawCircleFilled(const ImVec2& position, float radius, ImU32 color)
		{
			window->DrawList->AddCircleFilled(position, radius, color, (radius / 2 + 6));
		}
	}
};*/