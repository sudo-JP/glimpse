#pragma once

#include "renderer/context.hpp"
#include "renderer/graphics_pipeline.hpp"
#include "renderer/swapchain.hpp"
#include <cstdint>
#include <vulkan/vulkan_raii.hpp>

namespace glimpse {
    namespace renderer {
        class CommandRecorder {
        public:
            static CommandRecorder new_command_recorder(
                const VulkanContext& context
            );

            std::expected<void, std::string> record_command_buffer(
                uint32_t image_index,
                const glimpse::renderer::VulkanSwapchain& swapchain,
                const glimpse::renderer::GraphicsPipeline& pipeline
            );
        private:
            std::expected<void, std::string> transition_image_layout(
                uint32_t image_index,
                vk::ImageLayout old_layout,
                vk::ImageLayout new_layout,
                vk::AccessFlags2 src_access_mask,
                vk::AccessFlags2 dst_access_mask,
                vk::PipelineStageFlags2 src_stage_mask,
                vk::PipelineStageFlags2 dst_stage_mask,
                const glimpse::renderer::VulkanSwapchain& swapchain
            );
            vk::raii::CommandPool m_command_pool = nullptr;
            std::vector<vk::raii::CommandBuffer> m_command_buffers;
            uint32_t m_frame_index = 0;
        };
    }
}
