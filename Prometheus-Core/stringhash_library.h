#pragma once
#include <map>
#include <string>
#include <nlohmann/json.hpp>

struct component_info {
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(component_info, name, comment);

	std::string name;
	std::string comment;
};

namespace allmighty_hash_lib {
	extern std::map<int, std::string> hashes;
	extern std::map<__int64, std::string> comments;
	extern std::map<int, component_info> components;
	
	void display_component(int component_id);
	void display_hash(int hash, const char* prepend = nullptr);
	void add_hash(const std::string str);
	void add_comment(__int64 key, std::string value, bool force_override = false);
	void initialize();
	void save_all();
}


