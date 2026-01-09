#include "STUConfigVar_Custom.h"
#include "STU_Editable.h"

class STUConfigVar_impl_Custom {
	virtual void stu_1() {}
	virtual void stu_2() {}
	virtual void stu_3() {}
	virtual void stu_4() {}
	virtual void stu_5() {}
	virtual void stu_6() {}
	virtual void stu_7() {}
	virtual void stu_8() {}
	virtual void stu_9() {}
	virtual void stu_10() {}
	virtual void stu_11() {}
	virtual void stu_12() {}
	virtual void stu_13() {}
	virtual void stu_14() {}
	virtual void stu_15() {}
	virtual void stu_16() {}
	virtual void stu_17() {}
	virtual void stu_18() {}
	virtual void stu_19() {}
	virtual void stu_20() {}
	virtual void stu_21() {}
	virtual void stu_22() {}
	virtual void stu_23() {}
	virtual void stu_24() {}
	virtual void stu_25() {}
	virtual void stu_26() {}

	virtual char GetConfigVarValue(StatescriptInstance* ss, STUConfigVar* value_stu, StatescriptPrimitive* getter) {
		*getter = _value;
		return 1;
	}
	virtual char AmIAccessingRemoteEntities() {
		return true;
	}
public:
	STUConfigVar_impl_Custom(StatescriptPrimitive value) : _value(value) {}
private:
	StatescriptPrimitive _value;
};

STUConfigVar* STUConfigVar_Custom::get() {
	return (STUConfigVar*)_instance->value;
}

STUConfigVar_Custom::STUConfigVar_Custom(STUInfo* info, StatescriptPrimitive value) : 
	_instance(STU_Object::createNew(info)), 
	_getter(new STUConfigVar_impl_Custom(value)) {
	owassert(info);
	_instance->initialize_configVar();
	_instance->get_argument_primitive(0x83e83924).set_value((__int64)&_getter);
}

STUConfigVar_Custom::~STUConfigVar_Custom() {
	_instance->deallocate();
	delete _instance;
	delete _getter;
}