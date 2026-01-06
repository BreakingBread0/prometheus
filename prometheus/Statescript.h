#pragma once

#include <vector>
#include <string>
#include <format>
#include <iostream>
#include "STU.h"
#include "game.h"
#include <map>
#include <set>
#include "entity_admin.h"
#include "idadefs.h"

struct StatescriptInstance;
struct StatescriptVar_VarBag;
struct InheritanceInfo;
struct STUGraphLink;
struct StatescriptSyncMgr;
struct STUStatescriptBase;
struct StatescriptPrimitive;
struct Component_23_Statescript;
struct StatescriptVar_Base;
struct STUConfigVar_impl;
struct STUConfigVar;
struct StatescriptState;

enum StatescriptPrimitive_Type // 4 bytes
{                                       // XREF: SS_CV/r
	StatescriptPrimitive_BYTE = 0x1,
	StatescriptPrimitive_INT = 0x2,
	StatescriptPrimitive_FLT = 0x3,
	StatescriptPrimitive_INT64 = 0x4,
	StatescriptPrimitive_STRING = 0x5,
	StatescriptPrimitive_VEC3 = 0x6,
	StatescriptPrimitive_INTVEC4 = 0x7,        // confirmed
	StatescriptPrimitive_SS_INSTANCEID = 0x8,
	StatescriptPrimitive_FLT16VEC4 = 0x9,
	StatescriptPrimitive_ENTITY = 0xB,
	StatescriptPrimitive_COUNT = 0xC //not a type
};

struct StatescriptPrimitive {
	__int64 value;
	__int32 field_8;
	StatescriptPrimitive_Type type;

	std::string get_type_str();
	std::string get_value_str();
	StatescriptPrimitive clone();
	void reset();
	float get_convert_float(float default_value = 0.f);
	Vector3 get_convert_vec3(StatescriptInstance* ss, Vector3 default_value = {});
	Vector3 get_vec3();
	bool is_truthy();

	static std::string get_type_str(StatescriptPrimitive_Type type);
};

//start is at 0x18f8e88
struct StatescriptRTTI
{
	StatescriptRTTI* next_instance;
	__int64 field_8;                      ///< reference to factory func?
	__int64* (*factory_func)(); //Factory function for the implementation class.
	InheritanceInfo* stuclass_inherit; //Inheritance for the STU data.
	__int32 field_20;                     ///< zero
	__int32 field_24;
	__int64 field_28;                     ///< 0x1854d20 bei stustatescriptstatewait
	///< int ref f�r statistics?
};

struct StatescriptInstanceListWithId {
	StatescriptInstance* instance;
	short instance_id;
};

//called "user" internally
//
struct RID_User // sizeof=0x8
{
	__int16 ss_instance_id;
	__int16 ss_instance_id_2;
	__int16 resid; //lowbyte: stack state if its not -1, hi: resource id?
	__int16 field_6; //is currently storing into stack state
};

struct StatescriptPrimitive_Array;
struct StatescriptPrimitive_ArrayItem;
struct StatescriptNotification_LogicalButton;
struct STUStatescriptState;
struct StatescriptEntry;
struct StatescriptAction;

struct StatescriptEntry_vt {
	STUBase_vt stu_base;
	/*000000D0*/ char(__fastcall* TraverseTransitionPlug)(__int64 _this, StatescriptInstance* ss, StatescriptEntry* stu_data);
	/*000000D8*/ const char* (__fastcall* GetName)(__int64, __int64 stu_instance); // Die 2. Funktion scheint mehr ids zu haben dies returnen kann
	/*000000E0*/ bool(__fastcall* GetName2)(__int64, _DWORD* stu_instance);
	/*000000E8*/ void(__fastcall* field_13)();
	/*000000F0*/ char (*Condition_IsFullfilled)(StatescriptEntry* this_node, StatescriptEntry* old_node, StatescriptInstance*, STUStatescriptBase*);
	/*000000F8*/ __int64(__fastcall* ActionOrEntry_ExecuteAndContinue)(StatescriptEntry* this_node, StatescriptEntry* old_node, StatescriptInstance*, STUStatescriptBase*); // Execute and continue execution with next node... hat eigentlich 5 Argumente, HUH?
};

struct StatescriptAction_vt {
	/*00000000*/ StatescriptEntry_vt entry_base;
	/*00000100*/ char(__fastcall* StartActionExecution)(StatescriptAction_vt** impl, StatescriptState* state, StatescriptInstance* ss, StatescriptAction* stu_data);
	/*00000108*/ void(__fastcall* field_15)();
	/*00000110*/ char (*OnBeginState)(StatescriptState*); // schedules timeout expiry in StateWait
	/*00000118*/ char (*OnStateReentryMby)();
	/*00000120*/ char (*Update1Tick)();
};

