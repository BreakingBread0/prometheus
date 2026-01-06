#pragma once
#include <set>
#include <string>
#include "fmt/format.h"
#include "globals.h"
#include <imgui.h>
#include "stringhash_library.h"


class search_helper {
public:
	void update_needle(std::string str) {
		_needle = str;
		_needle_empty = str.empty();
		set_needs_haystack();
	}

	void set_needs_haystack() {
		_needs_haystack = true;
		_found_needles.clear();
		_identifier_count = 0;
		_curr_haystack.clear();
		_start_needle = nullptr;
	}

	bool needs_haystack() {
		return _needs_haystack;
	}

	bool found_needle(uint identifier) {
		return found_needle((void*)identifier);
	}
	//no duplicates
	bool found_needle(void* identifier) {
		if (_found_needles.find(identifier) != _found_needles.end() || _needle_empty) {
			if (_needs_haystack) {
				_finish_search();
			}
			return true;
		}
		if (_needs_haystack) {
			if (identifier == _start_needle) {
				_finish_search();
			}
			if (_start_needle == nullptr)
				_start_needle = identifier;
			_identifier_count++;
			//printf("needle: '%s', haystack: '%s'\n", _needle.c_str(), _curr_haystack.c_str());
			if (icontains(_curr_haystack, _needle)) {
				//printf("ID: %p - haystack: %s - needle: %s\n", identifier, _curr_haystack.c_str(), _needle.c_str());
				_found_needles.insert(identifier);
				_curr_haystack.clear();
				return true;
			}
		}
		_curr_haystack.clear();
		return false;
	}

	void haystack(std::string input) {
		if (_needs_haystack)
			_curr_haystack += " " + input;
	}
	void haystack(int input) {
		if (_needs_haystack)
			_curr_haystack += " " + std::to_string(input);
	}
	void haystack_hex(__int64 input) {
		if (_needs_haystack)
			_curr_haystack += " " + fmt::format("{:x}", input);
	}
	void haystack_stringhash(__int64 input) {
		if (!_needs_haystack)
			return;
		auto comment = allmighty_hash_lib::comments.find(input);
		if (comment != allmighty_hash_lib::comments.end()) {
			haystack(comment->second);
		}
		auto hash = allmighty_hash_lib::hashes.find(input);
		if (hash != allmighty_hash_lib::hashes.end()) {
			haystack(hash->second);
		}
		haystack_hex(input);
	}
protected:
	void _finish_search() {
		_needs_haystack = false;
		if (_needle_empty)
			printf("Search reset!\n");
		else
			printf("Search done. Identifier count: %d - Results: %d\n", _identifier_count, _found_needles.size());
	}

	bool _needs_haystack = true;
	std::set<void*> _found_needles;
	void* _start_needle;
	std::string _needle;
	std::string _curr_haystack;
	bool _needle_empty = true;
	int _identifier_count;
};

class search_helper_imgui : public search_helper {
public:
	bool search_box(std::string label) {
		if (ImGui::InputText(label.c_str(), _str_buf, sizeof(_str_buf))) {
			update_needle(_str_buf);
			return true;
		}
		if (!_needle_empty) {
			ImGui::SameLine();
			ImGui::Text("%d/%d", _found_needles.size(), _identifier_count);
		}
		return false;
	}

private:
	char _str_buf[124]{};
};