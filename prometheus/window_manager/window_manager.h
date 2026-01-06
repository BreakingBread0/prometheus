#pragma once
#include <memory>
#include "imgui.h"
#include <Windows.h>
#include "../globals.h"
#include <string>
#include <format>
#include <set>
#include <vector>
#include <imgui_internal.h>
#include "../Statescript.h"
#include "../STU_Editable.h"
#include <map>
#include <mutex>

typedef std::string window_type;

class window {
public:
	int window_id = 0;
	bool is_dependent = false;
	bool is_modal = false;

	const bool& has_dependents = _has_dependents;
	const ImGuiID& im_id = _im_id;
	const bool& is_collapsed = _is_collapsed;
	const bool& is_docked = _is_docked;
	const bool& is_focused = _is_focused;

	void set_focus_next_frame(bool focus = true) {
		_focus_next_frame = focus;
	}
	void set_collapsed(bool collapse = true) {
		_wants_collapse = collapse;
		_wants_show = !collapse;
	}
	void queue_deletion() {
		_wants_delete = true;
	}

	//always returns result
	std::weak_ptr<window> get_root_dock() {
		return _root_dock;
	}

	//only works if this is root dock
	std::vector<std::shared_ptr<window>> get_docked();
	window* purge_docking_tree();
	//size ratio of old window
	window* dock_item_right(window* other, float size_ratio);
	window* dock_item_down(window* other, float size_ratio);
	window* dock_tab_here(window* other);
protected:
	bool open_window(const char* title = (const char*)0, int flags = 0, ImVec2 size = ImVec2(600, 500));
	void display_addr(__int64 addr, const char* prepend = nullptr);
	void display_text(char* text, const char* prepend = nullptr);

	//F�r window registrar
	virtual window* create_self() = 0;
	virtual const char* category_name() = 0;
	virtual const char* window_name() = 0;
	virtual void render() = 0;
	virtual window_type get_window_type() = 0;
	virtual bool allow_auto_creation() = 0;
	virtual bool is_singleton() = 0;

	std::weak_ptr<window> created_by{};
	std::weak_ptr<window> this_instance{};

	template <typename T>
	inline T* get_creator_as() {
		static_assert(std::is_base_of_v<window, T>);
		auto ref = created_by.lock();
		if (ref) {
			auto ptr = ref.get();
			if (ptr && dynamic_cast<T*>(ptr)) {
				return (T*)ptr;
			}
		}
		return nullptr;
	}
private:

	//TODO refactor
	virtual void pre_render() {}
	virtual void initialize() {};
	virtual void preStartInitialize() {};

	friend class window_manager;
	friend class management_window; //lazy asf

	bool _first_render = true;
	bool _focus_next_frame = false;
	bool _has_dependents = false;
	ImGuiID _im_id = 0;
	bool _is_focused = false;
	bool _is_collapsed = 0;
	bool _wants_collapse = false;
	bool _wants_show = false;
	bool _wants_delete = false;
	bool _is_docked = false;
	int _exception_counter = 0;
	std::weak_ptr<window> _root_dock;
	//ImGuiID _dock_uniqueid;

	struct DockRequest {
		std::weak_ptr<window> target;
		ImGuiDir direction;
		float size_ratio;
	};

	std::vector<DockRequest> _dock_requests;
};

class window_manager {
public:
	struct window_data {
		//Stop, do not modify this and do not use it. I am just too lazy to remove this from the header.
		//Or do, I am just a sign.
		std::unique_ptr<window> helper_instance;

		std::string category;
		std::string name;
		int active_windows_cnt;
	};

	static void register_window(window* window_reference);
	static std::set<std::string>& get_window_categories();
	static const std::map<window_type, window_data>& get_all_windows();
	static void add_default_window(window_type type);
	//static std::shared_ptr<window> ensure_exists(window_type type);
	static void call_preStartInitialize();

	template <typename modal_window_type, typename arg_type>
	static inline void open_modal(window* from, arg_type arg) {
		auto new_wind = create_by_type<modal_window_type>(from);
		owassert(new_wind.get());
		auto wind_cast = dynamic_cast<modal_window_type*>(new_wind.get());
		wind_cast->set(arg);
		new_wind->is_dependent = true;
		new_wind->is_modal = true;
		new_wind->set_focus_next_frame(true);
	}

	template <typename modal_window_type>
	static inline void open_modal(window* from) {
		auto new_wind = create_by_type<modal_window_type>(from);
		new_wind->is_dependent = true;
		new_wind->is_modal = true;
		new_wind->set_focus_next_frame(true);
	}

	//not recursive
	static void kill_dependents(window* from);
	static std::shared_ptr<window> add_window(std::unique_ptr<window> window_reference, window* from = nullptr);
	static inline std::shared_ptr<window> add_window(window* window_reference, window* from = nullptr) {
		return add_window(std::unique_ptr<window>(window_reference), from);
	}
	const static const std::vector<std::shared_ptr<window>> get_window_list() {
		return s_windows;
	}
	//static void remove_window(window* window_reference);
	static void render();
	static void render_ex();
	static void render_error(const char* error);