struct StatescriptState_vt {
	StatescriptAction_vt action_base;
	/*00000128*/ void(__fastcall* field_19)();
	/*00000130*/ void(__fastcall* field_21)();
	/*00000138*/ void(__fastcall* OnTick_mby)(StatescriptState*, __int64 commandFrame);
	/*00000140*/ void(__fastcall* OnTimerTick)(StatescriptState*, int timerUserData);
	/*00000148*/ void(__fastcall* field_23)(); // called in DoFinishState
	/*00000150*/ void(__fastcall* field_150)(__int64);
	/*00000158*/ void(__fastcall* field_24)(); // called in DoFinishState & DoAbortState
	/*00000160*/ void(__fastcall* PreBeginState)(StatescriptState*, int); // called in DoBeginState mit flag 0x1000
	/*00000168*/ void(__fastcall* PreFinishState)(StatescriptState*, int); // called in DoFinishState, arg1: 1 or 6 if script_flags & x1000
	/*00000170*/ void(__fastcall* OnAttributeDependencyChanged)(StatescriptState*, StatescriptVar_Base*); // a2 = 0
	/*00000178*/ void(__fastcall* OnStackStateChanged)(__int64);
	/*00000180*/ void(__fastcall* OnDependencyChanged)(StatescriptState*, StatescriptVar_Base*, RID_User*);
	/*00000188*/ __int64 (*field_29)();
	/*00000190*/ _QWORD* (__fastcall* GetStateRuntime)(__int64, _QWORD*);
	/*00000198*/ _QWORD* (__fastcall* field_198)(__int64, _QWORD*, __int64);
	/*000001A0*/ void(__fastcall* field_30)();
	/*000001A8*/ __int64(__fastcall* GetStateNameStruct)(StatescriptState*, STUStatescriptState*); // call if script_flags & 8 != 0
	/*000001B0*/ char (*field_31)(); // state stack: 1
	/*000001B8*/ void(__fastcall* OnAimEntityChanged)(StatescriptState*, DWORD* controller_ent);
	/*000001C0*/ void(__fastcall* field_33)();
	/*000001C8*/ void(__fastcall* OnLogicalButtonChanged)(StatescriptState*, StatescriptNotification_LogicalButton* logical_button);
	/*000001D0*/ __int64(__fastcall* ScheduleBeginState)(__int64, __int64 timestamp);
	/*000001D8*/ __int64(__fastcall* ScheduleAbortState)(__int64, __int64 timestamp);
	/*000001E0*/ _QWORD* (__fastcall* field_1E0)(__int64, _QWORD*, __int64);
	/*000001E8*/ __int64(__fastcall* EvaluateExits)(__int64);
	/*000001F0*/ __int64(__fastcall* bsDataStore_WriteData)(__int64, /*bsDataStore*/__int64*, char);
	/*000001F8*/ bool(__fastcall* bsDataStore_readData)(__int64, /*bsDataStore*/__int64*, char, char);
	/*00000200*/ void(__fastcall* field_35)();
	/*00000208*/ void(__fastcall* field_36)();
	/*00000210*/ __int64 (*needs_clientonly_update)();
	/*00000218*/ void(__fastcall* SubscribeToVariables)(StatescriptState*); // Called in BeginState
	/*00000220*/ void* (__fastcall* UnsubscribeFromVariables)(StatescriptState*); // OnAttributeDependencyChanged_SIRef_FromCVD, aber auch called in DoFinishState etc?
	/*00000228*/ __int64 (*OnStackStateValueEmplaced)(StatescriptState*, RID_User, StatescriptPrimitive*); // hat was mit RID zu tun 0xf5e92b, GetStateDefinedValue?
	/*00000230*/ int (*GetStateDefinedValueArray)(StatescriptState*, RID_User current, StatescriptPrimitive_Array** result);
	/*00000238*/ __int64 (*GetStateDefinedValueIterator)(StatescriptState*, RID_User, StatescriptPrimitive_ArrayItem** value_inout, DWORD* count);
	/*00000240*/ char (*field_41)(); // returns 0 StateSwitch
	/*00000248*/ void(__fastcall* field_42)();
	/*00000250*/ void(__fastcall* field_43)();
};

struct StatescriptEntry {
	StatescriptEntry_vt* vfptr;
};

struct StatescriptAction {
	union {
		StatescriptEntry base;
		StatescriptAction_vt* vfptr;
	};
};

struct StatescriptState {
	union {
		STRUCT_MIN_SIZE(0xB8);
		StatescriptAction base;
		StatescriptState_vt* vfptr;
		STRUCT_PLACE(__int64, field_10, 0x10);
		STRUCT_PLACE(__int64, field_30, 0x30);
		STRUCT_PLACE(int, state_flags, 0x40);
		STRUCT_PLACE(short, field_44, 0x44);
		STRUCT_PLACE(__int64, field_48, 0x48);
		STRUCT_PLACE(__int64, field_50, 0x50);
		STRUCT_PLACE(__int64, field_58, 0x58);
		STRUCT_PLACE(__int64, field_60, 0x60);
		STRUCT_PLACE(__int64, field_68, 0x68);
		STRUCT_PLACE(int, idx_in_m_states, 0x70);
		STRUCT_PLACE(int, field_74, 0x74);
		STRUCT_PLACE(StatescriptInstance*, ss_instance, 0x78);
		STRUCT_PLACE(char, field_80, 0x80);
		STRUCT_PLACE(teList<__int64>, list_88, 0x88);
		STRUCT_PLACE(__int64, field_98, 0x98);
		STRUCT_PLACE(__int64, field_A0, 0xA0);
		STRUCT_PLACE(__int64, vtable_ret0_1, 0xA8);
		STRUCT_PLACE(__int64, vtable_ret0_2, 0xB0);
	};
};

//
// Statescript Graph
//

struct STUGraphExtents {
	union {
		STUBase<STUBase_vt> stu_base;
		STRUCT_MIN_SIZE(0x18);
	};
};

struct STUGraph {
	union {
		STRUCT_MIN_SIZE(0x48);
		STUBase<STUBase_vt> stu_base;
		STRUCT_PLACE(STUGraphExtents, graph_extents, 8);
		STRUCT_PLACE(__int64, field_28, 0x28);
		STRUCT_PLACE(__int64, field_38, 0x38);
	};
};

struct STUGraphNode {
	union {
		STRUCT_MIN_SIZE(0x38);
		STUBase<STUBase_vt> base;
		STRUCT_PLACE(__int64, m_displayName, 8);
		STRUCT_PLACE(__int64, m_comment, 0x18);
		STRUCT_PLACE(__int64, m_pos, 0x28);
		STRUCT_PLACE(__int64, m_uniqueId, 0x30);
	};
};

enum StatescriptNodeType {
	StatescriptNodeType_Unknown,
	StatescriptNodeType_State,
	StatescriptNodeType_Action,
	StatescriptNodeType_Condition,
	StatescriptNodeType_Entry,
	StatescriptNodeType_Other
};

struct STUGraphPlug {
	STUBase<STUBase_vt> base;
	STUBullshitListFull<STUGraphLink*> m_links;
	STUStatescriptBase* m_parentNode;
	__int64 arg1;
};

struct STUStatescriptOutputPlug {
	STUGraphPlug plug_base;
};

struct StatescriptSubgraphPlug {
	STUStatescriptOutputPlug output_plug;
	__int64 arg1;
};

struct STUStatescriptInputPlug {
	STUGraphPlug plug_base;
};

struct STUGraphLink {
	STUBase<STUBase_vt> base;
	STUStatescriptOutputPlug* m_outputPlug;
	STUStatescriptInputPlug* m_inputPlug;
};

struct STUStatescriptBase{
	union {
		STRUCT_MIN_SIZE(0x78);
		STUGraphNode graph_node;
		STRUCT_PLACE(__int64, m_debugText, 0x38);
		STRUCT_PLACE(__int64, arg1, 0x48);
		STRUCT_PLACE(__int64, arg2, 0x58);
		STRUCT_PLACE(int, idx_in_nodes, 0x68);
		STRUCT_PLACE(int, arg4, 0x6C);
		STRUCT_PLACE(int, idx_in_states, 0x70); //FALSCH
		STRUCT_PLACE(char, m_isState, 0x74);
		STRUCT_PLACE(char, m_clientOnly, 0x75);
		STRUCT_PLACE(char, m_serverOnly, 0x76);
	};

