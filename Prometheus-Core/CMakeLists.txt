cmake_minimum_required(VERSION 3.28)
project(Prometheus-Core
        DESCRIPTION "Core DLL Project"
        HOMEPAGE_URL "https://github.com/BreakingBread0/prometheus"
        LANGUAGES C CXX)

message("Prometheus-Core")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/*.cpp"
        "${PROJECT_SOURCE_DIR}/*.h")

add_library(Prometheus-Core SHARED
        $<TARGET_OBJECTS:Prometheus-Atlas-Extensions-Obj>
        "Common.h"
        "entity_admin.h"
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
        "UI/search_helper.h"
        "serialization.h"
        "Statescript.h"
        "StatescriptExpressionParser.h"
        "StatescriptVar.h"
        "statescript_logger.h"
        "state_replicator.h"
        "stringhash_library.h"
        "STUConfigVar_Custom.cpp"
        "STUConfigVar_Custom.h"
        "STUConfigVarDynamic_Custom.cpp"
        "STUConfigVarDynamic_Custom.h"
        "STU.h"
        "STU_Editable.h"
        "stu_resources.h"
        "Viewmodel.h"
        "UI/windows/AAA window template.h"
        "UI/windows/asset_pack_manager.h"
        "UI/windows/awful_demo_server.h"
        "UI/windows/bitconverter_calls.h"
        "UI/windows/blizzbbqueueq_window.h"
        "UI/windows/comments_window.h"
        "UI/windows/component_notes.h"
        "UI/windows/copy_window.h"
        "UI/windows/CVD_Edit.h"
        "UI/windows/CVD_List.h"
        "UI/windows/CVD_VarbagSelector.h"
        "UI/windows/cvexpression_viewer.h"
        "UI/windows/entadmin_compodependencies.h"
        "UI/windows/entityadmin_systems_window.h"
        "UI/windows/entityadmin_timing.h"
        "UI/windows/entityadmin_window.h"
        "UI/windows/entitydef_explorer.h"
        "UI/windows/entitydef_list.h"
        "UI/windows/entity_bounds_renderer.h"
        "UI/windows/entity_spawner.h"
        "UI/windows/entity_window.h"
        "UI/windows/filetype_window.h"
        "UI/windows/freecam_movement_recorder.h"
        "UI/windows/game_data_window.h"
        "UI/windows/game_msg_window.h"
        "UI/windows/hash_display_window.h"
        "UI/windows/herolineup_creator.h"
        "UI/windows/imgui_demo_window.h"
        "UI/windows/ini_settings_window.h"
        "UI/windows/jamprotocol_window.h"
        "UI/windows/jam_messagedetail_window.h"
        "UI/windows/example_window.h"
        "UI/windows/loadlist_visualizer.h"
        "UI/windows/lobbymap_tester.h"
        "UI/windows/local_player_component_insights.h"
        "UI/windows/logicalbutton_names.h"
        "UI/windows/log_window.h"
        "UI/windows/manager_4_secrets.h"
        "UI/windows/manager_infos.h"
        "UI/windows/message_window.h"
        "UI/windows/movementsys_editor.h"
        "UI/windows/movstate_visualizer.h"
        "UI/windows/name_value_struct.h"
        "UI/windows/pachimari_creator.h"
        "UI/windows/player_spawner_deluxe.h"
        "UI/windows/pvpgame_explorer.h"
        "UI/windows/radio_selector_window.h"
        "UI/windows/statescript_fake_logicalbtn.h"
        "UI/windows/statescript_graph_window.h"
        "UI/windows/statescript_info.h"
        "UI/windows/statescript_list.h"
        "UI/windows/statescript_log_window.h"
        "UI/windows/statescript_node_search.h"
        "UI/windows/statescript_registry.h"
        "UI/windows/statescript_varbag_window.h"
        "UI/windows/statescript_window.h"
        "UI/windows/string_rep_edit.h"
        "UI/windows/stu_bslist_edit.h"
        "UI/windows/stu_explorer.h"
        "UI/windows/stu_object_edit.h"
        "UI/windows/stu_primitive_edit.h"
        "UI/windows/stu_registry.h"
        "UI/windows/stu_types.h"
        "UI/windows/STU_Types_window.h"
        "UI/windows/textedit_window.h"
        "UI/windows/type_7b_strings.h"
        "UI/windows/viewmodel_selector.h"
        "UI/windows/viewmodel_viewer.h"
        "UI/windows/viewmodel_write_blocker.h"
        "UI/windows/voice_system_triggerer.h"
        "UI/window_manager/management_window.h"
        "UI/window_manager/window_manager.h"
        "Logs/Logs.h"
        "Logs/Detail.h"
        "Logs/Formatting.h"
        "Logs/Patterner.h"
        "dllmain.cpp"
        "entity_admin.cpp"
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
        "UI/windows/entity_window.cpp"
        "UI/windows/statescript_list.cpp"
        "UI/windows/statescript_window.cpp"
        "UI/windows/stu_explorer.cpp"
        "UI/windows/viewmodel_write_blocker.cpp"
        "UI/window_manager/management_window.cpp"
        "UI/window_manager/window_manager.cpp"
        "UI/windows/statescript_registry.cpp"
        "Logs/Logs.cpp"
        "Logs/Detail.cpp"
        "Utility/Exception.cpp"
        "UI/imgui_helpers.cpp"
        "UI/imgui_helpers.h"
        "UI/Drawers/EnumFlags.h"
        TimeScale.h
        Systems/SingleplayerSpawnSystem.h
        Systems/SystemBase.h
        Systems/SystemBase.h
        Systems/MapChangeObserver.h
        Components/Component.h
        UI/windows/component_window.h
        UI/windows/stu_enum_window.h
        UI/history_helper.h
        UI/windows/effect_tester.h
        Components/Component_4_Model.h
        displaytext_resources.h
        stu_serializer.h
        Components/Component_3_DataFlow.h
        Managers/DataFlowManager.h
        UI/windows/dataflow_window.h
        Components/Component_2_AssetManager.h
        Components/Component_54_LobbyMap.h
        UI/windows/teScene_window.h
        UI/windows/gamestate_window.h
        Systems/Preexisting/System_29_MirrorSystem.h
        Systems/Preexisting/System_22_NetworkEventSystem.h
        Systems/Preexisting/System_29_MirrorSystem.h
        UI/windows/MirrorSystem_explorer.h
        Components/Component_54_LobbyMap.cpp
)
target_include_directories(Prometheus-Core PRIVATE ${PROJECT_SOURCE_DIR}/)

# Project Config
Pro_ProjectVersion(Prometheus-Core 1 0 0 0)
Pro_ApplyPlatform_Dll(Prometheus-Core)

# Dll Config
set_target_properties(Prometheus-Core PROPERTIES OUTPUT_NAME "PrometheusCore")
if(MSVC)
    target_link_options(Prometheus-Core PRIVATE /ILK:${DIR_LIB}/Prometheus-Core.ilk)
endif ()

# Dependencies
target_link_libraries(Prometheus-Core PRIVATE Prometheus-Atlas-Extensions)

target_link_libraries(Prometheus-Core PRIVATE Prometheus-Generation)

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

find_package(imgui CONFIG REQUIRED)
target_link_libraries(Prometheus-Core PRIVATE imgui::imgui)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(Prometheus-Core PRIVATE fmt::fmt-header-only)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(Prometheus-Core PRIVATE spdlog::spdlog_header_only)

target_link_libraries(Prometheus-Core PRIVATE d3dcompiler dwmapi)
#get_target_property(IMGUI_INCLUDE_DIRS imgui::imgui
#        INTERFACE_INCLUDE_DIRECTORIES
#)
#
#message(STATUS IMGUI DIRS ${IMGUI_INCLUDE_DIRS})
#target_include_directories(Prometheus-Core ${IMGUI_INCLUDE_DIRS})

# TODO: PE does not support >8 characters and libbacktrace cannot find dwarf data?
# if (${PRO_PLATFORM} STREQUAL "MinGW")
#     message("Using MinGW compiler, enabling libbacktrace.")
#
#     find_library(BACKTRACE_LIB backtrace REQUIRED
#             PATHS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib")
#     find_path(BACKTRACE_INCLUDE backtrace.h REQUIRED
#             PATHS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include")
#
#     target_include_directories(Prometheus-Core PRIVATE ${BACKTRACE_INCLUDE})
#     target_link_libraries(Prometheus-Core PRIVATE ${BACKTRACE_LIB})
#
#     # Required for symbol resolution
#     target_compile_options(Prometheus-Core PRIVATE -g -gdwarf-2 -mllvm -no-dwarf-pub-sections)
# endif ()
