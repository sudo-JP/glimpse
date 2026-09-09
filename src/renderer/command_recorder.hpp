#pragma once

#include "renderer/context.hpp"
#include "renderer/graphics_pipeline.hpp"
#include "renderer/swapchain.hpp"
#include <cstddef>
#include <cstdint>
#include <vulkan/vulkan_raii.hpp>

namespace glimpse {
    namespace renderer {
        // Forward decl cuz apparently wont compile
        class Mesh;

        class CommandRecorder {
        public:
            CommandRecorder(
                const VulkanContext& context,
                size_t max_frames_in_flight
            );
            std::expected<void, std::string> record_command_buffer(
                uint32_t image_index,
                size_t frame_index,
                const glimpse::renderer::VulkanSwapchain& swapchain,
                const glimpse::renderer::GraphicsPipeline& pipeline,
                const glimpse::renderer::Mesh& mesh
            );

            void reset_command_buffer(size_t index);
            void copy_and_submit_immediate(
                vk::raii::Buffer& src_buffer, 
                vk::raii::Buffer& dst_buffer, 
                vk::DeviceSize size
            ) const;

            // getters
            const vk::raii::CommandBuffer& get_command_buffer(size_t index) const;
            const vk::raii::CommandPool& get_command_pool() const;
        private:
            std::expected<void, std::string> transition_image_layout(
                uint32_t image_index,
                vk::ImageLayout old_layout,
                vk::ImageLayout new_layout,
                vk::AccessFlags2 src_access_mask,
                vk::AccessFlags2 dst_access_mask,
                vk::PipelineStageFlags2 src_stage_mask,
                vk::PipelineStageFlags2 dst_stage_mask,
                size_t frame_index,
                const glimpse::renderer::VulkanSwapchain& swapchain
            );
            std::reference_wrapper<const glimpse::renderer::VulkanContext> m_vk_ctx;
            vk::raii::CommandPool m_command_pool = nullptr;
            vk::raii::CommandBuffers m_command_buffers;
        };
    }
}
