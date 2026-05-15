#include "entity_admin.h"
#include "Statescript.h"
#include "game.h"
#include <imgui.h>
#include <MinHook.h>
#include "STU_Editable.h"
#include "player_spawner.h"
#include "StatescriptVar.h"
#include "stu_resources.h"
#include "MovementState.h"
#include "Viewmodel.h"
#include "state_replicator.h"
#include "serialization.h"
#include "demo_cammoves.cpp"
#include "TimeScale.h"
#include "Components/Component_4_Model.h"
#include "Logs/Logs.h"
#include "Components/Component_54_LobbyMap.h"
#include "UI/windows/entity_bounds_renderer.h"

__int64 PrometheusSystem::get_inheritance() {
	return NULL;
}
bool PrometheusSystem::is_assignable_to(__int64) {
	return false;
}
bool PrometheusSystem::is_instance_of(__int64) {
	return false;
}

const char* PrometheusSystem::get_system_name() {
	return "PrometheusSystem";
}
void PrometheusSystem::deallocate() {
	
}

void PrometheusSystem::OnInitialize() {
	_local_controller_entities.ptr = nullptr;
	_local_controller_entities.num = 0;
	_local_controller_entities.max = 0;
	memset(_old_lbutton_states, 0, sizeof(_old_lbutton_states)); //Removing this crashes the game on initialization. You know what? idc.
	_input_component = nullptr;
	_world = nullptr;

	//0xcc6110 0xcc67f5
	//0xcc57d3 0xcc4c22
}

void PrometheusSystem::field_1() {
	auto str = stacktrace_str();
	printf("System field_1:\n%s\n", str.c_str());
}
void PrometheusSystem::PreDelete() {
	auto str = stacktrace_str();
	printf("System PreDelete:\n%s\n", str.c_str());
}

char* PrometheusSystem::GetSubscriptions(int* count) {
	static char subscribed_components[] = {
		0x2F
	};
	*count = sizeof(subscribed_components);
	return subscribed_components;
}
void PrometheusSystem::OnCreationAfterEmplaced(Entity* ent) {
	printf("OnDeferredCreation: %x\n", ent->entity_id);
}
void PrometheusSystem::OnCreation(Entity* cb_ent) {
	printf("OnCreation: %x\n", cb_ent->entity_id);
	if (cb_ent->getById(0x2F)) {
		_local_controller_entities.emplace_item(cb_ent);
		_newly_added_entities.push_back(cb_ent);
		_trigger_helloVoiceLine_ticks = 50;
	}
}
void PrometheusSystem::field_3() {
	auto str = stacktrace_str();
	printf("System field_3:\n%s\n", str.c_str());
}
void PrometheusSystem::OnDeletion(Entity* ent) {
	//TODO This does apparently not work
	for (int i = 0; i < _local_controller_entities.num; i++) {
		if (_local_controller_entities.ptr[i] == ent)
			_local_controller_entities.remove_item(i--);
	}
}

string_rep* (*get_displayString_orig)(string_rep*, __int64 id, __int64);
string_rep* get_displayString_func(string_rep* a1, __int64 id, __int64 a3) {
	if (id == 0xde00000000005df) {
		const char* str = "Welcome Back, Hero!";
		a1->extend_to(strlen(str));
		strcpy(a1->get(), str);
		a1->actual_size = strlen(str);
		return a1;
	}
	return get_displayString_orig(a1, id, a3);
}

void play_game_btn() {
	PrometheusSystem::instance()->demo_join_game = true;
}

//0xd774b0
char SendHeroSelection(StatescriptAction_vt** impl, StatescriptState* state, StatescriptInstance* ss, STUBase<>* stu_data) {
	auto stu = stu_data->to_editable();
	auto hero = stu.get_argument_object("m_hero");
	auto hero_cv = (STUConfigVar*)hero.value;
	StatescriptPrimitive hero_value;
	hero_cv->get_value(ss, &hero_value);

	auto behavior = stu.get_argument_primitive("m_behavior").get_value<int>();
	/*printf("HeroSelect behavior: %hx\n", behavior);*/
	//0 = just send teammate preview
	if (hero_value.value != 0 && behavior == 1) {
		owassert(hero_value.type == StatescriptPrimitive_INT64);
		printf("Selected Hero: %p\n", hero_value.value);

		auto old_local = GameEntityAdmin()->getLocalEnt();
		if (old_local) {
			auto hero_info = old_local->getById<Component_3F_PlayerInfo>(0x3F);
			if (hero_info->selected_heroid == hero_value.value) {
				return 1;
			}
		}

		PrometheusSystem::instance()->DeleteLocalEnt();

		auto hero_stu = stu_resources::GetByID(hero_value.value);
		auto model_entid = hero_stu->to_editable().get_argument_resource("m_gameplayEntity")->resource_id;

		player_spawner spawner(model_entid);
		spawner.controller_info.heroid = hero_value.value;
		auto local_ents = spawner.spawn();

		//TODO Move
		state_replicator::ChangeHeroMessage msg;
		msg.heroid = hero_value.value;
		msg.skinid = 0;
		state_replicator::emplace_hero_update(msg);
	}
	return 1;
}

