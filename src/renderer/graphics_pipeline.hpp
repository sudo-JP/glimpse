#pragma once

#include <expected>
#include <vulkan/vulkan_raii.hpp>
#include "context.hpp"

namespace glimpse {
    namespace renderer {
        class GraphicsPipeline {
        public:
            std::expected<vk::raii::ShaderModule, std::string> create_shader_module(
                const std::string& filename, const glimpse::renderer::VulkanContext& context
            );
        private:
            vk::raii::PipelineLayout m_pipeline_layout = nullptr; 
        };
    }
}
