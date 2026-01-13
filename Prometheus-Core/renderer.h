/*#pragma once
#include "globals.h"
#include "imgui.h"
#include "Math.h"
class ID3D11ShaderResourceView;
namespace renderer {
	extern HWND render_window;
	extern int window_width, window_height;
	extern float current_fov;
	extern ImFont* render_font;
	extern ImFont* header_1_font;
	extern ImFont* header_2_font;
	extern ImFont* header_3_font;
	void initialize();
	void begin_frame();
	void end_frame();
	void quit();
	void refresh_window_properties();
	bool LoadTextureFromFile(const char* filename, void** out_srv, int wanted_width, int wanted_height);
	void ChangeClickability(bool canclick);
	namespace drawing {
		void DrawFOV(float radious);
		void begin(char* window_name);
		void end();
		void DrawLine(const ImVec2& from, const ImVec2& to, ImU32 color, float thickness = 1.0f);
		void DrawCircle(const ImVec2& position, float radius, ImU32 color, float thickness = 1.0f);
		void DrawCircleFilled(const ImVec2& position, float radius, ImU32 color);
		void DrawRect(const ImVec2& from, const ImVec2& to, ImU32 color, float thickness = 1.0f);
		void DrawRect(const ImVec2& from, const ImVec2& to, ImU32 color, float thickness, float rounding);
		void DrawRectFilled(const ImVec2& from, const ImVec2& to, ImU32 color, float thickness, float rounding);
		void DrawTextRaw(const std::string& text, const ImVec2& pos, float size, const ImVec4& color);
		void DrawTextRaw(ImFont* pFont, const std::string& text, const ImVec2& pos, float size, const ImVec4& color);
		//float DrawTextX(const std::string& text, const ImVec2& pos, float size, const ImVec4& color, bool center);
		//float DrawTextX(ImFont* pFont, const std::string& text, const ImVec2& pos, float size, const ImVec4& color, bool center);
		void DrawHealthBar(int width, const ImVec2& from, int currentHealth, int maxHealth, int currentArmor, int maxArmor, int currentBarrier, int maxBarrier);
		void DrawQuad(const ImVec2& pos1, const ImVec2& pos2, const ImVec2& pos3, const ImVec2& pos4, ImU32 color, float thickness, float rounding);
		void DrawQuadFilled(const ImVec2& pos1, const ImVec2& pos2, const ImVec2& pos3, const ImVec2& pos4, ImU32 color, float thickness, float rounding);
		//void DrawRadar();
		float DrawString(std::string text, const ImVec2& pos, float size, uint32_t color, bool center);
		void DrawHealth(const ImVec2& scalepos, float width, float health, float maxHealth);
		//Vector3 WorldToRadar(const Vector3& location, const Vector3& origin, const Vector3& angles, int width, float scale = 16.f);
		void DrawHealthVertical(const ImVec2& scalepos, float width, float health, float maxHealth, int HealthColor);
	}
}*/