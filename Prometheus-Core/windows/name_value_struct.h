#pragma once
#include "../window_manager/window_manager.h"
#include "../JAM.h"
#include "../game.h"
#include <format>
#include "../stringhash_library.h"

class name_value_struct : public window {
	WINDOW_DEFINE(name_value_struct, "Resource", "Named Variable struct", true);

	struct TreeItem {
		__int64 vfptr;
		char* name;
		TreeItem* parent;
	};

	struct NamedItem_Info {
		__int64 vfptr;
		int id_1;
		int id_2;
		__int64 mutex_on_ps4;
	};

	struct NamedItem
	{
		int item_hash_xored_with_smth;
		int field_4;
		string_rep item_name;
		__int64 other_tree_location;
		__int64 tree_location;
		__int64 field_38;
		NamedItem_Info* info_ptr;
		int id_1_minus_8;
		__int64 some_vtable;
		char flag_1;
		char flag_2;
		char flag_3;
		char flag_4;
		__unaligned __declspec(align(1)) __int64 field_5C;
		int field_64;
		__int64 field_68;
		__int64 field_70;
		__int64 field_78;
		__int64 some_vtable_2;
		char field_88;
		__unaligned __declspec(align(4)) __int64 field_8C;
		int field_94;
		__int64 field_98;
		__int64 field_A0;
		__int64 field_A8;
	};


	std::map<TreeItem*, std::vector<NamedItem*>> treeItems;

	void refreshItems(int typ, bool addnames) {
		treeItems.clear();
		NamedItem** arr = *(NamedItem***)(globals::gameBase + 0x18a70b8);
		int size = *(int*)(globals::gameBase + 0x18a70b8 + 8);
		for (int i = 0; i < size; i++) {
			if (addnames) {
				stringhash_library::add_hash(arr[i]->item_name.get());
			}
			TreeItem* tree = (TreeItem*)(arr[i]->tree_location);
			if (typ == 1) {
				tree = (TreeItem*)(arr[i]->other_tree_location);
			}
			if (!tree) {
				char buf[256];
				sprintf_s(buf, "Invalid item at index %d", i);
				display_text(buf);
				continue;
			}
			treeItems[tree].push_back(arr[i]);
			do {
				treeItems.emplace(tree, std::vector<NamedItem*>{});
				tree = tree->parent;
			} while (tree != nullptr);
		}
	}
	std::string bufStr;//ghetto is love, ghetto is life

	void printNamedItem(NamedItem* item) {
		ImGui::PushID(item);

		char label[256];
		sprintf_s(label, "%s: %p", item->item_name.actual_size == 0 ? "(NULL)" : item->item_name.get(), item);
		ImGui::SeparatorText(label);

		display_addr((__int64)item, "addr");
		display_addr(item->item_hash_xored_with_smth, "Hash");
		
		if (ImGui::CollapsingHeader("Details")) {
			display_addr((__int64)item->tree_location, "Tree Location");
			display_addr(item->some_vtable, "Some VT1");
			display_addr(item->some_vtable_2, "Some VT2");

			ImGui::Separator();

			display_addr((__int64)item->other_tree_location, "__int64 field_28");
			display_addr(item->field_38, "__int64 field_38");
			/*display_addr(item->field_40, "__int64 field_40");
			display_addr(item->maybeSize, "int maybeSize");*/
			/*display_addr(item->field_58, "char field_58");
			display_addr(item->field_60, "int field_60");*/
			display_addr(item->field_64, "int field_64");
			display_addr(item->field_68, "__int64 field_68");
			display_addr(item->field_70, "__int64 field_70");
			display_addr(item->field_78, "__int64 field_78");
			display_addr(item->field_88, "char field_88");
			display_addr(item->field_8C, "__int64 field_8C");
			display_addr(item->field_94, "int field_94");
			display_addr(item->field_98, "__int64 field_98");
			display_addr(item->field_A0, "__int64 field_A0");
			display_addr(item->field_A8, "__int64 field_A8");
		}
		ImGui::PopID();
	}

