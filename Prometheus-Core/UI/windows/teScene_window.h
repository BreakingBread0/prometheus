#pragma once
#include "../window_manager/window_manager.h"
#include "entity_admin.h"
#include "Components/Component_2_AssetManager.h"

class teScene_window : public window {
	WINDOW_DEFINE_ARG(teScene_window, "Game", "teScene explorer", EntityAdminBase*);

	void print_resload_entry(const char* name, MisalignedResourceLoadEntry* entry)
	{
		ImGui::PushID(name);

		if (entry->valid())
		{
			ImGui::Text("%s: %p", name, entry->align()->resource_id);
			display_addr((__int64)entry->align());
		} else
		{
			ImGui::Text("%s: Invalid", name);
		}

		ImGui::PopID();
	}

	inline void render() override {
		if (open_window()) {
			display_addr((__int64)_arg, "EA");
			ImGui::SameLine();
			ImGui::Text("%s", _arg->vfptr->GetIsLiveReplayOrDeathStar(_arg));

			auto get_rotation_matrix = (void(*)(Component_1_SceneRendering*, Matrix4x4*))(globals::gameBase + 0x896d20);
			auto controller = _arg->getLocalEnt();
			if (controller)
			{
				auto model_ref = controller->getById<Component_20_ModelReference>(0x20);
				if (model_ref && model_ref->cam_attach_entid)
				{
					auto model_ent = _arg->getEntById(model_ref->cam_attach_entid);
					if (model_ent)
					{
						auto comp_1 = model_ent->getById<Component_1_SceneRendering>(1);
						Matrix4x4 rot_matrix{};
						get_rotation_matrix(comp_1, &rot_matrix);
						imgui_helpers::modifiable("rotation_matrix", &rot_matrix);
						ImGui::Text("needs_compute: %d", comp_1->needs_rotmatrix_compute);
					}
				}
			}


			auto comp_2 = (*_arg->system_1_sceneSystem)->Get_teScene_Comp2(_arg->system_1_sceneSystem);
			if (!comp_2)
			{
				ImGui::Text("Invalid comp2");
				return;
			}
			display_addr((__int64)comp_2, "Component2");
			display_addr((__int64)comp_2->scene, "teScene");
			if (comp_2->scene)
			{
				ImGui::Text("Map load state: %x", comp_2->scene->load_map_state);
				display_addr((__int64)comp_2->scene->resource_loader, "Resource Loader");
				if (comp_2->scene->resource_loader)
				{
					bool show_ents = ImGui::Button("Show Entities");
					auto loader = comp_2->scene->resource_loader;
					print_resload_entry("loader", loader->resload_entry);
					STRUCT_MODIFIABLE(loader, field_10);
					STRUCT_MODIFIABLE(loader, placeables_num);
					STRUCT_MODIFIABLE(loader, field_18);
					STRUCT_MODIFIABLE(loader, field_20);
					STRUCT_MODIFIABLE(loader, field_24);
					STRUCT_MODIFIABLE(loader, field_28);
					STRUCT_MODIFIABLE(loader, field_2A);
					STRUCT_MODIFIABLE(loader, field_2C);

					if (imgui_helpers::beginTable("placeables", {"Addr", "Type", "CASCData", "LoadState", "FilterFlags", "field_33", "field_34"}))
					{
						for (int i = 0; i < loader->placeables_num; i++)
						{
							auto item = &loader->placeables[i];
							ImGui::PushID(i);
							ImGui::TableNextRow();

							ImGui::TableNextColumn();
							print_resload_entry(std::to_string(i).c_str(), item->resload_entry);
							display_addr((__int64)item);

							ImGui::TableNextColumn();
							ImGui::TextUnformatted(teMAP_PLACEABLE_toString(item->map_placeable_type).c_str());

							ImGui::TableNextColumn();
							display_addr((__int64)item->placeable_resource);
							if (IsBadReadPtr((void*)item->placeable_resource, 0x10))
							{
							 	ImGui::TextUnformatted("Invalid");
							} else
							{
								auto cached_ent = _map_cache.find(item->hash);
							 	Entity* ent = nullptr;
								if (cached_ent != _map_cache.end())
								{
									ent = cached_ent->second;
								} else
								{
									_map_cache[item->hash] = ent = _arg->getByUUID(item->placeable_resource->uuid);
								}

								if (ent)
								{
									// ImGui::Text("%p", *(uint64_t*)&item->placeable_resource->uuid);
									ImGui::PushFont(imgui_helpers::BoldFont);
									ImGui::TextUnformatted(ent->toString().c_str());
									ImGui::PopFont();
									ImGui::TextUnformatted((item->placeable_resource->field_10 & 2) == 0 ? "Matches" : "Nope");

									if (show_ents)
									{
										auto model = ent->getById<Component_4_Model>(4);
										if (model)
										{
											model->EnableOutline(Component_4_Model::ModelRenderingFlags::OUTLINE_INVISIBLE, (item->filter_flags & FilterFlag_NeedsFilter) != 0 ? 0xFFFF00FF : 0x00FF00FF);
										}
									}
								}
							}

							ImGui::TableNextColumn();
							ImGui::Text("%x", item->load_state);

							ImGui::TableNextColumn();
							ImGui::Text("%x", item->filter_flags);

							ImGui::TableNextColumn();
							ImGui::Text("%x", item->field_33);

							ImGui::TableNextColumn();
							ImGui::Text("%x", item->field_34);

							ImGui::PopID();
						}

						ImGui::EndTable();
					}
				}
			}
		}
		ImGui::End();
	}

private:
	bool _show_ents = false;

	std::map<__int64, Entity*> _map_cache;

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(teScene_window);