#pragma once
#include "Statescript.h"
#include <mutex>
#include <imgui.h>

//TODO! This is still experimental and the output may be wrong sometimes!

#define opcassert(expr) owassert(expr); if (!(expr)) { ImGui::Text("%s:%d", "FAILED ASSERTION:\nAssertion: " #expr "\nAt: " __FILE__, __LINE__); return false; }

inline char ConfigVar_IsTruthy(StatescriptPrimitive* cv, char default_value) {
	auto func = (char(__fastcall*)(StatescriptPrimitive*, char))(globals::gameBase + 0xee8a80);
	return func(cv, default_value);
}

inline void CVD_ArrayGetAndSearchForIndexOrKey(
	StatescriptInstance* ss,
	uint32* entid,
	STUConfigVarDynamic* search_array,
	StatescriptPrimitive* search_value,
	int* out_resultArrIndex,
	StatescriptPrimitive_ArrayItem* out_result
) {
	auto func = (void(__fastcall*)(__int64 unused, StatescriptInstance*, uint32*, STUConfigVarDynamic*, StatescriptPrimitive*, int*, StatescriptPrimitive_ArrayItem*))(globals::gameBase + 0xf6c100);
	func(0, ss, entid, search_array, search_value, out_resultArrIndex, out_result);
}

inline bool ConfigVar_IsEqual(StatescriptPrimitive* first, StatescriptPrimitive* second) {
	auto func = (bool(__fastcall*)(StatescriptPrimitive*, StatescriptPrimitive*))(globals::gameBase + 0xee99c0);
	return func(first, second);
}

inline float inverse_sqrt(float input) {
	auto func = (float(__fastcall*)(float))(globals::gameBase + 0x8597a0);
	return func(input);
}

const float deg2rad = 0.017453292f;

inline uint16 to_shortflt(float input) {
	uint16 result{};
	auto func = (void(__fastcall*)(uint16*, float))(globals::gameBase + 0xee8360);
	func(&result, input);
	return result;
}

inline StatescriptPrimitive ConfigVar_RandomBetween(StatescriptPrimitive* min, StatescriptPrimitive* max) {
	StatescriptPrimitive result;
	auto func = (void(__fastcall*)(StatescriptPrimitive*, StatescriptPrimitive*, StatescriptPrimitive*))(globals::gameBase + 0xf6c310);
	func(&result, min, max);
	return result;
}

inline int GetRandom_Int(int max_value) {
	auto func = (int(__fastcall*)(int))(globals::gameBase + 0x87b390);
	return func(max_value);
}

//inline float VectorLen(Vector3 input) {
//	auto func = (float(__fastcall*)(Vector3*))(globals::gameBase + 0x8966b0);
//	return func(&input);
//}

class StatescriptExpressionParser {
public:
	class OpcodeBase;
	struct ExecutionContext {
		StatescriptExpressionParser* parser = nullptr;
		OpcodeBase* tva[8]{};
		int tva_index = -1;
		int opc_index = 0;
		uint32 entid_temp = 0;
		bool is_right_path = true;
	};

	class OpcodeBase {
	public:
		OpcodeBase() {}

		//virtual std::string to_string() = 0;
		virtual bool display_imgui() = 0;
		//return false if opcode is not valid in current context
		//Does not do runtime checks
		//virtual bool parse() = 0;
		
		virtual int get_size() = 0;
		virtual StatescriptPrimitive* get_cv() = 0;
		virtual std::string get_cv_origin() = 0;
		virtual std::string get_cv_formatted() = 0;

		ExecutionContext _context;
	};

	template <int size>
	class Opcode : public OpcodeBase {
	public:
		using OpcodeBase::OpcodeBase;

		int get_size() override {
			return size;
		}
		StatescriptPrimitive* get_cv() override {
			return nullptr;
		}
		std::string get_cv_origin() override {
			return "Unknown";
		}
		std::string get_cv_formatted() override {
			auto cv = get_cv();
			std::string cv_str = cv ? cv->get_value_str() : "INVALID";
			return fmt::format("[{}] {}", get_cv_origin(), cv_str);
		}
	};

	class Opcode_0_Exit : public Opcode<1> {
	public:
		using Opcode::Opcode;

		bool display_imgui() override {
			std::string result = "None";
			if (_context.tva_index >= 0)
				result = _context.tva[_context.tva_index]->get_cv_formatted();
			if (_context.is_right_path) {
				ImGui::Text("Result: %s", result.c_str());
			}
			else {
				ImGui::TextUnformatted("Fin.");
			}
			return true;
		}
	};

	class Opcode_1_Jump : public Opcode<2> {
	public:
		using Opcode::Opcode;
		/// 1: opcarr = opcarr[1], ExitCheck

		bool display_imgui() override {
			int jump_to = _context.parser->_expr->m_expression.m_opcodes.list()[_context.opc_index + 1];
			opcassert(jump_to < _context.parser->_expr->m_expression.m_opcodes.count());

			_context.parser->display_next(_context, _context.opc_index + jump_to);
			return true;
		}
	};

	class Opcode_2_4_FalseyConditionalJump : public Opcode<2> {
	public:
		using Opcode::Opcode;
		/// 2: Jump opcarr[1] if TVA Falsey, else TVA--
		/// 4: Dec TVA, Jump opcarr[1] if TVA Falsey
		bool _decrease_only_if_false;

		bool display_imgui() override {
			int jump_to = _context.parser->_expr->m_expression.m_opcodes.list()[_context.opc_index + 1];
			opcassert(jump_to < _context.parser->_expr->m_expression.m_opcodes.count());
			opcassert(_context.tva_index >= 0);

			auto cv_provider = _context.tva[_context.tva_index];
			opcassert(cv_provider);
			auto cv = cv_provider->get_cv();
			opcassert(cv);

			bool is_falsey = !ConfigVar_IsTruthy(cv, 0);
			if (is_falsey)
				ImGui::PushFont(imgui_helpers::BoldFont);
			ImGui::Text("if is_falsey(%s) -> %s {", cv_provider->get_cv_formatted().c_str(), is_falsey ? "True" : "False");
			if (is_falsey)
				ImGui::PopFont();
			ImGui::Indent();

			auto jump_context = _context;
			jump_context.is_right_path = is_falsey;
			if (!_decrease_only_if_false)
				jump_context.tva_index--;
			bool result = _context.parser->display_next(jump_context, _context.opc_index + jump_to);

			ImGui::Unindent();
			if (is_falsey)
				ImGui::PushFont(imgui_helpers::BoldFont);
			ImGui::TextUnformatted("}");
			if (is_falsey)
				ImGui::PopFont();
			opcassert(result);

			auto non_jump_context = _context;
			non_jump_context.tva_index--;
			non_jump_context.is_right_path = !is_falsey;

			_context.parser->display_next(non_jump_context, _context.opc_index + get_size());
			return true;
		}

		Opcode_2_4_FalseyConditionalJump(bool decrease_only_if_false) : _decrease_only_if_false(decrease_only_if_false) { }
	};

	class Opcode_3_5_TruthyConditionalJump : public Opcode<2> {
	public:
		using Opcode::Opcode;
		/// 3: Jump opcarr[1] if TVA Truthy, else TVA--
		/// 5: Dec TVA, Jump opcarr[1] if TVA Truthy
		bool _decrease_only_if_false;

		bool display_imgui() override {
			int jump_to = _context.parser->_expr->m_expression.m_opcodes.list()[_context.opc_index + 1];
			opcassert(jump_to < _context.parser->_expr->m_expression.m_opcodes.count());
			opcassert(_context.tva_index >= 0);

			auto cv_provider = _context.tva[_context.tva_index];
			opcassert(cv_provider);
			auto cv = cv_provider->get_cv();
			opcassert(cv);

			bool is_truthy = ConfigVar_IsTruthy(cv, 0);
			if (is_truthy)
				ImGui::PushFont(imgui_helpers::BoldFont);
			ImGui::Text("if is_truthy(%s) -> %s {", cv_provider->get_cv_formatted().c_str(), is_truthy ? "True" : "False");
			if (is_truthy)
				ImGui::PopFont();
			ImGui::Indent();

			auto jump_context = _context;
			jump_context.is_right_path = is_truthy;
			if (!_decrease_only_if_false)
				jump_context.tva_index--;
			bool result = _context.parser->display_next(jump_context, _context.opc_index + jump_to);

			ImGui::Unindent();
			if (is_truthy)
				ImGui::PushFont(imgui_helpers::BoldFont);
			ImGui::TextUnformatted("}");
			if (is_truthy)
				ImGui::PopFont();
			opcassert(result);

			auto new_context = _context;
			new_context.tva_index--;
			new_context.is_right_path = !is_truthy;

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		Opcode_3_5_TruthyConditionalJump(bool decrease_only_if_false) : _decrease_only_if_false(decrease_only_if_false) {}
	};

	class Opcode_30_LoadTVA_ConfigVar : public Opcode<2> {
	public:
		using Opcode::Opcode;
		int cv_index = 0;
		StatescriptPrimitive value{};

		bool display_imgui() override {
			auto new_context = _context;
			new_context.tva_index++;
			new_context.tva[new_context.tva_index] = this;

			cv_index = _context.parser->_expr->m_expression.m_opcodes.list()[_context.opc_index + 1];
			opcassert(cv_index < _context.parser->_expr->m_configVars.count());
			_context.parser->_expr->m_configVars.list()[cv_index]->get_value(_context.parser->_ss, &value);

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "CV-" + std::to_string(cv_index);
		}

		~Opcode_30_LoadTVA_ConfigVar() {
			value.reset();
		}
	};