	StatescriptNodeType get_type() {
		if (this->graph_node.base.vfptr->rtti.base.rtti_assignable_to((__int64)this, globals::gameBase + STU_RTTI::STUStatescriptGameEntry)) {
			return StatescriptNodeType_Entry;
		} else if (this->graph_node.base.vfptr->rtti.base.rtti_assignable_to((__int64)this, globals::gameBase + STU_RTTI::STUStatescriptActionTerminator)) {
			return StatescriptNodeType_Action;
		} else if (this->graph_node.base.vfptr->rtti.base.rtti_assignable_to((__int64)this, globals::gameBase + STU_RTTI::STUStatescriptState)) {
			return StatescriptNodeType_State;
		} else if (this->graph_node.base.vfptr->rtti.base.rtti_assignable_to((__int64)this, globals::gameBase + STU_RTTI::STUStatescriptCondition)) {
			return StatescriptNodeType_Condition;
		} else if (this->graph_node.base.vfptr->rtti.base.rtti_assignable_to((__int64)this, globals::gameBase + STU_RTTI::Unknown)) {
			return StatescriptNodeType_Unknown;
		}
		return StatescriptNodeType_Unknown;
	}

	std::map<__int64, std::vector<STUGraphPlug*>> get_output_plugs();
};

struct STUStatescriptActionTerminator {
	STUStatescriptBase ss_base;
	__int64 m_inPlug;
};

struct STUStatescriptAction {
	STUStatescriptActionTerminator action_terminator;
	STUStatescriptOutputPlug* out_plug;
};

struct STUConfigVar_impl_vt {
	STUBase_vt base;
	char(__fastcall* GetConfigVarValue)(STUConfigVar_impl*, StatescriptInstance*, STUConfigVar*, StatescriptPrimitive*);
	char (*ret_0_0)();
};

struct STUConfigVar_impl {
	STUConfigVar_impl_vt* vfptr;
};

struct STUConfigVar {
	STUBase<STUBase_vt> base;
	STUConfigVar_impl* cv_impl;

	bool is_dynamic() {
		return base.vfptr->rtti.base.rtti_assignable_to((__int64)this, globals::gameBase + STU_RTTI::STUConfigVarDynamic);
	}
	
	bool is_expression() {
		return base.vfptr->GetSTUInfo()->assignable_to_hash(STU_NAME::STUConfigVarExpression);
	}

	char get_value(StatescriptInstance* ss, StatescriptPrimitive* out_var) {
		__ow_try (EXCEPTION_EXECUTE_HANDLER)  {
			return cv_impl->vfptr->GetConfigVarValue(cv_impl, ss, this, out_var);
		}
		__ow_except {
			printf("CV read failed %p!\n", ss);
		}
	}
};

struct STUConfigVarDynamic {
	STUConfigVar cv_base;
	STUBullshitListFull<STUResourceReference> item_location;
};

struct STUConfigVarDynamic_Editable {
	struct fake_list {
		STUResourceReference* list;
		int count;
	};

	STUConfigVar cv_base;
	fake_list* list;
	int static_list_flag = 1;

	STUConfigVarDynamic_Editable(std::vector<__int64> ids);

	~STUConfigVarDynamic_Editable() {
		if (list) {
			delete list;
			delete[] list->list;
			list = nullptr;
		}
	}

	STUConfigVarDynamic* get() {
		return (STUConfigVarDynamic*)this;
	}
};

//TED specific? Unused in the CV getter...
struct STUConfigVarExpressionFragment {
	__int64 m_text;
	__int64 m_text_2;
	STUResourceReference m_guid;
};

