cmake_minimum_required(VERSION 3.21...3.25)
project(glimpse LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(glimpse
    src/main.cpp
)

target_link_libraries(glimpse PRIVATE Vulkan::Vulkan)