//0xd94930
char matchmaking_cv(__int64, __int64, __int64, StatescriptPrimitive* result) {
	result->type = StatescriptPrimitive_BYTE;
	result->value = PrometheusSystem::instance()->demo_join_game;
	return 1;
}

void pachipachi(Vector4 position, Vector4 rotation, Vector4 scaling) {
	auto res = try_load_resource(0x04000000000006CA);
	if (res == 0) //Not valid on all maps, this is only for hanamura
		return;
	auto ea = GameEntityAdmin();
	auto loader = EntityLoader::Create(0x04000000000006CA, res, false, false);
	//making this a stack local variable crashes the game EXACTLY at 0x00A04249
	//Why is this? I dont know
	//But at this point I dont care anymore.
	auto init_data = new Component_1_SceneRendering::InitData{};
	init_data->position = position;
	init_data->rotation = rotation;
	init_data->scale = scaling;

	loader->loader_entries[1].init_data = (__int64)init_data;
	delete init_data;
	loader->Spawn(ea);
}

struct demo_lobbyview {
	__int64 map_id;
	Entity* ent;
};

bool Entity::Debug_Highlight(uint color)
{
	auto model = this->getById<Component_4_Model>(4);
	if (model)
	{
		model->EnableOutline(Component_4_Model::ModelRenderingFlags::OUTLINE_INVISIBLE, color);
		return true;
	} else
	{
		LOG_CORE(Warn, "Cannot enable outline for entity {:s}: Has no model", toString());
		entity_bounds_renderer::create(nullptr)->set(this);
	}
	return false;
}

