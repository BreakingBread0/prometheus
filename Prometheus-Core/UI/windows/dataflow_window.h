#pragma once
#include "../window_manager/window_manager.h"
#include "../../Components/Component_3_DataFlow.h"
#include "../../globals.h"

class dataflow_window : public window {
	WINDOW_DEFINE_ARG(dataflow_window, "Game", "Dataflow Window", Component_3_DataFlow*);

	void print_dataflow_value(DataFlowValue value)
	{

	}

	inline void render() override {
		if (open_window()) {
			if (IsBadReadPtr(_arg, sizeof(Component_3_DataFlow)))
			{
				ImGui::Text("Invalid Component");
				return;
			}


		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(dataflow_window);

#endif