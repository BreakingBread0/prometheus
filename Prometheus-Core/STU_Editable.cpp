#include "STU_Editable.h"

#define IMPLEMENT_ACCESSOR_FUNC(return_type, name) \
return_type STU_Object:: name (uint hash) { return name (struct_info->argumentByHash(hash)); } \
return_type STU_Object:: name (const char* str_name) { return name (stringHash(str_name)); }

IMPLEMENT_ACCESSOR_FUNC(STU_Primitive, get_argument_primitive);
IMPLEMENT_ACCESSOR_FUNC(STU_Object, get_argument_object);
IMPLEMENT_ACCESSOR_FUNC(STU_ObjectList, get_argument_objectlist);
IMPLEMENT_ACCESSOR_FUNC(STU_PrimitiveList, get_argument_primitivelist);
IMPLEMENT_ACCESSOR_FUNC(STU_ResourceRefList, get_argument_resreflist);
IMPLEMENT_ACCESSOR_FUNC(STUResourceReference*, get_argument_resource);
IMPLEMENT_ACCESSOR_FUNC(STU_Map, get_argument_map);

STU_Primitive STU_Object::get_argument_primitive(STUArgumentInfo* arg) {
	owassert(arg);
	owassert(value);
	auto typ = arg->constraint->get_type_flag();
	owassert(typ == STU_ConstraintType_Enum || typ == STU_ConstraintType_Primitive);
	return STU_Primitive(valid() ? (void*)((__int64)value + arg->offset) : nullptr, arg->constraint->get_stu_type());
}

STU_Object STU_Object::get_argument_object(STUArgumentInfo* arg) {
	owassert(arg);
	auto typ = arg->constraint->get_type_flag();
	owassert(typ == STU_ConstraintType_Object || typ == STU_ConstraintType_InlinedObject);
	if (typ == STU_ConstraintType_Object)
		return STU_Object(GetSTUInfoByHash(arg->constraint->get_stu_type()), valid() ? *(void**)((__int64)value + arg->offset) : 0);
	return STU_Object(GetSTUInfoByHash(arg->constraint->get_stu_type()), valid() ? (void*)((__int64)value + arg->offset) : 0);
}

STU_ObjectList STU_Object::get_argument_objectlist(STUArgumentInfo* arg) {
	owassert(arg);
	return STU_ObjectList(arg, valid() ? (void*)((__int64)value + arg->offset) : 0);
}

STU_PrimitiveList STU_Object::get_argument_primitivelist(STUArgumentInfo* arg) {
	owassert(arg);
	return STU_PrimitiveList(arg, valid() ? (void*)((__int64)value + arg->offset) : 0);
}

STU_ResourceRefList STU_Object::get_argument_resreflist(STUArgumentInfo* arg) {
	owassert(arg);
	return STU_ResourceRefList(arg, valid() ? (void*)((__int64)value + arg->offset) : 0);
}

STUResourceReference* STU_Object::get_argument_resource(STUArgumentInfo* arg) {
	owassert(arg);
	auto typ = arg->constraint->get_type_flag();
	owassert(typ == STU_ConstraintType_ResourceRef || typ == STU_ConstraintType_NonSTUResourceRef);
	return valid() ? (STUResourceReference*)((__int64)value + arg->offset) : nullptr;
}

STU_Map STU_Object::get_argument_map(STUArgumentInfo* arg) {
	owassert(arg);
	return STU_Map(arg, (void*)((__int64)value + arg->offset));
}

void STU_Object::set_object(STUArgumentInfo* arg, STU_Object object) {
	owassert(arg);
	auto typ = arg->constraint->get_type_flag();
	owassert(typ == STU_ConstraintType_Object);
	owassert(valid());
	owassert(object.valid());
	*(void**)((__int64)value + arg->offset) = object.value;
}
void STU_Object::set_object(uint name_hash, STU_Object object) {
	set_object(struct_info->argumentByHash(name_hash), object);
}
void STU_Object::set_object(const char* str_name, STU_Object object) {
	set_object(stringHash(str_name), object);
}

//initializes all lists
void STU_Object::initialize() {
	for (auto arg : *struct_info) {
		if (arg.second->constraint->is_list()) {
			auto list = (STUBullshitListFull<__int64>*)((__int64)value + arg.second->offset);
			if (list->flag == 0) {
				list->flag = 2;
				list->list_ptr = (STUBullshitListPtr*)ow_memalloc(sizeof(STUBullshitListPtr*));
			}
		}
		if (arg.second->constraint->get_type_flag() == STU_ConstraintType_InlinedObject)
			get_argument_object(arg.second).initialize();
	}
}

void STU_Object::deallocate() {
	if (valid()) {
		auto stu = (STUBase<>*)value;
		stu->vfptr_stubase->rtti.destructor((__int64)stu, 1);
		value = nullptr;
	}
}

void STU_Object::initialize_configVar() {
	owassert(struct_info->assignable_to_hash(stringHash("STUConfigVar")));
	auto getStatescriptImpl_fn = (__int64(*)(__int64 unused, InheritanceInfo* stucv))(globals::gameBase + 0xf435c0);
	auto impl_value = get_argument_primitive(0x83e83924); //Special for ConfigVars: implementation classes get put into this special variable during initialization
	auto impl = getStatescriptImpl_fn(0, struct_info->rtti_info);
	if (!impl)
		impl = getStatescriptImpl_fn(0, GetSTUInfoByHash(stringHash("STUConfigVar"))->rtti_info); //Thats the default behaviour
	impl_value.set_value(impl);
}

STU_Object STU_Object::create(STUInfo* struct_info) {
	owassert(struct_info->create_instance_fn);
	STU_Object result(struct_info, (void*)struct_info->create_instance_fn());
	result.initialize();
	return result;
}
