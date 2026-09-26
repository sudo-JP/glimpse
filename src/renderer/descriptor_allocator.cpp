#include "descriptor_allocator.hpp"

namespace glimpse::renderer {
      
    DescriptorAllocator::DescriptorAllocator(
        const glimpse::renderer::VulkanContext& context,
        const glimpse::renderer::GraphicsPipeline& pipeline,
        size_t max_frames_in_flight
    ) :
    m_max_frames_in_flight(max_frames_in_flight),
    m_vk_ctx(context)
    {
        const auto& device = context.get_device();
        std::array<vk::DescriptorPoolSize, 2> pool_sizes = {
            vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, static_cast<uint32_t>(m_max_frames_in_flight)),
            vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, static_cast<uint32_t>(m_max_frames_in_flight))
        };
    
        auto pool_info = vk::DescriptorPoolCreateInfo()
            .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
            .setMaxSets(static_cast<uint32_t>(m_max_frames_in_flight))
            .setPoolSizes(pool_sizes);

        m_descriptor_pool = vk::raii::DescriptorPool(device, pool_info);

        const auto& layout = pipeline.get_descriptor_set_layout();
        std::vector<vk::DescriptorSetLayout> layouts(m_max_frames_in_flight, layout);
        auto alloc_info = vk::DescriptorSetAllocateInfo()
            .setDescriptorPool(m_descriptor_pool)
            .setDescriptorSetCount(static_cast<uint32_t>(layouts.size()))
            .setPSetLayouts(layouts.data());

        m_descriptor_sets = device.allocateDescriptorSets(alloc_info);
    }

    const std::optional<
        std::reference_wrapper<const vk::raii::DescriptorSet>
    > GraphicsPipeline::get_descriptor_set(size_t index) const {
        if (!m_descriptor_sets.has_value()) return std::nullopt;
        const auto& descriptor_sets = *m_descriptor_sets;
        if (index >= descriptor_sets.size()) return std::nullopt;
        return std::reference_wrapper{descriptor_sets[index]};
    }

}
