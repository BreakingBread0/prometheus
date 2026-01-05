#pragma once
#include "../window_manager/window_manager.h"
#include "../stringhash_library.h"
#include "../game.h"

class comments_window : public window {
	WINDOW_DEFINE(comments_window, "Resource", "Type Comments", false);
	__int64 _typ;

	inline void render() override {
		if (open_window(nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Editing for '%p'", _typ);
			auto item = allmighty_hash_lib::comments.find(_typ);
			if (item == allmighty_hash_lib::comments.end())
				return;

			auto hashitem = allmighty_hash_lib::hashes.find(_typ);
			if (hashitem != allmighty_hash_lib::hashes.end()) {
				ImGui::Text("Hash found: %s", hashitem->second.c_str());
			}
			char* buf = new char[item->second.size() + 32];
			memcpy(buf, item->second.c_str(), item->second.size());
			buf[item->second.size()] = 0;
			if (ImGui::InputText("Comment", buf, item->second.size() + 32)) {
				allmighty_hash_lib::comments[_typ] = buf;
			}
			delete[] buf;
			ImGui::Text("Expected %x got %x", _typ, stringHash(item->second.c_str()));
			if (stringHash(item->second.c_str()) == _typ) {
				ImGui::Text("Hash is good, saving in hashes!");
			}
			if (ImGui::Button("Save")) {
				if (stringHash(item->second.c_str()) == _typ) {
					allmighty_hash_lib::add_hash(item->second);
					allmighty_hash_lib::comments.erase(_typ);
				}
				allmighty_hash_lib::save_all();
				queue_deletion();
			}
			ImGui::SameLine();
			if (ImGui::Button("Delete")) {
				allmighty_hash_lib::comments.erase(_typ);
				allmighty_hash_lib::save_all();
				queue_deletion();
			}
			ImGui::SameLine();
			if (ImGui::Button("Copy Key")) {
				imgui_helpers::openCopyWindow(_typ);
			}
			ImGui::SameLine();
			if (ImGui::Button("Copy comment")) {
				imgui_helpers::openCopyWindow(std::string(item->second));
			}
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
public:
	comments_window() {}
	comments_window(__int64 type) : _typ(type) {
		if (allmighty_hash_lib::comments.find(type) == allmighty_hash_lib::comments.end()) {
			char* buf = new char[256];
			sprintf_s(buf, 256, "Unknown Item");
			allmighty_hash_lib::comments.emplace(type, buf);
		}
		auto& comment = allmighty_hash_lib::comments[type];
		comment.resize(comment.size() + 10);
	}
};

WINDOW_REGISTER(comments_window);
