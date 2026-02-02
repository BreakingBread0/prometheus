#pragma once
#include "STU.h"
#include "Statescript.h"

class STU_Object;
class STUConfigVar_impl_Custom;
class STUConfigVar_Custom {
private:
	STUInfo* _info;
	//incomplete types are great!
	STU_Object* _instance;
	STUConfigVar_impl_Custom* _getter;
public:
	STUConfigVar* get();
	STU_Object get_editable();

	STUConfigVar_Custom(STUInfo* info, StatescriptPrimitive value);
	STUConfigVar_Custom(uint hash, StatescriptPrimitive value) : STUConfigVar_Custom(STURegistry::Get()->GetSTUInfoByHash(hash), value) {}
	STUConfigVar_Custom(StatescriptPrimitive value) : STUConfigVar_Custom(stringHash("STUConfigVar"), value) {}
	~STUConfigVar_Custom();
};