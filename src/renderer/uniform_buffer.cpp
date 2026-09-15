#include "uniform_buffer.hpp"
#include "renderer/buffer_utils.hpp"
#include "renderer/types.hpp"
#include <algorithm>

namespace glimpse::renderer {
    
    template<typename T>
    std::expected<UniformBuffer<T>, std::string> UniformBuffer<T>::new_uniform_buffer(
        size_t max_frames_in_flight,
        const glimpse::renderer::VulkanContext& context
    ) {

        std::vector<vk::raii::Buffer> uniform_buffers;
        std::vector<vk::raii::DeviceMemory> uniform_buffers_memory; 
        std::vector<T *> uniform_buffers_mapped;

        for (size_t i = 0; i < max_frames_in_flight; i++) {
            vk::DeviceSize buffer_size = sizeof(T);
            auto buffer_res = create_buffer(
                buffer_size, 
                vk::BufferUsageFlagBits::eUniformBuffer,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, 
                context
            );
            if (!buffer_res) return std::unexpected(std::move(buffer_res).error());
            auto [buffer, buffer_memory] = std::move(buffer_res).value();

            // Populate vector
            uniform_buffers.emplace_back(std::move(buffer));
            uniform_buffers_memory.emplace_back(std::move(buffer_memory));
            uniform_buffers_mapped.emplace_back(static_cast<T *>(uniform_buffers_memory.back().mapMemory(0, buffer_size)));
        }
        return UniformBuffer<T>(
            std::move(uniform_buffers),
            std::move(uniform_buffers_memory),
            std::move(uniform_buffers_mapped)
        );
    }
    
    template<typename T>
    UniformBuffer<T>::UniformBuffer(
        std::vector<vk::raii::Buffer> uniform_buffers,
        std::vector<vk::raii::DeviceMemory> uniform_buffers_memory,
        std::vector<T *> uniform_buffers_mapped
    ) : m_uniform_buffers(std::move(uniform_buffers)),
        m_uniform_buffers_memory(std::move(uniform_buffers_memory)),
        m_uniform_buffers_mapped(std::move(uniform_buffers_mapped)) 
    {}

    template<typename T>
    const std::vector<vk::raii::Buffer>& UniformBuffer<T>::get_uniform_buffers() const {
        return m_uniform_buffers;
    }


    template<typename T>
    void UniformBuffer<T>::update(size_t frame_index, const T& data) {
        *m_uniform_buffers_mapped[frame_index] = data;
    }

    template class UniformBuffer<glimpse::renderer::MVP>;
}