//sz: 0x68
struct STUConfigVarExpression {
	/// OPCODES:
	/// execution vars:
	/// * entid_temp: temp for searching
	/// * TVA: TempValueArray (SS_CV[8]), index starts at -1
	/// funcs:
	/// * searchDynamicArr: take entid_temp, value (array), search_value -> search in array / indexof if search_value is int/float/byte (32 is for dynamic, 37 is for cv)
	/// * searchArr: take value (array), search_value -> search in array / indexof if search_value is int/float/byte
	/// * interceptor: StatescriptInstance->CVD_Satisfies_InterceptorArr, returns CV
	/// * count: Count m_ConfigVars iterable / array
	/// * getpos: Get Position if CV is Entity or CV, else get value from char/int/float
	/// * floor: float floor
	/// * powf: x to the power of y
	/// * inverse_sqrt: was der name sagt
	/// * sinf, cosf, cross_product, clamp, fminf, fmaxf: yup
	/// * deg2rad: degree zu radian angle
	/// * vec_len: vector length or converts number to positive if float/int/char
	///
	/// 0: Graceful Exit, TVA to results
	/// 1: opcarr = opcarr[1], ExitCheck
	/// 2: Jump opcarr[1] if TVA Falsey else TVA--
	/// 3: Jump opcarr[1] if TVA Truthy else TVA--
	/// 4: Jump opcarr[1] if TVA Falsey, TVA--
	/// 5: Jump opcarr[1] if TVA Truthy, TVA--
	/// 30: TVA++, index = opcarr[1], value = get m_configVar[index], TVA = value, cache
	/// 31: TVA++, index = opcarr[1], value = get m_dynamicVars[index], TVA = value, cache
	/// 32: search_temp = TVA, index = opcarr[i], value = m_dynamicVars[index], TVA = searchDynamicArr(entid_temp, value, search_temp)->value
	/// 33: search_temp = TVA, index = opcarr[i], value = m_dynamicVars[index], TVA = searchDynamicArr(entid_temp, value, search_temp)->key
	/// 34: search_temp = TVA, index = opcarr[i], value = m_dynamicVars[index], TVA = searchDynamicArr(entid_temp, value, search_temp)->array_index
	/// 35 (1): entid_temp = TVA, TVA--
	/// 36: TVA++, index = opcarr[1], TVA = interceptor(m_dynamicVars[index])
	/// 37: search_temp = TVA, index = opcarr[i], value = m_configVars[index], TVA = searchArr(value, search_temp)->value
	/// 38: search_temp = TVA, index = opcarr[i], value = m_configVars[index], TVA = searchArr(value, search_temp)->key
	/// 39: search_temp = TVA, index = opcarr[i], value = m_configVars[index], TVA = searchArr(value, search_temp)->array_index
	/// 40: TVA++, index = opcarr[i], TVA = count(m_configVars[index])
	/// 70: TVA++, index = opcarr[i], TVA = float_constants[index]
	/// 100 (1): value1 = TVA, TVA--, value2 = TVA, TVA = getpos(value2) + getpos(value1)
	/// 101 (1): value1 = TVA, TVA--, value2 = TVA, TVA = getpos(value2) - getpos(value1)
	/// 102 (1): value1 = TVA, TVA--, value2 = TVA, TVA = getpos(value2) * getpos(value1)
	/// 103 (1): value1 = TVA, TVA--, value2 = TVA, TVA = getpos(value2) / getpos(value1) (div0 safe)
	/// 104 (1): value1 = TVA, TVA--, value2 = TVA, TVA = value2 - (value1 * floor(value2 / value1)) -> remainder calculation, no vector support
	/// 105 (1): value1 = TVA, TVA--, value2 = TVA, TVA = powf(value2, value1) -> no vector support
	/// 106 (1): TVA = negate(TVA)
	/// 120 (1): TVA = IsFalsey(TVA)
	/// 140 (1): value1 = TVA, TVA--, value2 = TVA, TVA = value1 >= value2
	/// 141 (1): value1 = TVA, TVA--, value2 = TVA, TVA = value1 > value2
	/// 142 (1): value1 = TVA, TVA--, value2 = TVA, TVA = value2 >= value1
	/// 143 (1): value1 = TVA, TVA--, value2 = TVA, TVA = value2 > value1
	/// 144 (1): value1 = TVA, TVA--, value2 = TVA, TVA = value1 == value2
	/// 145 (1): value1 = TVA, TVA--, value2 = TVA, TVA = value1 != value2
	/// 160 (1): value1 = TVA, TVA = inverse_sqrt(value1)
	/// 161 (1): value1 = TVA, TVA = sinf(value1)
	/// 162 (1): value1 = TVA, TVA = cosf(value1)
	/// 163 (1): value1 = TVA, TVA = sinf(deg2rad(value1))
	/// 164 (1): value1 = TVA, TVA = cosf(deg2rad(value1))
	/// 165, 166 (1): value1 = TVA, TVA = vec_len(value1)
	/// 167 (1): value1 = TVA, TVA--, value2 = TVA, TVA = vec_len(value1 - value2)
	/// 168 (1): value1 = TVA, TVA--, value2 = TVA, TVA = dot_product(value1, value2)
	/// 169 (1): value1 = TVA, TVA--, value2 = TVA, TVA = cross_product(value1, value2)
	/// 170 (1): value1 = TVA, TVA = vector_normalize(value1)
	/// 171 (1): value1 = TVA, TVA--, value2 = TVA, TVA--, value3 = TVA, TVA = vec3(value3, value2, value1)
	/// 172 (1): value1 = TVA, TVA--, value2 = TVA, TVA--, value3 = TVA, TVA--, value4 = TVA, TVA = flt16vec4(value4, value3, value2, value1)
	/// 173 (1): value = TVA, TVA = value->X
	/// 174 (1): value = TVA, TVA = value->Y
	/// 175 (1): value = TVA, TVA = value->Z
	/// 200 (1): value1 = TVA, TVA--, value2 = TVA, TVA = fminf(value2, value1)
	/// 201 (1): value1 = TVA, TVA--, value2 = TVA, TVA = fmaxf(value2, value1)
	/// 202 (1): value1 = TVA, TVA--, value2 = TVA, TVA--, value3 = TVA, TVA = clamp(value3, value2, value1) -> min/max sind value1/2, spielt keine rolle wo
	/// 203 (1): value = TVA, TVA = floor(value)
	/// 204 (1): value = TVA, TVA = ceil(value)
	/// 205 (1): value = TVA, TVA = int_round(value)
	/// 206 (1): value1 = TVA, TVA--, value2 = TVA, TVA = random(value2, value1)
	/// 207 (1): value1 = TVA, TVA--, value2 = TVA, TVA = int_round(random(value2, value1) -> converts back to float
	struct STUConfigVarExpressionData {
		STUBullshitListFull<STUConfigVarExpressionFragment*> m_fragments;
		STUBullshitListFull<unsigned char> m_opcodes;
		STUBullshitListFull<STUConfigVarDynamic*> m_dynamicVars;
		STUBullshitListFull<float> float_constants;
	};

	union {
		STUConfigVar cv_base;
		STRUCT_PLACE(STUConfigVarExpressionData, m_expression, 0x10);
		STRUCT_PLACE(STUBullshitListFull<STUConfigVar*>, m_configVars, 0x58);
	};
};

struct STUStatescriptState {
	union {
		STRUCT_MIN_SIZE(0x100);
		STUStatescriptBase ss_base;
		STRUCT_PLACE(__int64, m_stateGroup, 8);
		STRUCT_PLACE(__int64, m_transitionPlug, 8);
		STRUCT_PLACE(__int64, arg1, 0x98);
		STRUCT_PLACE(__int64, arg2, 0xa8);
		STRUCT_PLACE(__int64, arg3, 0xb8);
		STRUCT_PLACE(__int64, arg4, 0xc8);
		STRUCT_PLACE(__int64, m_beginPlug, 0xd0);
		STRUCT_PLACE(__int64, m_abortPlug, 0xd8);
		STRUCT_PLACE(__int64, m_onBeginPlug, 0xe0);
		STRUCT_PLACE(__int64, m_onEndPlug, 0xe8);
		STRUCT_PLACE(__int64, m_subgraphPlug, 0xf0);
		STRUCT_PLACE(int, arg5, 0xf8);
	};
};

struct STUConfigVarStackPriority {
	STUConfigVar* base;
	STUResourceReference m_priority;
};

struct STUStatescriptStateStack {
	STUStatescriptState state;
	STUConfigVarDynamic* m_out_Var;
	STUConfigVarStackPriority* m_priority; //just a resource reference
	StatescriptSubgraphPlug* m_topSubgraphPlug;
	StatescriptSubgraphPlug* m_underSubgraphPlug;
};

struct StatescriptStateStack {
	union {
		StatescriptState state_base;
		STRUCT_PLACE(float, current_m_priority, 0xB8);
		STRUCT_PLACE(short, field_BC, 0x8C);
		STRUCT_PLACE(short, field_BE, 0xBE);
		STRUCT_PLACE(char, field_C0, 0xC0);
		STRUCT_PLACE(char, field_C4, 0xC4);
		STRUCT_PLACE(char, field_C5, 0xC5);
		STRUCT_PLACE(char, field_C6, 0xC6);
		STRUCT_PLACE(char, field_C7, 0xC7);
		STRUCT_PLACE(__int64, tick_cf_timestamp, 0xC8);
		//STRUCT_PLACE(__int64, unsubscriber_vt, 0xD0);
		STRUCT_PLACE(__int64, field_E8, 0xE8);
		STRUCT_PLACE(__int64, field_F8, 0xF8);
		STRUCT_PLACE(__int64, field_100, 0x100);
	};
	
};

