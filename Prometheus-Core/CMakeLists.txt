cmake_minimum_required(VERSION 3.31)
project(Prometheus-Core
        DESCRIPTION "Core DLL Project"
        HOMEPAGE_URL "https://github.com/BreakingBread0/prometheus"
        LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DIR_BIN ${CMAKE_BINARY_DIR}/bin/core)
set(DIR_LIB ${CMAKE_BINARY_DIR}/lib/core)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/*.cpp"
        "${PROJECT_SOURCE_DIR}/*.h")

add_library(Prometheus-Core SHARED
        "entity_admin.h"
        "ExceptionFormatter.h"
        "filetype_library.h"
        "framework.h"
        "game.h"
        "globals.h"
        "hash_exporter.h"
        "idadefs.h"
        "ImguiRenderer.h"
        "imgui_json.h"
        "JAM.h"
        "lazy_importer.h"
        "MovementState.h"
        "pch.h"
        "pe.h"
        "player_spawner.h"
        "search_helper.h"
        "serialization.h"
        "Statescript.h"
        "StatescriptExpressionParser.h"
        "StatescriptVar.h"
        "statescript_logger.h"
        "state_replicator.h"
        "stringhash_library.h"
        "STU.h"
        "STU_Editable.h"
        "stu_resources.h"
        "Viewmodel.h"
        "windows/AAA window template.h"
        "windows/asset_pack_manager.h"
        "windows/awful_demo_server.h"
        "windows/bitconverter_calls.h"
        "windows/blizzbbqueueq_window.h"
        "windows/comments_window.h"
        "windows/component_notes.h"
        "windows/copy_window.h"
        "windows/CVD_Edit.h"
        "windows/CVD_List.h"
        "windows/CVD_VarbagSelector.h"
        "windows/cvexpression_viewer.h"
        "windows/entadmin_compodependencies.h"
        "windows/entityadmin_systems_window.h"
        "windows/entityadmin_timing.h"
        "windows/entityadmin_window.h"
        "windows/entitydef_explorer.h"
        "windows/entitydef_list.h"
        "windows/entity_bounds_renderer.h"
        "windows/entity_spawner.h"
        "windows/entity_window.h"
        "windows/filetype_window.h"
        "windows/freecam_movement_recorder.h"
        "windows/game_data_window.h"
        "windows/game_msg_window.h"
        "windows/hash_display_window.h"
        "windows/herolineup_creator.h"
        "windows/imgui_demo_window.h"
        "windows/ini_settings_window.h"
        "windows/jamprotocol_window.h"
        "windows/jam_messagedetail_window.h"
        "windows/example_window.h"
        "windows/loadlist_visualizer.h"
        "windows/lobbymap_tester.h"
        "windows/local_player_component_insights.h"
        "windows/logicalbutton_names.h"
        "windows/log_window.h"
        "windows/manager_4_secrets.h"
        "windows/manager_infos.h"
        "windows/message_window.h"
        "windows/movementsys_editor.h"
        "windows/movstate_visualizer.h"
        "windows/name_value_struct.h"
        "windows/pachimari_creator.h"
        "windows/player_spawner_deluxe.h"
        "windows/pvpgame_explorer.h"
        "windows/radio_selector_window.h"
        "windows/statescript_fake_logicalbtn.h"
        "windows/statescript_fuzzer.h"
        "windows/statescript_graph_window.h"
        "windows/statescript_info.h"
        "windows/statescript_list.h"
        "windows/statescript_log_window.h"
        "windows/statescript_node_search.h"
        "windows/statescript_registry.h"
        "windows/statescript_varbag_window.h"
        "windows/statescript_window.h"
        "windows/string_rep_edit.h"
        "windows/stu_bslist_edit.h"
        "windows/stu_explorer.h"
        "windows/stu_object_edit.h"
        "windows/stu_primitive_edit.h"
        "windows/stu_registry.h"
        "windows/stu_types.h"
        "windows/STU_Types_window.h"
        "windows/textedit_window.h"
        "windows/type_7b_strings.h"
        "windows/viewmodel_selector.h"
        "windows/viewmodel_viewer.h"
        "windows/viewmodel_write_blocker.h"
        "windows/voice_system_triggerer.h"
        "window_manager/management_window.h"
        "window_manager/window_manager.h"
        "dllmain.cpp"
        "entity_admin.cpp"
        "ExceptionFormatter.cpp"
        "filetype_library.cpp"
        "globals.cpp"
        "ImguiRenderer.cpp"
        "pch.cpp"
        "player_spawner.cpp"
        "ResourceManager.h"
        "serialization.cpp"
        "Statescript.cpp"
        "state_replicator.cpp"
        "stringhash_library.cpp"
        "STU.cpp"
        "STU_Editable.cpp"
        "stu_resources.cpp"
        "windows/entity_window.cpp"
        "windows/statescript_list.cpp"
        "windows/statescript_window.cpp"
        "windows/stu_explorer.cpp"
        "windows/viewmodel_write_blocker.cpp"
        "window_manager/management_window.cpp"
        "window_manager/window_manager.cpp"
)
target_include_directories(Prometheus-Core PRIVATE ${PROJECT_SOURCE_DIR}/)

# Project Config
Pro_ProjectVersion(Prometheus-Core 1 0 0 0)
Pro_ApplyPlatform(Prometheus-Core)
Pro_SetOutput(Prometheus-Core ${DIR_BIN} ${DIR_LIB})

# Dll Config
set_target_properties(Prometheus-Core PROPERTIES OUTPUT_NAME "PrometheusCore")
target_link_options(Prometheus-Core PRIVATE /ILK:${DIR_LIB}/Prometheus-Core.ilk)

# Packages
find_package(minhook CONFIG REQUIRED)
#add_library(minhook)
target_link_libraries(Prometheus-Core PRIVATE minhook::minhook)

find_package(Freetype REQUIRED)
target_link_libraries(Prometheus-Core PRIVATE Freetype::Freetype)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(Prometheus-Core PRIVATE nlohmann_json::nlohmann_json)

find_path(FAST_CPP_CSV_PARSER_INCLUDE_DIRS "fast-cpp-csv-parser/csv.h")
target_include_directories(Prometheus-Core PRIVATE ${FAST_CPP_CSV_PARSER_INCLUDE_DIRS})

find_package(ixwebsocket CONFIG REQUIRED)
target_link_libraries(Prometheus-Core PRIVATE ixwebsocket::ixwebsocket)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(Prometheus-Core PRIVATE imgui::imgui)
#get_target_property(IMGUI_INCLUDE_DIRS imgui::imgui
#        INTERFACE_INCLUDE_DIRECTORIES
#)
#
#message(STATUS IMGUI DIRS ${IMGUI_INCLUDE_DIRS})
#target_include_directories(Prometheus-Core ${IMGUI_INCLUDE_DIRS})