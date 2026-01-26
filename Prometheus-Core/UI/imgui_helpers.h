#pragma once

#include "globals.h"
#include "idadefs.h"
#include "Atlas/STU/RTTI/STUArgumentInfo.h"
#include "window_manager/window_manager.h"

using namespace Atlas::STU::RTTI;

struct STUConfigVarDynamic;
struct StatescriptInstance;
class STU_Primitive;
class STUConfigVar;
namespace imgui_helpers {
	extern ImFont* BoldFont;

	inline bool CenteredButton(const char* label, float alignment = 0.5f)
	{
		ImGuiStyle& style = ImGui::GetStyle();

		float size = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
		float avail = ImGui::GetContentRegionAvail().x;

		float off = (avail - size) * alignment;
		if (off > 0.0f)
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

		return ImGui::Button(label);
	}

	void openCopyWindow(std::string value);
	void openCopyWindow(__int64 value);
	void messageBox(std::string data, std::string title = "", window* window = nullptr);
	void messageBox(std::string data, window* window);

	inline void printTableHeader(std::vector<std::string> list) {
		for (auto& item : list) {
			ImGui::TableSetupColumn(item.c_str());
		}
		ImGui::TableHeadersRow();
	}

	inline bool beginTable(const char* name, int headerCount, int flags = /*ImGuiTableFlags_ScrollY |*/ ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersH | ImGuiTableFlags_HighlightHoveredColumn) {
		return ImGui::BeginTable(name, headerCount, flags/*, ImVec2(-1, -1)*/);
	}

	inline bool beginTable(const char* name, std::vector<std::string> headers, int flags = /*ImGuiTableFlags_ScrollY |*/ ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersH | ImGuiTableFlags_HighlightHoveredColumn) {
		bool result = beginTable(name, headers.size(), flags);
		if (result)
			printTableHeader(headers);
		return result;
	}

	inline bool TooltipButton(const char* button_text, const char* tooltip) {
		bool result = ImGui::Button(button_text);
		if (ImGui::IsItemHovered()) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 255, 255, 255));
			ImGui::SetTooltip(tooltip);
			ImGui::PopStyleColor();
		}
		return result;
	}

	inline bool TooltipCheckbox(const char* check_text, bool* value, const char* tooltip) {
		bool result = ImGui::Checkbox(check_text, value);
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip(tooltip);
		}
		return result;
	}

	inline bool TooltipRadioButton(const char* radio_text, bool value, const char* tooltip) {
		bool result = ImGui::RadioButton(radio_text, value);
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip(tooltip);
		}
		return result;
	}

	inline bool InputHex(std::string label, __int64* num) {
		owassert(num != nullptr);
		char buf[32];
		sprintf_s(buf, "%p", *num);
		if (ImGui::InputText(label.c_str(), buf, 32)) {
			*num = _strtoi64(buf, nullptr, 16);
			return true;
		}
		return false;
	}

	inline bool InputHex(std::string label, int* num) {
		owassert(num != nullptr);
		char buf[32];
		sprintf_s(buf, "%x", *num);
		if (ImGui::InputText(label.c_str(), buf, 32)) {
			*num = _strtoi64(buf, nullptr, 16);
			return true;
		}
		return false;
	}

	inline uint32 color_fade(ImVec4 from, ImVec4 to, int steps, int step) {
		if (step > steps)
			return IM_COL32(to.x, to.y, to.z, to.w);
		float r = (to.x - from.x) / steps;
		float g = (to.y - from.y) / steps;
		float b = (to.z - from.z) / steps;
		float a = (to.w - from.w) / steps;
		return IM_COL32(
			from.x + r * step,
			from.y + g * step,
			from.z + b * step,
			from.w + a * step
		);
	}

	void modifiable(const char* text, char* value, window* window = nullptr);
	void modifiable(const char* text, short* value, window* window = nullptr);
	void modifiable(const char* text, int* value, window* window = nullptr);
	void modifiable(const char* text, uint32* value, window* window = nullptr);
	void modifiable(const char* text, float* value, window* window = nullptr);
	void modifiable(const char* text, __int64* value, window* window = nullptr);
	void modifiable(const char* text, double* value, window* window = nullptr);
	void modifiable(const char* text, Vector4* value, window* window = nullptr);
	void modifiable(const char* text, Matrix4x4* value, window* window = nullptr);
	inline void modifiable(const char* text, bool* value, window* window = nullptr) {
		modifiable(text, (char*)value, window);
	}
	template <typename T>
	void modifiable(const char* text, teList<T>* value, window* window = nullptr)
	{
		ImGui::Text("%s array: (%d/%d items)", text, value->num, value->max);
		int i = 0;
		ImGui::Indent();
		for (auto& item : *value) {
			ImGui::BulletText("%d", i++);
			ImGui::SameLine();
			modifiable("", &item, window);
		}
		ImGui::Unindent();
	}

	inline void render_vec4(const char* text, Vector4* vec) {
		if (text) {
			ImGui::Text("%s: %f %f %f %f", text, vec->X, vec->Y, vec->Z, vec->W);
		}
		else {
			ImGui::Text("%f %f %f %f", vec->X, vec->Y, vec->Z, vec->W);
		}
	}

	inline void render_matrix4x4(const char* text, Vector4* vec) {
		ImGui::TextUnformatted(text);
		render_vec4(nullptr, &vec[0]);
		render_vec4(nullptr, &vec[1]);
		render_vec4(nullptr, &vec[2]);
		render_vec4(nullptr, &vec[3]);
	}

	bool display_cv(STUConfigVar* node, StatescriptInstance* ss, STUArgumentInfo* arg_info = nullptr, bool display_logicalButton = false);
	void item_path_print(STUConfigVarDynamic* cv);
	bool display_type(__int64 type, bool color, bool edit = true, bool hash_show = true);

	void render_primitive(STU_Primitive value, uint32 hash);
	void editor_primitive(STU_Primitive value, uint32 hash);
}