	//Put here since the MSVC linker is bullshit. 
	template <typename T>
	static inline std::shared_ptr<window> get_latest_or_create(window* from, bool focus = true, bool is_dependent = false) {
		auto self = from->this_instance.lock();
		if (!self)
			return {};
		for (auto& wind : *s_all_windows) {
			if (dynamic_cast<T*>(wind.second.helper_instance.get()) != nullptr) {
				if (is_dependent) {
					for (auto& window : get_all_by_type(wind.second.helper_instance->get_window_type())) {
						if (window->is_dependent && window->created_by.lock() == self) {
							window->_focus_next_frame = focus;
							return window;
						}
					}
					auto result = add_window(wind.second.helper_instance->create_self(), from);
					if (result) {
						result->is_dependent = true;
						result->_focus_next_frame = focus;
					}
					return result;
				}
				else {
					auto result = get_docked(wind.second.helper_instance->get_window_type(), from);
					if (!result)
						result = get_latest_by_type(wind.first);
					if (!result)
						result = add_window(wind.second.helper_instance->create_self(), from);
					if (result)
						result->_focus_next_frame = focus;
					//printf("get_latest_or_create %s -> %s\n", from->window_name(), result ? result->window_name() : "Invalid");
					return result;
				}
			}
		}
		return {};
	}


	template <typename T>
	static inline std::shared_ptr<window> create_by_type(window* from) {
		auto self = from->this_instance.lock();
		if (!self)
			return {};
		for (auto& wind : *s_all_windows) {
			if (dynamic_cast<T*>(wind.second.helper_instance.get()) != nullptr) {
				return add_window(wind.second.helper_instance->create_self(), from);
			}
		}
		return {};
	}

	template <typename T>
	static inline std::shared_ptr<window> get_latest_if_exists(window* from) {
		for (auto& window : *s_all_windows) {
			if (dynamic_cast<T*>(window.second.helper_instance.get()) != nullptr) {
				auto result = get_docked(window.second.helper_instance->get_window_type(), from);
				if (!result)
					result = get_latest_by_type(window.first);
				return result;
			}
		}
		return {};
	}

	static std::shared_ptr<window> get_latest_by_type(window_type typ);
	static inline int max_window_id() {
		return s_id_counter;
	}
private:
	static std::shared_ptr<window> get_docked(window_type typ, window* from);
	static std::vector<std::shared_ptr<window>> get_all_by_type(window_type typ);
	static void remove_window_internal(window* window_reference);
	static bool window_id_exists(int window_id);
	static inline std::vector<std::shared_ptr<window>> s_window_add_queue{};
	static inline std::vector<std::shared_ptr<window>> s_windows{};
	static inline std::map<ImGuiID, std::weak_ptr<window>> s_windows_by_im_id{};
	static inline std::map<window_type, std::weak_ptr<window>> s_latest_windows{};
	static inline volatile long s_id_counter;
	static inline int s_focused_window;
	static void call_window_render(window*);

	//STATIC INITIALIZERS ARE GREAT!
	static inline std::set<std::string>* s_window_categories = nullptr;
	static inline std::map<window_type, window_data>* s_all_windows = nullptr;

	//friend class management_window;
	friend class window;

};

template<class window_instance>
class windowRegistrar {
public:
	static_assert(std::is_base_of<window, window_instance>::value, "Registrar: Must not register a non-window class." );
	windowRegistrar() {
		window_instance* instance = new window_instance;
		window_manager::register_window(instance);
	}
};

#define WINDOW_REGISTER(cls) static windowRegistrar<cls> s_##cls##_window_registration{};
//Zu faul um alles umzubenennen TODO
#define WINDOW_DEFINE_2(cls, category, name, allow_auto, isSingleton) \
public: \
inline const char* window_name() override { return ##name ; } \
inline const char* category_name() override { return ##category ; } \
inline window_type get_window_type() override { return #cls ; } \
inline window* create_self() override { return new cls; } \
/*inline static cls* get_latest(window* from) { return (cls*)window_manager::get_latest_if_exists<cls>(from).get(); }*/ \
inline static cls* get_latest_or_create(window* from, bool focus = true, bool is_dependent = false) { return (cls*)window_manager::get_latest_or_create<cls>(from, focus, is_dependent).get(); } \
inline static cls* create(window* from) { return (cls*)window_manager::create_by_type<cls>(from).get(); } \
inline bool allow_auto_creation() override { return allow_auto; } \
inline bool is_singleton() override { return isSingleton; }

#define WINDOW_DEFINE(cls, category, name, allow_auto) WINDOW_DEFINE_2(cls, category, name, allow_auto, false);
#define WINDOW_DEFINE_ARG(cls, category, name, arg) \
WINDOW_DEFINE_2(cls, category, name, false, false) \
private: \
##arg _arg{}; \
public: \
cls* set(##arg new_arg) { _arg = new_arg; return this; }
#define STRUCT_MODIFIABLE(stru, member) imgui_helpers::modifiable(#member, &stru->member, this); 

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
	void modifiable(const char* text, uint* value, window* window = nullptr);
	void modifiable(const char* text, float* value, window* window = nullptr);
	void modifiable(const char* text, __int64* value, window* window = nullptr);
	void modifiable(const char* text, double* value, window* window = nullptr);
	void modifiable(const char* text, Vector4* value, window* window = nullptr);
	void modifiable(const char* text, Matrix4x4* value, window* window = nullptr);
	inline void modifiable(const char* text, bool* value, window* window = nullptr) {
		modifiable(text, (char*)value, window);
	}
	template <typename T>
	void modifiable(const char* text, teList<T>* value, window* window = nullptr);

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

	void render_primitive(STU_Primitive value);
	void editor_primitive(STU_Primitive value);
}

namespace ImGui {
	IMGUI_API inline bool Checkbox(const char* label, char* v) {
		return Checkbox(label, (bool*)v);
	}
}
