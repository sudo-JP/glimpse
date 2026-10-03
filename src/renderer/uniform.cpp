#include "uniform.hpp"
#include "renderer/types.hpp"
#include "renderer/utils.hpp"
#include <algorithm>

namespace glimpse::renderer {

template <typename T>
std::expected<Uniform<T>, std::string>
Uniform<T>::new_uniform(
    size_t max_frames_in_flight,
    const VulkanContext& context,
    T data
) {
    std::vector<vk::raii::Buffer> uniform_buffers;
    std::vector<vk::raii::DeviceMemory> uniform_buffers_memory;
    std::vector<T *> uniform_buffers_mapped;

    for (size_t i = 0; i < max_frames_in_flight; i++) {
        vk::DeviceSize buffer_size = sizeof(T);
        auto buffer_res = create_buffer(
            buffer_size,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible |
                vk::MemoryPropertyFlagBits::eHostCoherent,
            context
        );
        if (!buffer_res)
            return std::unexpected(std::move(buffer_res).error());
        auto [buffer, buffer_memory] = std::move(buffer_res).value();

        // Populate vector
        uniform_buffers.emplace_back(std::move(buffer));
        uniform_buffers_memory.emplace_back(std::move(buffer_memory));
        uniform_buffers_mapped.emplace_back(static_cast<T *>(
            uniform_buffers_memory.back().mapMemory(0, buffer_size)));
    }
    return Uniform<T>(
        std::move(uniform_buffers),
        std::move(uniform_buffers_memory),
        std::move(uniform_buffers_mapped),
        std::move(data)
    );
}

template <typename T>
Uniform<T>::Uniform(
    std::vector<vk::raii::Buffer> uniform_buffers,
    std::vector<vk::raii::DeviceMemory> uniform_buffers_memory,
    std::vector<T *> uniform_buffers_mapped,
    T data
)
    : m_uniform_buffers(std::move(uniform_buffers)),
      m_uniform_buffers_memory(std::move(uniform_buffers_memory)),
      m_uniform_buffers_mapped(std::move(uniform_buffers_mapped)),
      m_data(std::move(data)) {}

template <typename T>
const std::vector<vk::raii::Buffer> &
Uniform<T>::get_uniform_buffers() const {
    return m_uniform_buffers;
}

template <typename T>
void Uniform<T>::set_data(const T& data) {
    m_data = data;
}

template <typename T>
void Uniform<T>::update(size_t frame_index) {
    *m_uniform_buffers_mapped[frame_index] = m_data;
}

template class Uniform<MVP>;
} // namespace glimpse::renderer
