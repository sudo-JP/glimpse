#include "descriptor_allocator.hpp"
#include "renderer/types.hpp"

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

    template <typename T>
    void DescriptorAllocator::attach_resources(
        const std::vector<vk::raii::Buffer>& uniform_buffers,
        const glimpse::renderer::Texture& texture
    ) {
        const auto& device = m_vk_ctx.get().get_device();
        for (size_t i = 0; i < m_max_frames_in_flight; ++i) {
            auto buffer_info = vk::DescriptorBufferInfo()
                .setBuffer(*uniform_buffers[i])
                .setOffset(0)
                .setRange(sizeof(T));

            auto write_uniform = vk::WriteDescriptorSet()
                .setDstSet(*m_descriptor_sets[i]) 
                .setDstBinding(0)
                .setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setPBufferInfo(&buffer_info);

            auto image_info = vk::DescriptorImageInfo()
                .setSampler(texture.get_texture_sampler())
                .setImageView(texture.get_texture_image_view())
                .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

            auto write_sampler = vk::WriteDescriptorSet()
                .setDstSet(*m_descriptor_sets[i])
                .setDstBinding(1)
                .setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                .setPImageInfo(&image_info);

            std::array<vk::WriteDescriptorSet, 2> writes = { write_uniform, write_sampler };
            device.updateDescriptorSets(writes, {});
        } 
    }

    const std::optional<
        std::reference_wrapper<const vk::raii::DescriptorSet>
    > DescriptorAllocator::get_descriptor_set(size_t index) const {
        if (index >= m_descriptor_sets.size()) return std::nullopt;
        return std::reference_wrapper{m_descriptor_sets[index]};
    }

    template void glimpse::renderer::DescriptorAllocator::attach_resources<glimpse::renderer::MVP>( 
        const std::vector<vk::raii::Buffer>&,
        const glimpse::renderer::Texture& 
    );
}
