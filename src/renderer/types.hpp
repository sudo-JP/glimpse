#pragma once
#include "vulkan/vulkan.hpp"
#include <array>
#include <glm/glm.hpp>

namespace glimpse {
    namespace types {
        struct VulkanVertex {
            glm::vec2 pos; 
            glm::vec3 color;

            static vk::VertexInputBindingDescription get_binding_descriptor();
            static std::array<vk::VertexInputAttributeDescription, 2> get_attributee_descriptions();
        };
    }
}