void PrometheusSystem::OnTick(float tick) {
	Entity* local_controller = _game_ea->getLocalEnt();
	Entity* local_model = nullptr;

	if (local_controller) {
		auto pet_controller = local_controller->getById<Component_20_ModelReference>(0x20);
		if (pet_controller) {
			local_model = _game_ea->getEntById(pet_controller->cam_attach_entid);
			if (local_model) {
				auto ss = local_model->getById<Component_23_Statescript>(0x23);
				auto health_stru = local_model->getById<Component_28_STUHealthComponent>(0x28);

				float health = health_stru->vfptr->GetFullCurrentHealth(health_stru);
				float health_max = health_stru->vfptr->GetFullMaxHealth(health_stru);

				StatescriptPrimitive health_cv{};
				health_cv.type = StatescriptPrimitive_FLT;

				*(float*)&health_cv.value = health;
				ss->ss_inner.rid_entity_varbag->SetVar({ StatescriptVar_ENTITY_VARBAG, 0x0D800000000006C4 }, health_cv);

				*(float*)&health_cv.value = health_max;
				ss->ss_inner.rid_entity_varbag->SetVar({ StatescriptVar_ENTITY_VARBAG, 0x0D800000000006C9 }, health_cv);

				//Naming is a COMPLETE guess. When this is 1 the statescript component does not add the "spectator" graph. Assuming 1 == alive?
				StatescriptPrimitive alive{};
				alive.type = StatescriptPrimitive_BYTE;
				alive.value = 1;
				ss->ss_inner.rid_entity_varbag->SetVar({ StatescriptVar_ENTITY_VARBAG, 0xd8000000000094c }, alive);
			}
		}
	}

	if (globals::isDemo) {
		if (!_applied_demo) {
			_applied_demo = true;

			//MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0x8537c0), (LPVOID)get_displayString_func, (PVOID*)&get_displayString_orig));
			//MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0x8537c0)));

			MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xc87220), (LPVOID)play_game_btn, (PVOID*)0));
			MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xc87220)));

			MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xd774b0), (LPVOID)SendHeroSelection, (PVOID*)0));
			MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xd774b0)));

			MH_VERIFY(MH_CreateHook((PVOID)(globals::gameBase + 0xd94930), (LPVOID)matchmaking_cv, (PVOID*)0));
			MH_VERIFY(MH_EnableHook((PVOID)(globals::gameBase + 0xd94930)));

			auto lobby_ss = LobbyEntityAdmin()->getSingletonComponent<Component_23_Statescript>(0x23);
			lobby_ss->ss_inner.getByResourceId(0x58000000000041a)->StartStopState(9, true); //developer mode

			//*(char*)(globals::gameBase + 0x1859db4) = 1; //tank_showMode
			//*(char*)(globals::gameBase + 0x1859dcc) = 1; //feature flag 11 (developer mode?)

			//0xc87220

			auto settings = LobbyEntityAdmin()->getSingletonComponent<Component_53_Settings>(0x53);
			settings->tutorial_dialog_dismissed = 1;
		}

	
		auto vmroot = GetViewModelRoot();
		for (auto vm : vmroot->viewmodel_list) {
			auto name = vm->vfptr->GetName();
			if (!strcmp(name, "LobbyMainMenuVM")) {
				__int64 buttons[] = { 0xac0000000000223, 0xac0000000000916, 0xac0000000000917, 0xac0000000000918, 0xac0000000000936, 0xad2ab0c7 };
				for (auto button : buttons) {
					auto play_enabled = vm->getByKey(button);
					if (play_enabled) {
						play_enabled->property.has_value = true;
						play_enabled->property.value = !demo_join_game;
						SendViewModelPropUpdate(vm, play_enabled);
					}
				}
			}
			else if (!strcmp(name, "LobbyPartyMemberMeVM")) {
				{
					auto name = vm->getByKey(0xfe11d138);
					string_rep* rep = (string_rep*)name->property.value;
					const char* new_name = "Jeff Kaplan";
					rep->extend_to(strlen(new_name));
					rep->actual_size = strlen(new_name);
					strcpy(rep->get(), new_name);
					SendViewModelPropUpdate(vm, name);
				}
				{
					auto background = vm->getByKey(0x7cac602a);
					background->property.value = 0xac00000000002cd;
					SendViewModelPropUpdate(vm, background);
				}
				{
					auto icon = vm->getByKey(0x11db7719);
					icon->property.value = 0xc0000000000135c;
					SendViewModelPropUpdate(vm, icon);
				}
			}
		}
	}
	if (!_input_component) {
		_input_component = LobbyEntityAdmin()->getSingletonComponent<Component_50_Input>(0x50);
	}
	else {
		if (ImGui::IsKeyPressed(ImGuiKey_M, false)) {
			//0x580000000000352
			if (local_model) {
				auto ss = local_model->getById<Component_23_Statescript>(0x23);
				if (ss) {
					auto script = ss->ss_inner.getByResourceId(0x580000000000352);
					if (script) {
						script->ToggleState(17); //toggle_map logical button does not work
					}
				}
			}
		}

		if (ImGui::IsKeyPressed(ImGuiKey_UpArrow, false)) {
			for (auto ent : _local_controller_entities) {
				auto ref_comp = ent->getById<Component_20_ModelReference>(0x20);
				auto aim_entid = ref_comp->aim_entid;
				//Many checks are probably not needed but better be safe than sorry...
				if (aim_entid > 0) {
					auto aim_ent = _game_ea->getEntById(aim_entid);
					if (aim_ent) {
						auto comp = aim_ent->getById<__int64>(0x15);
						if (comp) {
							((Vector4*)((__int64)comp + 0xC70))->Y += 10.f;
						}
					}
				}
			}
		} else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow, false)) {
			for (auto ent : _local_controller_entities) {
				auto ref_comp = ent->getById<Component_20_ModelReference>(0x20);
				auto aim_entid = ref_comp->aim_entid;
				//Many checks are probably not needed but better be safe than sorry...
				if (aim_entid > 0) {
					auto aim_ent = _game_ea->getEntById(aim_entid);
					if (aim_ent) {
						auto comp = aim_ent->getById<__int64>(0x15);
						if (comp) {
							((Vector4*)((__int64)comp + 0xC70))->Y -= 2.f;
						}
					}
				}
			}
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false)) {
			(*TimeScale::ServerPhysicsTimescale()) -= 0.2f;
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow, false)) {
			(*TimeScale::ServerPhysicsTimescale()) += 0.2f;
		}

		//memcpy(_old_lbutton_states, curr, sizeof(_old_lbutton_states));
	}
	
	if (_trigger_helloVoiceLine_ticks > 0) {
		auto heroSelectActive_cvget = (char(*)(__int64, __int64, __int64, StatescriptPrimitive*))(globals::gameBase + 0xd8ee00);
		StatescriptPrimitive cv{};
		if (heroSelectActive_cvget(0, 0, 0, &cv) && !cv.is_truthy()) {
			if (--_trigger_helloVoiceLine_ticks == 0 && local_model) {
				if (local_model->resload_entry->valid()) {
					auto model_resid = local_model->resload_entry->align()->resource_id;
					auto mapping = s_modelEntToHelloVoiceMap.find(model_resid);
					if (mapping != s_modelEntToHelloVoiceMap.end()) {
						VoiceSystem_PlayVoice play_stru;
						play_stru.entity_1 = play_stru.entity_2 = play_stru.instigator_ent = local_model->entity_id;

						play_stru.field_20 = 0;
						play_stru.field_21 = 0;
						play_stru.voice_stimulus = 0xee0000000000001; //just taking the first one worked... lol
						play_stru.voice_line = mapping->second;

						auto voice_sys = GetVoiceSystem(_game_ea);
						voice_sys->vfptr->PlayVoice(voice_sys, &play_stru);
					}
				}
			}
		}
	}

	if (local_freeLookView) {
		for (auto ent : _local_controller_entities) {
			auto ref_comp = ent->getById<Component_20_ModelReference>(0x20);
			auto aim_entid = ref_comp->aim_entid;
			//Many checks are probably not needed but better be safe than sorry...
			if (aim_entid > 0) {
				auto aim_ent = _game_ea->getEntById(aim_entid);
				if (aim_ent) {
					auto filterbits = aim_ent->getById<Component_10_FilterBits>(0x10);
					filterbits->filter_bits |= 0x10 | 0x400000;
				}
			}
		}

		Vector4 position_delta{};
		/*if (_local_controller_entities.num > 0) {
			auto lp = _local_controller_entities.ptr[0]->getById<Component_2F_LocalPlayer>(0x2f);
			if (lp) {
				local_freeLookView->base.vfptr->field_30(&local_freeLookView->base, lp->mouse_rel_x, lp->mouse_rel_y);
			}
		}*/
		auto getMouseRel_fn = (void(__fastcall*)(Component_50_Input*, float* x, float* y))(globals::gameBase + 0xcee3f0);
		float x, y;
		if (_input_component) {
			__int64 viewmodel_list = *(__int64*)(globals::gameBase + 0x18a6688);
			if (viewmodel_list && !*(char*)(viewmodel_list + 0x95A)) {
				getMouseRel_fn(_input_component, &x, &y);
				local_freeLookView->base.vfptr->field_30(&local_freeLookView->base, x, y);

				float delta = freeLookView_movDelta;
				if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
					delta *= 2;
				}
				if (ImGui::IsKeyDown(ImGuiKey_W)) {
					position_delta.Z += delta;
				}
				else if (ImGui::IsKeyDown(ImGuiKey_S)) {
					position_delta.Z -= delta;
				}
				if (ImGui::IsKeyDown(ImGuiKey_A)) {
					position_delta.X -= delta;
				}
				else if (ImGui::IsKeyDown(ImGuiKey_D)) {
					position_delta.X += delta;
				}
			}
		}
		local_freeLookView->position_delta = position_delta;
	}
	else {
		for (auto ent : _local_controller_entities) {
			auto ref_comp = ent->getById<Component_20_ModelReference>(0x20);
			auto aim_entid = ref_comp->aim_entid;
			//Many checks are probably not needed but better be safe than sorry...
			if (aim_entid > 0) {
				auto aim_ent = _game_ea->getEntById(aim_entid);
				if (aim_ent) {
					auto filterbits = aim_ent->getById<Component_10_FilterBits>(0x10);
					filterbits->filter_bits &= ~(0x10 | 0x400000);
				}
			}
		}
	}

	for (auto it = _newly_added_entities.begin(); it != _newly_added_entities.end();) {
		Entity* controller = *it;
		Entity* model = _game_ea->getEntById(controller->getById<Component_20_ModelReference>(0x20)->aim_entid);
		if (model) {
			//Moved into player spawner
			/*auto ss = model->getById<StatescriptComponent>(0x23);
			StatescriptPrimitive cv{};
			cv.type = StatescriptPrimitive_BYTE;
			cv.value = 1;
			ss->ss_inner.rid_entity_varbag->SetVar({ StatescriptVar_ENTITY_VARBAG, 0x0D800000000000FD }, cv);*/
			if (globals::isDemo) {
				auto localplayer = controller->getById<Component_2F_LocalPlayer>(0x2F);
				localplayer->SetRotation(Vector4(0.000000, -0.713039, 0.000000, 0));
				auto comp = model->getById<__int64>(0x15);
				if (comp) {
					*((Vector4*)((__int64)comp + 0xC70)) = Vector4(108.336914, 0.999023, 0.142578, 0);
					//Is inside MovementState, i need to finisht the demo this is ugly ;(
				}
			}

			auto tell_statescript_this_is_localent = (uint64(__fastcall*)(Entity*, uint*))(globals::gameBase + 0xd07830);
			tell_statescript_this_is_localent(model, &controller->entity_id);

			it = _newly_added_entities.erase(it);
			break;
		}

		++it;
	}

	if (!_world)
		_world = get_system27_WorldEngineSystem(_game_ea);
	else {
		if (_world->world_state == 6) {
			_world->Call_OnStateChance();

			auto init_ss_fn = (void(*)(Component_23_Statescript*))(globals::gameBase + 0x103acb0);
			for (auto ent : *_game_ea) {
				auto ss = ent->getById<Component_23_Statescript>(0x23);
				if (ss)
					init_ss_fn(ss);
			}

			if (globals::isDemo) {
				//state_replicator::start_server();
			}
			
			if (globals::isDemo) {

			}
		}
		else if (_world->world_state == 5) {
			_world->Call_OnStateChance();
		}
	}
}

