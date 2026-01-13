#pragma once
#include "../window_manager/window_manager.h"
#include "../entity_admin.h"
#include "../stu_resources.h"
#include "stu_explorer.h"

class voice_system_triggerer : public window {
	WINDOW_DEFINE(voice_system_triggerer, "Game", "Voice System Triggerer", true);

	void initialize() override
	{
		voice.voice_stimulus = 0xee0000000000001; //default to functional one.
	}

	inline void render() override {
		if (open_window()) {
			ImGui::Text("Play on local ent");
			auto ea = GameEntityAdmin();
			ImGui::Text("Local Ent: %x", ea->local_entid);
			if (ea->local_entid) {
				auto petref = ea->getEntById(ea->local_entid)->getById<Component_20_ModelReference>(0x20);
				ImGui::Text("Cam Attach: %x", petref->cam_attach_entid);
				if (petref->cam_attach_entid) {
					auto model_ent = ea->getEntById(petref->cam_attach_entid);
					if (model_ent) {
						auto model_entdef = stu_resources::GetByID(model_ent->resload_entry->align()->resource_id);
						auto voice_comp = model_entdef->to_editable().get_argument_map("m_componentMap").value->get_by_key(stringHash("STUVoiceSetComponent"));
						if (voice_comp && voice_comp->value) {
							display_addr((__int64)voice_comp->value, "STUVoiceSetComponent");
							auto voice_set = ((STUBase<>*)voice_comp->value)->to_editable().get_argument_resource("m_voiceDefinition")->get_STU()->to_editable();
							if (voice_set.valid()) {
								display_addr((__int64)voice_set.value, "STUVoiceSet");
								auto voice_lines = voice_set.get_argument_objectlist("m_voiceLineInstances");
								if (ImGui::BeginListBox("##voicelines", ImVec2(600, 250))) {
									for (auto voice_line_inst : voice_lines) {
										auto voice_line = voice_line_inst.get_argument_resource("m_voiceLine");
										ImGui::PushID(voice_line->resource_id);

										if (ImGui::RadioButton("", voice_line->resource_id == voice.voice_line)) {
											voice.voice_line = voice_line->resource_id;
										}
										ImGui::SameLine();
										if (ImGui::Button("E")) {
											stu_explorer::get_latest_or_create(this)->navigate_to(voice_line_inst.struct_info, (__int64)voice_line_inst.value, nullptr);
										}
										ImGui::SameLine();
										imgui_helpers::display_type(voice_line->resource_id, true, true, false);

										ImGui::PopID();
									}
									ImGui::EndListBox();
								}
							}
						}

						voice.entity_1 = voice.entity_2 = voice.instigator_ent = model_ent->entity_id;
					}
				}
			}

			imgui_helpers::InputHex("Voice Stimulus", &voice.voice_stimulus);
			imgui_helpers::InputHex("Voice Line", &voice.voice_line);

			ImGui::Checkbox("field_20", &voice.field_20);
			ImGui::Checkbox("field_21", &voice.field_21);

			if (ImGui::Button("Play")) {
				auto voicesys = GetVoiceSystem(ea);
				if (voicesys) {
					voicesys->vfptr->PlayVoice(voicesys, &voice);
				}
				else {
					imgui_helpers::messageBox("Voice System null!", this);
				}
			}
		}
		ImGui::End();
	}

	VoiceSystem_PlayVoice voice{};

	//inline void preStartInitialize() override {}
};

WINDOW_REGISTER(voice_system_triggerer);