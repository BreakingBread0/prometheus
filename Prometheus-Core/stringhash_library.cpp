#include "stringhash_library.h";
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include "globals.h"
#include "game.h"
#include "window_manager/window_manager.h"
#include <thread>
#include <mutex>

namespace stringhash_library {
	std::map<int, std::string> hashes{};
	std::map<__int64, std::string> comments{};
	std::map<int, component_info> components{};
	std::recursive_mutex mut{};
	bool has_error = false;
	bool error_shown = false;
	std::string error = "No error";

	void enable_library_saving() {
		has_error = false;
		error_shown = false;
	}

	void show_error() {
		imgui_helpers::messageBox(std::format("Has error: %s\nError: %s", has_error ? "Yes" : "No", error), "Stringhash Library", nullptr);
	}
	
	void display_hash(int hash, const char* prepend) {
		std::unique_lock lock(mut);
		if (prepend) {
			ImGui::Text("%s ", prepend);
			ImGui::SameLine();
		}
		if (hashes.find(hash) != hashes.end()) {
			ImGui::Text("%x (%s)", hash, hashes[hash].c_str());
		}
		else {
			ImGui::Text("%x", hash);
		}
		ImGui::SameLine();
		if (ImGui::Button("Copy")) {
			imgui_helpers::openCopyWindow(hash);
		}
	}
	const char* filename = "hashlibrary.json";
	void display_component(int component_id) {
		std::unique_lock lock(mut);
		auto result = components.find(component_id);
		if (result != components.end()) {
			ImGui::Text("%hhx (%s)", component_id, result->second.name.c_str());
		}
		else {
			ImGui::Text("%hhx", component_id);
		}
	}

	std::string _empty_string;
	const std::string& find_component(__int64 hash) {
		auto result = comments.find(hash);
		if (result != comments.end())
			return _empty_string;
		return result->second;
	}

	const std::string& find_hash(int hash) {
		auto result = comments.find(hash);
		if (result != comments.end())
			return _empty_string;
		return result->second;
	}

	void add_comment(__int64 key, std::string value, bool force_override) {
		std::unique_lock lock(mut);
		if (comments.find(key) == comments.end()) {
			comments.emplace(key, value);
			save_all();
		}
		else if (force_override) {
			std::string buf = comments[key];
			if (buf == value)
				return;
			printf("Override %p: from '%s' to '%s'\n", key, buf.c_str(), value.c_str());
			comments[key] = value;
			save_all();
		}
	}

	std::thread save_thread = {};
	//Mutexed
	void save_all() {
		if (has_error && !error_shown) {
			show_error();
			return;
		}
		//save_thread.joinable() is unreliable
		if (save_thread.joinable()) {
			return;
		}
		save_thread = std::thread([] {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			std::unique_lock lock(mut);
			try {
				nlohmann::json result{};
				for (auto& hash : hashes) {
					char buf[16];
					sprintf_s(buf, "%x", hash.first);
					result["db"].emplace(buf, hash.second);
				}
				for (auto& comment : comments) {
					char buf[32];
					sprintf_s(buf, "%p", comment.first);
					result["comments"].emplace(buf, comment.second);
				}
				for (auto& compo : components) {
					char buf[32];
					sprintf_s(buf, "%p", compo.first);
					result["components"].emplace(buf, compo.second);
				}
				std::ofstream fstream(filename, std::ios::out | std::ios::trunc);
				if (!fstream.is_open()) {
					imgui_helpers::messageBox("Filestream open error", "Hashlib");
					return;
				}
				fstream << result.dump(4);
				fstream.flush();
				printf("Allmighty HASHLIB: saved :)\n");
				save_thread.detach();
			}
			catch (nlohmann::json::exception ex) {
				imgui_helpers::messageBox("Failed to save hashlibrary.json. Saving is disabled. JSON Error: " + std::string(ex.what()), "Hashlib");
			}
			catch (std::exception ex) {
				imgui_helpers::messageBox("Failed to save hashlibrary.json. Saving is disabled. Error: " + std::string(ex.what()), "Hashlib");
			}
			catch (...) {
				imgui_helpers::messageBox("Failed to save hashlibrary.json. Saving is disabled", "Hashlib");
			}
		});
	}

	void add_hash(const std::string str) {
		std::unique_lock lock(mut);
		int hash = stringHash(str.c_str());
		if (hashes.find(hash) == hashes.end()) {
			hashes.emplace(hash, str);
			printf("Saving new entry into hashlib: %x - %s\n", hash, str.c_str());
			save_all();
		}
	}
	
	void initialize() {
		std::unique_lock lock(mut);
		try {
			nlohmann::json json;
			{
				if (!std::filesystem::exists(filename)) {
					return;
				}
				std::ifstream input_file(filename);
				if (!input_file.is_open()) {
					has_error = true;
					error = "Failed to open input hash file.";
					return;
				}
				json = nlohmann::json::parse(input_file);
			}
			if (!json.is_object()) {
				return;
			}

			auto db = json["db"];
			if (db.is_object()) {
				for (auto it = db.begin(); it != db.end(); ++it) {
					int expectedHash = strtoll(it.key().c_str(), nullptr, 16);
					if (expectedHash == 0) {
						continue;
					}
					auto value = it.value().get<std::string>();
					if (expectedHash != stringHash(value.c_str())) {
						printf("Invalid hash: Expected %x but got %x (%s)\n", expectedHash, stringHash(value.c_str()), value.c_str());
						continue;
					}

					hashes.emplace(expectedHash, value);
				}
			}
			else {
				error = "Failed to open input hash file.";
				has_error = true;
				return;
			}
			auto comm = json["comments"];
			for (auto it = comm.begin(); it != comm.end(); ++it) {
				__int64 expectedHash = _strtoi64(it.key().c_str(), nullptr, 16);
				if (expectedHash == 0) {
					continue;
				}
				auto value = it.value().get<std::string>();
				comments.emplace(expectedHash, value);
			}

			auto compos = json["components"];
			for (auto it = compos.begin(); it != compos.end(); ++it) {
				__int64 expectedHash = _strtoi64(it.key().c_str(), nullptr, 16);
				if (expectedHash == 0) {
					continue;
				}
				auto value = it.value().get<component_info>();
				components.emplace(expectedHash, value);
			}

			if (json.contains("add") && json["add"].is_array()) {
				for (auto it = json["add"].begin(); it != json["add"].end(); ++it) {
					add_hash(it.value().get<std::string>());
				}
			}
			return;
		}
		catch (nlohmann::json::exception ex) {
			error = "Failed to open hashlibrary.json. Saving is disabled. JSON Error: %s\n", ex.what();
		}
		catch (std::bad_alloc& ex) {
			error = "Failed to open hashlibrary.json. Saving is disabled. Error: %s\n", ex.what();
		}
		catch (std::exception& ex) {
			error = "Failed to open hashlibrary.json. Saving is disabled. Error: %s\n", ex.what();
		}
		catch (...) {
			error = "Failed to open hashlibrary.json. Saving is disabled.\n";
		}
		has_error = true;
	}

	/*void initialize() {
		__try {
			initialize2();
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			printf("WHAT IS THIS ERROR\n");
			system("pause");
			__fastfail(1);
		}
	}*/
}