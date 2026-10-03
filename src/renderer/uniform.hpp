#pragma once

#include "renderer/context.hpp"
#include <cstddef>
#include <expected>
#include <string>
#include <vector>
#include <vulkan/vulkan_raii.hpp>
namespace glimpse::renderer {

template <typename T>
class Uniform {
  public:
    static std::expected<Uniform<T>, std::string> new_uniform(
        size_t max_frames_in_flight,
        const VulkanContext& context,
        T data
    );

    void set_data(const T& data);
    void update(size_t frame_index);
    const std::vector<vk::raii::Buffer>& get_uniform_buffers() const;

  private:
    Uniform(
        std::vector<vk::raii::Buffer> uniform_buffers,
        std::vector<vk::raii::DeviceMemory> uniform_buffers_memory,
        std::vector<T *> uniform_buffers_mapped,
        T data
    );

    T m_data;
    std::vector<vk::raii::Buffer> m_uniform_buffers;
    std::vector<vk::raii::DeviceMemory> m_uniform_buffers_memory;
    std::vector<T *> m_uniform_buffers_mapped;
};
} // namespace glimpse::renderer
