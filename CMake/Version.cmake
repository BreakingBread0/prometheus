macro(Pro_ProjectVersion Target Major Minor Patch Tweak)
    set_target_properties(${Target} PROPERTIES
            VERSION ${Major}.${Minor}.${Patch}.${Tweak}
            SOVERSION ${Major}
    )

    string(TOUPPER "${Target}" TARGET_DEF)
    string(REPLACE "-" "_" TARGET_DEF "${TARGET_DEF}")

    target_compile_definitions(${Target} PRIVATE
            ${TARGET_DEF}=1
            VERSION=${Major},${Minor},${Patch},${Tweak}
            VERSION_STRING="${Major}.${Minor}.${Patch}.${Tweak}"
            VERSION_MAJOR=${Major}
            VERSION_MINOR=${Minor}
            VERSION_PATCH=${Patch}
            VERSION_TWEAK=${Tweak}
            BUILD_TYPE="${CMAKE_BUILD_TYPE}")

    set(    ${TARGET_DEF}=1
            VERSION=${Major},${Minor},${Patch},${Tweak}
            VERSION_STRING="${Major}.${Minor}.${Patch}.${Tweak}"
            VERSION_MAJOR=${Major}
            VERSION_MINOR=${Minor}
            VERSION_PATCH=${Patch}
            VERSION_TWEAK=${Tweak})

    if(DEFINED(DIR_LIB))
        configure_file(${CMAKE_SOURCE_DIR}/CMake/version.rc.in ${DIR_LIB}/version.rc @ONLY)
        set(VRC ${DIR_LIB}/version.rc)
        target_sources(${Target} PRIVATE ${VRC})
    endif()
endmacro()
