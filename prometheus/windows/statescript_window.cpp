#include "statescript_window.h"
#include "../stringhash_library.h"
#include "stu_explorer.h"
#include "statescript_info.h"
#include <imnodes/imnodes_internal.h>
#include "cvexpression_viewer.h"
#include "CVD_List.h"

void statescript_window::render() {
    std::string title;
    if (!_instance_invalid)
        title = std::format("{:x} on Entity {:x}", _curr_instance->script_id, _curr_instance->ss_inner->component_ref->base.entity_backref->entity_id);
    else
        title = "Invalid Statescript Graph";
    if (open_window(title.c_str(), ImGuiWindowFlags_MenuBar, ImVec2(1500, 800))) {
        if (_instance_invalid) {
            ImGui::Text("Invalid instance!");
            ImGui::End();
            return;
        }

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Graph View")) {
                ImGui::MenuItem("Addresses", nullptr, &_display_settings.addresses);
                ImGui::MenuItem("STU Args", nullptr, &_display_settings.stu_args);

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Tools")) {
                if (ImGui::MenuItem("Sort Graph")) {
                    sort_graph();
                }
                if (ImGui::MenuItem("Re-Nav to node")) {
                    nav_to_node();
                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Modals")) {
                if (ImGui::MenuItem("Information")) {
                    this->dock_item_right(statescript_info::get_latest_or_create(this, true, true), 0.8f);
                }

                if (ImGui::MenuItem("VarBag 1")) {
                    this->dock_item_down(CVD_List::get_latest_or_create(this, true, true)->set(_curr_instance->rid_instance_varbag), 0.7f);
                }

                if (ImGui::MenuItem("VarBag 2")) {
                    this->dock_item_down(CVD_List::get_latest_or_create(this, true, true)->set(_curr_instance->second_inst_varbag), 0.7f);
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImNodes::EditorContextSet(_nodes_context);

        auto& style = ImNodes::GetStyle();
        style.PinCircleRadius = 6;
        style.PinTriangleSideLength = 8;
        style.NodeBorderThickness = 3;

        ImNodes::BeginNodeEditor();

        int link_counter = 0;
        auto script_state = statescript_logger::GetScriptState(_curr_instance);
        for (auto node : _curr_instance->graph->m_nodes) {
            ImGui::PushID(node);
            auto type = node->get_type();
            auto stu = node->graph_node.base.vfptr->GetSTUInfo();

            switch (type) {
            case StatescriptNodeType_Action:
                ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(0xef, 0x47, 0x6f, 200));
                ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(0xef, 0x47, 0x6f, 100));
                break;
            case StatescriptNodeType_Entry:
                ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(6, 0xd6, 0xa0, 200));
                ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(6, 0xd6, 0xa0, 100));
                break;
            case StatescriptNodeType_State: {
                ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(0xff, 0xd1, 0x66, 200));
                ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(0xff, 0xd1, 0x66, 100));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 255));
                break;
            }
            case StatescriptNodeType_Condition:
                ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(0x11, 0x8a, 0xb2, 200));
                ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(0x11, 0x8a, 0xb2, 100));
                break;
            case StatescriptNodeType_Other:
                ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(7, 0x3b, 0x4c, 200));
                ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(7, 0x3b, 0x4c, 100));
                break;
            case StatescriptNodeType_Unknown:
                ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(0, 0, 0, 0));
                ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(0, 0, 0, 0));
                break;
            }

            auto& node_state = script_state[node->idx_in_nodes];
            if (node->m_isState) {
                if (node_state.is_finish_state)
                    ImNodes::PushColorStyle(ImNodesCol_NodeOutline, imgui_helpers::color_fade(ImVec4(255, 0, 0, 255), ImVec4(50, 50, 50, 255), 600, _curr_instance->ss_inner->cf_timestamp - node_state.change_timestamp));
                else
                    ImNodes::PushColorStyle(ImNodesCol_NodeOutline, IM_COL32(0, 255, 0, 255));
            }
            else
                ImNodes::PushColorStyle(ImNodesCol_NodeOutline, imgui_helpers::color_fade(ImVec4(255, 0, 255, 255), ImVec4(50, 50, 50, 255), 600, _curr_instance->ss_inner->cf_timestamp - node_state.change_timestamp));

            ImNodes::BeginNode(node->idx_in_nodes);
            ImNodes::BeginNodeTitleBar();
            if (type == StatescriptNodeType_Entry || type == StatescriptNodeType_Other || type == StatescriptNodeType_Action) {
                if (imgui_helpers::TooltipButton(EMOJI_PLAY "##action", "Execute as Action")) {
                    _curr_instance->ExecuteNode(node->idx_in_nodes);
                }
                ImGui::SameLine();
            }
            if (node->m_isState) {
                bool is_running = !script_state[node->idx_in_nodes].is_finish_state;
                if (imgui_helpers::TooltipButton(is_running ? EMOJI_STOP : EMOJI_PLAY, "Run / Stop as State")) {
                    _curr_instance->StartStopState(node->idx_in_nodes, !is_running);
                }
                ImGui::SameLine();
            }
            if (node->m_clientOnly) {
                ImGui::PushFont(imgui_helpers::BoldFont);
                ImGui::Text("[CO]");
                ImGui::SameLine();
                ImGui::PopFont();
            }
            if (node->m_serverOnly) {
                ImGui::PushFont(imgui_helpers::BoldFont);
                ImGui::Text("[SERVER]");
                ImGui::SameLine();
                ImGui::PopFont();
            }
            print_node_location(node);
            ImGui::SameLine();
            imgui_helpers::display_type(stu->name_hash, false, true, true);
            if (node->m_isState && node->graph_node.base.vfptr->rtti.base.rtti_assignable_to((__int64)node, globals::gameBase + STU_RTTI::STUStatescriptStateUXBase)) {
                ImGui::PushFont(imgui_helpers::BoldFont);
                ImGui::SameLine();
                ImGui::Text("(UX)");
                ImGui::PopFont();
            }
            ImNodes::EndNodeTitleBar();
            ImNodes::PopColorStyle();
            ImNodes::PopColorStyle();
            ImNodes::PopColorStyle();
            //ImNodes::PopColorStyle();
            if (node->m_isState) {
                ImGui::PopStyleColor();
            }

            if (type != StatescriptNodeType_Entry) {
                ImNodes::BeginInputAttribute(0xE000 | node->idx_in_nodes, ImNodesPinShape_CircleFilled);
                ImGui::TextUnformatted("In");
                ImNodes::EndInputAttribute();
            }

            auto connections = node->get_output_plugs();
            int outplug_i = 0;
            for (auto out_node : connections) {
                bool empty = true;
                for (auto& item : out_node.second) {
                    if (item->m_links.count() != 0) {
                        empty = false;
                        break;
                    }
                }
                if (empty)
                    continue;
                int outplug_id = 0xD000 | node->idx_in_nodes | outplug_i++ << 20;
                ImNodes::BeginOutputAttribute(outplug_id, ImNodesPinShape_CircleFilled);
                std::string plug_name;
                if (allmighty_hash_lib::hashes.find(out_node.first) != allmighty_hash_lib::hashes.end()) {
                    plug_name = allmighty_hash_lib::hashes[out_node.first];
                }
                else {
                    plug_name = std::format("{:x}", out_node.first);
                }
                if (allmighty_hash_lib::comments.find(out_node.first) != allmighty_hash_lib::comments.end()) {
                    plug_name += " (" + allmighty_hash_lib::comments[out_node.first] + ")";
                }
                int indent_size = ImNodes::GetNodeDimensions(node->idx_in_nodes).x - ImGui::CalcTextSize(plug_name.c_str()).x - 20;
                if (indent_size < 10)
                    indent_size = 10;

                ImGui::Indent(indent_size);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(100, 255, 0, 255));
                ImGui::TextUnformatted(plug_name.c_str());
                ImGui::PopStyleColor();
                ImGui::Unindent();

                //display_type(out_node.first, true, false, false);
                auto plug_type = out_node.second.front()->base.to_editable().struct_info;
                bool plug_has_color = false;
                ImNodes::PushColorStyle(ImNodesCol_LinkSelected, IM_COL32(180, 180, 180, 255));
                if (plug_type->assignable_to_hashes({ STU_NAME::Subgraph_Plug_, STU_NAME::Subgraph_Plug_2_ })) {
                    plug_has_color = true;
                    int col = IM_COL32(110, 70, 70, 255);
                    if (plug_type->assignable_to_hashes({ STU_NAME::ClientOnly_Subgraph_Plug_, STU_NAME::ClientOnly_Subgraph_Plug_2_ })) {
                        col = IM_COL32(0, 120, 0, 255);
                    }
                    else if (plug_type->assignable_to_hashes({STU_NAME::ServerOnly_Subgraph_Plug_})) {
                        col = IM_COL32(255, 20, 220, 255);
                    }
                    ImNodes::PushColorStyle(ImNodesCol_Link, col);
                    ImNodes::PushColorStyle(ImNodesCol_LinkHovered, (col & ~IM_COL32_A_MASK) | (150 << IM_COL32_A_SHIFT));
                }

                if (out_node.second.size() == 1) {
                    for (auto link : out_node.second[0]->m_links) {
                        ImNodes::Link(0xC000 | link_counter++, outplug_id, 0xE000 | link->m_inputPlug->plug_base.m_parentNode->idx_in_nodes);
                    }
                }
                else {
                    int plug_index = 0;
                    for (auto out_path : out_node.second) {
                        ImNodes::EndOutputAttribute();
                        outplug_id = 0xD000 | node->idx_in_nodes | outplug_i++ << 20;
                        ImNodes::BeginOutputAttribute(outplug_id, ImNodesPinShape_Circle);

                        ImGui::Indent(indent_size);
                        ImGui::Text("Index %x", plug_index++);
                        ImGui::Unindent();

                        for (auto link : out_path->m_links) {
                            ImNodes::Link(0xC000 | link_counter++, outplug_id, 0xE000 | link->m_inputPlug->plug_base.m_parentNode->idx_in_nodes);
                        }
                    }
                }
                ImNodes::EndOutputAttribute();
                ImNodes::PopColorStyle();
                if (plug_has_color) {
                    ImNodes::PopColorStyle();
                    ImNodes::PopColorStyle();
                }
            }


            if (_display_settings.addresses) {
                display_addr((__int64)node, "STU Instance");
                ImGui::SameLine();
                if (ImGui::Button("Explore"))
                    stu_explorer::get_latest_or_create(this)->navigate_to(node->graph_node.base.vfptr->GetSTUInfo(), (__int64)node, _curr_instance);

                if (type == StatescriptNodeType_Action) {
                    auto impl = GetStatescriptActionImplNode(_curr_instance, (STUStatescriptAction*)node);
                    display_addr((__int64)impl, "Action Implementation");
                }
                else if (type == StatescriptNodeType_State) {
                    auto impl = _curr_instance->stateImplFromSTU((STUStatescriptState*)node);
                    if (impl) {
                        display_addr((__int64)impl, "State Implementation");
                    }
                    else {
                        ImGui::Text("State Impl not allocated.");
                    }
                }
            }
            if (_display_settings.stu_args) {
                for (auto arg_info : *node->graph_node.base.vfptr->GetSTUInfo()) {
                    if (arg_info.first->name_hash == STU_NAME::STUStatescriptState ||
                        arg_info.first->name_hash == STU_NAME::STUStatescriptAction)
                        break;
                    auto arg = arg_info.second;
                    if (arg->constraint->get_type_flag() == STU_ConstraintType_Object) {
                        auto arg_typ = GetSTUInfoByHash(arg->constraint->get_stu_type());
                        if (arg_typ && arg_typ->assignable_to_hash(STU_NAME::STUConfigVar)) {
                            STUConfigVar* cv = *(STUConfigVar**)((__int64)node + arg->offset);
                            if (cv) {
                                imgui_helpers::display_cv(cv, _curr_instance, arg, false);
                                if (cv->is_expression()) {
                                    ImGui::SameLine();
                                    if (ImGui::Button("Expression")) {
                                        cvexpression_viewer::get_latest_or_create(this)->display_expression((STUConfigVarExpression*)cv, _curr_instance);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            ImNodes::EndNode();
            if (ImNodes::NumSelectedNodes() == 1 && ImNodes::IsNodeSelected(node->idx_in_nodes) && _selected_node != node->idx_in_nodes) {
                _selected_node = node->idx_in_nodes;
                /*auto explorer = stu_explorer::get_latest(this);
                if (explorer) {
                    explorer->navigate_to(node->graph_node.base.vfptr->GetSTUInfo(), (__int64)node);
                }*/
            }
            ImGui::PopID();
        }

        ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_BottomRight);
        ImNodes::EndNodeEditor();
    }
    ImGui::End();
}

void statescript_window::display_instance(StatescriptInstance* ss, STUStatescriptBase* stu) {
    dealloc_old();
    _comments.clear();
    if (ss && ss->graph && is_graph_valid(ss)) {
        char buf[64];
        sprintf_s(buf, "./statescript/%p.json", ss->script_id);
        std::ifstream input(buf, std::ios::in);
        if (input.is_open()) {
            try {
                nlohmann::json json;
                json << input;
                std::vector<ImVec2> node_locations = json["locations"].get<std::vector<ImVec2>>();
                ImNodes::EditorContextSet(_nodes_context);
                for (int i = 0; i < node_locations.size(); i++) {
                    ImNodes::SetNodeScreenSpacePos(i, node_locations[i]);
                }
                std::vector<GraphComment> comments = json["comments"].get<std::vector<GraphComment>>();
                for (int i = 0; i < comments.size(); i++) {
                    ImNodes::SetNodeScreenSpacePos(0xF000 | i, comments[i].location);

                    char* buf = new char[256];
                    strcpy_s(buf, 256, comments[i].comment.c_str());
                    _comments.push_back(buf);
                }
            }
            catch (nlohmann::json::exception& ex) {
                printf("Failed to load statescript graph locations! %p\n", ss->script_id);
            }
        }
        else {
            printf("No config file found for statescript script %p.\n", ss->script_id);
            for (int i = 0; i < ss->graph->m_nodes.count(); i++) {
                ImNodes::SetNodeScreenSpacePos(i, ImVec2());
            }
        }

        _curr_instance = ss;
        _instance_invalid = false;

        if (stu) {
            _nav_node = stu;
            nav_to_node();
        }

        statescript_logger::Subscribe(_curr_instance);
    }
}

void statescript_window::nav_to_node() {
    if (_nav_node && !_instance_invalid && _curr_instance->graph->m_nodes.contains(_nav_node)) {
        ImNodes::EditorContextSet(_nodes_context);
        ImVec2 target = ImNodes::GetNodeGridSpacePos(_nav_node->idx_in_nodes);
        ImVec2 screen = GImNodes->CanvasRectScreenSpace.GetSize() / 2;
        _nodes_context->Panning = -target + screen;
    }
}

statescript_window::statescript_window() {
    _curr_instance = nullptr;
    _instance_invalid = true;
    _nodes_context = ImNodes::EditorContextCreate();
    _deletion_callback = std::make_shared<std::function<void(StatescriptInstance*)>>([&](StatescriptInstance* deleted_script) {
        if (_curr_instance == deleted_script) {
            _curr_instance = nullptr;
            _instance_invalid = true;
        }
    });
    statescript_logger::SubscribeDeallocation(_deletion_callback);
}

statescript_window::~statescript_window() {
    dealloc_old();
    ImNodes::EditorContextFree(_nodes_context);
}

void statescript_window::emplace_row(int row_id, std::vector<std::vector<STUStatescriptBase*>>& rows, STUStatescriptBase* item) {
    if (row_id != 0) {
        for (int i = 0; i < row_id; i++) {
            auto& row = rows[i];
            std::erase(row, item);
        }
    }
    while (rows.size() <= row_id) {
        rows.push_back({});
    }
    for (int i = row_id + 1; i < rows.size(); i++) {
        auto& row = rows[i];
        if (std::find(row.begin(), row.end(), item) != row.end())
            return;
    }
    rows[row_id].push_back(item);
}

void statescript_window::sort_item(int row_id, std::vector<std::vector<STUStatescriptBase*>>& rows, std::set<STUStatescriptBase*>& visited, STUStatescriptBase* item) {
    if (visited.find(item) != visited.end())
        return;
    visited.emplace(item);
    emplace_row(row_id, rows, item);
    for (auto plug : item->get_output_plugs()) {
        for (auto path : plug.second) {
            for (auto link : path->m_links) {
                sort_item(row_id + 1, rows, visited, link->m_inputPlug->plug_base.m_parentNode);
            }
        }
    }
}

void statescript_window::sort_graph() {
    std::vector<std::vector<STUStatescriptBase*>> rows;
    std::set<STUStatescriptBase*> visited;
    for (auto entry_node : _curr_instance->graph->m_entries) {
        sort_item(0, rows, visited, entry_node);
    }
    float x_counter = 0;
    for (auto& row : rows) {
        float max_x = 0;
        for (auto item : row) {
            auto dim = ImNodes::GetNodeDimensions(item->idx_in_nodes);
            if (max_x < dim.x)
                max_x = dim.x;
        }
        float curr_y = 0;
        for (auto item : row) {
            auto dim = ImNodes::GetNodeDimensions(item->idx_in_nodes);
            ImNodes::SetNodeScreenSpacePos(item->idx_in_nodes, ImVec2(x_counter, curr_y));
            curr_y += 50 + dim.y;
        }
        if (x_counter == 0) {
            for (auto node : _curr_instance->graph->m_nodes) {
                if (visited.find(node) == visited.end()) {
                    auto dim = ImNodes::GetNodeDimensions(node->idx_in_nodes);
                    ImNodes::SetNodeScreenSpacePos(node->idx_in_nodes, ImVec2(x_counter, curr_y));
                    curr_y += 50 + dim.y;
                }
            }
        }
        x_counter += 50 + max_x;
    }
}

void statescript_window::save_inst() {
    if (_curr_instance) {
        std::vector<ImVec2> node_locations{};
        std::vector<GraphComment> comments{};
        for (int i = 0; i < _curr_instance->graph->m_nodes.count(); i++) {
            node_locations.push_back(ImNodes::GetNodeScreenSpacePos(i));
        }
        for (int i = 0; i < _comments.size(); i++) {
            comments.push_back(GraphComment{ std::string(_comments[i]), ImNodes::GetNodeScreenSpacePos(0xF000 | i) });
        }
        char buf[64];
        sprintf_s(buf, "statescript/%p.json", _curr_instance->script_id);
        std::ofstream output(buf, std::ios::out | std::ios::trunc);
        if (output.is_open()) {
            nlohmann::json json;
            json["locations"] = node_locations;
            json["comments"] = comments;
            output << json.dump();
        }
    }
}

void statescript_window::print_node_location(STUStatescriptBase* node) {
    if (node->m_isState) {
        int states_idx = _curr_instance->graph->m_states.indexof((STUStatescriptState*)node);
        ImGui::Text("<%d, %d>", node->idx_in_nodes, states_idx);
    }
    else {
        ImGui::Text("<%d>", node->idx_in_nodes);
    }
}

bool statescript_window::is_graph_valid(StatescriptInstance* ss) {
    __ow_try (EXCEPTION_EXECUTE_HANDLER) {
        if (IsBadReadPtr(ss->graph, 8))
            return false;
        assert(ss->graph->m_nodes.count() < 0x1000);
        assert(ss->graph->m_states.count() < ss->graph->m_nodes.count());
        if (ss->graph->m_nodes.count() >= 0x1000)
            return false;
        if (ss->graph->m_states.count() > ss->graph->m_nodes.count())
            return false;
    }
    __ow_except {
        return false;
    }
    return true;
}

void statescript_window::dealloc_old() {
    if (_curr_instance) {
        statescript_logger::Unsubscribe(_curr_instance);
        if (_curr_instance)
            save_inst();
        for (int i = 0; i < _comments.size(); i++) {
            delete[] _comments[i];
        }
        _curr_instance = nullptr;
    }
    _instance_invalid = true;
}
