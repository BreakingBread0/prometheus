#pragma once
#include "../window_manager/window_manager.h"
#include "../StatescriptExpressionParser.h"
#include "../Statescript.h"
#include "stu_explorer.h"

class cvexpression_viewer : public window {
	WINDOW_DEFINE(cvexpression_viewer, "Statescript", "ConfigVarExpression Viewer", true);

	void display_expression(STUConfigVarExpression* expr, StatescriptInstance* ss) {
		_expr = expr;
		_ss = ss;
	}

	inline void render() override {
		if (open_window()) {
			if (IsBadReadPtr(_expr, sizeof(STUConfigVarExpression))) {
				ImGui::TextUnformatted("Invalid Expression");
				return;
			}
			if (IsBadReadPtr(_ss, sizeof(StatescriptInstance))) {
				ImGui::TextUnformatted("Invalid Statescript Instance");
				return;
			}
			display_addr((__int64)_expr, "Expression");
			ImGui::SameLine();
			if (ImGui::Button("Explore")) {
				stu_explorer::get_latest_or_create(this)->navigate_to(_expr->cv_base.base.vfptr->GetSTUInfo(), (__int64)_expr, _ss);
			}
			display_addr((__int64)_ss, "Statescript Instance");
			StatescriptExpressionParser parser(_expr, _ss);
			parser.display();
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
private:
	STUConfigVarExpression* _expr{};
	StatescriptInstance* _ss{};
};

WINDOW_REGISTER(cvexpression_viewer);