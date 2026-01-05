#include "STU.h"
#include "STU_Editable.h"
#include "stu_resources.h"

namespace STU_NAME {
	namespace Primitive {
		std::set<uint32> _all = {
			teMtx43A,
			teVec3A,
			teVec2,
			teVec3,
			teVec4,
			teQuat,
			teColorRGB,
			teColorRGBA,
			teStructuredDataDateAndTime,

			DBID,
			teUUID,
			teString,

			s16,
			s32,
			s64,

			u8,
			u16,
			u32,
			u64,

			f32,
			f64,
		};
	}
}

namespace STURegistryData {
	std::set<STUBase_vt*> vfptr_addresses{};

	void emplaceHeader(STURegistry* header)
	{
		auto instance = header->info->create_instance_fn();
		if (instance && instance->vfptr) {
			/*	printf("Invalid instance (%x): %p\n", header->info->name_hash, instance);
			}
			else {*/
			vfptr_addresses.emplace(instance->vfptr);
		}
		//instance->vfptr->rtti.VM_Destructor((__int64)instance, true);
	}

	void initialize() {
		STURegistry* header = GetSTURegistry();
		while (header) {
			__try {
				//C2712: Cannot use __try in functions that require object unwinding
				//split the body above
				emplaceHeader(header);
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {
				printf("Failed to instantiate %x!\n", header->info->name_hash);
			}
			header = header->next;
		}
		printf("STU count: %d\n", vfptr_addresses.size());
	}


}

template<>
STU_Object STUBase<>::to_editable() {
	return STU_Object(vfptr_stubase->GetSTUInfo(), this);
}


STUBase<>* STUResourceReference::get_STU() const {
	return stu_resources::GetByID(resource_id);
}