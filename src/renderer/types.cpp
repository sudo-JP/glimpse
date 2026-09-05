#include "types.hpp"
#include <cstddef>

namespace glimpse::types {
    vk::VertexInputBindingDescription VulkanVertex::get_binding_descriptor() {
        return vk::VertexInputBindingDescription()
            .setBinding(0)
            .setStride(sizeof(VulkanVertex))
            .setInputRate(vk::VertexInputRate::eVertex);
    } 

    std::array<vk::VertexInputAttributeDescription, 2> VulkanVertex::get_attributee_descriptions() {
        auto pos = vk::VertexInputAttributeDescription()
            .setLocation(0)
            .setBinding(0)
            .setFormat(vk::Format::eR32G32Sfloat)
            .setOffset(offsetof(VulkanVertex, pos));
        auto color = vk::VertexInputAttributeDescription()
            .setLocation(1)
            .setBinding(0)
            .setFormat(vk::Format::eR32G32B32Sfloat)
            .setOffset(offsetof(VulkanVertex, color));
        return {pos, color};
    }
}
