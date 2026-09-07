#pragma once

#include "renderer/context.hpp"
#include "renderer/types.hpp"
#include <cstdint>
#include <expected>
#include <string>
#include <vector>
#include <vulkan/vulkan_raii.hpp>
namespace glimpse {
    namespace renderer {
        class Mesh {
        public:
            static std::expected<Mesh, std::string> new_mesh(
                const std::vector<glimpse::renderer::VulkanVertex>& vertices,
                const glimpse::renderer::VulkanContext& context
            );

            // getters
            const vk::raii::Buffer& get_vertex_buffer() const;
            uint32_t get_size() const;
        private:
            Mesh(
                uint32_t size,
                vk::raii::Buffer vertex_buffer,
                vk::raii::DeviceMemory vertex_buffer_memory
            );
            uint32_t m_size;
            vk::raii::Buffer m_vertex_buffer = nullptr;
            vk::raii::DeviceMemory m_vertex_buffer_memory = nullptr;
        };
    }
}