	void displayTree(std::pair<TreeItem*, std::vector<NamedItem*>> item, bool isRoot = false) {
		char buf[256];
		sprintf_s(buf, isRoot ? "Root %p: %s" : "Item %p: %s", item.first, item.first->name);
		if (ImGui::TreeNodeEx(buf, ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("NamedItems: %d", item.second.size());
			if (imgui_helpers::beginTable("NameValueTable", { "Ptr", "Name", "Hash w/ XOR", "Hash Calc", "XOR" , "Hash w/o", "VT1", "VT2", "Flags", "nv->id1-8", "info(ptr)", "info(vfptr)", "id1", "id2" })) {

				for (int i = 0; i < item.second.size(); i++) {
					NamedItem* namedItem = item.second[i];
					std::string searchFor = namedItem->item_name.get();
					searchFor += std::format("{:x}", (uint)namedItem->item_hash_xored_with_smth) + " ";
					searchFor += std::format("{:x}", (uint)(namedItem->item_hash_xored_with_smth ^ namedItem->item_name.hash)) + " ";
					searchFor += std::format("{:x}", (uint)namedItem->item_name.hash);
					if (!bufStr.empty() && !icontains(searchFor, bufStr))
						continue;
					//printNamedItem(item.second[i]);
					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					display_addr((__int64)namedItem);

					ImGui::TableNextColumn();
					ImGui::Text("%s", namedItem->item_name.actual_size == 0 ? "(NULL)" : namedItem->item_name.get());

					ImGui::TableNextColumn();
					display_addr(namedItem->item_hash_xored_with_smth);

					ImGui::TableNextColumn();
					display_addr(stringHash(namedItem->item_name.get()));

					ImGui::TableNextColumn();
					stringhash_library::display_hash(namedItem->item_hash_xored_with_smth ^ stringHash(namedItem->item_name.get()));

					ImGui::TableNextColumn();
					display_addr(namedItem->item_name.hash);

					ImGui::TableNextColumn();
					display_addr(namedItem->some_vtable);

					ImGui::TableNextColumn();
					display_addr(namedItem->some_vtable_2);

					ImGui::TableNextColumn();
					ImGui::Text("%d %d %d %d", (char)namedItem->flag_1, (char)namedItem->flag_2, (char)namedItem->flag_3, (char)namedItem->flag_4);

					ImGui::TableNextColumn();
					ImGui::Text("%x", namedItem->id_1_minus_8);

					ImGui::TableNextColumn();
					display_addr((__int64)namedItem->info_ptr);

					if (!namedItem->info_ptr)
						continue;
					ImGui::TableNextColumn();
					display_addr(namedItem->info_ptr->vfptr);

					ImGui::TableNextColumn();
					display_addr(namedItem->info_ptr->id_1);

					ImGui::TableNextColumn();
					display_addr(namedItem->info_ptr->id_2);
				}
				ImGui::EndTable();
			}
			for (auto& child : treeItems) {
				if (child.first->parent == item.first) {
					displayTree(child);
				}
			}
			ImGui::TreePop();
		}
	}

	inline void render() override {
		if (open_window()) {
			static int treeLoc = 0;
			if (ImGui::Button("Refresh tree items")) {
				refreshItems(treeLoc, false);
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Main Tree (0x30)", treeLoc == 0)) {
				treeLoc = 0;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Other Tree (0x20)", treeLoc == 1)) {
				treeLoc = 1;
			}
			ImGui::SameLine();
			if (ImGui::Button("Refresh & add hashes")) {
				refreshItems(treeLoc, true);
			}
			static char buf[256];
			ImGui::InputText("Filter: ", buf, 256);
			bufStr = buf;

			int named_item_size = *(int*)(globals::gameBase + 0x18a70b8 + 8);
			ImGui::Text("Named Items: %d", named_item_size);
			ImGui::Text("Tree Items", treeItems.size());

			for (auto& root : treeItems) {
				if (root.first->parent != nullptr)
					continue;
				displayTree(root, true);
			}

			//beginTable("NV struct", {"Name", "Hash"});
			///*for (int i = 0; i < size; i++) {
			//	ImGui::TableNextRow();
			//	ImGui::TableNextColumn();

			//	__int64 item = *(__int64*)(arr + i * 8);

			//	ImGui::Text("%s", readStringRep(item + 8));

			//	ImGui::TableNextColumn();

			//	ImGui::Text("%x", *(int*)(item));
			//}*/
			//ImGui::EndTable();
		}
		ImGui::End();
	}

	//inline void preStartInitialize() override {}
	//inline void initialize() override {}
};

WINDOW_REGISTER(name_value_struct);