	class Opcode_31_LoadTVA_DynamicVar : public Opcode<2> {
	public:
		using Opcode::Opcode;
		int cv_index = 0;
		StatescriptPrimitive value{};

		bool display_imgui() override {
			auto new_context = _context;
			new_context.tva_index++;
			new_context.tva[new_context.tva_index] = this;

			cv_index = _context.parser->_expr->m_expression.m_opcodes.list()[_context.opc_index + 1];
			opcassert(cv_index < _context.parser->_expr->m_expression.m_dynamicVars.count());
			auto cvd = _context.parser->_expr->m_expression.m_dynamicVars.list()[cv_index];
			if (_context.entid_temp != 0) {
				_context.parser->_ss->vfptr->GetDynamicVar_RemoteEnt(_context.parser->_ss, _context.entid_temp, cvd, &value);
				ImGui::Text("Get CVD-%d from ent %x", cv_index, _context.entid_temp);
			}
			else if (!_context.parser->_expr->m_expression.m_dynamicVars.list()[cv_index]->cv_base.get_value(_context.parser->_ss, &value)) {
				ImGui::Text("Failed getting CVD-%d.", cv_index);
				value.type = StatescriptPrimitive_FLT;
				value.value = 0;
			}

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "CVD-" + std::to_string(cv_index);
		}

		~Opcode_31_LoadTVA_DynamicVar() {
			value.reset();
		}
	};

	class Opcode_32_SearchOrIndex_DynamicVar_Value : public Opcode<2> {
	public:
		using Opcode::Opcode;
		/// 32: search_temp = TVA, index = opcarr[i], value = m_dynamicVars[index], TVA = searchDynamicArr(entid_temp, value, search_temp)->value
		//index of search_array in m_dynamicVars
		int search_arr_index = 0;
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 0);
			auto search_value_provider = _context.tva[_context.tva_index];
			opcassert(search_value_provider);
			auto search_value_cv = search_value_provider->get_cv();
			opcassert(search_value_cv);

			search_arr_index = _context.parser->_expr->m_expression.m_opcodes.list()[_context.opc_index + 1];
			opcassert(search_arr_index < _context.parser->_expr->m_expression.m_dynamicVars.count());
			STUConfigVarDynamic* search_arr = _context.parser->_expr->m_expression.m_dynamicVars.list()[search_arr_index];
			opcassert(search_arr);
			
			int result_idx = 0;
			StatescriptPrimitive_ArrayItem inout_result{};
			CVD_ArrayGetAndSearchForIndexOrKey(
				_context.parser->_ss,
				&_context.entid_temp,
				search_arr,
				search_value_cv,
				&result_idx,
				&inout_result
			);
			value = inout_result.value.clone();
			inout_result.reset();

			ImGui::Text("TVA-%d = searchDynamicArr([CVD-%d], %s, entid_temp).value",
				_context.tva_index,
				search_arr_index,
				search_value_provider->get_cv_formatted().c_str()
			);

