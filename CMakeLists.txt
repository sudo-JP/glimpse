cmake_minimum_required(VERSION 3.21...3.25)
project(glimpse LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED MODULE)
find_package(Ktx CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

include(cmake/FindSlang.cmake)
include(cmake/CompileSlang.cmake)

find_slang_files(SLANG_FILES)
compile_slangs(${SLANG_FILES})

add_executable(${PROJECT_NAME})

target_compile_definitions(glimpse PRIVATE 
    SHADER_DIR="${CMAKE_BINARY_DIR}/shaders"
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_subdirectory(src)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    Vulkan::Vulkan
    glfw
    glm::glm
    KTX::ktx
)
