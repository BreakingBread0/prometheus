cmake_minimum_required(VERSION 3.15)
project(prometheus VERSION 1.0.0 LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Source files
set(SOURCES
        dllmain.cpp
        entity_admin.cpp
        ExceptionFormatter.cpp
        filetype_library.cpp
        globals.cpp
        ImguiRenderer.cpp
        pch.cpp
        player_spawner.cpp
        ResourceManager.h
        serialization.cpp
        Statescript.cpp
        state_replicator.cpp
        stringhash_library.cpp
        STU.cpp
        STU_Editable.cpp
        stu_resources.cpp
        windows/entity_window.cpp
        windows/statescript_list.cpp
        windows/statescript_window.cpp
        windows/stu_explorer.cpp
        windows/viewmodel_write_blocker.cpp
        window_manager/management_window.cpp
        window_manager/window_manager.cpp
)

# Header files
set(HEADERS
        entity_admin.h
        ExceptionFormatter.h
        filetype_library.h
        framework.h
        game.h
        globals.h
        hash_exporter.h
        idadefs.h
        ImguiRenderer.h
        imgui_json.h
        JAM.h
        lazy_importer.h
        MovementState.h
        pch.h
        pe.h
        player_spawner.h
        search_helper.h
        serialization.h
        Statescript.h
        StatescriptExpressionParser.h
        StatescriptVar.h
        statescript_logger.h
        state_replicator.h
        stringhash_library.h
        STU.h
        STU_Editable.h
        stu_resources.h
        Viewmodel.h
        "windows/AAA window template.h"
        windows/asset_pack_manager.h
        windows/awful_demo_server.h
        windows/bitconverter_calls.h
        windows/blizzbbqueueq_window.h
        windows/comments_window.h
        windows/component_notes.h
        windows/copy_window.h
        windows/CVD_Edit.h
        windows/CVD_List.h
        windows/CVD_VarbagSelector.h
        windows/cvexpression_viewer.h
        windows/entadmin_compodependencies.h
        windows/entityadmin_systems_window.h
        windows/entityadmin_timing.h
        windows/entityadmin_window.h
        windows/entitydef_explorer.h
        windows/entitydef_list.h
        windows/entity_bounds_renderer.h
        windows/entity_spawner.h
        windows/entity_window.h
        windows/filetype_window.h
        windows/freecam_movement_recorder.h
        windows/game_data_window.h
        windows/game_msg_window.h
        windows/hash_display_window.h
        windows/herolineup_creator.h
        windows/imgui_demo_window.h
        windows/ini_settings_window.h
        windows/jamprotocol_window.h
        windows/jam_messagedetail_window.h
        windows/example_window.h
        windows/loadlist_visualizer.h
        windows/lobbymap_tester.h
        windows/local_player_component_insights.h
        windows/logicalbutton_names.h
        windows/log_window.h
        windows/manager_4_secrets.h
        windows/manager_infos.h
        windows/message_window.h
        windows/movementsys_editor.h
        windows/movstate_visualizer.h
        windows/name_value_struct.h
        windows/pachimari_creator.h
        windows/player_spawner_deluxe.h
        windows/pvpgame_explorer.h
        windows/radio_selector_window.h
        windows/statescript_fake_logicalbtn.h
        windows/statescript_fuzzer.h
        windows/statescript_graph_window.h
        windows/statescript_info.h
        windows/statescript_list.h
        windows/statescript_log_window.h
        windows/statescript_node_search.h
        windows/statescript_registry.h
        windows/statescript_varbag_window.h
        windows/statescript_window.h
        windows/string_rep_edit.h
        windows/stu_bslist_edit.h
        windows/stu_explorer.h
        windows/stu_object_edit.h
        windows/stu_primitive_edit.h
        windows/stu_registry.h
        windows/stu_types.h
        windows/STU_Types_window.h
        windows/textedit_window.h
        windows/type_7b_strings.h
        windows/viewmodel_selector.h
        windows/viewmodel_viewer.h
        windows/viewmodel_write_blocker.h
        windows/voice_system_triggerer.h
        window_manager/management_window.h
        window_manager/window_manager.h
        mingw_compat/ow_excpt.h
)

# Create the DLL
add_library(prometheus SHARED ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(prometheus PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../external
)

# Preprocessor definitions
target_compile_definitions(prometheus PRIVATE
        UNICODE
        _UNICODE
        prometheus_EXPORTS
        _WINDOWS
        _USRDLL
        IMGUI_DEFINE_MATH_OPERATORS
        IMGUI_USE_WCHAR32
        _CRT_SECURE_NO_WARNINGS
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
        $<$<CONFIG:Release>:_WINSOCKAPI_>
        $<$<CONFIG:Release>:WIN32_LEAN_AND_MEAN>
)

# Compiler options (MSVC vs GCC/MinGW)
if(MSVC)
    target_compile_options(prometheus PRIVATE
            /W3  # Warning level 3
            $<$<CONFIG:Release>:/O2>  # Optimize for speed
            $<$<CONFIG:Release>:/Oi>  # Enable intrinsic functions
            $<$<CONFIG:Release>:/GL>  # Whole program optimization
            $<$<CONFIG:Release>:/GS->  # Disable buffer security check
    )

    # Linker options
    target_link_options(prometheus PRIVATE
            /SUBSYSTEM:WINDOWS
            $<$<CONFIG:Release>:/LTCG>  # Link-time code generation
            $<$<CONFIG:Release>:/OPT:REF>  # Remove unreferenced functions
            $<$<CONFIG:Release>:/OPT:ICF>  # Enable COMDAT folding
    )

    # Disable User Account Control
    set_target_properties(prometheus PROPERTIES
            LINK_FLAGS "/MANIFESTUAC:NO"
    )
else()
    # GCC/MinGW options
    target_compile_options(prometheus PRIVATE
            -Wall
            -Wextra
            $<$<CONFIG:Release>:-O3>
            $<$<CONFIG:Release>:-flto>
    )

    # MinGW linker options
    target_link_options(prometheus PRIVATE
            -mwindows
            $<$<CONFIG:Release>:-s>  # Strip symbols
            $<$<CONFIG:Release>:-flto>
    )

    include_directories(SYSTEM /usr/x86_64-w64-mingw32/include)
endif()


find_path(FAST_CPP_CSV_PARSER_INCLUDE_DIRS "fast-cpp-csv-parser/csv.h")
target_include_directories(prometheus PRIVATE ${FAST_CPP_CSV_PARSER_INCLUDE_DIRS})

find_package(Freetype REQUIRED)
target_link_libraries(prometheus PRIVATE Freetype::Freetype)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(prometheus PRIVATE imgui::imgui)

find_package(minhook CONFIG REQUIRED)
target_link_libraries(prometheus PRIVATE minhook::minhook)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(prometheus PRIVATE nlohmann_json::nlohmann_json)


# Install rules
install(TARGETS prometheus
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)