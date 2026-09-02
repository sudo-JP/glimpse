#include "command_recorder.hpp"
#include "renderer/graphics_pipeline.hpp"
#include "renderer/swapchain.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <expected>

namespace glimpse::renderer {
    CommandRecorder CommandRecorder::new_command_recorder(
        const VulkanContext& context
    ) {
        const uint32_t graphics_queue_index = context.get_graphics_queue_index();

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

    std::expected<void, std::string> CommandRecorder::transition_image_layout(
        uint32_t image_index,
        vk::ImageLayout old_layout,
        vk::ImageLayout new_layout,
        vk::AccessFlags2 src_access_mask,
        vk::AccessFlags2 dst_access_mask,
        vk::PipelineStageFlags2 src_stage_mask,
        vk::PipelineStageFlags2 dst_stage_mask,
        const glimpse::renderer::VulkanSwapchain& swapchain
    ) {
        const auto image_res = swapchain.get_image(image_index);
        if (!image_res) return std::unexpected(std::move(image_res).error());

        const auto& image = std::move(image_res).value();

        const auto subresource_range = vk::ImageSubresourceRange()
            .setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel(0)
            .setLevelCount(0)
            .setBaseArrayLayer(0)
            .setLayerCount(1);

        const auto barrier = vk::ImageMemoryBarrier2()
            .setSrcStageMask(src_stage_mask)
            .setSrcAccessMask(src_access_mask)
            .setDstStageMask(dst_stage_mask)
            .setDstAccessMask(dst_access_mask)
            .setOldLayout(old_layout)
            .setNewLayout(new_layout)
            .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setImage(image)
            .setSubresourceRange(subresource_range);

        auto dependency_info = vk::DependencyInfo()
            .setDependencyFlags({})
            .setImageMemoryBarrierCount(1)
            .setPImageMemoryBarriers(&barrier);

        m_command_buffers[m_frame_index].pipelineBarrier2(dependency_info);
        return {};
    }

    std::expected<void, std::string> CommandRecorder::record_command_buffer(
        uint32_t image_index,
        const glimpse::renderer::VulkanSwapchain& swapchain,
        const glimpse::renderer::GraphicsPipeline& pipeline
    ) {
        const auto& command_buffer = m_command_buffers[m_frame_index];
        command_buffer.begin({});
        auto transition_res = transition_image_layout(
            image_index,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal,
            {},
            vk::AccessFlagBits2::eColorAttachmentWrite,
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            swapchain
        );
        if (!transition_res) {
            command_buffer.reset();
            return std::unexpected(std::move(transition_res).error());
        }

        auto& image_view_res = swapchain.get_image_view(image_index);
        if (!image_view_res) {
            command_buffer.reset();
            return std::unexpected(std::move(image_view_res).error());
        }
        const auto& image_view = std::move(image_view_res).value();

        const auto clear_color = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
        auto attachment_info = vk::RenderingAttachmentInfo()
            .setImageView(image_view)
            .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setClearValue(clear_color);

        const auto& swapchain_extent = swapchain.get_extent();

        const auto render_area = vk::Rect2D()
            .setOffset(vk::Offset2D(0, 0))
            .setExtent(swapchain_extent);

        auto rendering_info = vk::RenderingInfo()
            .setRenderArea(render_area)
            .setLayerCount(1)
            .setColorAttachmentCount(1)
            .setPColorAttachments(&attachment_info);

        command_buffer.beginRendering(rendering_info);
        const auto& graphics_pipeline = pipeline.get_graphics_pipeline();

        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphics_pipeline);
        command_buffer.setViewport(
            0, 
            vk::Viewport(
                0.0f, 
                0.0f, 
                static_cast<float>(swapchain_extent.width), 
                static_cast<float>(swapchain_extent.height),
                0.0f,
                1.0f
            )
        );

        command_buffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchain_extent));

        command_buffer.draw(
            3,  // Vertex count
            1,  // Instance count
            0,  // first vertex offset
            0   // first instance offset
        );

        command_buffer.endRendering();

        transition_res = transition_image_layout(
            image_index,
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::ePresentSrcKHR,
            vk::AccessFlagBits2::eColorAttachmentWrite,
            {},
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            vk::PipelineStageFlagBits2::eBottomOfPipe,
            swapchain
        );
        if (!transition_res) {
            command_buffer.reset();
            return std::unexpected(std::move(transition_res).error());
        }

        command_buffer.end();
        return {};
    }

    const vk::raii::CommandBuffer& CommandRecorder::get_command_buffer() const {
        return m_command_buffers[m_frame_index];
    }
}
