function(find_images_file OUTPUT_NAME)
    set(WORLD_DIR ${CMAKE_CURRENT_LIST_DIR}/src/world) 
    file(GLOB_RECURSE IMAGE_FILES CONFIGURE_DEPENDS
        "${WORLD_DIR}/*.png"
        "${WORLD_DIR}/*.jpg"
        "${WORLD_DIR}/*.jpeg"
    )
    set(${OUTPUT_NAME} ${IMAGE_FILES} PARENT_SCOPE)

endfunction()
