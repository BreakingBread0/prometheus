#pragma once
#include "../window_manager/window_manager.h"
#include <MinHook.h>
#include <map>
#include "../JAM.h"

class jam_messagedetail_window : public window {
public:
	WINDOW_DEFINE(jam_messagedetail_window, "Jam", "Jam Messages", false);

	std::vector<JamMessage*> _messages;
	std::string _pool_name;
	jam_messagedetail_window(std::vector<__int64> message_instances, std::string pool_name) {
		_messages = {};
		_pool_name = pool_name;
		for (auto& message : message_instances) {
			_messages.push_back((JamMessage*)message);
		}
	}
	jam_messagedetail_window() {}

	std::string get_type_str(int type) {
		std::string result;
		switch (type) {
		case 0:
			result = "BOOL";
			break;
		case 1:
		case 2:
		case 3:
			result = "BYTE";
			break;
		case 4:
		case 5:
			result = "SHORT";
			break;
		case 6:
		case 7:
			result = "INT";
			break;
		case 8:
		case 11:
			result = "DOUBLE";
			break;
		case 9:
			result = "QWORD";
			break;
		case 10:
			result = "FLOAT";
			break;
		case 12:
			result = "STRING_ARR";
			break;
		case 13:
			result = "STRING";
			break;
		case 14:
			result = "STRUCT";
			break;
		case 15:
			result = "object?";
			break;
		case 16:
			result = "DYNAMIC";
			break;
		case 17:
			result = "END";
			break;
		}
		return result + " (" + std::to_string(type) + ")";
	}

	const char* tableName = "An awesome jamming table";

	inline void printHeader() {
		imgui_helpers::printTableHeader({ "Name", "Type", "Offset", "Length", "Extra Data", "Array Type", "Some Func?" });
	}

	inline void printItem(JamMessageItemInfo* item) {
		ImGui::TableNextRow();
		ImGui::PushID(item);
		ImGui::TableNextColumn();
		ImGui::Text("%s", item->name_str);

		ImGui::TableNextColumn();
		ImGui::Text("%s", get_type_str(item->message_type));

		ImGui::TableNextColumn();
		ImGui::Text("%x %d", item->offset, item->offset);

		ImGui::TableNextColumn();
		ImGui::Text("%x %d", item->size, item->size);

		ImGui::TableNextColumn();
		ImGui::Text("%x %d", item->has_extra_flag, item->has_extra_flag);

		ImGui::TableNextColumn();
		display_addr((__int64)item->inner_data_ptr);

		ImGui::TableNextColumn();
		ImGui::PushID(ImGui::TableGetColumnIndex());
		display_addr((__int64)item->decoding_func);
		ImGui::PopID();
		ImGui::PopID();

		if (item->inner_data_ptr) {
			ImGui::EndTable();
			ImGui::PushID(item->inner_data_ptr + 1);
			char header_label[64];
			const char id_label[] = "=> ID";
			auto isId = ((__int64)item->inner_data_ptr - globals::gameBase) == 0x172c910;
			sprintf_s(header_label, "Struct for off 0x%x (rva: 0x%p)%s", item->offset, item->inner_data_ptr, isId ? id_label : "");
			if (ImGui::CollapsingHeader(header_label)) {
				display_addr((__int64)item->inner_data_ptr, "PTR");
				imgui_helpers::beginTable(tableName, 7);
				printHeader();
				JamMessageItemInfo* inner = item->inner_data_ptr;
				while (inner->message_type != 0x11) {
					printItem(inner);
					inner++;
				}

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::TableNextColumn();
				ImGui::TextUnformatted(get_type_str(inner->message_type).c_str());

				ImGui::EndTable();
			}
			ImGui::PopID();
			imgui_helpers::beginTable(tableName, 7);
		}
	}

	inline void render() override {
		if (open_window()) {
			for (JamMessage* message : _messages) {
				ImGui::PushID(message);
				auto info = message->vfptr->get_message_info();
				char separator_buf[128];
				sprintf_s(separator_buf, "%p (%x | %x): %s", message, info->message_protocol, info->message_id, info->message_name == nullptr ? "(UNK)" : info->message_name);
				ImGui::SeparatorText(separator_buf);
				display_addr((__int64)message, "Instance Addr");
				display_addr((__int64)message->vfptr, "VTable");
				display_text(info->message_name, "Message Name");
				display_text(info->message_protocol, "Message Protocol");
				display_addr(info->crc, "CRC");
				display_addr(info->message_id, "ID");

				if (imgui_helpers::beginTable(tableName, 7)) {
					printHeader();

					JamMessageItemInfo* item = info->items;
					while (item->message_type != 0x11) {
						printItem(item);
						item++;
					}
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::TableNextColumn();
					ImGui::Text(get_type_str(item->message_type).c_str());
					ImGui::EndTable();
				}

				ImGui::NewLine(); ImGui::NewLine(); ImGui::NewLine();
				ImGui::PopID();
			}
			if (imgui_helpers::CenteredButton("Close")) {
				queue_deletion();
			}
		}
		ImGui::End();
	}
};

WINDOW_REGISTER(jam_messagedetail_window);