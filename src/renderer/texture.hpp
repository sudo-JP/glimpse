#pragma once

#include "renderer/context.hpp"
#include <expected>
#include <string>
#include <vulkan/vulkan_raii.hpp>

namespace glimpse {
    namespace renderer {
        class CommandRecorder;
        class Texture {
        public:
            static std::expected<Texture, std::string> new_texture(
                const std::string& filename,
                const glimpse::renderer::CommandRecorder& recorder,
                const glimpse::renderer::VulkanContext& context
            );

            // getters
            const vk::raii::Sampler& get_texture_sampler() const;
            const vk::raii::ImageView& get_texture_image_view() const;
        private:
            Texture(
                vk::raii::Image texture_image,
                vk::raii::DeviceMemory texture_image_memory,
                vk::raii::ImageView texture_image_view,
                vk::raii::Sampler texture_sampler
            );

        vk::raii::Image m_texture_image = nullptr;
        vk::raii::DeviceMemory m_texture_image_memory = nullptr;
        vk::raii::ImageView m_texture_image_view = nullptr;
        vk::raii::Sampler m_texture_sampler = nullptr;
         
        };
    }
}
