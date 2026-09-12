#pragma once
#include "vulkan/vulkan.hpp"
#include <array>
#include <glm/glm.hpp>

namespace glimpse {
    namespace renderer {
        struct VulkanVertex {
            glm::vec2 pos; 
            glm::vec3 color;

            static vk::VertexInputBindingDescription get_binding_description();
            static std::array<vk::VertexInputAttributeDescription, 2> get_attribute_descriptions();
        };

        struct MVP {
            glm::mat4x4 model;
            glm::mat4x4 view;
            glm::mat4x4 proj; 
        };
    }
}