struct StatescriptSyncMgr_StackStateList {
	StatescriptPrimitive cv;
	StatescriptStateStack* first_or_lowest_prio_stack;
	StatescriptStateStack* highest_priority_stack;
	int field_20;
	__int64 stack_state_index;
};

struct StatescriptSyncMgr_StackState {
	teList<StatescriptSyncMgr_StackStateList> list;
	__int64 field_10;
};

//Ja, ein SS-Graph hat 3 vtables....
struct STUStatescriptGraph {
	STUBase<STUBase_vt> base;
	STUGraph m_graph;
	STUBullshitListFull<STUStatescriptBase*> m_nodes;
	STUBullshitListFull<STUStatescriptState*> m_states; // flag f�r m_states 0xf49a06
	STUBullshitListFull<STUStatescriptBase*> m_entries;
	__int64 m_consoleCommand;
	__int64 field_88;
	__int64 arg1;                       // hash 0xCE243EEE
	__int64 field_98;
	__int64 arg2;                       // hash 0xA258A936
	__int64 field_A8;
	__int64 arg3;                       // hash 0xFE8F3A77
	__int64 field_B8;
	__int64 arg4;                       // hash 0x2AEFDE3F
	__int64 field_C8;
	__int64 arg5;                       // hash 0xE22F7341
	__int64 field_D8;
	__int64 arg6;                       // hash 0xE03B7F7B
	__int64 field_E8;
	__int64 arg7;                       // hash 0xD9B17C50
	__int64 field_F8;
	__int64 arg8;                       // hash 0x229D2B04
	__int64 field_108;
	__int64 arg9;                       // hash 0x3EFC52DE
	__int64 field_118;
	__int64 m_remoteSyncNodes;
	__int64 field_128;
	__int64 m_syncVars;
	__int64 field_138;
	__int64 arg10;                      // hash 0x6B840897
	__int64 field_148;
	__int64 arg11;                      // hash 0xEF4C042
	__int64 field_158;
	__int64 m_publicSchema;
	__int32 m_playScriptHotReloadBehavior;
	__int32 arg12;                      // hash 0xBCE4CB5
	__int32 arg13;                      // hash 0x36391723
	__int32 m_predictionBehavior;
	__int32 m_nodesBitCount;
	__int32 m_statesBitCount;
	__int32 m_remoteSyncNodesBitCount;
	__int32 m_syncVarsBitCount;
	__int32 arg14;                      // hash 0x47E36E2C
	__int32 arg15;                      // hash 0xAC1C3244
	__int32 arg16;                      // hash 0x1169D71
	__int32 arg17;                      // hash 0xFE10F816

	STUStatescriptState* STUFromState(StatescriptState* state) {
		if (state && state->idx_in_m_states < m_states.count()) {
			return m_states.list()[state->idx_in_m_states];
		}
		return nullptr;
	}

	int NodesIndex(STUStatescriptBase* state) {
		int result = m_nodes.indexof(state);
		owassert(result == state->idx_in_nodes);
		return result;
	}

	int StatesIndex(STUStatescriptState* state) {
		return m_states.indexof(state);
	}

	STUStatescriptBase** begin() {
		return m_nodes.begin();
	}

	STUStatescriptBase** end() {
		return m_nodes.end();
	}
};

struct StatescriptPrimitive_ArrayItem {
	StatescriptPrimitive key;
	StatescriptPrimitive value;

	void reset() {
		key.reset();
		value.reset();
	}
};

struct StatescriptPrimitive_Array {
	StatescriptPrimitive_ArrayItem* arr_items;
	int item_count;
	int max;
};

struct StatescriptNotification_Base;
struct StatescriptSyncMgr;

struct Statescript23ComponentInner_vt {
	union {
		STRUCT_PLACE_CUSTOM(a, 0xA0, StatescriptNotification_Base* (__fastcall* Allocate_StatescriptE)(StatescriptSyncMgr* a1, char a2));
		STRUCT_PLACE_CUSTOM(b, 0x28, void(__fastcall* StopScriptByInstanceId)(StatescriptSyncMgr* a1, short));
	};
};

struct StatescriptInstance_vt {
	//TODO This needs cleanup.
	