PrometheusSystem* PrometheusSystem::create(EntityAdminBase* ea) {
	if (s_instance) {
		printf("PrometheusSystem already created! %p\n", s_instance);
		return nullptr;
	}

	auto sys = new PrometheusSystem;
	//sys->vfptr = new PrometheusSystem_vt;
	sys->_game_ea = ea;
	ea->GameEA_mapfunc_arr.emplace_item((mapsystem_callback_vt**)&sys->_mapfunc);
	s_instance = sys;

	MH_VERIFY_RET(MH_CreateHook((PVOID)(globals::gameBase + 0xcfe920), (LPVOID)deallocate_view_hook, (LPVOID*)&deallocate_view_orig), nullptr);
	MH_VERIFY_RET(MH_EnableHook((PVOID)(globals::gameBase + 0xcfe920)), nullptr);
	return sys;
}

View* PrometheusSystem::deallocate_view_hook(View* view, char dealloc) {
	if (view == &s_instance->local_freeLookView->base) {
		s_instance->local_freeLookView = nullptr;
	}
	return deallocate_view_orig(view, dealloc);
}

void PrometheusSystem_Mapfunc::field_0() {
	printf("PrometheusSystem_Mapfunc: field_0\n");
}
void PrometheusSystem_Mapfunc::nullsub_1() {
	printf("PrometheusSystem_Mapfunc: nullsub_1\n");
}
void PrometheusSystem_Mapfunc::change_state_3_globalloading() {
	printf("PrometheusSystem_Mapfunc: change_state_3_globalloading\n");
}
void PrometheusSystem_Mapfunc::change_state_4_worldloading() {
	printf("PrometheusSystem_Mapfunc: change_state_4_worldloading\n");
}
void PrometheusSystem_Mapfunc::change_state_5_worldloaded() {
	printf("PrometheusSystem_Mapfunc: change_state_5_worldloaded\n");
}
void PrometheusSystem_Mapfunc::change_state_6_world_replicating() {
	printf("PrometheusSystem_Mapfunc: change_state_6_world_replicating\n");
}
void PrometheusSystem_Mapfunc::change_state_7_gamemode_loading() {
	printf("PrometheusSystem_Mapfunc: change_state_7_gamemode_loading\n");
}
void PrometheusSystem_Mapfunc::maybe_on_gamemode_unload() {
	printf("PrometheusSystem_Mapfunc: maybe_on_gamemode_unload\n");
}
void PrometheusSystem_Mapfunc::on_gamemode_loading() {
	printf("PrometheusSystem_Mapfunc: on_gamemode_loading\n");
}
void PrometheusSystem_Mapfunc::maybe_on_map_unload() {
	printf("PrometheusSystem_Mapfunc: maybe_on_map_unload\n");
}
void PrometheusSystem_Mapfunc::OnWorldLoadedAndInGameEnttiyAdmin() {
	printf("PrometheusSystem_Mapfunc: OnWorldLoadedAndInGameEnttiyAdmin\n");
}
void PrometheusSystem_Mapfunc::OnWorldLoadingAndInGameEntityAdmin() {
	printf("PrometheusSystem_Mapfunc: OnWorldLoadingAndInGameEntityAdmin\n");
}
void PrometheusSystem_Mapfunc::field_5() {
	printf("PrometheusSystem_Mapfunc: field_5\n");
}
