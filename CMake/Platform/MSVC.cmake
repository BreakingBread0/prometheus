if(!MSVC)
    message(FATAL_ERROR "Prometheus currently only supports MSVC builds on Windows.")
endif()

set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo,Release>:ProgramDatabase>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

function(Pro_ApplyPlatform TargetName)
    set(TARGET_COMPILE_OPTIONS /W0 /Gy /Oi /sdl /GS- /std:c++20 /permissive-)
    set(TARGET_COMPILE_DEFS _WINDOWS _WINSOCKAPI_ prometheus_EXPORTS NOGDI _USRDLL IMGUI_DEFINE_MATH_OPERATORS IMGUI_USE_WCHAR32)
    set(TARGET_COMPILE_FEATURES cxx_std_20)
    set(TARGET_LINK_OPTIONS /DLL)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
        set(TARGET_COMPILE_OPTIONS ${TARGET_COMPILE_OPTIONS} /Gy /Zi )
        set(TARGET_COMPILE_DEFS ${TARGET_COMPILE_DEFS} WIN32_LEAN_AND_MEAN NDEBUG IMGUI_DEFINE_MATH_OPERATORS IMGUI_USE_WCHAR32 _CRT_SECURE_NO_WARNINGS)
        set(TARGET_LINK_OPTIONS ${TARGET_LINK_OPTIONS} /DEBUG)
    endif()

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        set(TARGET_COMPILE_OPTIONS ${TARGET_COMPILE_OPTIONS} /EHa- /EHs- /EHc- /EHr-)
        set(TARGET_LINK_OPTIONS ${TARGET_LINK_OPTIONS} /DEBUG)
        set_target_properties(${TargetName} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
    endif()

    message("Compiling target: ${TargetName} with options:")
    message(STATUS "Linker options          : ${TARGET_LINK_OPTIONS}")
    message(STATUS "Compilation options     : ${TARGET_COMPILE_OPTIONS}")
    message(STATUS "Compilation definitions : ${TARGET_COMPILE_DEFS}")
    message(STATUS "Compilation features    : ${TARGET_COMPILE_FEATURES}")
    message(STATUS "Toolchain: ${CMAKE_TOOLCHAIN_FILE}")

    target_link_options(${TargetName} PRIVATE ${TARGET_LINK_OPTIONS})
    target_compile_options(${TargetName} INTERFACE ${TARGET_COMPILE_OPTIONS})
    target_compile_definitions(${TargetName} PRIVATE ${TARGET_COMPILE_DEFS})
    target_compile_features(${TargetName} PRIVATE ${TARGET_COMPILE_FEATURES})

    target_include_directories(${TargetName} PUBLIC ${VCPKG_IMPORT_PREFIX}/include)
endfunction()