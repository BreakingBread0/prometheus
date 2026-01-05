#pragma once
#include "../window_manager/window_manager.h"
#include <imnodes/imnodes.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include "../statescript_logger.h"
#include "../imgui_json.h"

/*
Graph IDs:
Nodes: index of m_nodes
Comments: index | 0xF000
Input Links: index | 0xE000
Output Links: index | 0xD000
Multiple Output LInks: index | 0xD000 | i << 20
Link: 0xC000 | i (incremental)
*/

class statescript_window : public window {
	WINDOW_DEFINE(statescript_window, "Statescript", "Statescript Graph Neo", true);

    struct GraphComment {
        std::string comment;
        ImVec2 location;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(GraphComment, comment, location);
    };

    void display_instance(StatescriptInstance* ss, STUStatescriptBase* stu = nullptr);

    void render() override;

	//inline void preStartInitialize() override { }
	//inline void initialize() override {}

    statescript_window();

    ~statescript_window();

    inline StatescriptInstance* get_curr_instance() {
        return _curr_instance;
    }

    inline bool get_instance_invalid() {
        return _instance_invalid;
    }
private:
	StatescriptInstance* _curr_instance;
    STUStatescriptBase* _nav_node;
    bool _instance_invalid;
    std::vector<char*> _comments;
    ImNodesEditorContext* _nodes_context;
    std::shared_ptr<std::function<void(StatescriptInstance*)>> _deletion_callback;
    int _selected_node = -1;

    struct {
        bool addresses;
        bool stu_args;
    } _display_settings{};

    void emplace_row(int row_id, std::vector<std::vector<STUStatescriptBase*>>& rows, STUStatescriptBase* item);

    void sort_item(int row_id, std::vector<std::vector<STUStatescriptBase*>>& rows, std::set<STUStatescriptBase*>& visited, STUStatescriptBase* item);

    void sort_graph();

    void save_inst();

    void print_node_location(STUStatescriptBase* node);

    bool is_graph_valid(StatescriptInstance* ss);

    void dealloc_old();

    void nav_to_node();
};

WINDOW_REGISTER(statescript_window);
