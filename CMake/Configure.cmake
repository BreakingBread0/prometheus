function(Pro_SetOutput Target Bin Lib)
    if(NOT DEFINED Bin)
        set(Bin ${DIR_BIN})
    endif()
    if(NOT DEFINED Lib)
        set(Lib ${DIR_LIB})
    endif()

    set_target_properties(${Target} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${Bin}"
            ARCHIVE_OUTPUT_DIRECTORY "${Lib}"
            LIBRARY_OUTPUT_DIRECTORY "${Lib}"
            PDB_OUTPUT_DIRECTORY     "${Lib}"
    )
endfunction()