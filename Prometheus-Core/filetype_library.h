#pragma once
#include "globals.h"
#include <map>

namespace filetype_library {
	extern std::map<int, char*> library;

	void init();
	void save_filetypes();

	void display_filetype(__int64 filetype);
}