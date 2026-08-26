function(compile_slangs SLANG_FILES)
    set(SHADERS_OUTPUT_DIR ${CMAKE_BINARY_DIR}/shaders)
    file(MAKE_DIRECTORY ${SHADERS_OUTPUT_DIR})

    set(SLANG_ENTRY -entry vertMain -entry fragMain)
    set(SLANG_TARGET -target spirv -profile spirv_1_4 -emit-spirv-directly)
    set(SLANG_FLAGS ${SLANG_TARGET} -fvk-use-entrypoint-name ${SLANG_ENTRY} -o)
    foreach(SLANG_FILE ${SLANG_FILES})
        cmake_path(GET SLANG_FILE STEM SHADER_NAME)
        set(SHADER_SPV ${SHADERS_OUTPUT_DIR}/${SHADER_NAME}.spv)
        add_custom_command(
            OUTPUT ${SHADER_SPV}
            COMMAND slangc ${SLANG_FILE} ${SLANG_FLAGS} ${SHADER_SPV}
            DEPENDS ${SLANG_FILE}
        )
        list(APPEND SPV_OUTPUTS ${SHADER_SPV})
    endforeach()
    add_custom_target(compile_shaders ALL DEPENDS ${SPV_OUTPUTS})
endfunction()
