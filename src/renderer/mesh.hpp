#pragma once

#include "renderer/context.hpp"
#include "renderer/types.hpp"
#include <expected>
#include <string>
#include <vector>
#include <vulkan/vulkan_raii.hpp>
namespace glimpse {
    namespace renderer {
        class Mesh {
        public:
            std::expected<Mesh, std::string> new_mesh(
                const std::vector<glimpse::renderer::VulkanVertex>& vertices,
                const glimpse::renderer::VulkanContext& context
            );
        private:
            Mesh();
            vk::raii::Buffer m_vertex_buffer = nullptr;
            vk::raii::DeviceMemory m_vertex_buffer_memory = nullptr;
        };
    }
}
