#pragma once

#include "context.hpp"
#include "swapchain.hpp"
#include <expected>
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
                const glimpse::renderer::VulkanContext& context,
                const glimpse::renderer::VulkanSwapchain& swapchain
            );
        private:
            GraphicsPipeline(
                vk::raii::PipelineLayout pipeline_layout,
                vk::raii::Pipeline graphics_pipeline
            );
            vk::raii::PipelineLayout m_pipeline_layout = nullptr; 
            vk::raii::Pipeline m_graphics_pipeline = nullptr;
        };
    }
}
