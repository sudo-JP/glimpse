#pragma once

#include "context.hpp"
#include "renderer/texture.hpp"
#include "swapchain.hpp"
#include <expected>
#include <optional>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace glimpse {
    namespace renderer {
        struct ShaderStageConfig {
            std::string filename;
            std::string vert_entry = "vertMain";
            std::string frag_entry = "fragMain";
        };

        class GraphicsPipeline {
        public:
            static std::expected<GraphicsPipeline, std::string> new_graphics_pipeline(
                const ShaderStageConfig& shader_config, 
                size_t max_frames_in_flight,
                const glimpse::renderer::VulkanContext& context,
                const glimpse::renderer::VulkanSwapchain& swapchain
            );

            template <typename T>
            void attach_resources(
                size_t max_frames_in_flight,
                const std::vector<vk::raii::Buffer>& uniform_buffers,
                const glimpse::renderer::Texture& texture
            );

            // Getters
            const vk::raii::Pipeline& get_graphics_pipeline() const;

            const std::optional<
                std::reference_wrapper<const vk::raii::DescriptorSet>
            > get_descriptor_set(size_t index) const;

            const vk::raii::PipelineLayout& get_pipeline_layout() const;
        private:
            GraphicsPipeline(
                const glimpse::renderer::VulkanContext& context,
                vk::raii::DescriptorSetLayout descriptor_set_layout,
                vk::raii::PipelineLayout pipeline_layout,
                vk::raii::Pipeline graphics_pipeline
            );
            std::reference_wrapper<const glimpse::renderer::VulkanContext> m_vk_ctx;
            vk::raii::DescriptorSetLayout m_descriptor_set_layout = nullptr;
            vk::raii::PipelineLayout m_pipeline_layout = nullptr; 
            vk::raii::Pipeline m_graphics_pipeline = nullptr;

            // For uniforms
            std::optional<vk::raii::DescriptorPool> m_descriptor_pool;
            std::optional<std::vector<vk::raii::DescriptorSet>> m_descriptor_sets;
        };
    }
}
