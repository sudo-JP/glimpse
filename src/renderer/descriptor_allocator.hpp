#pragma once
#include "renderer/context.hpp"
#include "renderer/graphics_pipeline.hpp"
#include "renderer/material.hpp"
#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace glimpse {
namespace renderer {
class DescriptorAllocator {
  public:
    DescriptorAllocator(
        const VulkanContext &context,
        const GraphicsPipeline &pipeline,
        size_t max_frames_in_flight
    );

    template <typename T>
    std::vector<vk::raii::DescriptorSet> attach_resources(
        const std::vector<vk::raii::Buffer> &uniform_buffers,
        const Material &material
    );

  private:
    size_t m_max_frames_in_flight;
    std::reference_wrapper<const VulkanContext> m_vk_ctx;
    vk::raii::DescriptorPool m_descriptor_pool = nullptr;
};
} // namespace renderer
} // namespace glimpse
