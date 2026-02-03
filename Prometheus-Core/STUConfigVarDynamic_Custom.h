#pragma once
#include "STU.h"
#include <vector>
#include "Statescript.h"

struct STUConfigVarDynamic_Custom
{
public:
	struct fake_list {
		STUResourceReference* list;
		int count;
	};

	STUConfigVar cv_base;
	fake_list* list;
	int static_list_flag = 1;

	STUConfigVarDynamic_Custom(std::vector<__int64> ids);

	~STUConfigVarDynamic_Custom() {
		if (list) {
			delete[] list->list;
			delete list;
			list = nullptr;
		}
	}

	STUConfigVarDynamic* get() {
		return (STUConfigVarDynamic*)this;
	}
};