			auto new_context = _context;
			new_context.tva[new_context.tva_index] = this;

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index);
		}

		~Opcode_32_SearchOrIndex_DynamicVar_Value() {
			value.reset();
		}
	};

	class Opcode_33_SearchOrIndex_DynamicVar_Key : public Opcode<2> {
	public:
		using Opcode::Opcode;
		/// 33: search_temp = TVA, index = opcarr[i], value = m_dynamicVars[index], TVA = searchDynamicArr(entid_temp, value, search_temp)->key
		//index of search_array in m_dynamicVars
		int search_arr_index = 0;
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 0);
			auto search_value_provider = _context.tva[_context.tva_index];
			opcassert(search_value_provider);
			auto search_value_cv = search_value_provider->get_cv();
			opcassert(search_value_cv);

			search_arr_index = _context.parser->_expr->m_expression.m_opcodes.list()[_context.opc_index + 1];
			opcassert(search_arr_index < _context.parser->_expr->m_expression.m_dynamicVars.count());
			STUConfigVarDynamic* search_arr = _context.parser->_expr->m_expression.m_dynamicVars.list()[search_arr_index];
			opcassert(search_arr);

			int result_idx = 0;
			StatescriptPrimitive_ArrayItem inout_result{};
			CVD_ArrayGetAndSearchForIndexOrKey(
				_context.parser->_ss,
				&_context.entid_temp,
				search_arr,
				search_value_cv,
				&result_idx,
				&inout_result
			);
			value = inout_result.key.clone();
			inout_result.reset();

			ImGui::Text("TVA-%d = searchDynamicArr([CVD-%d], %s, entid_temp).key",
				_context.tva_index,
				search_arr_index,
				search_value_provider->get_cv_formatted().c_str()
			);

			auto new_context = _context;
			new_context.tva[new_context.tva_index] = this;

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index);
		}

		~Opcode_33_SearchOrIndex_DynamicVar_Key() {
			value.reset();
		}
	};

	class Opcode_34_SearchOrIndex_DynamicVar_ArrayIndex : public Opcode<2> {
	public:
		using Opcode::Opcode;
		/// 34: search_temp = TVA, index = opcarr[i], value = m_dynamicVars[index], TVA = searchDynamicArr(entid_temp, value, search_temp)->array_index
		//index of search_array in m_dynamicVars
		int search_arr_index = 0;
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 0);
			auto search_value_provider = _context.tva[_context.tva_index];
			opcassert(search_value_provider);
			auto search_value_cv = search_value_provider->get_cv();
			opcassert(search_value_cv);

			search_arr_index = _context.parser->_expr->m_expression.m_opcodes.list()[_context.opc_index + 1];
			opcassert(search_arr_index < _context.parser->_expr->m_expression.m_dynamicVars.count());
			STUConfigVarDynamic* search_arr = _context.parser->_expr->m_expression.m_dynamicVars.list()[search_arr_index];
			opcassert(search_arr);

			int result_idx = 0;
			StatescriptPrimitive_ArrayItem inout_result{};
			CVD_ArrayGetAndSearchForIndexOrKey(
				_context.parser->_ss,
				&_context.entid_temp,
				search_arr,
				search_value_cv,
				&result_idx,
				&inout_result
			);
			value.type = StatescriptPrimitive_INT;
			value.value = result_idx;
			inout_result.reset();

			ImGui::Text("TVA-%d = searchDynamicArr([CVD-%d], %s, entid_temp).array_index",
				_context.tva_index,
				search_arr_index,
				search_value_provider->get_cv_formatted().c_str()
			);

			auto new_context = _context;
			new_context.tva[new_context.tva_index] = this;

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index);
		}

		~Opcode_34_SearchOrIndex_DynamicVar_ArrayIndex() {
			value.reset();
		}
	};

	class Opcode_35_LoadEntidTemp : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 35 (1): entid_temp = TVA, TVA--
		//index of search_array in m_dynamicVars

		bool display_imgui() override {
			opcassert(_context.tva_index >= 0);
			auto value_provider = _context.tva[_context.tva_index];
			opcassert(value_provider);
			auto cv = value_provider->get_cv();
			opcassert(cv);
			if (cv->type == StatescriptPrimitive_ENTITY) {
				ImGui::Text("entid_temp = %s", value_provider->get_cv_formatted().c_str());
			}
			else {
				ImGui::Text("entid_temp = %s (INVALID)", value_provider->get_cv_formatted().c_str());
			}

			auto new_context = _context;
			new_context.tva[new_context.tva_index] = this;

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}
	};

	class Opcode_36_LoadTVA_DynamicVar_Interceptor : public Opcode<2> {
	public:
		using Opcode::Opcode;
		/// 36: TVA++, index = opcarr[1], TVA = interceptor(m_dynamicVars[index])

		StatescriptPrimitive value{};
		int tva_index = 0;

		bool display_imgui() override {
			auto new_context = _context;
			tva_index = ++new_context.tva_index;
			new_context.tva[new_context.tva_index] = this;

			int cv_index = _context.parser->_expr->m_expression.m_opcodes.list()[_context.opc_index + 1];
			opcassert(cv_index < _context.parser->_expr->m_expression.m_dynamicVars.count());
			//_context.parser->_expr->m_expression.m_dynamicVars.list()[cv_index]->cv_base.get_value(_context.parser->_ss, &value);
			auto cvd = _context.parser->_expr->m_expression.m_dynamicVars.list()[cv_index];
			opcassert(cvd);
			char result = _context.parser->_ss->vfptr->CVD_Satisfies_InterceptorArr(_context.parser->_ss, cvd, &value);

			ImGui::Text("TVA-%d = interceptor([CVD-%d] %s) (Interceptor Res: %d)", new_context.tva_index, cv_index, value.get_value_str().c_str(), result);

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "iTVA-" + std::to_string(tva_index);
		}

		~Opcode_36_LoadTVA_DynamicVar_Interceptor() {
			value.reset();
		}
	};

	class Opcode_37_SearchOrIndex_ConfigVar_Value : public Opcode<2> {
	public:
		using Opcode::Opcode;
		/// 37: search_temp = TVA, index = opcarr[i], value = m_configVars[index], TVA = searchArr(value, search_temp)->value
		//index of search_array in m_configVars
		int search_arr_index = 0;
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 0);
			auto search_value_provider = _context.tva[_context.tva_index];
			opcassert(search_value_provider);
			auto search_value_cv = search_value_provider->get_cv();
			opcassert(search_value_cv);

			search_arr_index = _context.parser->_expr->m_expression.m_opcodes.list()[_context.opc_index + 1];
			opcassert(search_arr_index < _context.parser->_expr->m_configVars.count());
			STUConfigVar* search_arr = _context.parser->_expr->m_configVars.list()[search_arr_index];
			opcassert(search_arr);

			uint32 result_idx = 0;
			StatescriptPrimitive_ArrayItem inout_result{};
			_context.parser->_ss->vfptr->ConfigVarArr_Search(
				_context.parser->_ss,
				search_arr,
				search_value_cv,
				&result_idx,
				&inout_result
			);
			value = inout_result.value.clone();
			inout_result.reset();

			ImGui::Text("TVA-%d = searchArr([CV-%d], %s, entid_temp).value",
				_context.tva_index,
				search_arr_index,
				search_value_provider->get_cv_formatted().c_str()
			);

			auto new_context = _context;
			new_context.tva[new_context.tva_index] = this;

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index);
		}

		~Opcode_37_SearchOrIndex_ConfigVar_Value() {
			value.reset();
		}
	};

	class Opcode_38_SearchOrIndex_ConfigVar_Key : public Opcode<2> {
	public:
		using Opcode::Opcode;
		/// 38: search_temp = TVA, index = opcarr[i], value = m_configVars[index], TVA = searchArr(value, search_temp)->key
		//index of search_array in m_configVars
		int search_arr_index = 0;
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 0);
			auto search_value_provider = _context.tva[_context.tva_index];
			opcassert(search_value_provider);
			auto search_value_cv = search_value_provider->get_cv();
			opcassert(search_value_cv);

			search_arr_index = _context.parser->_expr->m_expression.m_opcodes.list()[_context.opc_index + 1];
			opcassert(search_arr_index < _context.parser->_expr->m_configVars.count());
			STUConfigVar* search_arr = _context.parser->_expr->m_configVars.list()[search_arr_index];
			opcassert(search_arr);

			uint32 result_idx = 0;
			StatescriptPrimitive_ArrayItem inout_result{};
			_context.parser->_ss->vfptr->ConfigVarArr_Search(
				_context.parser->_ss,
				search_arr,
				search_value_cv,
				&result_idx,
				&inout_result
			);
			value = inout_result.key.clone();
			inout_result.reset();

			ImGui::Text("TVA-%d = searchArr([CV-%d], %s, entid_temp).key",
				_context.tva_index,
				search_arr_index,
				search_value_provider->get_cv_formatted().c_str()
			);

			auto new_context = _context;
			new_context.tva[new_context.tva_index] = this;

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index);
		}

		~Opcode_38_SearchOrIndex_ConfigVar_Key() {
			value.reset();
		}
	};

	class Opcode_39_SearchOrIndex_ConfigVar_ArrayIndex : public Opcode<2> {
	public:
		using Opcode::Opcode;
		/// 39: search_temp = TVA, index = opcarr[i], value = m_configVars[index], TVA = searchArr(value, search_temp)->array_index
		//index of search_array in m_configVars
		int search_arr_index = 0;
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 0);
			auto search_value_provider = _context.tva[_context.tva_index];
			opcassert(search_value_provider);
			auto search_value_cv = search_value_provider->get_cv();
			opcassert(search_value_cv);

			search_arr_index = _context.parser->_expr->m_expression.m_opcodes.list()[_context.opc_index + 1];
			opcassert(search_arr_index < _context.parser->_expr->m_configVars.count());
			STUConfigVar* search_arr = _context.parser->_expr->m_configVars.list()[search_arr_index];
			opcassert(search_arr);

			uint32 result_idx = 0;
			StatescriptPrimitive_ArrayItem inout_result{};
			_context.parser->_ss->vfptr->ConfigVarArr_Search(
				_context.parser->_ss,
				search_arr,
				search_value_cv,
				&result_idx,
				&inout_result
			);
			value.type = StatescriptPrimitive_INT;
			value.value = result_idx;
			inout_result.reset();

			ImGui::Text("TVA-%d = searchArr([CV-%d], %s, entid_temp).array_index",
				_context.tva_index,
				search_arr_index,
				search_value_provider->get_cv_formatted().c_str()
			);

			auto new_context = _context;
			new_context.tva[new_context.tva_index] = this;

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index);
		}

		~Opcode_39_SearchOrIndex_ConfigVar_ArrayIndex() {
			value.reset();
		}
	};

	class Opcode_40_Count_ConfigVarArray : public Opcode<2> {
	public:
		using Opcode::Opcode;
		/// 40: TVA++, index = opcarr[i], TVA = count(m_configVars[index])
		StatescriptPrimitive value{};

		bool display_imgui() override {
			auto new_context = _context;
			new_context.tva_index++;
			new_context.tva[new_context.tva_index] = this;

			int search_arr_index = _context.parser->_expr->m_expression.m_opcodes.list()[_context.opc_index + 1];
			opcassert(search_arr_index < _context.parser->_expr->m_configVars.count());
			STUConfigVar* search_arr = _context.parser->_expr->m_configVars.list()[search_arr_index];
			opcassert(search_arr);

			int count = _context.parser->_ss->vfptr->ConfigVarArr_Count(
				_context.parser->_ss,
				search_arr
			);
			value.type = StatescriptPrimitive_INT;
			value.value = count;

			ImGui::Text("TVA-%d = count([CV-%d])", new_context.tva_index, search_arr_index);

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index + 1);
		}

		~Opcode_40_Count_ConfigVarArray() {
			value.reset();
		}
	};

	class Opcode_70_Load_FloatConstant : public Opcode<2> {
	public:
		using Opcode::Opcode;
		/// 70: TVA++, index = opcarr[i], TVA = float_constants[index]
		int float_index = 0;
		StatescriptPrimitive value{};

		bool display_imgui() override {
			auto new_context = _context;
			new_context.tva_index++;
			new_context.tva[new_context.tva_index] = this;

			float_index = _context.parser->_expr->m_expression.m_opcodes.list()[_context.opc_index + 1];
			opcassert(float_index < _context.parser->_expr->m_expression.float_constants.count());
			float flt = _context.parser->_expr->m_expression.float_constants.list()[float_index];

			value.type = StatescriptPrimitive_FLT;
			*(float*)&value.value = flt;

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "FLT-" + std::to_string(float_index);
		}

		~Opcode_70_Load_FloatConstant() {
			value.reset();
		}
	};

	class Opcode_100_Add : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 100 (1): value1 = TVA, TVA--, value2 = TVA, TVA = getpos(value2) + getpos(value1)
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value1_provider = _context.tva[_context.tva_index];
			opcassert(value1_provider);
			auto value1_cv = value1_provider->get_cv();
			opcassert(value1_cv);

			auto value2_provider = _context.tva[_context.tva_index - 1];
			opcassert(value2_provider);
			auto value2_cv = value2_provider->get_cv();
			opcassert(value2_cv);

			auto new_context = _context;
			new_context.tva_index--;
			new_context.tva[new_context.tva_index] = this;

			if ((value1_cv->type == StatescriptPrimitive_ENTITY || value1_cv->type == StatescriptPrimitive_VEC3) &&
				(value2_cv->type == StatescriptPrimitive_ENTITY || value2_cv->type == StatescriptPrimitive_VEC3)) {
				Vector3 value1 = value1_cv->get_convert_vec3(_context.parser->_ss);
				Vector3 value2 = value2_cv->get_convert_vec3(_context.parser->_ss);

				Vector3 result = value2 + value1;
				value.type = StatescriptPrimitive_VEC3;
				memcpy(&value.value, &result, sizeof(Vector3));

				ImGui::Text("TVA-%d = getpos(%s) + getpos(%s)", new_context.tva_index, value2_provider->get_cv_formatted().c_str(), value1_provider->get_cv_formatted().c_str());
			}
			else {
				float value1 = value1_cv->get_convert_float();
				float value2 = value2_cv->get_convert_float();

				value.type = StatescriptPrimitive_FLT;
				*(float*)&value.value = value2 + value1;

				ImGui::Text("TVA-%d = %s + %s", new_context.tva_index, value2_provider->get_cv_formatted().c_str(), value1_provider->get_cv_formatted().c_str());
			}

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index - 1);
		}

		~Opcode_100_Add() {
			value.reset();
		}
	};

	class Opcode_101_Sub : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 101 (1): value1 = TVA, TVA--, value2 = TVA, TVA = getpos(value2) - getpos(value1)
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value1_provider = _context.tva[_context.tva_index];
			opcassert(value1_provider);
			auto value1_cv = value1_provider->get_cv();
			opcassert(value1_cv);

			auto value2_provider = _context.tva[_context.tva_index - 1];
			opcassert(value2_provider);
			auto value2_cv = value2_provider->get_cv();
			opcassert(value2_cv);

			auto new_context = _context;
			new_context.tva_index--;
			new_context.tva[new_context.tva_index] = this;

			if ((value1_cv->type == StatescriptPrimitive_ENTITY || value1_cv->type == StatescriptPrimitive_VEC3) &&
				(value2_cv->type == StatescriptPrimitive_ENTITY || value2_cv->type == StatescriptPrimitive_VEC3)) {
				Vector3 value1 = value1_cv->get_convert_vec3(_context.parser->_ss);
				Vector3 value2 = value2_cv->get_convert_vec3(_context.parser->_ss);

				Vector3 result = value2 - value1;
				value.type = StatescriptPrimitive_VEC3;
				memcpy(&value.value, &result, sizeof(Vector3));

				ImGui::Text("TVA-%d = getpos(%s) - getpos(%s)", new_context.tva_index, value2_provider->get_cv_formatted().c_str(), value1_provider->get_cv_formatted().c_str());
			}
			else {
				float value1 = value1_cv->get_convert_float();
				float value2 = value2_cv->get_convert_float();

				value.type = StatescriptPrimitive_FLT;
				*(float*)&value.value = value2 - value1;

				ImGui::Text("TVA-%d = %s - %s", new_context.tva_index, value2_provider->get_cv_formatted().c_str(), value1_provider->get_cv_formatted().c_str());
			}

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index - 1);
		}

		~Opcode_101_Sub() {
			value.reset();
		}
	};

	class Opcode_102_Mul : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 102 (1): value1 = TVA, TVA--, value2 = TVA, TVA = getpos(value2) * getpos(value1)
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value1_provider = _context.tva[_context.tva_index];
			opcassert(value1_provider);
			auto value1_cv = value1_provider->get_cv();
			opcassert(value1_cv);

			auto value2_provider = _context.tva[_context.tva_index - 1];
			opcassert(value2_provider);
			auto value2_cv = value2_provider->get_cv();
			opcassert(value2_cv);

			auto new_context = _context;
			new_context.tva_index--;
			new_context.tva[new_context.tva_index] = this;

			if ((value1_cv->type == StatescriptPrimitive_ENTITY || value1_cv->type == StatescriptPrimitive_VEC3) &&
				(value2_cv->type == StatescriptPrimitive_ENTITY || value2_cv->type == StatescriptPrimitive_VEC3)) {
				Vector3 value1 = value1_cv->get_convert_vec3(_context.parser->_ss);
				Vector3 value2 = value2_cv->get_convert_vec3(_context.parser->_ss);

				Vector3 result = value2 * value1;
				value.type = StatescriptPrimitive_VEC3;
				memcpy(&value.value, &result, sizeof(Vector3));

				ImGui::Text("TVA-%d = getpos(%s) * getpos(%s)", new_context.tva_index, value2_provider->get_cv_formatted().c_str(), value1_provider->get_cv_formatted().c_str());
			}
			else {
				float value1 = value1_cv->get_convert_float();
				float value2 = value2_cv->get_convert_float();

				value.type = StatescriptPrimitive_FLT;
				*(float*)&value.value = value2 * value1;

				ImGui::Text("TVA-%d = %s * %s", new_context.tva_index, value2_provider->get_cv_formatted().c_str(), value1_provider->get_cv_formatted().c_str());
			}

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index - 1);
		}

		~Opcode_102_Mul() {
			value.reset();
		}
	};

	class Opcode_103_Div : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 103 (1): value1 = TVA, TVA--, value2 = TVA, TVA = getpos(value2) / getpos(value1) (div0 safe)
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value1_provider = _context.tva[_context.tva_index];
			opcassert(value1_provider);
			auto value1_cv = value1_provider->get_cv();
			opcassert(value1_cv);

			auto value2_provider = _context.tva[_context.tva_index - 1];
			opcassert(value2_provider);
			auto value2_cv = value2_provider->get_cv();
			opcassert(value2_cv);

			auto new_context = _context;
			new_context.tva_index--;
			new_context.tva[new_context.tva_index] = this;

			if ((value1_cv->type == StatescriptPrimitive_ENTITY || value1_cv->type == StatescriptPrimitive_VEC3) &&
				(value2_cv->type == StatescriptPrimitive_ENTITY || value2_cv->type == StatescriptPrimitive_VEC3)) {
				Vector3 value1 = value1_cv->get_convert_vec3(_context.parser->_ss);
				Vector3 value2 = value2_cv->get_convert_vec3(_context.parser->_ss);

				Vector3 result = value2 / value1;
				value.type = StatescriptPrimitive_VEC3;
				memcpy(&value.value, &result, sizeof(Vector3));

				ImGui::Text("TVA-%d = getpos(%s) / getpos(%s)", new_context.tva_index, value2_provider->get_cv_formatted().c_str(), value1_provider->get_cv_formatted().c_str());
			}
			else {
				float value1 = value1_cv->get_convert_float();
				float value2 = value2_cv->get_convert_float();

				value.type = StatescriptPrimitive_FLT;
				float result = 0;
				if (value1 != 0)
					result = value2 / value1;
				*(float*)&value.value = result;

				ImGui::Text("TVA-%d = %s / %s", new_context.tva_index, value2_provider->get_cv_formatted().c_str(), value1_provider->get_cv_formatted().c_str());
			}

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index - 1);
		}

		~Opcode_103_Div() {
			value.reset();
		}
	};

	class Opcode_104_Remainder : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 104 (1): value1 = TVA, TVA--, value2 = TVA, TVA = value2 - (value1 * floor(value2 / value1)) -> remainder calculation, no vector support
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value1_provider = _context.tva[_context.tva_index];
			opcassert(value1_provider);
			auto value1_cv = value1_provider->get_cv();
			opcassert(value1_cv);

			auto value2_provider = _context.tva[_context.tva_index - 1];
			opcassert(value2_provider);
			auto value2_cv = value2_provider->get_cv();
			opcassert(value2_cv);

			float value1 = value1_cv->get_convert_float();
			float value2 = value2_cv->get_convert_float();

			auto new_context = _context;
			new_context.tva_index--;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = %s % %s", new_context.tva_index, value2_provider->get_cv_formatted().c_str(), value1_provider->get_cv_formatted().c_str());

			float result = value2 - (float)((float)(int)(float)(value2 / value1) * value1);
			value.type = StatescriptPrimitive_FLT;
			*(float*)&value.value = result;

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index);
		}

		~Opcode_104_Remainder() {
			value.reset();
		}
	};

	class Opcode_105_Pow : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 105 (1): value1 = TVA, TVA--, value2 = TVA, TVA = powf(value2, value1) -> no vector support
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value1_provider = _context.tva[_context.tva_index];
			opcassert(value1_provider);
			auto value1_cv = value1_provider->get_cv();
			opcassert(value1_cv);

			auto value2_provider = _context.tva[_context.tva_index - 1];
			opcassert(value2_provider);
			auto value2_cv = value2_provider->get_cv();
			opcassert(value2_cv);

			float value1 = value1_cv->get_convert_float();
			float value2 = value2_cv->get_convert_float();

			auto new_context = _context;
			new_context.tva_index--;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = powf(%s, %s)", new_context.tva_index, value2_provider->get_cv_formatted().c_str(), value1_provider->get_cv_formatted().c_str());

			float result = 0;
			if (value2 > 0) {
				result = powf(value2, value1);
			}
			value.type = StatescriptPrimitive_FLT;
			*(float*)&value.value = result;

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index - 1);
		}

		~Opcode_105_Pow() {
			value.reset();
		}
	};

	class Opcode_106_Negate : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 106 (1): TVA = negate(TVA)
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 0);

			auto value_provider = _context.tva[_context.tva_index];
			opcassert(value_provider);
			auto value_cv = value_provider->get_cv();
			opcassert(value_cv);
			if (value_cv->type == StatescriptPrimitive_ENTITY || value_cv->type == StatescriptPrimitive_VEC3) {
				Vector3 result = value_cv->get_convert_vec3(_context.parser->_ss) * -1;

				value.type = StatescriptPrimitive_VEC3;
				memcpy(&value.value, &result, sizeof(Vector3));
			}
			else {
				float result = -0.f;
				if (value_cv->type == StatescriptPrimitive_BYTE) {
					if ((char)value_cv->value)
						result = -1.f;
				}
				else if (value_cv->type == StatescriptPrimitive_INT) {
					result = (int)value_cv->value * -1;
				}
				else if (value_cv->type == StatescriptPrimitive_FLT) {
					result = *(float*)&value_cv->value * -1;
				}

				value.type = StatescriptPrimitive_FLT;
				*(float*)&value.value = result;
			}

			auto new_context = _context;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = %s * -1", new_context.tva_index, value_provider->get_cv_formatted().c_str());

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index);
		}

		~Opcode_106_Negate() {
			value.reset();
		}
	};

	class Opcode_120_IsFalsey : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 120 (1): TVA = IsFalsey(TVA)
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 0);

			auto value_provider = _context.tva[_context.tva_index];
			opcassert(value_provider);
			auto value_cv = value_provider->get_cv();
			opcassert(value_cv);

			bool is_falsey = ConfigVar_IsTruthy(value_cv, 0);
			value.type = StatescriptPrimitive_BYTE;
			value.value = is_falsey;

			auto new_context = _context;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = is_falsey(%s)", new_context.tva_index, value_provider->get_cv_formatted().c_str());

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index);
		}

		~Opcode_120_IsFalsey() {
			value.reset();
		}
	};

	class Opcode_140_Value1_GreaterEq : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 140 (1): value1 = TVA, TVA--, value2 = TVA, TVA = value1 >= value2
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value1_provider = _context.tva[_context.tva_index];
			opcassert(value1_provider);
			auto value1_cv = value1_provider->get_cv();
			opcassert(value1_cv);

			auto value2_provider = _context.tva[_context.tva_index - 1];
			opcassert(value2_provider);
			auto value2_cv = value2_provider->get_cv();
			opcassert(value2_cv);

			float value1 = value1_cv->get_convert_float();
			float value2 = value2_cv->get_convert_float();

			auto new_context = _context;
			new_context.tva_index--;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = %s >= %s", new_context.tva_index, value1_provider->get_cv_formatted().c_str(), value2_provider->get_cv_formatted().c_str());

			value.type = StatescriptPrimitive_BYTE;
			value.value = value1 >= (value2 - 0.000011920929f);

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index - 1);
		}

		~Opcode_140_Value1_GreaterEq() {
			value.reset();
		}
	};

	class Opcode_141_Value1_Greater : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 141 (1): value1 = TVA, TVA--, value2 = TVA, TVA = value1 > value2
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value1_provider = _context.tva[_context.tva_index];
			opcassert(value1_provider);
			auto value1_cv = value1_provider->get_cv();
			opcassert(value1_cv);

			auto value2_provider = _context.tva[_context.tva_index - 1];
			opcassert(value2_provider);
			auto value2_cv = value2_provider->get_cv();
			opcassert(value2_cv);

			float value1 = value1_cv->get_convert_float();
			float value2 = value2_cv->get_convert_float();

			auto new_context = _context;
			new_context.tva_index--;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = %s > %s", new_context.tva_index, value1_provider->get_cv_formatted().c_str(), value2_provider->get_cv_formatted().c_str());

			value.type = StatescriptPrimitive_BYTE;
			value.value = (value1 - 0.000011920929f) > value2;

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index - 1);
		}

		~Opcode_141_Value1_Greater() {
			value.reset();
		}
	};

	class Opcode_142_Value2_GreaterEq : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 142 (1): value1 = TVA, TVA--, value2 = TVA, TVA = value2 >= value1
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value1_provider = _context.tva[_context.tva_index];
			opcassert(value1_provider);
			auto value1_cv = value1_provider->get_cv();
			opcassert(value1_cv);

			auto value2_provider = _context.tva[_context.tva_index - 1];
			opcassert(value2_provider);
			auto value2_cv = value2_provider->get_cv();
			opcassert(value2_cv);

			float value1 = value1_cv->get_convert_float();
			float value2 = value2_cv->get_convert_float();

			auto new_context = _context;
			new_context.tva_index--;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = %s >= %s", new_context.tva_index, value2_provider->get_cv_formatted().c_str(), value1_provider->get_cv_formatted().c_str());

			value.type = StatescriptPrimitive_BYTE;
			value.value = (value2 + 0.000011920929f) >= value1;

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index - 1);
		}

		~Opcode_142_Value2_GreaterEq() {
			value.reset();
		}
	};

	class Opcode_143_Value2_Greater : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 143 (1): value1 = TVA, TVA--, value2 = TVA, TVA = value2 > value1
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value1_provider = _context.tva[_context.tva_index];
			opcassert(value1_provider);
			auto value1_cv = value1_provider->get_cv();
			opcassert(value1_cv);

			auto value2_provider = _context.tva[_context.tva_index - 1];
			opcassert(value2_provider);
			auto value2_cv = value2_provider->get_cv();
			opcassert(value2_cv);

			float value1 = value1_cv->get_convert_float();
			float value2 = value2_cv->get_convert_float();

			auto new_context = _context;
			new_context.tva_index--;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = %s > %s", new_context.tva_index, value2_provider->get_cv_formatted().c_str(), value1_provider->get_cv_formatted().c_str());

			value.type = StatescriptPrimitive_BYTE;
			value.value = value2 > (value1 + 0.000011920929f);

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index - 1);
		}

		~Opcode_143_Value2_Greater() {
			value.reset();
		}
	};

	class Opcode_144_IsEqual : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 144 (1): value1 = TVA, TVA--, value2 = TVA, TVA = value1 == value2
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value1_provider = _context.tva[_context.tva_index];
			opcassert(value1_provider);
			auto value1_cv = value1_provider->get_cv();

			auto value2_provider = _context.tva[_context.tva_index - 1];
			opcassert(value2_provider);
			auto value2_cv = value2_provider->get_cv();

			auto new_context = _context;
			new_context.tva_index--;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = %s == %s", new_context.tva_index, value1_provider->get_cv_formatted().c_str(), value2_provider->get_cv_formatted().c_str());

			value.type = StatescriptPrimitive_BYTE;
			value.value = ConfigVar_IsEqual(value1_cv, value2_cv);

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index - 1);
		}

		~Opcode_144_IsEqual() {
			value.reset();
		}
	};

	class Opcode_145_IsUnequal : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 145 (1): value1 = TVA, TVA--, value2 = TVA, TVA = value1 != value2
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value1_provider = _context.tva[_context.tva_index];
			opcassert(value1_provider);
			auto value1_cv = value1_provider->get_cv();

			auto value2_provider = _context.tva[_context.tva_index - 1];
			opcassert(value2_provider);
			auto value2_cv = value2_provider->get_cv();

			auto new_context = _context;
			new_context.tva_index--;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = %s != %s", new_context.tva_index, value1_provider->get_cv_formatted().c_str(), value2_provider->get_cv_formatted().c_str());

			value.type = StatescriptPrimitive_BYTE;
			value.value = !ConfigVar_IsEqual(value1_cv, value2_cv);

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index - 1);
		}

		~Opcode_145_IsUnequal() {
			value.reset();
		}
	};

	class Opcode_160_InverseSqrt : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 160 (1): value1 = TVA, TVA = inverse_sqrt(value1)
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 0);

			auto value_provider = _context.tva[_context.tva_index];
			opcassert(value_provider);
			auto value_cv = value_provider->get_cv();
			opcassert(value_cv);

			float result = value_cv->get_convert_float();

			value.type = StatescriptPrimitive_FLT;
			value.value = inverse_sqrt(result);

			auto new_context = _context;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = inverse_sqrt(%s)", new_context.tva_index, value_provider->get_cv_formatted().c_str());

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index);
		}

		~Opcode_160_InverseSqrt() {
			value.reset();
		}
	};

	class Opcode_161_Sin : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 161 (1): value1 = TVA, TVA = sinf(value1)
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 0);

			auto value_provider = _context.tva[_context.tva_index];
			opcassert(value_provider);
			auto value_cv = value_provider->get_cv();
			opcassert(value_cv);

			float result = value_cv->get_convert_float();

			value.type = StatescriptPrimitive_FLT;
			value.value = sinf(result);

			auto new_context = _context;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = sin(%s)", new_context.tva_index, value_provider->get_cv_formatted().c_str());

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index);
		}

		~Opcode_161_Sin() {
			value.reset();
		}
	};

	class Opcode_162_Cos : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 162 (1): value1 = TVA, TVA = cosf(value1)
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 0);

			auto value_provider = _context.tva[_context.tva_index];
			opcassert(value_provider);
			auto value_cv = value_provider->get_cv();
			opcassert(value_cv);

			float result = value_cv->get_convert_float();

			value.type = StatescriptPrimitive_FLT;
			value.value = cosf(result);

			auto new_context = _context;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = cos(%s)", new_context.tva_index, value_provider->get_cv_formatted().c_str());

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index);
		}

		~Opcode_162_Cos() {
			value.reset();
		}
	};

	class Opcode_163_SinDeg : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 163 (1): value1 = TVA, TVA = sinf(deg2rad(value1))
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 0);

			auto value_provider = _context.tva[_context.tva_index];
			opcassert(value_provider);
			auto value_cv = value_provider->get_cv();
			opcassert(value_cv);

			float result = value_cv->get_convert_float();

			value.type = StatescriptPrimitive_FLT;
			value.value = sinf(result * deg2rad);

			auto new_context = _context;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = sin(deg2rad(%s))", new_context.tva_index, value_provider->get_cv_formatted().c_str());

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index);
		}

		~Opcode_163_SinDeg() {
			value.reset();
		}
	};

	class Opcode_164_CosDeg : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 164 (1): value1 = TVA, TVA = cosf(deg2rad(value1))
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 0);

			auto value_provider = _context.tva[_context.tva_index];
			opcassert(value_provider);
			auto value_cv = value_provider->get_cv();
			opcassert(value_cv);

			float result = value_cv->get_convert_float();

			value.type = StatescriptPrimitive_FLT;
			value.value = cosf(result * deg2rad);

			auto new_context = _context;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = cos(deg2rad(%s))", new_context.tva_index, value_provider->get_cv_formatted().c_str());

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index);
		}

		~Opcode_164_CosDeg() {
			value.reset();
		}
	};

	class Opcode_165_166_VectorLen : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 165, 166 (1): value1 = TVA, TVA = vec_len(value1)
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 0);

			auto value_provider = _context.tva[_context.tva_index];
			opcassert(value_provider);
			auto value_cv = value_provider->get_cv();
			opcassert(value_cv);

			if (value_cv->type == StatescriptPrimitive_ENTITY || value_cv->type == StatescriptPrimitive_VEC3) {
				Vector3 result = value_cv->get_convert_vec3(_context.parser->_ss);

				value.type = StatescriptPrimitive_FLT;
				*(float*)&value.value = result.length();
				//memcpy(&value.value, &result, sizeof(Vector3));
			}
			else {
				value.type = StatescriptPrimitive_FLT;
				*(float*)&value.value = value_cv->get_convert_float();
			}

			auto new_context = _context;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = vector_len(%s)", new_context.tva_index, value_provider->get_cv_formatted().c_str());

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index);
		}

		~Opcode_165_166_VectorLen() {
			value.reset();
		}
	};

	class Opcode_167_VecDist : public Opcode<1> {
	public:
		using Opcode::Opcode;
		//TODO: vector calculation seems to be value1 - value2, but primitive seems to be value2 - value1?
		/// 167 (1): value1 = TVA, TVA--, value2 = TVA, TVA = vec_len(value1 - value2)
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value1_provider = _context.tva[_context.tva_index];
			opcassert(value1_provider);
			auto value1_cv = value1_provider->get_cv();
			opcassert(value1_cv);

			auto value2_provider = _context.tva[_context.tva_index - 1];
			opcassert(value2_provider);
			auto value2_cv = value2_provider->get_cv();
			opcassert(value2_cv);

			auto new_context = _context;
			new_context.tva_index--;
			new_context.tva[new_context.tva_index] = this;

			if ((value1_cv->type == StatescriptPrimitive_ENTITY || value1_cv->type == StatescriptPrimitive_VEC3) &&
				(value2_cv->type == StatescriptPrimitive_ENTITY || value2_cv->type == StatescriptPrimitive_VEC3)) {
				Vector3 value1 = value1_cv->get_convert_vec3(_context.parser->_ss);
				Vector3 value2 = value2_cv->get_convert_vec3(_context.parser->_ss);

				value.type = StatescriptPrimitive_FLT;
				*(float*)&value.value = (value1 - value2).length();

				ImGui::Text("TVA-%d = vec_len(%s - %s)", new_context.tva_index, value1_provider->get_cv_formatted().c_str(), value2_provider->get_cv_formatted().c_str());
			}
			else {
				float value1 = value1_cv->get_convert_float();
				float value2 = value2_cv->get_convert_float();

				value.type = StatescriptPrimitive_FLT;
				*(float*)&value.value = value2 - value1;

				ImGui::Text("TVA-%d = vec_len(%s - %s)", new_context.tva_index, value2_provider->get_cv_formatted().c_str(), value1_provider->get_cv_formatted().c_str());
			}

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index - 1);
		}

		~Opcode_167_VecDist() {
			value.reset();
		}
	};

	class Opcode_168_DotProduct : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 168 (1): value1 = TVA, TVA--, value2 = TVA, TVA = dot_product(value1, value2)
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value1_provider = _context.tva[_context.tva_index];
			opcassert(value1_provider);
			auto value1_cv = value1_provider->get_cv();
			opcassert(value1_cv);

			auto value2_provider = _context.tva[_context.tva_index - 1];
			opcassert(value2_provider);
			auto value2_cv = value2_provider->get_cv();
			opcassert(value2_cv);

			auto new_context = _context;
			new_context.tva_index--;
			new_context.tva[new_context.tva_index] = this;

			if ((value1_cv->type == StatescriptPrimitive_ENTITY || value1_cv->type == StatescriptPrimitive_VEC3) &&
				(value2_cv->type == StatescriptPrimitive_ENTITY || value2_cv->type == StatescriptPrimitive_VEC3)) {
				Vector3 value1 = value1_cv->get_convert_vec3(_context.parser->_ss);
				Vector3 value2 = value2_cv->get_convert_vec3(_context.parser->_ss);

				value.type = StatescriptPrimitive_FLT;
				*(float*)&value.value = value1.dot_product(value2);
			}
			else {
				float value1 = value1_cv->get_convert_float();
				float value2 = value2_cv->get_convert_float();

				value.type = StatescriptPrimitive_FLT;
				*(float*)&value.value = value1 * value2;
			}

			ImGui::Text("TVA-%d = dot_product(%s, %s)", new_context.tva_index, value1_provider->get_cv_formatted().c_str(), value2_provider->get_cv_formatted().c_str());

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index - 1);
		}

		~Opcode_168_DotProduct() {
			value.reset();
		}
	};

	class Opcode_169_CrossProduct : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 169 (1): value1 = TVA, TVA--, value2 = TVA, TVA = cross_product(value1, value2)
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value1_provider = _context.tva[_context.tva_index];
			opcassert(value1_provider);
			auto value1_cv = value1_provider->get_cv();
			opcassert(value1_cv);

			auto value2_provider = _context.tva[_context.tva_index - 1];
			opcassert(value2_provider);
			auto value2_cv = value2_provider->get_cv();
			opcassert(value2_cv);

			auto new_context = _context;
			new_context.tva_index--;
			new_context.tva[new_context.tva_index] = this;

			Vector3 value1 = value1_cv->get_convert_vec3(_context.parser->_ss);
			Vector3 value2 = value2_cv->get_convert_vec3(_context.parser->_ss);

			Vector3 result = value1.cross_product(value2);
			value.type = StatescriptPrimitive_FLT;
			memcpy(&value.value, &result, sizeof(Vector3));

			ImGui::Text("TVA-%d = cross_product(%s, %s)", new_context.tva_index, value1_provider->get_cv_formatted().c_str(), value2_provider->get_cv_formatted().c_str());

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index - 1);
		}

		~Opcode_169_CrossProduct() {
			value.reset();
		}
	};

	class Opcode_170_VecNormalize : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 170 (1): value1 = TVA, TVA = vector_normalize(value1)
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value_provider = _context.tva[_context.tva_index];
			opcassert(value_provider);
			auto value_cv = value_provider->get_cv();
			opcassert(value_cv);

			auto new_context = _context;
			new_context.tva[new_context.tva_index] = this;

			if (value_cv->type == StatescriptPrimitive_ENTITY || value_cv->type == StatescriptPrimitive_VEC3) {
				Vector3 result = value_cv->get_convert_vec3(_context.parser->_ss).normalize();
				value.type = StatescriptPrimitive_VEC3;
				memcpy(&value.value, &result, sizeof(Vector3));
			}
			else {
				float result = value_cv->get_convert_float();
				value.type = StatescriptPrimitive_FLT;
				*(float*)&value.value = result < 0 ? -1.f : 1.f;
			}

			ImGui::Text("TVA-%d = vec_normalize(%s)", new_context.tva_index, value_provider->get_cv_formatted().c_str());

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index);
		}

		~Opcode_170_VecNormalize() {
			value.reset();
		}
	};

	class Opcode_171_CreateVec : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 171 (1): value1 = TVA, TVA--, value2 = TVA, TVA--, value3 = TVA, TVA = vec3(value3, value2, value1)
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value1_provider = _context.tva[_context.tva_index];
			opcassert(value1_provider);
			auto value1_cv = value1_provider->get_cv();
			opcassert(value1_cv);

			auto value2_provider = _context.tva[_context.tva_index - 1];
			opcassert(value2_provider);
			auto value2_cv = value2_provider->get_cv();
			opcassert(value2_cv);

			auto value3_provider = _context.tva[_context.tva_index - 2];
			opcassert(value3_provider);
			auto value3_cv = value3_provider->get_cv();
			opcassert(value3_cv);

			auto new_context = _context;
			new_context.tva_index -= 2;
			new_context.tva[new_context.tva_index] = this;

			float Z = value1_cv->get_convert_float();
			float Y = value2_cv->get_convert_float();
			float X = value3_cv->get_convert_float();

			Vector3 result = Vector3(X, Y, Z);
			value.type = StatescriptPrimitive_FLT;
			memcpy(&value.value, &result, sizeof(Vector3));

			ImGui::Text("TVA-%d = vec3(%s, %s, %s)", 
				new_context.tva_index, 
				value3_provider->get_cv_formatted().c_str(),
				value2_provider->get_cv_formatted().c_str(),
				value1_provider->get_cv_formatted().c_str()
			);

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index - 2);
		}

		~Opcode_171_CreateVec() {
			value.reset();
		}
	};

	class Opcode_172_CreateFlt16Vec4 : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 172 (1): value1 = TVA, TVA--, value2 = TVA, TVA--, value3 = TVA, TVA--, value4 = TVA, TVA = flt16vec4(value4, value3, value2, value1)
		StatescriptPrimitive value{};

		struct flt16vec4 {
			uint16 X;
			uint16 Y;
			uint16 Z;
			uint16 W;
		};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value1_provider = _context.tva[_context.tva_index];
			opcassert(value1_provider);
			auto value1_cv = value1_provider->get_cv();
			opcassert(value1_cv);

			auto value2_provider = _context.tva[_context.tva_index - 1];
			opcassert(value2_provider);
			auto value2_cv = value2_provider->get_cv();
			opcassert(value2_cv);

			auto value3_provider = _context.tva[_context.tva_index - 2];
			opcassert(value3_provider);
			auto value3_cv = value3_provider->get_cv();
			opcassert(value3_cv);

			auto value4_provider = _context.tva[_context.tva_index - 3];
			opcassert(value4_provider);
			auto value4_cv = value4_provider->get_cv();
			opcassert(value4_cv);

			auto new_context = _context;
			new_context.tva_index -= 3;
			new_context.tva[new_context.tva_index] = this;

			float W = value1_cv->get_convert_float();
			float Z = value2_cv->get_convert_float();
			float Y = value3_cv->get_convert_float();
			float X = value4_cv->get_convert_float();

			flt16vec4 result = flt16vec4{ to_shortflt(X), to_shortflt(Y), to_shortflt(Z), to_shortflt(W) };
			value.type = StatescriptPrimitive_FLT16VEC4;
			memcpy(&value.value, &result, sizeof(flt16vec4));

			ImGui::Text("TVA-%d = flt16vec4(%s, %s, %s, %s)", 
				new_context.tva_index, 
				value4_provider->get_cv_formatted().c_str(),
				value3_provider->get_cv_formatted().c_str(),
				value2_provider->get_cv_formatted().c_str(),
				value1_provider->get_cv_formatted().c_str()
			);

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index - 3);
		}

		~Opcode_172_CreateFlt16Vec4() {
			value.reset();
		}
	};

	class Opcode_173_GetX : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 173 (1): value = TVA, TVA = value->X
		StatescriptPrimitive value{};


		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value_provider = _context.tva[_context.tva_index];
			opcassert(value_provider);
			auto value_cv = value_provider->get_cv();
			opcassert(value_cv);

			auto new_context = _context;
			new_context.tva[new_context.tva_index] = this;

			value.type = StatescriptPrimitive_FLT;
			*(float*)&value.value = value_cv->get_convert_vec3(nullptr).X;

			ImGui::Text("TVA-%d = get_x(%s)", new_context.tva_index, value_provider->get_cv_formatted().c_str());

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index);
		}

		~Opcode_173_GetX() {
			value.reset();
		}
	};

	class Opcode_174_GetY : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 174 (1): value = TVA, TVA = value->Y
		StatescriptPrimitive value{};


		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value_provider = _context.tva[_context.tva_index];
			opcassert(value_provider);
			auto value_cv = value_provider->get_cv();
			opcassert(value_cv);

			auto new_context = _context;
			new_context.tva[new_context.tva_index] = this;

			value.type = StatescriptPrimitive_FLT;
			*(float*)&value.value = value_cv->get_convert_vec3(nullptr).Y;

			ImGui::Text("TVA-%d = get_y(%s)", new_context.tva_index, value_provider->get_cv_formatted().c_str());

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index);
		}

		~Opcode_174_GetY() {
			value.reset();
		}
	};

	class Opcode_175_GetZ : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 175 (1): value = TVA, TVA = value->Z
		StatescriptPrimitive value{};


		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value_provider = _context.tva[_context.tva_index];
			opcassert(value_provider);
			auto value_cv = value_provider->get_cv();
			opcassert(value_cv);

			auto new_context = _context;
			new_context.tva[new_context.tva_index] = this;

			value.type = StatescriptPrimitive_FLT;
			*(float*)&value.value = value_cv->get_convert_vec3(nullptr).Z;

			ImGui::Text("TVA-%d = get_z(%s)", new_context.tva_index, value_provider->get_cv_formatted().c_str());

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index);
		}

		~Opcode_175_GetZ() {
			value.reset();
		}
	};

	class Opcode_200_Min : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 200 (1): value1 = TVA, TVA--, value2 = TVA, TVA = fminf(value2, value1)
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value1_provider = _context.tva[_context.tva_index];
			opcassert(value1_provider);
			auto value1_cv = value1_provider->get_cv();
			opcassert(value1_cv);

			auto value2_provider = _context.tva[_context.tva_index - 1];
			opcassert(value2_provider);
			auto value2_cv = value2_provider->get_cv();
			opcassert(value2_cv);

			float value1 = value1_cv->get_convert_float();
			float value2 = value2_cv->get_convert_float();

			auto new_context = _context;
			new_context.tva_index--;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = fminf(%s, %s)", new_context.tva_index, value2_provider->get_cv_formatted().c_str(), value1_provider->get_cv_formatted().c_str());

			value.type = StatescriptPrimitive_FLT;
			*(float*)&value.value = fminf(value1, value2);

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index - 1);
		}

		~Opcode_200_Min() {
			value.reset();
		}
	};

	class Opcode_201_Max : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 201 (1): value1 = TVA, TVA--, value2 = TVA, TVA = fmaxf(value2, value1)
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value1_provider = _context.tva[_context.tva_index];
			opcassert(value1_provider);
			auto value1_cv = value1_provider->get_cv();
			opcassert(value1_cv);

			auto value2_provider = _context.tva[_context.tva_index - 1];
			opcassert(value2_provider);
			auto value2_cv = value2_provider->get_cv();
			opcassert(value2_cv);

			float value1 = value1_cv->get_convert_float();
			float value2 = value2_cv->get_convert_float();

			auto new_context = _context;
			new_context.tva_index--;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = fmaxf(%s, %s)", new_context.tva_index, value2_provider->get_cv_formatted().c_str(), value1_provider->get_cv_formatted().c_str());

			value.type = StatescriptPrimitive_FLT;
			*(float*)&value.value = fmaxf(value1, value2);

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index - 1);
		}

		~Opcode_201_Max() {
			value.reset();
		}
	};

	class Opcode_202_Clamp : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 202 (1): value1 = TVA, TVA--, value2 = TVA, TVA--, value3 = TVA, TVA = clamp(value3, value2, value1) -> min/max sind value1/2, spielt keine rolle wo
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value1_provider = _context.tva[_context.tva_index];
			opcassert(value1_provider);
			auto value1_cv = value1_provider->get_cv();
			opcassert(value1_cv);

			auto value2_provider = _context.tva[_context.tva_index - 1];
			opcassert(value2_provider);
			auto value2_cv = value2_provider->get_cv();
			opcassert(value2_cv);

			auto value3_provider = _context.tva[_context.tva_index - 1];
			opcassert(value3_provider);
			auto value3_cv = value3_provider->get_cv();
			opcassert(value3_cv);

			float value1 = value1_cv->get_convert_float();
			float value2 = value2_cv->get_convert_float();
			float value3 = value3_cv->get_convert_float();

			auto new_context = _context;
			new_context.tva_index -= 2;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = clamp(%s, min: %s, max: %s)",
				new_context.tva_index,
				value3_provider->get_cv_formatted().c_str(),
				value2_provider->get_cv_formatted().c_str(),
				value1_provider->get_cv_formatted().c_str()
			);

			value.type = StatescriptPrimitive_FLT;
			*(float*)&value.value = fmaxf(fminf(value3, value1), value2);

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index - 2);
		}

		~Opcode_202_Clamp() {
			value.reset();
		}
	};

	class Opcode_203_Floor : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 203 (1): value = TVA, TVA = floor(value)
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value_provider = _context.tva[_context.tva_index];
			opcassert(value_provider);
			auto value_cv = value_provider->get_cv();
			opcassert(value_cv);

			auto new_context = _context;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = floor(%s)", new_context.tva_index, value_provider->get_cv_formatted().c_str());

			value.type = StatescriptPrimitive_FLT;
			*(float*)&value.value = floorf(value_cv->get_convert_float());

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index);
		}

		~Opcode_203_Floor() {
			value.reset();
		}
	};

	class Opcode_204_Ceil : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 204 (1): value = TVA, TVA = ceil(value)
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value_provider = _context.tva[_context.tva_index];
			opcassert(value_provider);
			auto value_cv = value_provider->get_cv();
			opcassert(value_cv);

			auto new_context = _context;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = ceil(%s)", new_context.tva_index, value_provider->get_cv_formatted().c_str());

			value.type = StatescriptPrimitive_FLT;
			*(float*)&value.value = ceilf(value_cv->get_convert_float());

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index);
		}

		~Opcode_204_Ceil() {
			value.reset();
		}
	};

	class Opcode_205_RoundToInt : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 205 (1): value = TVA, TVA = int_round(value)
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value_provider = _context.tva[_context.tva_index];
			opcassert(value_provider);
			auto value_cv = value_provider->get_cv();
			opcassert(value_cv);

			auto new_context = _context;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = round(%s)", new_context.tva_index, value_provider->get_cv_formatted().c_str());

			float val = value_cv->get_convert_float();
			int result = (int)val;
			if (val > (float)(int)val)
				result++;

			value.type = StatescriptPrimitive_FLT;
			*(float*)&value.value = (float)result;

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index);
		}

		~Opcode_205_RoundToInt() {
			value.reset();
		}
	};

	class Opcode_206_Random : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 206 (1): value1 = TVA, TVA--, value2 = TVA, TVA = random(value2, value1)
		StatescriptPrimitive value{};

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value1_provider = _context.tva[_context.tva_index];
			opcassert(value1_provider);
			auto value1_cv = value1_provider->get_cv();
			opcassert(value1_cv);

			auto value2_provider = _context.tva[_context.tva_index - 1];
			opcassert(value2_provider);
			auto value2_cv = value2_provider->get_cv();
			opcassert(value2_cv);

			value = ConfigVar_RandomBetween(value2_cv, value1_cv);

			auto new_context = _context;
			new_context.tva_index--;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = random(%s, %s)", new_context.tva_index, value2_provider->get_cv_formatted().c_str(), value1_provider->get_cv_formatted().c_str());

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index - 1);
		}

		~Opcode_206_Random() {
			value.reset();
		}
	};

	class Opcode_207_RandomRounded : public Opcode<1> {
	public:
		using Opcode::Opcode;
		/// 207 (1): value1 = TVA, TVA--, value2 = TVA, TVA = int_round(random(value2, value1) -> converts back to float
		StatescriptPrimitive value{};
		
		int __fastcall int_round(float f)
		{
			if (f >= 0.0)
				return (int)(float)(f + 0.5f);
			else
				return (int)(float)(f - 0.5f);
		}

		bool display_imgui() override {
			opcassert(_context.tva_index >= 1);

			auto value1_provider = _context.tva[_context.tva_index];
			opcassert(value1_provider);
			auto value1_cv = value1_provider->get_cv();
			opcassert(value1_cv);

			auto value2_provider = _context.tva[_context.tva_index - 1];
			opcassert(value2_provider);
			auto value2_cv = value2_provider->get_cv();
			opcassert(value2_cv);

			float value1 = value1_cv->get_convert_float();
			float value2 = value2_cv->get_convert_float();
			value1 -= value2;
			float result = 0;
			if (value1 < 0) {
				result = (float)int_round(value2) - (float)GetRandom_Int(int_round(value1 * -1));
			}
			else {
				result = GetRandom_Int(int_round(value1)) + int_round(value2);
			}
			value.type = StatescriptPrimitive_FLT;
			*(float*)&value.value = result;

			auto new_context = _context;
			new_context.tva_index--;
			new_context.tva[new_context.tva_index] = this;

			ImGui::Text("TVA-%d = random_rounded(%s, %s)", new_context.tva_index, value2_provider->get_cv_formatted().c_str(), value1_provider->get_cv_formatted().c_str());

			_context.parser->display_next(new_context, _context.opc_index + get_size());
			return true;
		}

		StatescriptPrimitive* get_cv() override {
			return &value;
		}

		std::string get_cv_origin() override {
			return "TVA-" + std::to_string(_context.tva_index - 1);
		}

		~Opcode_207_RandomRounded() {
			value.reset();
		}
	};

	StatescriptExpressionParser(STUConfigVarExpression* expr, StatescriptInstance* ss) : 
		_expr(expr),
		_ss(ss)
	{
	}

	void display() {
		ImGui::PushID(_expr);

		if (_expr->m_configVars.count() > 0) {
			int cnt = _expr->m_configVars.count();
			ImGui::Text("m_configVars: %d", cnt);
			ImGui::PushID("cv");
			ImGui::Indent();
			for (int i = 0; i < cnt; i++) {
				ImGui::PushID(i);

				ImGui::BulletText("CV-%d:", i);
				ImGui::SameLine();
				imgui_helpers::display_cv(_expr->m_configVars.list()[i], _ss, nullptr, false);

				ImGui::PopID();
			}
			ImGui::Unindent();
			ImGui::PopID();
			ImGui::NewLine();
		}
		if (_expr->m_expression.m_dynamicVars.count() > 0) {
			int cnt = _expr->m_expression.m_dynamicVars.count();
			ImGui::Text("m_dynamicVars: %d", cnt);
			ImGui::PushID("cvd");
			ImGui::Indent();
			for (int i = 0; i < cnt; i++) {
				ImGui::PushID(i);

				ImGui::BulletText("CVD-%d:", i);
				ImGui::SameLine();
				imgui_helpers::display_cv(&_expr->m_expression.m_dynamicVars.list()[i]->cv_base, _ss, nullptr, false);

				ImGui::PopID();
			}
			ImGui::Unindent();
			ImGui::PopID();
			ImGui::NewLine();
		}
		if (_expr->m_expression.float_constants.count() > 0) {
			int cnt = _expr->m_expression.float_constants.count();
			ImGui::Text("float constants: %d", cnt);
			ImGui::PushID("flt");
			ImGui::Indent();
			for (int i = 0; i < cnt; i++) {
				ImGui::PushID(i);

				ImGui::BulletText("FLT-%d: %f", i, _expr->m_expression.float_constants.list()[i]);

				ImGui::PopID();
			}
			ImGui::Unindent();
			ImGui::PopID();
			ImGui::NewLine();
		}

		display_next(ExecutionContext{this}, 0);
		ImGui::PopID();
	}
