#include "command_recorder.hpp"
#include "renderer/graphics_pipeline.hpp"
#include "renderer/mesh.hpp"
#include "renderer/swapchain.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <expected>

namespace glimpse::renderer {
    namespace {
        void copy_to_buffer_image(
            vk::raii::CommandBuffer& command_buffer, 
            const vk::raii::Buffer& buffer, 
            const vk::raii::Image& image, 
            uint32_t width, 
            uint32_t height
        ) {
            auto image_subresource = vk::ImageSubresourceLayers()
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setMipLevel(0)
                .setBaseArrayLayer(0)
                .setLayerCount(1);

		    auto region = vk::BufferImageCopy()
		        .setBufferOffset(0)
		        .setBufferRowLength(0)
		        .setBufferImageHeight(0)
		        .setImageSubresource(image_subresource)
		        .setImageOffset({0, 0, 0})
		        .setImageExtent({width, height, 1});
		    command_buffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
	    }
    } // end helper namespace

    // Constructor
    CommandRecorder::CommandRecorder(
    const VulkanContext& context,
    size_t max_frames_in_flight
    ) : 
        m_command_pool(context.get_device(), vk::CommandPoolCreateInfo()
            .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
            .setQueueFamilyIndex(context.get_graphics_queue_index())),

        m_command_buffers(context.get_device(), vk::CommandBufferAllocateInfo()
            .setCommandPool(m_command_pool)
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(max_frames_in_flight)),
        m_vk_ctx(context)
    {}


    std::expected<void, std::string> CommandRecorder::transition_swapchain_image(
        uint32_t image_index,
        vk::ImageLayout old_layout,
        vk::ImageLayout new_layout,
        vk::AccessFlags2 src_access_mask,
        vk::AccessFlags2 dst_access_mask,
        vk::PipelineStageFlags2 src_stage_mask,
        vk::PipelineStageFlags2 dst_stage_mask,
        size_t frame_index,
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

        m_command_buffers[frame_index].pipelineBarrier2(dependency_info);
        return {};
    }

    std::expected<void, std::string> CommandRecorder::transition_image_layout(
        vk::raii::CommandBuffer& command_buffer, 
        const vk::raii::Image& image, 
        vk::ImageLayout old_layout, 
        vk::ImageLayout new_layout
    ) const {
        auto subresource_range = vk::ImageSubresourceRange()
            .setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setLevelCount(1)
            .setLayerCount(1);

        auto barrier = vk::ImageMemoryBarrier()
            .setOldLayout(old_layout)
            .setNewLayout(new_layout)
            .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setImage(image)
            .setSubresourceRange(subresource_range);

        vk::PipelineStageFlags src_stage;
        vk::PipelineStageFlags dst_stage;

        if (old_layout == vk::ImageLayout::eUndefined
        && new_layout == vk::ImageLayout::eTransferDstOptimal) {
            barrier.srcAccessMask = {};
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

            src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
            dst_stage = vk::PipelineStageFlagBits::eTransfer;
        } else if (old_layout == vk::ImageLayout::eTransferDstOptimal
        && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            src_stage = vk::PipelineStageFlagBits::eTransfer;
            dst_stage = vk::PipelineStageFlagBits::eFragmentShader;
        } else {
            return std::unexpected("unsupported layout transition");
        }

        command_buffer.pipelineBarrier(src_stage, dst_stage, {}, {}, {}, barrier);

        return {};
    }

