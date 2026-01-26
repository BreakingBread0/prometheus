#include "STU.h"
#include "STU_Editable.h"
#include "stu_resources.h"
#include "AtlasExt/Utility/Modules.h"

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
		auto instance = (STUBase<>*) header->Info->CreateInstance();
		if (instance && instance->vfptr) {
			/*	printf("Invalid instance (%x): %p\n", header->info->name_hash, instance);
			}
			else {*/
			vfptr_addresses.emplace(instance->vfptr);
		}
		//instance->vfptr->rtti.VM_Destructor((__int64)instance, true);
	}

	void initialize() {

		printf("attempting to load stu registry...\n");
		printf("Gamebase at %p  - Gamebase + 0x18f74e0 = %p\n", globals::gameBase, (void*)(globals::gameBase + 0x18f74e0));
		printf("Programbase at %p  - Programbase + 0x18f74e0 = %p\n", (void*)Atlas::Utility::Modules::ProgramBounds().Base(), (void*)Atlas::Utility::Modules::ProgramBounds().VA( 0x18f74e0));

		auto test =  *(STURegistry**)(globals::gameBase + 0x18f74e0);
		printf("STU regstest: %p\n", test);

		STURegistry* header = STURegistry::Get();
		while (header) {
			__try {
				//C2712: Cannot use __try in functions that require object unwinding
				//split the body above
				emplaceHeader(header);
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {
				printf("Failed to instantiate %x!\n", header->Info->Hash);
			}
			header = header->Next;
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