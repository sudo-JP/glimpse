function(compile_images IMAGE_FILES)
    set(SHADERS_OUTPUT_DIR ${CMAKE_BINARY_DIR}/shaders)
    file(MAKE_DIRECTORY ${SHADERS_OUTPUT_DIR})

    set(TOKTX_ASSIGN --assign_oetf srgb --assign_primaries bt709)
    set(TOKTX_FORMAT --t2 --target_type RGBA)
    set(TOKTX_FLAGS ${TOKTX_ASSIGN} ${TOKTX_FORMAT})
    foreach(IMAGE_FILE ${IMAGE_FILES})
        cmake_path(GET IMAGE_FILE STEM SHADER_NAME)
        set(SHADER_KTX ${SHADERS_OUTPUT_DIR}/${SHADER_NAME}.ktx2)
        list(APPEND KTX_OUTPUTS ${SHADER_KTX})
        add_custom_command(
            OUTPUT ${SHADER_KTX}
            COMMAND toktx ${TOKTX_FLAGS} ${SHADER_KTX} ${IMAGE_FILE}
            DEPENDS ${IMAGE_FILE}
        )
    endforeach()

    file(GLOB EXISTING_KTX_FILES "${SHADERS_OUTPUT_DIR}/*.ktx2")
    foreach(EXISTING_KTX_FILE ${EXISTING_KTX_FILES})
        list(FIND KTX_OUTPUTS "${EXISTING_KTX_FILE}" EXPECTED_INDEX)
        if(EXPECTED_INDEX EQUAL -1)
            file(REMOVE "${EXISTING_KTX_FILE}")
        endif()
    endforeach()

    add_custom_target(compile_images ALL DEPENDS ${KTX_OUTPUTS})
endfunction()
