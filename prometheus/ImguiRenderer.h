#pragma once
#include <imgui.h>
#include <imgui_internal.h>
#include <string>
#include <stdint.h>
#undef DrawText

//thanks to that random guy from UC ;)
//https://www.unknowncheats.me/forum/d3d-tutorials-and-source/198625-rendering-imgui.html
class ImguiRenderer
{
public:
	void Initialize();

	void BeginScene();
	void EndScene();

	float DrawOutlinedText(ImFont* pFont, const std::string& text, const ImVec2& position, float size, uint32_t color, bool center);
	float DrawString(ImFont* pFont, const std::string& text, const ImVec2& position, float size, uint32_t color, bool center);
	void DrawLine(const ImVec2& from, const ImVec2& to, uint32_t color, float thickness = 1.0f);
	void DrawCircle(const ImVec2& position, float radius, uint32_t color, float thickness = 1.0f);
	void DrawCircleScale(const ImVec2& position, float radius, uint32_t color, const ImVec2& scalepos, const ImVec2& scaleHeadPosition, float thickness = 1.0f);
	void DrawCircleFilled(const ImVec2& position, float radius, uint32_t color);
	void DrawBox(const ImVec2& position, const ImVec2& HeadPosition, uint32_t color);

	static ImguiRenderer* GetInstance();

private:
	ImguiRenderer();
	~ImguiRenderer();

	static inline ImguiRenderer* m_pInstance;
};