private:
	STUConfigVarExpression* _expr;
	StatescriptInstance* _ss;
	//std::shared_ptr<OpcodeBase> _tva[8]; //temp_value_array
	//SS_CV _temp_value_array[8];

	std::unique_ptr<OpcodeBase> get_by_opc(int opc) {
		switch (opc) {
		case 0:
			return std::unique_ptr<OpcodeBase>(new Opcode_0_Exit{});
		case 1:
			return std::unique_ptr<OpcodeBase>(new Opcode_1_Jump{});
		case 2:
			return std::unique_ptr<OpcodeBase>(new Opcode_2_4_FalseyConditionalJump(true));
		case 3:
			return std::unique_ptr<OpcodeBase>(new Opcode_3_5_TruthyConditionalJump(true));
		case 4:
			return std::unique_ptr<OpcodeBase>(new Opcode_2_4_FalseyConditionalJump(false));
		case 5:
			return std::unique_ptr<OpcodeBase>(new Opcode_3_5_TruthyConditionalJump(false));
		case 30:
			return std::unique_ptr<OpcodeBase>(new Opcode_30_LoadTVA_ConfigVar{});
		case 31:
			return std::unique_ptr<OpcodeBase>(new Opcode_31_LoadTVA_DynamicVar{});
		case 32:
			return std::unique_ptr<OpcodeBase>(new Opcode_32_SearchOrIndex_DynamicVar_Value{});
		case 33:
			return std::unique_ptr<OpcodeBase>(new Opcode_33_SearchOrIndex_DynamicVar_Key{});
		case 34:
			return std::unique_ptr<OpcodeBase>(new Opcode_34_SearchOrIndex_DynamicVar_ArrayIndex{});
		case 35:
			return std::unique_ptr<OpcodeBase>(new Opcode_35_LoadEntidTemp{});
		case 36:
			return std::unique_ptr<OpcodeBase>(new Opcode_36_LoadTVA_DynamicVar_Interceptor{});
		case 37:
			return std::unique_ptr<OpcodeBase>(new Opcode_37_SearchOrIndex_ConfigVar_Value{});
		case 38:
			return std::unique_ptr<OpcodeBase>(new Opcode_38_SearchOrIndex_ConfigVar_Key{});
		case 39:
			return std::unique_ptr<OpcodeBase>(new Opcode_39_SearchOrIndex_ConfigVar_ArrayIndex{});
		case 40:
			return std::unique_ptr<OpcodeBase>(new Opcode_40_Count_ConfigVarArray{});
		case 70:
			return std::unique_ptr<OpcodeBase>(new Opcode_70_Load_FloatConstant{});
		case 100:
			return std::unique_ptr<OpcodeBase>(new Opcode_100_Add{});
		case 101:
			return std::unique_ptr<OpcodeBase>(new Opcode_101_Sub{});
		case 102:
			return std::unique_ptr<OpcodeBase>(new Opcode_102_Mul{});
		case 103:
			return std::unique_ptr<OpcodeBase>(new Opcode_103_Div{});
		case 104:
			return std::unique_ptr<OpcodeBase>(new Opcode_104_Remainder{});
		case 105:
			return std::unique_ptr<OpcodeBase>(new Opcode_105_Pow{});
		case 106:
			return std::unique_ptr<OpcodeBase>(new Opcode_106_Negate{});
		case 120:
			return std::unique_ptr<OpcodeBase>(new Opcode_120_IsFalsey{});
		case 140:
			return std::unique_ptr<OpcodeBase>(new Opcode_140_Value1_GreaterEq{});
		case 141:
			return std::unique_ptr<OpcodeBase>(new Opcode_141_Value1_Greater{});
		case 142:
			return std::unique_ptr<OpcodeBase>(new Opcode_142_Value2_GreaterEq{});
		case 143:
			return std::unique_ptr<OpcodeBase>(new Opcode_143_Value2_Greater{});
		case 144:
			return std::unique_ptr<OpcodeBase>(new Opcode_144_IsEqual{});
		case 145:
			return std::unique_ptr<OpcodeBase>(new Opcode_145_IsUnequal{});
		case 160:
			return std::unique_ptr<OpcodeBase>(new Opcode_160_InverseSqrt{});
		case 161:
			return std::unique_ptr<OpcodeBase>(new Opcode_161_Sin{});
		case 162:
			return std::unique_ptr<OpcodeBase>(new Opcode_162_Cos{});
		case 163:
			return std::unique_ptr<OpcodeBase>(new Opcode_163_SinDeg{});
		case 164:
			return std::unique_ptr<OpcodeBase>(new Opcode_164_CosDeg{});
		case 165:
		case 166:
			return std::unique_ptr<OpcodeBase>(new Opcode_165_166_VectorLen{});
		case 167:
			return std::unique_ptr<OpcodeBase>(new Opcode_167_VecDist{});
		case 168:
			return std::unique_ptr<OpcodeBase>(new Opcode_168_DotProduct{});
		case 169:
			return std::unique_ptr<OpcodeBase>(new Opcode_169_CrossProduct{});
		case 170:
			return std::unique_ptr<OpcodeBase>(new Opcode_170_VecNormalize{});
		case 171:
			return std::unique_ptr<OpcodeBase>(new Opcode_171_CreateVec{});
		case 172:
			return std::unique_ptr<OpcodeBase>(new Opcode_172_CreateFlt16Vec4{});
		case 173:
			return std::unique_ptr<OpcodeBase>(new Opcode_173_GetX{});
		case 174:
			return std::unique_ptr<OpcodeBase>(new Opcode_174_GetY{});
		case 175:
			return std::unique_ptr<OpcodeBase>(new Opcode_175_GetZ{});
		case 200:
			return std::unique_ptr<OpcodeBase>(new Opcode_200_Min{});
		case 201:
			return std::unique_ptr<OpcodeBase>(new Opcode_201_Max{});
		case 202:
			return std::unique_ptr<OpcodeBase>(new Opcode_202_Clamp{});
		case 203:
			return std::unique_ptr<OpcodeBase>(new Opcode_203_Floor{});
		case 204:
			return std::unique_ptr<OpcodeBase>(new Opcode_204_Ceil{});
		case 205:
			return std::unique_ptr<OpcodeBase>(new Opcode_205_RoundToInt{});
		case 206:
			return std::unique_ptr<OpcodeBase>(new Opcode_206_Random{});
		case 207:
			return std::unique_ptr<OpcodeBase>(new Opcode_207_RandomRounded{});
		default:
			return std::unique_ptr<OpcodeBase>(nullptr);
		}
	}

	bool display_next(ExecutionContext context, int new_index) {
		opcassert(new_index < _expr->m_expression.m_opcodes.count());
		int opc_typ = _expr->m_expression.m_opcodes.list()[new_index];
		auto opc = get_by_opc(opc_typ);
		if (!opc) {
			ImGui::Text("Opcode %d not found!", opc_typ);
			return false;
		}
		opcassert(new_index + opc->get_size() <= _expr->m_expression.m_opcodes.count());
		context.opc_index = new_index;
		opc->_context = context;
		if (!opc->display_imgui()) {
			ImGui::Text("-> Opcode %d failed", opc_typ);
			return false;
		}
		return true;
	}

	friend class OpcodeBase;
};