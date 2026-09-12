#pragma once

#include <vector>
#include <vulkan/vulkan_raii.hpp>
namespace glimpse {
    namespace renderer {

        template <typename T>
        class UniformBuffer {
        public:
        private: 
            std::vector<vk::raii::Buffer> m_uniform_buffers;
            std::vector<vk::raii::DeviceMemory> m_uniform_buffers_memory; 
            std::vector<T> m_uniform_buffers_mapped;
        };
    }
}
