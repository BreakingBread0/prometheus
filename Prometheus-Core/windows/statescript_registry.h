#pragma once
#include "../window_manager/window_manager.h"
#include "../Statescript.h"
#include "search_helper.h"

class statescript_registry : public window {
	WINDOW_DEFINE(statescript_registry, "Statescript", "Statescript Registry", true);

	void render() override;

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}

private:
	search_helper_imgui _search;
	StatescriptRTTI* selected = nullptr;
	STURegistry* headerBase;

	bool CanShowRTTI(StatescriptRTTI* rtti, STURegistry* header);
	STURegistry* FindRegistryForRTTI(const StatescriptRTTI* rtti) const;
};

WINDOW_REGISTER(statescript_registry);
