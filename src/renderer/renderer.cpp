#include "renderer.hpp"
#include "renderer/context.hpp"
#include "window/window.hpp"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <cstdlib>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace glimpse::renderer {
    
    std::expected<Renderer, std::string> Renderer::new_renderer() {
        constexpr int width = 1920, height = 1080;
        auto window_res = Window::new_window(width, height, std::move("glimpse of..."));
        if (!window_res) return std::unexpected(std::move(window_res).error());
        const auto window = std::move(window_res).value();

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
            app_info, engine_ctx, window, true
        );
        if (!vk_ctx_res) {
            return std::unexpected(std::move(vk_ctx_res).error()); 
        }

        auto vk_ctx = std::move(vk_ctx_res).value();

        const auto& device = vk_ctx.get_device();
        auto present_complete_semaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
        auto render_finished_semaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
        auto draw_fench = vk::raii::Fence(device, {vk::FenceCreateFlagBits::eSignaled});

        auto command_recorder = CommandRecorder::new_command_recorder(vk_ctx);
        std::vector<vk::raii::Semaphore> render_finished_semaphores;
        std::vector<vk::raii::Semaphore> present_complete_semaphores;
        std::vector<vk::raii::Fence> in_flight_fences;

        auto swapchain_res = VulkanSwapchain::new_vk_swapchain(vk_ctx, window);
        if (!swapchain_res) return std::unexpected(std::move(swapchain_res).error());
        const auto swapchain = std::move(swapchain_res).value();

        const auto& swapchain_images = swapchain.get_swapchain_images();
        for (size_t i = 0; i < swapchain_images.size(); i++) {
            render_finished_semaphores.emplace_back(device, vk::SemaphoreCreateInfo());
        }

        for (size_t i = 0; i < command_recorder.max_frames_in_flight; i++) {
            present_complete_semaphores.emplace_back(device, vk::SemaphoreCreateInfo());
            in_flight_fences.emplace_back(device, vk::FenceCreateInfo{vk::FenceCreateFlagBits::eSignaled});
        }

        return Renderer(); 
    }

    void Renderer::run() {
        auto window = m_window.get_window();
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            auto result = draw_frame();
            if (!result) std::abort();
        }
        const auto& device = m_vulkan_context.get_device();
        device.waitIdle();
    }

    std::expected<void, std::string> Renderer::draw_frame() {
        const auto& device = m_vulkan_context.get_device();
        auto fence_res = device.waitForFences(
        *m_draw_fence, vk::True, UINT64_MAX);
        if (fence_res != vk::Result::eSuccess) return std::unexpected("failed to wait for fence");
        device.resetFences(*m_draw_fence);

        auto [result, image_idx] = m_swapchain.acquire_next_image(m_present_complete_semaphore);
        auto err = m_command_recorder.record_command_buffer(
            image_idx, 
            m_swapchain, 
            m_pipeline   
        );
        if (!err) return std::unexpected(std::move(err).error());

        submit();
        present(image_idx);

        return {};
    }

    void Renderer::submit() {
        vk::PipelineStageFlags wait_destination_stage_mask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
        const auto& command_buffer = m_command_recorder.get_command_buffer();
        const auto submit_info = vk::SubmitInfo()
            .setWaitSemaphoreCount(1)
            .setPWaitSemaphores(&*m_present_complete_semaphore)
            .setPWaitDstStageMask(&wait_destination_stage_mask)
            .setCommandBufferCount(1)
            .setPCommandBuffers(&*command_buffer)
            .setSignalSemaphoreCount(1)
            .setPSignalSemaphores(&*m_render_finished_semaphore);
    }

    void Renderer::present(uint32_t image_idx) {
        const auto& swapchain = m_swapchain.get_swapchain();
        const auto present_info_khr = vk::PresentInfoKHR() 
            .setWaitSemaphoreCount(1)
            .setPWaitSemaphores(&*m_render_finished_semaphore)
            .setSwapchainCount(1)
            .setPSwapchains(&*swapchain)
            .setPImageIndices(&image_idx);
        
        const auto& queue = m_vulkan_context.get_queue();
        auto res = queue.presentKHR(present_info_khr);
    }
}
