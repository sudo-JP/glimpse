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

        template <typename T>
        std::expected<std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>,
            std::string> create_staging_buffer(
            const glimpse::renderer::VulkanContext& context,
            const std::vector<T> data,
            const vk::DeviceSize size
        );
    }
}
