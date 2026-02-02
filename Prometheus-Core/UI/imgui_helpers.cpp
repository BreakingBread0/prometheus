//
// Created by cereal on 16.01.2026.
//

#include "imgui_helpers.h"

#include "filetype_library.h"
#include "Statescript.h"
#include "stringhash_library.h"
#include "STU_Editable.h"
#include "windows/copy_window.h"
#include "windows/message_window.h"
#include "windows/radio_selector_window.h"
#include "windows/comments_window.h"

namespace imgui_helpers {
	void openCopyWindow(std::string value) {
		window_manager::add_window(new copy_window(value));
	}
	void openCopyWindow(__int64 value) {
		window_manager::add_window(new copy_window(value));
	}
	void messageBox(std::string data, std::string title, window* window) {
		window_manager::add_window(new message_window(data, title), window);
	}
	void messageBox(std::string data, window* window) {
		window_manager::add_window(new message_window(data, ""), window);
	}

	template <typename T>
	void modifiable_int(const char* text, T* value, window* window, const char* format_specifier, const char* format_specifier_2 = nullptr) {
		ImGui::PushID((__int64)value);
		if (TooltipButton(EMOJI_COPY, "Copy Address / Value")) {
			std::vector<std::string> choices = { "Copy Address", "Copy Value" };
			if (format_specifier_2) {
				choices.push_back("Copy Value (Hexadecimal)");
			}
			window_manager::add_window(new radio_selector_window("What to copy?", choices, [value, format_specifier, format_specifier_2](int sel) {
				if (sel == 0) {
					openCopyWindow((__int64)value);
				}
				else {
					char buf[64];
					sprintf_s(buf, sel == 1 ? format_specifier : format_specifier_2, *value);
					openCopyWindow(std::string(buf));
				}
			}));
		}

		bool alt = ImGui::IsKeyDown(ImGuiKey_Menu);
		if (alt) {
			ImGui::SameLine();
			if (ImGui::Button("- 0x100")) {
				*value -= 0x100;
			}
			ImGui::SameLine();
			if (ImGui::Button("- 0x10")) {
				*value -= 0x10;
			}
			ImGui::SameLine();
			if (ImGui::Button("- 1")) {
				*value -= 1;
			}
			ImGui::SameLine();
			if (ImGui::Button("0")) {
				*value = 0;
			}
			ImGui::SameLine();
			if (ImGui::Button("+ 1")) {
				*value += 1;
			}
			ImGui::SameLine();
			if (ImGui::Button("+ 0x10")) {
				*value += 0x10;
			}
			ImGui::SameLine();
			if (ImGui::Button("+ 0x100")) {
				*value += 0x100;
			}
		}

		ImGui::SameLine();
		char buf[32];
		if (format_specifier_2) {
			sprintf_s(buf, "%%s: %s (%s)", format_specifier, format_specifier_2);
			ImGui::Text(buf, text, *value, *value);
		} else {
			sprintf_s(buf, "%%s: %s", format_specifier);
			ImGui::Text(buf, text, *value);
		}

		ImGui::PopID();
	}

	void modifiable(const char* text, char* value, window* window) {
		modifiable_int<char>(text, value, window, "%hhd", "%hhx");
	}

	void modifiable(const char* text, short* value, window* window) {
		modifiable_int<short>(text, value, window, "%hd", "%hx");
	}

	void modifiable(const char* text, int* value, window* window) {
		modifiable_int<int>(text, value, window, "%d", "%x");
	}

	void modifiable(const char* text, uint32* value, window* window) {
		modifiable_int<uint32>(text, value, window, "%d", "%x");
	}

	void modifiable(const char* text, float* value, window* window) {
		modifiable_int<float>(text, value, window, "%f");
	}

	void modifiable(const char* text, __int64* value, window* window) {
		modifiable_int<__int64>(text, value, window, "%lld", "%p");
	}

	void modifiable(const char* text, double* value, window* window) {
		modifiable_int<double>(text, value, window, "%Lf");
	}