    std::expected<void, std::string> CommandRecorder::record_command_buffer(
        uint32_t image_index,
        size_t frame_index,
        const glimpse::renderer::VulkanSwapchain& swapchain,
        const glimpse::renderer::GraphicsPipeline& pipeline, 
        const glimpse::renderer::Mesh& mesh
    ) {
        // Start command buffer, after select the current command buffer index
        const auto& command_buffer = m_command_buffers[frame_index];
        command_buffer.begin({});
        auto transition_res = transition_swapchain_image(
            image_index,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal,
            {},
            vk::AccessFlagBits2::eColorAttachmentWrite,
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            frame_index,
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
        const auto& vertex_buffer = mesh.get_vertex_buffer();
        const auto& index_buffer = mesh.get_index_buffer();

        // TODO: add offset to mesh, currently hard code 0 
        command_buffer.bindVertexBuffers(0, *vertex_buffer, {0});
        command_buffer.bindIndexBuffer(*index_buffer, 0, vk::IndexType::eUint16); // TODO: add get type for mesh

        const auto& descriptor_set_option = pipeline.get_descriptor_set(frame_index);
        if (descriptor_set_option.has_value()) {
            const auto& pipeline_layout = pipeline.get_pipeline_layout();

            const auto& descriptor_set = descriptor_set_option->get();
            command_buffer.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                pipeline_layout,
                0,
                *descriptor_set,
                nullptr
            );
        }

        command_buffer.drawIndexed(mesh.get_indices_size(), 1, 0, 0, 0);

        command_buffer.endRendering();

        transition_res = transition_swapchain_image(
            image_index,
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::ePresentSrcKHR,
            vk::AccessFlagBits2::eColorAttachmentWrite,
            {},
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            vk::PipelineStageFlagBits2::eBottomOfPipe,
            frame_index,
            swapchain
        );
        if (!transition_res) {
            command_buffer.reset();
            return std::unexpected(std::move(transition_res).error());
        }

        command_buffer.end();
        return {};
    }

    vk::raii::CommandBuffer CommandRecorder::begin_single_command_time_commands() const {
        auto alloc_info = vk::CommandBufferAllocateInfo()
            .setCommandPool(m_command_pool)
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(1);

        const auto& context = m_vk_ctx.get();
        const auto& device = context.get_device();
        auto command_buffer = std::move(device.allocateCommandBuffers(alloc_info).front());

        // Buffer 
        command_buffer.begin({
            vk::CommandBufferUsageFlagBits::eOneTimeSubmit
        });
        
        return std::move(command_buffer);
    }

    std::expected<void, std::string> CommandRecorder::upload_texture(
        const vk::raii::Buffer& staging_buffer,
        const vk::raii::Image& image, 
        uint32_t width,
        uint32_t height
    ) const {
        auto command_buffer = begin_single_command_time_commands();
        auto transition_res = transition_image_layout(
            command_buffer, 
            image,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eTransferDstOptimal
        );
        if (!transition_res) return std::unexpected(std::move(transition_res).error());
        copy_to_buffer_image(
            command_buffer,
            staging_buffer,
            image,
            width,
            height
        );
        transition_res = transition_image_layout(
            command_buffer, 
            image,
            vk::ImageLayout::eTransferDstOptimal,
            vk::ImageLayout::eShaderReadOnlyOptimal
        );
        if (!transition_res) return std::unexpected(std::move(transition_res).error());

        end_single_time_command(std::move(command_buffer));

        return {};
    }

    void CommandRecorder::end_single_time_command(vk::raii::CommandBuffer&& command_buffer) const {
        command_buffer.end();

        const auto& queue = m_vk_ctx.get().get_queue();
        const auto submit_info = vk::SubmitInfo()
            .setCommandBufferCount(1)
            .setPCommandBuffers(&*command_buffer);
        queue.submit(submit_info, nullptr);
        queue.waitIdle();
    }

    void CommandRecorder::copy_and_submit_immediate(
        vk::raii::Buffer& src_buffer, 
        vk::raii::Buffer& dst_buffer, 
        vk::DeviceSize size
    ) const {
        auto command_copy_buffer = begin_single_command_time_commands();
        command_copy_buffer.copyBuffer(*src_buffer, *dst_buffer, vk::BufferCopy(0, 0, size));
        end_single_time_command(std::move(command_copy_buffer));
    }


    const vk::raii::CommandBuffer& CommandRecorder::get_command_buffer(size_t index) const {
        return m_command_buffers[index];
    }

    void CommandRecorder::reset_command_buffer(size_t index) {
        m_command_buffers[index].reset();
    }

    const vk::raii::CommandPool& CommandRecorder::get_command_pool() const {
        return m_command_pool;
    }
}