	//STRUCT_PLACE_CUSTOM(a, 0x1B0, void(__fastcall* StopScript)(StatescriptInstance*));
	//STRUCT_PLACE_CUSTOM(b, 0x148, void(__fastcall* EnqueueE)(StatescriptInstance*, StatescriptNotification_Base*));
	//STRUCT_PLACE_CUSTOM(c, 0xc8, char(__fastcall* CVD_Satisfies_InterceptorArr)(StatescriptInstance*, STUConfigVarDynamic*, StatescriptPrimitive*));
	//STRUCT_PLACE_CUSTOM(d, 0xd8, __int64(__fastcall* ConfigVarArr_Search)(StatescriptInstance*, STUConfigVar* input, StatescriptPrimitive* search_value, _DWORD* index_out, StatescriptPrimitive_ArrayItem* value_out));
	//STRUCT_PLACE_CUSTOM(e, 0xe0, __int64(__fastcall* ConfigVarArr_Count)(StatescriptInstance*, STUConfigVar*));
	//STRUCT_PLACE_CUSTOM(f, 0xb8, char(__fastcall* GetEntityPos)(StatescriptInstance*, uint32, Vector3*));
	//STRUCT_PLACE_CUSTOM(g, 0xe8, __int64(__fastcall* GetDynamicVar_RemoteEnt)(StatescriptInstance*, int entid, STUConfigVarDynamic*, StatescriptPrimitive*));
	/*00000000*/ _QWORD* (__fastcall* deallocate)(_QWORD*, char);
	/*00000008*/ __int64(__fastcall* deallocate_outer)(__int64);
	/*00000010*/ __int64(__fastcall* field_10)(__int64, __int16);
	/*00000018*/ void(__fastcall* nullsub_1)();
	/*00000020*/ _QWORD* (__fastcall* GetSomeTimingInfo)(_QWORD*, _QWORD*);
	//Seems very odd. Maybe some behaviour which the server overrides?
	/*00000028*/ StatescriptInstance* (__fastcall* ret_itself)(__int64);
	/*00000030*/ StatescriptInstance* (__fastcall* ret_itself_1)(__int64);
	/*00000038*/ StatescriptInstance* (__fastcall* ret_itself_2)(__int64);
	/*00000040*/ StatescriptInstance* (__fastcall* ret_itself_3)(__int64);
	/*00000048*/ __int64 (*ret_0)();
	/*00000050*/ __int64 (*ret_0_1)();
	/*00000058*/ StatescriptSyncMgr* (__fastcall* get_ss_component_inner_3)(__int64);
	/*00000060*/ StatescriptSyncMgr* (__fastcall* get_ss_component_inner_2)(__int64);
	/*00000068*/ StatescriptSyncMgr* (__fastcall* get_ss_component_inner_1)(__int64);
	/*00000070*/ StatescriptSyncMgr* (__fastcall* get_ss_component_inner)(__int64);
	/*00000078*/ __int64 (*field_4)();
	/*00000080*/ __int64 (*field_5)();
	/*00000088*/ _BOOL8(__fastcall* IsGraphNodeClientOnly)(__int64, __int64);
	/*00000090*/ __int64(__fastcall* CallSSInnerField110)(__int64); // a1->ss_comp_inner_ref->vfptr->field_110((__int64)a1->ss_comp_inner_ref);
	/*00000098*/ __int64(__fastcall* CallSSInnerField118)(__int64);
	/*000000A0*/ char(__fastcall* GetLocalConfigVarValueArray)(StatescriptInstance*, STUConfigVar* cv_in, StatescriptPrimitive_ArrayItem** result_inout, _DWORD* out_result_count, StatescriptPrimitive_ArrayItem* default_value);
	/*000000A8*/ void(__fastcall* ret_0_2)();
	/*000000B0*/ char(__fastcall* field_B0)(__int64, __int64*);
	/*000000B8*/ char(__fastcall* GetEntityPos)(StatescriptInstance*, int, Vector3*);
	/*000000C0*/ __int64(__fastcall* GetInstanceID)(__int64);
	/*000000C8*/ char(__fastcall* CVD_Satisfies_InterceptorArr)(StatescriptInstance*, STUConfigVarDynamic*, StatescriptPrimitive*);
	/*000000D0*/ void(__fastcall* field_D0)(__int64, __int64, _DWORD*, __int64);
	/*000000D8*/ __int64(__fastcall* ConfigVarArr_Search)(StatescriptInstance*, STUConfigVar* input, StatescriptPrimitive*, _DWORD* index_out, StatescriptPrimitive_ArrayItem* value_out);
	/*000000E0*/ __int64(__fastcall* ConfigVarArr_Count)(StatescriptInstance*, STUConfigVar*);
	/*000000E8*/ __int64(__fastcall* GetDynamicVar_RemoteEnt)(StatescriptInstance*, int entid, STUConfigVarDynamic*, StatescriptPrimitive*);
	/*000000F0*/ __int64(__fastcall* GetDynamicVarArray_RemoteEnt)(StatescriptInstance*, __int64 entid, STUConfigVar* cvd_in, StatescriptPrimitive_ArrayItem** cvd_inout, _DWORD* result_out_count, StatescriptPrimitive_ArrayItem* default_value);
	/*000000F8*/ __int64(__fastcall* EnsureInstantiated)(StatescriptInstance* a1, unsigned int a2);
	/*00000100*/ int(__fastcall* j_teStatescriptInstance__loadGraph)(__int64 graph);
	/*00000108*/ __int64(__fastcall* DestroyResource)(__int64);
	/*00000110*/ void(__fastcall* Add10ToExecutionFlags)(__int64);
	/*00000118*/ __int64(__fastcall* field_118)(StatescriptInstance*);
	/*00000120*/ __int64(__fastcall* CallField118)(__int64);
	/*00000128*/ void(__fastcall* field_128)(StatescriptInstance*);
	/*00000130*/ bool(__fastcall* checkSomeClientOnlyFlags)(__int64, __int64);
	/*00000138*/ char(__fastcall* field_138)(StatescriptInstance*, unsigned __int8);
	/*00000140*/ __int64(__fastcall* EmplaceStatescriptE)(StatescriptInstance*, StatescriptNotification_Base*);
	/*00000148*/ __int64(__fastcall* EnqueueE)(StatescriptInstance*, StatescriptNotification_Base*);
	/*00000150*/ __int64(__fastcall* EnqueueOrRequeueE)(StatescriptInstance*, StatescriptNotification_Base* new_e, StatescriptNotification_Base* old_e);
	/*00000158*/ _BOOL8(__fastcall* execute_tick_script)(StatescriptInstance*, StatescriptNotification_Base*);
	/*00000160*/ __int64(__fastcall* Statescript_TickScript)(StatescriptInstance*, StatescriptNotification_Base*);
	/*00000168*/ void(__fastcall* Remove_E_BasedOnStateIndex)(__int64, int, char delete_all);
	/*00000170*/ bool(__fastcall* GetClientOnlyStateByMStateIndex)(StatescriptInstance* a1, int idx_in_states);
	/*00000178*/ char(__fastcall* E_IsReferencedStateClientOnly)(StatescriptInstance*, StatescriptNotification_Base*);
	/*00000188*/ __int64 (*ret_1)();
	/*00000190*/ bool(__fastcall* RedoCheckIsAimEnt)(StatescriptInstance*);
	/*00000198*/ __int64(__fastcall* inc_clientonly_counter_ss_comp_inner)(__int64);
	/*000001A0*/ __int64(__fastcall* dec_clientonly_counter_ss_comp_inner)(__int64);
	/*000001A8*/ __int64(__fastcall* CheckAndWriteGraphField12)(_QWORD*, __int64, int);
	/*000001B0*/ __int64(__fastcall* StopScript)(StatescriptInstance* a1);
	/*000001B8*/ __int64(__fastcall* SyncHistoryStuff)(__int64);
	/*000001C0*/ __int64(__fastcall* field_1C0)(__int64);
	/*000001C8*/ int* (__fastcall* GetScriptOwnerEntId)(__int64, int*);
	/*000001D0*/ char (*ret_0_0)();
	/*000001D8*/ void(__fastcall* UpdateAimEnt)(_QWORD*, __int64);
	/*000001E0*/ __int64(__fastcall* DoSomethingWithMirrorData)(__int64, __int64, __int64);
	/*000001E8*/ _QWORD* (__fastcall* PrepareGameMessageEntry)(__int64, __int64, __int64, int, int);
	/*000001F0*/ __int64* (__fastcall* PreparePlayScriptEntryProbably)(__int64, __int64, __int64, int, int);
	/*000001F8*/ __int64(__fastcall* PrePreparePlayScriptEntry)(__int64, __int64);
	/*00000200*/ void (*ErrorLogMby)(StatescriptInstance*, const char* format, ...);
	/*00000208*/ void(__fastcall* field_11)();
	/*00000210*/ void (*StatescriptInstance__Log)(StatescriptInstance*, const char*, ...);
	/*00000218*/ _DWORD* (__fastcall* MoveArg3ToArg2Ptr)(__int64, _DWORD*, int);
	/*00000220*/ bool(__fastcall* CompareSomeFilterBits)(__int64);
	/*00000228*/ __int64(__fastcall* Instantiate_StatescriptGraph)(StatescriptInstance* a1);
	/*00000230*/ void(__fastcall* CallBecauseOfGraphIdk)();
	/*00000238*/ __int64(__fastcall* CallSomeEntityAdminFunc)(__int64);
	/*00000240*/ _DWORD* (__fastcall* GetSomethingFromSTUAITargetingComponent)(__int64, _DWORD*, unsigned int, int, _DWORD*);
	/*00000248*/ __m128* (__fastcall* DoMirroredAnimStuff)(__int64, __m128*, unsigned int, __m128*);
	/*00000250*/ bool(__fastcall* MoreAIStuff)(__int64, unsigned int*, char*);
	/*00000258*/ char(__fastcall* SomeWeaponStuff)(__int64, _DWORD*);
	/*00000260*/ char(__fastcall* GetOwnerEntIdOfPvpGameComp)(__int64, _DWORD*);
	/*00000268*/ char(__fastcall* GetEntityReference)(__int64, _DWORD*, __int64);
	/*00000270*/ char(__fastcall* ResetThirdVarbagIfEntIsAimEnt)(StatescriptInstance*);
	/*00000278*/ void(__fastcall* field_13)();
	/*00000280*/ double (*field_280)();
	/*00000288*/ __int64 (*field_14)();
	/*00000290*/ __int64(__fastcall* GetAimEntity)(_QWORD*);
	/*00000298*/ double(__fastcall* field_298)(__int64, __int64, __int64, char);
};


