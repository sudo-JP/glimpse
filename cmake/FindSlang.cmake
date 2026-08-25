function(find_slang_files OUTPUT_NAME)
    set(WORLD_DIR ${CMAKE_CURRENT_LIST_DIR}/src/world) 
    file(GLOB_RECURSE SLANG_FILES
        "${WORLD_DIR}/*.slang"
    )
    set(${OUTPUT_NAME} ${SLANG_FILES} PARENT_SCOPE)

endfunction()
