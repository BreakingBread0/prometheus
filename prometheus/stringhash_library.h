#pragma once
#include <map>
#include <string>
#include <nlohmann/json.hpp>

struct component_info {
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(component_info, name, comment);

	std::string name;
	std::string comment;
};

namespace stringhash_library {
	extern std::map<int, std::string> hashes;
	extern std::map<__int64, std::string> comments;
	extern std::map<int, component_info> components;
	
	void display_component(int component_id);
	void display_hash(int hash, const char* prepend = nullptr);
	void add_hash(const std::string str);
	void add_comment(__int64 key, std::string value, bool force_override = false);
	const std::string& find_component(__int64 hash);
	const std::string& find_hash(int hash);

	//If it errors out
	void enable_library_saving();
	void show_error();
	
	void initialize();
	void save_all();
}


