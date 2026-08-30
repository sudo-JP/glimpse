#include "command_recorder.hpp"
#include <cstdint>

namespace glimpse::renderer {
    CommandRecorder CommandRecorder::new_command_recorder(
        const VulkanContext& context
    ) {
        const uint32_t graphics_queue_index = context.get_graphics_queue_index();
        constexpr int max_frames_in_flight = 2;

        auto pool_info = vk::CommandPoolCreateInfo()
            .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
            .setQueueFamilyIndex(graphics_queue_index);

        auto const& device = context.get_device();
        auto command_pool = vk::raii::CommandPool(device, pool_info);

        auto alloc_info = vk::CommandBufferAllocateInfo()
            .setCommandPool(command_pool)
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(max_frames_in_flight);

        auto command_buffer = vk::raii::CommandBuffers(device, alloc_info);

        return CommandRecorder();
    }

    void CommandRecorder::transition_image_layout(
        uint32_t image_index,
        vk::ImageLayout old_layout,
        vk::ImageLayout new_layout,
        vk::AccessFlags2 src_access_mask,
        vk::AccessFlags2 dst_access_mask,
        vk::PipelineStageFlags2 src_stage_mask,
        vk::PipelineStageFlags2 dst_stage_mask
    ) {
        
    }

    void CommandRecorder::record_command_buffer(uint32_t image_index) {
        
    }
}
