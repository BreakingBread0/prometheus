add_compile_options("$<$<CONFIG:Debug,RelWithDebInfo,Release>:-gcodeview>")
# set(CMAKE_SHARED_LINKER_FLAGS "-static-libgcc -static-libstdc++")

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_INSTALL_LIBDIR "lib" CACHE PATH "") # CMake generation error
set(CMAKE_SHARED_LIBRARY_PREFIX "") # Remove lib prefix

# add_link_options(-v)

function(Pro_ApplyPlatform TargetName)
    set(TARGET_COMPILE_OPTIONS
            -w -ffunction-sections -fdata-sections -fbuiltin
            -fno-stack-protector -std=c++20 -fms-extensions
            $<$<CONFIG:Release>:-gcodeview>
            $<$<CONFIG:Debug>:-fignore-exceptions -fno-asynchronous-unwind-tables>
    )

    set(TARGET_COMPILE_DEFS
            _WINDOWS _WIN32 _WINSOCKAPI_ prometheus_EXPORTS NOGDI _USRDLL
            SPDLOG_FMT_EXTERNAL IMGUI_DEFINE_MATH_OPERATORS IMGUI_USE_WCHAR32
            WINVER=0x0601 _WIN32_WINNT=0x0601
            $<$<CONFIG:Release>:WIN32_LEAN_AND_MEAN NDEBUG _CRT_SECURE_NO_WARNINGS>
    )

    set(TARGET_COMPILE_FEATURES
            cxx_std_20
    )

    set(TARGET_LINK_OPTIONS
            -shared -Wl,--gc-sections
            -static
            -static-libgcc
            -static-libstdc++
            -lc++
    )

    message("Configuring target: ${TargetName} with options:")
    message(STATUS "Linker options          : ${TARGET_LINK_OPTIONS}")
    message(STATUS "Compilation options     : ${TARGET_COMPILE_OPTIONS}")
    message(STATUS "Compilation definitions : ${TARGET_COMPILE_DEFS}")
    message(STATUS "Compilation features    : ${TARGET_COMPILE_FEATURES}")
    message(STATUS "Toolchain               : ${CMAKE_TOOLCHAIN_FILE}")

    target_link_options(${TargetName} PRIVATE ${TARGET_LINK_OPTIONS})
    target_compile_options(${TargetName} PRIVATE ${TARGET_COMPILE_OPTIONS})
    target_compile_definitions(${TargetName} PRIVATE ${TARGET_COMPILE_DEFS})
    target_compile_features(${TargetName} PRIVATE ${TARGET_COMPILE_FEATURES})
    set_target_properties(${TargetName} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
    target_include_directories(${TargetName} PUBLIC ${VCPKG_IMPORT_PREFIX}/include)

    add_compile_definitions(
        MINGW # So we can include un-caputalized header files instead
    )
endfunction()