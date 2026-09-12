#pragma once

#include "renderer/context.hpp"
#include <expected>
#include <string>
#include <vulkan/vulkan_raii.hpp>

namespace glimpse {
    namespace renderer {
        std::expected<
            std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>, 
            std::string> create_buffer(
            vk::DeviceSize size, 
            vk::BufferUsageFlags usage,
            vk::MemoryPropertyFlags properties,
            const glimpse::renderer::VulkanContext& context
        );
    }
}
