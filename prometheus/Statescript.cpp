#include "Statescript.h"
#include "STU_Editable.h"
#include "statescript_logger.h"

bool StatescriptPrimitive::is_truthy() {
	auto cv_isTruthy_fn = (char(*)(StatescriptPrimitive*))(globals::gameBase + 0xee97b0);
	return cv_isTruthy_fn(this);
}

Vector3 StatescriptPrimitive::get_convert_vec3(StatescriptInstance* ss, Vector3 default_value) {
	Vector3 result = default_value;
	if (type == StatescriptPrimitive_ENTITY && ss) {
		ss->vfptr->GetEntityPos(ss, (uint32)value, &result);
	}
	else if (type == StatescriptPrimitive_VEC3) {
		memcpy(&result, &value, sizeof(Vector3));
	}
	return result;
}
Vector3 StatescriptPrimitive::get_vec3() {
	Vector3 result{};
	if (type == StatescriptPrimitive_VEC3) {
		memcpy(&result, &value, sizeof(Vector3));
	}
	return result;
}

float StatescriptPrimitive::get_convert_float(float default_value) {
	switch (type) {
	case StatescriptPrimitive_BYTE:
		return value ? 1.f : 0;
	case StatescriptPrimitive_INT:
		return (float)(int)value;
	case StatescriptPrimitive_FLT:
		return *(float*)&value;
	default:
		return default_value;
	}
}

inline StructPageAllocator* CV_StringBuffer() {
	return *(StructPageAllocator**)(globals::gameBase + 0x17b9a20);
}

void StatescriptPrimitive::reset() {
	if (type == StatescriptPrimitive_STRING) {
		auto buf = CV_StringBuffer();
		buf->vfptr->instance_unlink(buf, (void*)value);
	}
	type = StatescriptPrimitive_BYTE;
	value = 0;
}

std::string StatescriptPrimitive::get_type_str() {
	return get_type_str(type);
}

std::string StatescriptPrimitive::get_type_str(StatescriptPrimitive_Type type) {
	switch (type) {
	case StatescriptPrimitive_BYTE:
		return "CV_TYPE_BYTE";
	case StatescriptPrimitive_INT:
		return "CV_TYPE_INT";
	case StatescriptPrimitive_INT64:
		return "CV_TYPE_INT64";
	case StatescriptPrimitive_STRING:
		return "CV_TYPE_STRING";
	case StatescriptPrimitive_INTVEC4:
		return "CV_TYPE_INTVEC4";
	case StatescriptPrimitive_VEC3:
		return "CV_TYPE_VEC3";
	case StatescriptPrimitive_SS_INSTANCEID:
		return "CV_TYPE_SS_INSTANCEID";
	case StatescriptPrimitive_ENTITY:
		return "CV_TYPE_ENTITY";
	case StatescriptPrimitive_FLT:
		return "CV_TYPE_FLT";
	case StatescriptPrimitive_FLT16VEC4:
		return "CV_TYPE_FLT16VEC4";
	default:
		return "UNKNOWN (" + std::to_string(type) + ")";
	}
}

std::string StatescriptPrimitive::get_value_str() {
	if (!this) {
		return "NULL";
	}
	if (type == 0 || type >= StatescriptPrimitive_COUNT) {
		return "INVALID (" + std::to_string(type) + ")";
	}
	char buf[128]{};
	typedef void (*fn)(StatescriptPrimitive* cv_value, char* out_buf, int out_buf_size, int typ_m1);
	fn func = (fn)(globals::gameBase + 0xeea070);
	func(this, buf, sizeof(buf), type - 1);
	return buf;
}

void CV_CloneItemCopy(StatescriptPrimitive* to, StatescriptPrimitive* from) {
	auto func = (void(__fastcall*)(StatescriptPrimitive*, StatescriptPrimitive*))(globals::gameBase + 0xeea470);
	func(to, from);
}

StatescriptPrimitive StatescriptPrimitive::clone() {
	StatescriptPrimitive result{};
	CV_CloneItemCopy(&result, this);
	return result;
}

std::map<__int64, std::vector<STUGraphPlug*>> STUStatescriptBase::get_output_plugs() {
	std::map<__int64, std::vector<STUGraphPlug*>> result{};
	for (auto arg_it : *this->graph_node.base.vfptr->GetSTUInfo()) {
		auto arg = arg_it.second;
		auto arg_typ = arg->constraint->get_type_flag();
		if (arg_typ == STU_ConstraintType_Object || arg_typ == STU_ConstraintType_BSList_Object) {
			auto arg_info = GetSTUInfoByHash(arg->constraint->get_stu_type());
			if (arg_info->assignable_to_hash(STU_NAME::STUStatescriptOutputPlug)) {
				std::vector<STUGraphPlug*> plugs{};
				if (arg_typ == STU_ConstraintType_Object) {
					plugs.push_back((STUGraphPlug*)graph_node.base.to_editable().get_argument_object(arg).value);
				}
				else {
					for (auto item : graph_node.base.to_editable().get_argument_objectlist(arg)) {
						plugs.push_back((STUGraphPlug*)item.value);
					}
				}
				result[arg->name_hash] = plugs;
			}
		}
	}
	return result;
}


void StatescriptInstance::ExecuteNode(int m_nodes_index) {
	auto node = graph->m_nodes.list()[m_nodes_index];
	auto type = node->get_type();
	if (type == StatescriptNodeType_Action) {
		StatescriptAction* impl = GetStatescriptActionImplNode(this, (STUStatescriptAction*)node);
		if (!impl) {
			printf("Could not find action impl node\n");
		}
		else {
			impl->vfptr->entry_base.ActionOrEntry_ExecuteAndContinue(&impl->base, nullptr, this, node);
		}
	}
	else {
		StatescriptNotification_TraverseFrom* trav = (StatescriptNotification_TraverseFrom*)ss_inner->vfptr->Allocate_StatescriptE(ss_inner, ETYPE_TRAVERSEFROM);
		trav->base.m_timestamp = ss_inner->cf_timestamp;
		trav->base.m_instanceId = instance_id;
		trav->m_uBaseIndex = node->idx_in_nodes;
		statescript_logger::TickScript(this, (StatescriptNotification_Base*)trav);
	}
}

void StatescriptInstance::StartStopState(int m_nodes_index, bool start) {
	auto node = graph->m_nodes.list()[m_nodes_index];
	StatescriptNotification_Lifetime* trav = (StatescriptNotification_Lifetime*)ss_inner->vfptr->Allocate_StatescriptE(ss_inner, ETYPE_LIFETIME);
	trav->base.m_timestamp = ss_inner->increasing_counter3;
	trav->base.m_instanceId = instance_id;
	trav->m_uStateIndex = graph->m_states.indexof((STUStatescriptState*)node);
	trav->m_uLifetimeType = !start;
	statescript_logger::TickScript(this, (StatescriptNotification_Base*)trav);
}

void StatescriptInstance::ToggleState(int m_nodes_index) {
	auto log = statescript_logger::GetScriptNodeState(this, m_nodes_index);
	StartStopState(m_nodes_index, log.is_finish_state);
}