	void modifiable(const char* text, Vector4* value, window* window) {
		ImGui::Text("Vector4 <%s>:", text);
		modifiable_int<float>("X", &value->X, window, "%f");
		modifiable_int<float>("Y", &value->Y, window, "%f");
		modifiable_int<float>("Z", &value->Z, window, "%f");
		modifiable_int<float>("W", &value->W, window, "%f");
	}
	void modifiable(const char* text, Matrix4x4* value, window* window) {
		ImGui::Text("Matrix4x4 <%s>:", text);
		ImGui::Text("%f %f %f %f", value->row_1.X, value->row_1.Y, value->row_1.Z, value->row_1.W);
		ImGui::Text("%f %f %f %f", value->row_2.X, value->row_2.Y, value->row_2.Z, value->row_2.W);
		ImGui::Text("%f %f %f %f", value->row_3.X, value->row_3.Y, value->row_3.Z, value->row_3.W);
		ImGui::Text("%f %f %f %f", value->row_4.X, value->row_4.Y, value->row_4.Z, value->row_4.W);
	}

	bool display_cv(STUConfigVar* cv, StatescriptInstance* ss, STUArgumentInfo* arg_info, bool display_logicalButton) {
		ImGui::PushID(cv);
		if (arg_info) {
			display_type(arg_info->Hash, false, true, false);
			ImGui::Indent();
		}
		if (cv->base.valid()) {
			if (IsBadReadPtr((void*)cv->base.vfptr, 8)) {
				ImGui::Text("Invalid");
				if (arg_info)
					ImGui::Unindent();
				ImGui::PopID();
				return false;
			}
			uint stu_hash = cv->base.vfptr->GetSTUInfo()->Hash;
			display_type(stu_hash, true);
			StatescriptPrimitive value{};
			if (cv->get_value(ss, &value)) {
				ImGui::PushFont(imgui_helpers::BoldFont);
				ImGui::Text("value:");
				ImGui::PopFont();
				ImGui::SameLine();
				display_logicalButton |= stu_hash == STU_NAME::STUConfigVarLogicalButton;
				if (value.type == StatescriptPrimitive_INT64 || value.type == StatescriptPrimitive_INT) {
					if (display_logicalButton) {
						ImGui::Text("%d (%x): %s", value.value, value.value, LogicalButtonById(value.value)->name);
					}
					else {
						display_type(value.value, true, true, false);
					}
				}
				else {
					ImGui::TextUnformatted(value.get_value_str().c_str());
				}
			}
			if (cv->is_dynamic()) {
				ImGui::TextUnformatted("Path:");
				ImGui::SameLine();
				item_path_print((STUConfigVarDynamic*)cv);
			}
		}
		if (arg_info)
			ImGui::Unindent();
		ImGui::PopID();
		return false;
	}

