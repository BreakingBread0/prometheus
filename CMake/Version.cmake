macro(Pro_ProjectVersion Target Major Minor Patch Tweak)
    set_target_properties(${Target} PROPERTIES
            VERSION ${Major}.${Minor}.${Patch}.${Tweak}
            SOVERSION ${Major}
    )

    target_compile_definitions(${Target} PRIVATE
            VERSION=${Major},${Minor},${Patch},${Tweak}
            VERSION_STRING="${Major}.${Minor}.${Patch}.${Tweak}"
            VERSION_MAJOR=${Major}
            VERSION_MINOR=${Minor}
            VERSION_PATCH=${Patch}
            VERSION_TWEAK=${Tweak})

    set(    VERSION=${Major},${Minor},${Patch},${Tweak}
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