struct StatescriptInstance {
public:
	union {
		StatescriptInstance_vt* vfptr;
		STRUCT_PLACE(StatescriptSyncMgr*, ss_inner, 0x10);
		STRUCT_PLACE(short, instance_id, 0x18);
		STRUCT_PLACE(short, parent_instance_id, 0x1A);
		STRUCT_PLACE(__int64, wtf_mby_self_ref, 0x28);
		STRUCT_PLACE(teList<StatescriptInstance*>, m_apCachedSubscriptInstances, 0x50);
		STRUCT_PLACE(StatescriptVar_VarBag*, rid_instance_varbag, 0x98);
		STRUCT_PLACE(teList<StatescriptState*>, state_impl_list, 0xA0);
		STRUCT_PLACE(teList<int>, states_int_list, 0xB8); //spiegelt m_states indexes in graph?
		STRUCT_PLACE(teList<StatescriptAction*>, action_impl_list, 0xD0); //ist ja static -> keine instance daten

		STRUCT_PLACE(StatescriptNotification_Base*, updates_1, 0xE8);
		STRUCT_PLACE(StatescriptNotification_Base*, updates_2, 0xF0);

		STRUCT_PLACE(teList<__int64>, list_120, 0x120);
		STRUCT_PLACE(__int64, script_id, 0x150);
		STRUCT_PLACE(STUStatescriptGraph*, graph, 0x160);
		STRUCT_PLACE(int, script_flags, 0x258);
		STRUCT_PLACE(StatescriptVar_VarBag*, second_inst_varbag, 0x270);
		STRUCT_PLACE(StatescriptVar_VarBag*, third_inst_varbag, 0x280);
		STRUCT_PLACE(uint32, execution_flags, 0x258);
	};

	//MAY BE NULL
	StatescriptState* stateImplFromSTU(STUStatescriptState* stu) {
		int m_states_idx = graph->m_states.indexof(stu);
		for (auto& state : state_impl_list) {
			if (state && state->idx_in_m_states == m_states_idx)
				return state;
		}
		return nullptr;
	}

	void ExecuteNode(int m_nodes_index);
	void StartStopState(int m_nodes_index, bool start);
	void ToggleState(int m_nodes_index);

	void Stop() {
		execution_flags &= ~0x100000u;
		vfptr->StopScript(this);
	}
};

//
// Statescript Component & Instance
//

struct StatescriptSyncMgr
{
	union {
		Statescript23ComponentInner_vt* vfptr;
		STRUCT_PLACE_CUSTOM(stack, 0x10, StatescriptSyncMgr_StackState stack_states[7]);
		STRUCT_PLACE(teList<StatescriptInstance*>, g1_instanceArr, 0xF0);
		STRUCT_PLACE(teList<int>, g2_some_flags, 0x108); //ist ja nicht states_initialized flags
		STRUCT_PLACE(teList<StatescriptInstanceListWithId>, g3_arrWithIds, 0x120);
		STRUCT_PLACE(int, increasing_counter1, 0x2c4);
		STRUCT_PLACE(__int64, cf_timestamp, 0x2d0); //Current command frame / timestamp
		STRUCT_PLACE(__int64, increasing_counter3, 0x2d8);
		STRUCT_PLACE(StatescriptVar_VarBag*, rid_entity_varbag, 0x2E0);
		STRUCT_PLACE(Component_23_Statescript*, component_ref, 0x450);

		STRUCT_PLACE(char, needs_e_process_0, 0x2c0);
		STRUCT_PLACE(char, needs_process_e, 0x359);
		STRUCT_PLACE(char, field_358, 0x358);
		STRUCT_PLACE(char, field_35A, 0x35A);
		STRUCT_PLACE(__int64, field_360, 0x360);
		STRUCT_PLACE(int, ontick_updates_enabled, 0x368);
		STRUCT_PLACE(int, field_36C, 0x36C);
		STRUCT_PLACE(int, field_370, 0x370);

		STRUCT_PLACE(char, flag_374, 0x374);
		STRUCT_PLACE(char, is_instantiating, 0x375);
		STRUCT_PLACE(char, has_clientonly_scripts, 0x376);
		STRUCT_PLACE(char, latest_logbtn_down, 0x377);
		STRUCT_PLACE(int, latest_logbtn, 0x378);
		STRUCT_PLACE(teList<int>, logicalBtn_list, 0x380);

