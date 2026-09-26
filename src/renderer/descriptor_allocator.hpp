#pragma once
#include "renderer/context.hpp"
#include "renderer/graphics_pipeline.hpp"
#include <vulkan/vulkan_raii.hpp>
#include <vector>

namespace glimpse {
    namespace renderer {
        class DescriptorAllocator {
        public:
            DescriptorAllocator(
                const glimpse::renderer::VulkanContext& context,
                const glimpse::renderer::GraphicsPipeline& pipeline,
                size_t max_frames_in_flight
            );

            template <typename T>
            void attach_resources(
                const std::vector<vk::raii::Buffer>& uniform_buffers,
                const glimpse::renderer::Texture& texture
            );

            const std::optional<
                std::reference_wrapper<const vk::raii::DescriptorSet>
            > get_descriptor_set(size_t index) const; 
        private:
            size_t m_max_frames_in_flight;
            std::reference_wrapper<const glimpse::renderer::VulkanContext> m_vk_ctx;
            vk::raii::DescriptorPool m_descriptor_pool = nullptr;
            std::vector<vk::raii::DescriptorSet> m_descriptor_sets;
        };
    }
}
