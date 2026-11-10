#pragma once
#include "vulkan/vulkan.hpp"
#include <array>
#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

namespace glimpse::renderer {
struct VulkanVertex {
    glm::vec3 pos;
    //glm::vec3 color;
    glm::vec2 tex_coord;

    static vk::VertexInputBindingDescription get_binding_description();
    static std::array<vk::VertexInputAttributeDescription, 2>
    get_attribute_descriptions();
};

struct MVP {
    glm::mat4x4 model;
    glm::mat4x4 view;
    glm::mat4x4 proj;
};

struct VideoFrame {
    int width; 
    int height;
    std::vector<uint8_t> pixels;
};
} // namespace glimpse::renderer
