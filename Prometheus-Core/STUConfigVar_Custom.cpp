#include "STUConfigVar_Custom.h"
#include "STU_Editable.h"

class STUConfigVar_impl_Custom {
	virtual void stu_1() { owassert(false); }
	virtual void stu_2() { owassert(false); }
	virtual void stu_3() { owassert(false); }
	virtual void stu_4() { owassert(false); }
	virtual void stu_5() { owassert(false); }
	virtual void stu_6() { owassert(false); }
	virtual void stu_7() { owassert(false); }
	virtual void stu_8() { owassert(false); }
	virtual void stu_9() { owassert(false); }
	virtual void stu_10() { owassert(false); }
	virtual void stu_11() { owassert(false); }
	virtual void stu_12() { owassert(false); }
	virtual void stu_13() { owassert(false); }
	virtual void stu_14() { owassert(false); }
	virtual void stu_15() { owassert(false); }
	virtual void stu_16() { owassert(false); }
	virtual void stu_17() { owassert(false); }
	virtual void stu_18() { owassert(false); }
	virtual void stu_19() { owassert(false); }
	virtual void stu_20() { owassert(false); }
	virtual void stu_21() { owassert(false); }
	virtual void stu_22() { owassert(false); }
	virtual void stu_23() { owassert(false); }
	virtual void stu_24() { owassert(false); }
	virtual void stu_25() { owassert(false); }
	virtual void stu_26() { owassert(false); }

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

STU_Object STUConfigVar_Custom::get_editable() {
	return *_instance;
}

STUConfigVar_Custom::STUConfigVar_Custom(STUInfo* info, StatescriptPrimitive value) : 
	_instance(STU_Object::createNew(info)), 
	_getter(new STUConfigVar_impl_Custom(value)) {
	owassert(info);
	_instance->initialize_configVar();
	_instance->get_argument_primitive(0x83e83924).set_value((__int64)_getter);
}

STUConfigVar_Custom::~STUConfigVar_Custom() {
	_instance->deallocate();
	delete _instance;
	delete _getter;
}