	bool display_type(__int64 type, bool color, bool edit, bool hash_show, std::set<__int64> ptrs) {
		if (ptrs.find(type) != ptrs.end()) {
			return false;
		}

		ImGui::PushID(type);
		ptrs.emplace(type);
		bool hover = false;
		if (edit) {
			if (ImGui::Button(EMOJI_EDIT)) {
				window_manager::add_window(new comments_window(type));
			}
			ImGui::SameLine();
			if (ImGui::Button(EMOJI_COPY)) {
				imgui_helpers::openCopyWindow(type);
			}
			hover |= ImGui::IsItemHovered();
			ImGui::SameLine();
		}
		if (hash_show) {
			if (type > UINT_MAX)
				ImGui::Text("%llx", type);
			else
				ImGui::Text("%x", type);
			hover |= ImGui::IsItemHovered();
			ImGui::SameLine();
		}

		if (type < UINT_MAX) {
			auto found_dehash = stringhash_library::hashes.find((uint)type);
			if (found_dehash != stringhash_library::hashes.end()) {
				if (color) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(100, 255, 0, 255));
					ImGui::TextUnformatted(found_dehash->second.c_str());
					ImGui::PopStyleColor();
				}
				else {
					ImGui::Text("[%s]", found_dehash->second.c_str());
				}
				hover |= ImGui::IsItemHovered();
				ImGui::SameLine();
				hash_show = true;
			}
		}
		else {
			int file_type = bitswap(16 * (type & 0xFFFF000000000000uLL)) + 1;
			if (filetype_library::library.find(file_type) != filetype_library::library.end()) {
				auto type_str = filetype_library::library[file_type];
				if (color) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(100, 255, 0, 255));
					ImGui::TextUnformatted(type_str);
					ImGui::PopStyleColor();
				}
				else {
					ImGui::Text("[%s]", type_str);
				}
				hover |= ImGui::IsItemHovered();
				ImGui::SameLine();
			}
		}

		auto comment = stringhash_library::comments.find(type);
		if (comment != stringhash_library::comments.end()) {
			/*std::string cmt = comment->second;
			std::regex regex("0x([0-9a-f]{8,16})", std::regex_constants::ECMAScript | std::regex_constants::icase);
			auto beg = std::sregex_iterator(cmt.begin(), cmt.end(), regex);
			auto end = std::sregex_iterator();*/
			auto write_text = [&hover, color](std::string str, const char* pre = "[", const char* post = "]") {
				if (color) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 200, 0, 255));
					ImGui::TextUnformatted(str.c_str());
					ImGui::PopStyleColor();
				}
				else {
					ImGui::Text("%s%s%s", pre, str.c_str(), post);
				}
				hover |= ImGui::IsItemHovered();
			};

			/*if (beg != end) {
				int latest_pos = 0;
				for (std::sregex_iterator i = beg; i != end; ++i) {
					std::smatch match = *i;
					auto str = match.str();
					__int64 match_id = _strtoi64(str.data(), nullptr, 16);

					ImGui::PushID(i->position());

					if (ImGui::Button(EMOJI_COPY)) {
						imgui_helpers::openCopyWindow(match_id);
					}
					ImGui::SameLine();

					write_text(cmt.substr(latest_pos, match.position()), "[", "");
					latest_pos = match.position() + match.length();
					ImGui::SameLine();
					display_type(match_id, color, edit, hash_show, ptrs);
					ImGui::SameLine();

					ImGui::PopID();
				}
				write_text(cmt.substr(latest_pos), "");
			}
			else {
			}*/
			write_text(comment->second);
		}
		else if (!hash_show) {
			if (type > UINT_MAX)
				ImGui::Text("%p", type);
			else
				ImGui::Text("%llx", type);
			hover |= ImGui::IsItemHovered();
		}
		else {
			ImGui::NewLine();
		}
		ImGui::PopID();
		return hover;
	}

	bool display_type(__int64 type, bool color, bool edit, bool hash_show) {
		return display_type(type, color, edit, hash_show, {});
	}

	void item_path_print(STUConfigVarDynamic* cv) {
		bool first = true;
		for (int i = 0; i < cv->item_location.count(); i++) {
			if (!first) {
				ImGui::TextUnformatted("=>");
				ImGui::SameLine();
			}
			first = false;
			//printf("%p %p\n", item_location.list(), this);
			auto item = cv->item_location.list()[i];
			display_type(item.resource_id, true, true, false);
			ImGui::SameLine();
		}
		ImGui::NewLine();
	}

	void render_primitive(STU_Primitive value) {
		__try {
			switch (value.type) {
			case STU_NAME::Primitive::teMtx43A:
			case STU_NAME::Primitive::teVec3A:
			case STU_NAME::Primitive::teVec2:
			case STU_NAME::Primitive::teVec3:
			case STU_NAME::Primitive::teVec4:
			case STU_NAME::Primitive::teQuat:
			case STU_NAME::Primitive::teColorRGB:
			case STU_NAME::Primitive::teColorRGBA:
			case STU_NAME::Primitive::DBID:
			case STU_NAME::Primitive::teUUID:
			case STU_NAME::Primitive::teStructuredDataDateAndTime:
				ImGui::Text("ImplementMe!");
				break;
			case STU_NAME::Primitive::teString: {
				auto str = value.get_value<const char*>();
				ImGui::Text("%s", strlen(str) == 0 ? "(empty)" : str);
			}
											  break;
			case STU_NAME::Primitive::s16:
				ImGui::Text("%hd", value.get_value<short>());
				break;
			case STU_NAME::Primitive::s32:
				ImGui::Text("%d", value.get_value<int>());
				break;
			case STU_NAME::Primitive::s64:
				ImGui::Text("%lld", value.get_value<__int64>());
				break;
			case STU_NAME::Primitive::u8:
				ImGui::Text("%hhd (%hhx)", value.get_value<char>(), value.get_value<char>());
				break;
			case STU_NAME::Primitive::u16:
				ImGui::Text("%hd (%hx)", value.get_value<ushort>(), value.get_value<ushort>());
				break;
			case STU_NAME::Primitive::u32:
				ImGui::Text("%d (%x)", value.get_value<uint32>(), value.get_value<uint32>());
				break;
			case STU_NAME::Primitive::teEntityID:
				ImGui::Text("%x", value.get_value<uint32>());
				break;
			case STU_NAME::Primitive::u64:
				ImGui::Text("%lld (%p)", value.get_value<__int64>(), value.get_value<__int64>());
				break;
			case STU_NAME::Primitive::f32:
				ImGui::Text("%f", value.get_value<float>());
				break;
			case STU_NAME::Primitive::f64:
				ImGui::Text("%lf", value.get_value<double>());
				break;
			default:
				ImGui::TextUnformatted("Unknown Primitive");
				break;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			ImGui::Text("Exception while rendering primitive.");
		}
	}

	void editor_primitive(STU_Primitive value) {
		ImGui::PushID(value.value);
		switch (value.type) {
		case STU_NAME::Primitive::teMtx43A:
		case STU_NAME::Primitive::teVec3A:
		case STU_NAME::Primitive::teVec2:
		case STU_NAME::Primitive::teVec3:
		case STU_NAME::Primitive::teVec4:
		case STU_NAME::Primitive::teQuat:
		case STU_NAME::Primitive::teColorRGB:
		case STU_NAME::Primitive::teColorRGBA:
		case STU_NAME::Primitive::DBID:
		case STU_NAME::Primitive::teUUID:
		case STU_NAME::Primitive::teStructuredDataDateAndTime:
			ImGui::Text("ImplementMe!");
			ImGui::PopID();
			return;
		case STU_NAME::Primitive::teString: {
			char buf[256];
			auto str = value.get_value<const char*>();
			strcpy_s(buf, str);
			if (ImGui::InputText("", buf, sizeof(buf)))
				value.set_value(buf);
			ImGui::PopID();
			return;
		}
		case STU_NAME::Primitive::f32:
			ImGui::InputFloat("", (float*)value.value);
			ImGui::PopID();
			return;
		case STU_NAME::Primitive::f64:
			ImGui::InputDouble("", (double*)value.value);
			ImGui::PopID();
			return;
		case STU_NAME::Primitive::s64:
		case STU_NAME::Primitive::u64: {
			__int64 temp = value.get_value<uint64>();
			if (imgui_helpers::InputHex("", &temp))
				value.set_value<__int64>(temp);
			ImGui::PopID();
			return;
		}
		}
		auto storage = ImGui::GetStateStorage();
		ImGuiID id = ImGui::GetActiveID();
		auto hex = storage->GetBoolRef(id, false);
		if (value.type != STU_NAME::Primitive::f32 && value.type != STU_NAME::Primitive::f64) {
			ImGui::Checkbox("Hex", hex);
			ImGui::SameLine();
		}
		switch (value.type) {
		case STU_NAME::Primitive::u8: {
			int temp = value.get_value<unsigned char>();
			if (hex ? imgui_helpers::InputHex("", &temp) : ImGui::InputInt("", &temp))
				value.set_value<unsigned char>(temp);
			break;
		}
		case STU_NAME::Primitive::s16: {
			int temp = value.get_value<short>();
			if (hex ? imgui_helpers::InputHex("", &temp) : ImGui::InputInt("", &temp))
				value.set_value<short>(temp);
			break;
		}
		case STU_NAME::Primitive::u16: {
			int temp = value.get_value<ushort>();
			if (hex ? imgui_helpers::InputHex("", &temp) : ImGui::InputInt("", &temp))
				value.set_value<ushort>(temp);
			break;
		}
		case STU_NAME::Primitive::s32: {
			int temp = value.get_value<int>();
			if (hex ? imgui_helpers::InputHex("", &temp) : ImGui::InputInt("", &temp))
				value.set_value<int>(temp);
			break;
		}
		case STU_NAME::Primitive::u32: {
			int temp = value.get_value<uint>();
			if (hex ? imgui_helpers::InputHex("", &temp) : ImGui::InputInt("", &temp))
				value.set_value<uint>(temp);
			break;
		}
		}
		ImGui::PopID();
	}

	ImFont* BoldFont = nullptr;
}