#include "STUConfigVarDynamic_Custom.h"
#include "STU_Editable.h"

STUConfigVarDynamic_Custom::STUConfigVarDynamic_Custom(std::vector<__int64> ids) {
	GetSTUInfoByHash(stringHash("STUConfigVarDynamic"))->clear_instance_fn((__int64)this);
	//printf("%p\n", cv_base.base.vfptr);
	cv_base.base.to_editable().initialize_configVar();
	list = new fake_list;
	auto count = ids.size();
	list->list = new STUResourceReference[count];
	list->count = count;
	for (int i = 0; i < count; i++) {
		list->list[i].resource_id = ids[i];
		list->list[i].resource_load_entry = (MisalignedResourceLoadEntry*)-1;
	}
	static_list_flag = 1;
}