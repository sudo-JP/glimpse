#include "renderer.hpp"
#include "renderer/context.hpp"
#include "renderer/graphics_pipeline.hpp"
#include "window/window.hpp"
#include <GLFW/glfw3.h>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace glimpse::renderer {
    
    std::expected<Renderer, std::string> Renderer::new_renderer() {
        constexpr int width = 1920, height = 1080;
        auto window_res = Window::new_window(width, height, std::move("glimpse of..."));
        if (!window_res) return std::unexpected(std::move(window_res).error());
        auto window = std::move(window_res).value();

        glimpse::Version version {
            1, 0, 0
        };
        glimpse::ContextAppInfo app_info {
            "app", version
        };

        glimpse::ContextAppInfo engine_ctx {
            "engine", version
        };

        auto vk_ctx_res = glimpse::renderer::VulkanContext::new_vk_context(
            app_info, engine_ctx, window, false
        );
        if (!vk_ctx_res) {
            return std::unexpected(std::move(vk_ctx_res).error()); 
        }

        auto vk_ctx = std::move(vk_ctx_res).value();

        const auto& device = vk_ctx.get_device();

        auto command_recorder = CommandRecorder::new_command_recorder(vk_ctx, m_max_frames_in_flight);
        std::vector<vk::raii::Semaphore> render_finished_semaphores;
        std::vector<vk::raii::Semaphore> present_complete_semaphores;
        std::vector<vk::raii::Fence> in_flight_fences;

        auto swapchain_res = VulkanSwapchain::new_vk_swapchain(vk_ctx, window);
        if (!swapchain_res) return std::unexpected(std::move(swapchain_res).error());
        auto swapchain = std::move(swapchain_res).value();

        const auto& swapchain_images = swapchain.get_swapchain_images();
        for (size_t i = 0; i < swapchain_images.size(); i++) {
            render_finished_semaphores.emplace_back(device, vk::SemaphoreCreateInfo());
        }

        for (size_t i = 0; i < m_max_frames_in_flight; i++) {
            present_complete_semaphores.emplace_back(device, vk::SemaphoreCreateInfo());
            in_flight_fences.emplace_back(device, vk::FenceCreateInfo{vk::FenceCreateFlagBits::eSignaled});
        }


        auto pipeline_res = GraphicsPipeline::new_graphics_pipeline(
            {std::string(SHADER_DIR) + "/sandbox.spv"}, 
            vk_ctx, 
            swapchain
        );
        if (!pipeline_res) return std::unexpected(std::move(pipeline_res).error());
        auto pipeline = std::move(pipeline_res).value();

        VulkanCore core {
            std::move(vk_ctx),
            std::move(swapchain),
            std::move(command_recorder),
            std::move(pipeline)
        };
        VulkanSyncPrimitives sync_primitives {
            std::move(present_complete_semaphores),
            std::move(render_finished_semaphores),
            std::move(in_flight_fences)
        };
        return Renderer(
            std::move(core),
            std::move(sync_primitives),
            std::move(window)
        ); 
    }

    void Renderer::run() {
        auto window = m_window.get_window();
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            if (glfwGetKey(window, GLFW_KEY_Q)) {
                glfwSetWindowShouldClose(window, true);
            }
            auto result = draw_frame();
            if (!result) std::abort();
        }
        const auto& device = m_vulkan_context.get_device();
        device.waitIdle();
    }

    std::expected<void, std::string> Renderer::draw_frame() {
        const auto& device = m_vulkan_context.get_device();
        auto fence_res = device.waitForFences(
            *m_in_flight_fences[m_frame_index], vk::True, UINT64_MAX
        );
        if (fence_res != vk::Result::eSuccess) return std::unexpected("failed to wait for fence");

        auto [result, image_idx] = m_swapchain.acquire_next_image(m_present_complete_semaphores[m_frame_index]);
        if (result == vk::Result::eErrorOutOfDateKHR
        || result == vk::Result::eSuboptimalKHR
        || m_frame_buffer_resized) {
            m_frame_buffer_resized = false;
            m_swapchain.recreate_swapchain(m_vulkan_context);
            return {};
        } 
        if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
            if (result == vk::Result::eTimeout || result == vk::Result::eNotReady) {
                return std::unexpected("failed to acquire swap chain image");
            }
        }

        device.resetFences(*m_in_flight_fences[m_frame_index]);

        m_command_recorder.reset_command_buffer(m_frame_index);
        auto err = m_command_recorder.record_command_buffer(
            image_idx, 
            m_frame_index,
            m_swapchain, 
            m_pipeline   
        );
        if (!err) return std::unexpected(std::move(err).error());

        submit();
        present(image_idx);

        m_frame_index = (m_frame_index + 1) % m_max_frames_in_flight;
        return {};
    }

    void Renderer::submit() {
        vk::PipelineStageFlags wait_destination_stage_mask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
        const auto& command_buffer = m_command_recorder.get_command_buffer(m_frame_index);
        const auto submit_info = vk::SubmitInfo()
            .setWaitSemaphoreCount(1)
            .setPWaitSemaphores(&*m_present_complete_semaphores[m_frame_index])
            .setPWaitDstStageMask(&wait_destination_stage_mask)
            .setCommandBufferCount(1)
            .setPCommandBuffers(&*command_buffer)
            .setSignalSemaphoreCount(1)
            .setPSignalSemaphores(&*m_render_finished_semaphores[m_frame_index]);

        const auto& queue = m_vulkan_context.get_queue();
        queue.submit(submit_info, *m_in_flight_fences[m_frame_index]);
    }

    void Renderer::present(uint32_t image_idx) {
        const auto& swapchain = m_swapchain.get_swapchain();
        const auto present_info_khr = vk::PresentInfoKHR() 
            .setWaitSemaphoreCount(1)
            .setPWaitSemaphores(&*m_render_finished_semaphores[m_frame_index])
            .setSwapchainCount(1)
            .setPSwapchains(&*swapchain)
            .setPImageIndices(&image_idx);

        
        const auto& queue = m_vulkan_context.get_queue();
        auto result = queue.presentKHR(present_info_khr);

        if ((result == vk::Result::eSuboptimalKHR) 
        || (result == vk::Result::eErrorOutOfDateKHR)) {
            m_swapchain.recreate_swapchain(m_vulkan_context);
        } else {
            assert(result == vk::Result::eSuccess);
        }
    }

    Renderer::Renderer(
        VulkanCore core,  
        VulkanSyncPrimitives sync_primitives,
        Window window
    ) : m_vulkan_context(std::move(core.vulkan_context)),
    m_swapchain(std::move(core.swapchain)),
    m_command_recorder(std::move(core.command_recorder)),
    m_pipeline(std::move(core.pipeline)),
    m_present_complete_semaphores(std::move(sync_primitives.present_complete_semaphores)),
    m_render_finished_semaphores(std::move(sync_primitives.render_finished_semaphores)),
    m_in_flight_fences(std::move(sync_primitives.in_flight_fences)),
    m_window(std::move(window))
    {}
}
