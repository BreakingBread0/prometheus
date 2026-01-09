#include "stu_resources.h"
#include "STU_Editable.h"

__int64 stu_resources::STUD_ConstructFromResource(ResourceLoadEntry* entry) {
	__int64 result = stud_construct_orig(entry);
	_constructed_stus.emplace(entry->resource_id, (STUBase<>*)result);
	if (globals::isDemo) {
		if (entry->resource_id == 0x9a00000000000b2) {
			//auto uxscreen = ((STUBase<>*)result)->to_editable();
			//auto values = uxscreen.get_argument_objectlist("m_values");
			//auto play_btn = values[18].get_runtime_root();
			//if (play_btn.valid()) {
			//	auto res = play_btn.get_argument_resource("m_displayText")->resource_id;
			//	SetUXDisplayString(res, "Welcome Back, Hero!");
			//	//play_btn.get_argument_primitive("m_value").set_value("Welcome Back, Hero!");
			//}
		}
	}
	if (entry->resource_id == 0x09A00000000001FB) {
		//STUUX for health pack, doesnt work properly, just disable it for the demo ^^
		entry->resource_state = 11;
		return 0;
	}
	return result;
}

//bool __declspec(noinline) test123(ResourceLoadEntry* entry) {
//	if (globals::isDemo) {
//		if (entry->resource_id == 0xde00000000005df) { //PLAY
//			printf("here\n");
//		}
//	}
//	return false;
//}

__int64 stu_resources::DisplayText_ConstructFromResource(ResourceLoadEntry* entry) {
	/*if (test123(entry))
		return 1;*/
	return DisplayText_construct_orig(entry);;
	
}

//