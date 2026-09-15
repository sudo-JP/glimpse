#pragma once

#include "renderer/context.hpp"
#include <cstddef>
#include <expected>
#include <string>
#include <vector>
#include <vulkan/vulkan_raii.hpp>
namespace glimpse {
    namespace renderer {

        template <typename T>
        class UniformBuffer {
        public:
            static std::expected<UniformBuffer<T>, std::string> new_uniform_buffer(
                size_t max_frames_in_flight,
                const glimpse::renderer::VulkanContext& context
            );

            void update(size_t frame_index, const T& data);
            const std::vector<vk::raii::Buffer>& get_uniform_buffers() const;
        private: 
            UniformBuffer(
                std::vector<vk::raii::Buffer> uniform_buffers,
                std::vector<vk::raii::DeviceMemory> uniform_buffers_memory,
                std::vector<T *> uniform_buffers_mapped
            );
            std::vector<vk::raii::Buffer> m_uniform_buffers;
            std::vector<vk::raii::DeviceMemory> m_uniform_buffers_memory; 
            std::vector<T *> m_uniform_buffers_mapped;
        };
    }
}