		STRUCT_PLACE(int, field_3C8, 0x3c8);
		STRUCT_PLACE(int, field_3CC, 0x3CC);
		STRUCT_PLACE(int, entity_admin_scope, 0x3D0); //0: OWNER ENT   1: GAME EA   2: LOBBY EA   3: EMBEDAI   4: LOBBYMAP EA
		STRUCT_PLACE(char, field_444, 0x444);
		STRUCT_PLACE(int, has_player_entbackref, 0x448);
		STRUCT_PLACE(char, field_4D0, 0x4D0);
		STRUCT_PLACE(char, field_4D1, 0x4D1);
		STRUCT_PLACE(char, is_aim_ent, 0x4D2);
		STRUCT_PLACE(char, is_aim_ent_assigned, 0x4D3); //is above value assigend

		STRUCT_PLACE(teList<__int64*>, cvd_get_callbacks, 0x410);
	};

	StatescriptInstance* getByInstanceId(int instance_id) {
		for (int i = 0; i < g3_arrWithIds.num; i++) {
			auto script = g3_arrWithIds.ptr[i];
			if (script.instance_id == instance_id)
				return script.instance;
		}
		return nullptr;
	}

	StatescriptInstance* getByResourceId(__int64 resource_id) {
		for (int i = 0; i < g1_instanceArr.num; i++) {
			auto script = g1_instanceArr.ptr[i];
			if (script->script_id == resource_id)
				return script;
		}
		return nullptr;
	}

	StatescriptInstance** begin() {
		return g1_instanceArr.begin();
	}

	StatescriptInstance** end() {
		return g1_instanceArr.end();
	}
};

struct Component_23_Statescript;
struct Statescript_vt {
	union {
		STRUCT_PLACE_CUSTOM(load_graph, 0x58, __int64(__fastcall* load_statescript_script)(Component_23_Statescript* a1, __int64 resource_id, char zero, uint32 ent_id));
	};
};

struct Component_23_Statescript {
	union {
		ComponentBase base;
		Statescript_vt* vfptr;
		STRUCT_PLACE(StatescriptSyncMgr, ss_inner, 0xC0);
	};

	void loadScript(__int64 resource_id) {
		vfptr->load_statescript_script(this, resource_id, 0, 0);
	}
};

struct StatescriptNotification_Base;

/// 0xf4e716
///
/// 0: size 0x38 Lifetime
/// 1: size 0x38 Timer
/// 2: size 0x38 Evaluate Exits
/// 3: size 0x38 Traverse From
/// 4: size 0x50 SetVar
/// 5: size 0x58 SetVarArray
/// 6: size 0x30 Stop Instance (Hat nicht mehr Argumente
/// 7: size 0x38 Logical Button
/// 8: size 0x38 On Attribute Dependency Changed

enum StatescriptNotificationType : __int8
{                                       // XREF: Statescript_E/r
     ETYPE_LIFETIME = 0x0,
     ETYPE_TIMER = 0x1,
     ETYPE_EVALEXITS = 0x2,
     ETYPE_TRAVERSEFROM = 0x3,
     ETYPE_SETVAR = 0x4,
     ETYPE_SETVARARRAY = 0x5,
     ETYPE_STOPINST = 0x6,
     ETYPE_LOGICALBUTTON = 0x7,
     ETYPE_ONATTRDEPCHANGE = 0x8,
};

struct Statescript_E_vt // sizeof=0x50
{                                       // XREF: .rdata:Statescript_E_7/r
                                        // .rdata:Statescript_E_8/r ...
    _QWORD * (__fastcall * deallocate)(StatescriptNotification_Base*, char);
    char(__fastcall* clone_from)(__int64, __int64 unused_bia, __int64);
    __int64 (*Get_StateIndex)();        // m_states in STUStatescriptGraph
    __int64 (*always_ret_0)();
    __int64(__fastcall * field_20)(__int64, __int64);
    unsigned __int64(__fastcall* field_28)(__int64, __int64);
    _BYTE * (__fastcall * ToString)(StatescriptNotification_Base*, char*, int); // "Missing ToString() implementation" (eboot)
    char (*always_0)();
    char (*always_0_0)();
    char (*always_1)();
};

struct StatescriptNotification_Base {
	Statescript_E_vt* vfptr;
	StatescriptNotification_Base* parent;
	StatescriptNotification_Base* child;
	int m_timestamp;
	int field_1C;
	short m_instanceId;
	StatescriptNotificationType type;
	int inc_counter_ss_inner;
	char field_28;

	std::string ToString() {
		char buf[256];
		vfptr->ToString(this, buf, 256);
		return buf;
	}
};

struct StatescriptNotification_AttrDependencyChanged {
	StatescriptNotification_Base base;
	short m_uStateIndex;
};

struct StatescriptNotification_EvaluateExits {
	StatescriptNotification_Base base;
	short m_uStateIndex;
};

struct StatescriptNotification_Lifetime {
	StatescriptNotification_Base base;
	short m_uLifetimeType; //0: doBeginState, 1: doFinishState, 2: doAbortState
	short m_uStateIndex;
};

struct StatescriptNotification_LogicalButton {
	StatescriptNotification_Base base;
	int m_logicalButton;
	char m_bButtonGoingDown;
};

struct StatescriptNotification_SetVar {
	StatescriptNotification_Base base;
	short ss_inst_id; // (%d: %d) mit field below bei tostring
	short field_32;
	short variable_guid;
	__int64 user;
	StatescriptPrimitive variable;
};

struct StatescriptNotification_SetVarArray {
	StatescriptNotification_Base base;
	short ss_inst_id; // (%d: %d) mit field below bei tostring
	short field_32;
	short variable_guid;
	__int64 user;
	teList<StatescriptPrimitive> variables; //guessed
};

struct StatescriptNotification_Timer {
	StatescriptNotification_Base base;
	__int32 m_nTimerUserData;
	__int16 m_uStateIndex;
};

struct StatescriptNotification_TraverseFrom {
    StatescriptNotification_Base base;
    __int16 m_uBaseIndex;
};


inline StatescriptAction* GetStatescriptActionImplNode(StatescriptInstance* a1, STUStatescriptAction* a2) {
	typedef StatescriptAction* (*fn)(StatescriptInstance* a1, STUStatescriptAction* a2);
	fn func = (fn)(globals::gameBase + 0xf4e1a0);
	return func(a1, a2);
}

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