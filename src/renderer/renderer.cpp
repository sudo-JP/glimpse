#include "renderer.hpp"
#include "renderer/context.hpp"
#include "window/window.hpp"
#include <GLFW/glfw3.h>
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

        return Renderer(); 
    }

    void Renderer::run() {
        //auto window = 
        //while (!glfwWindowShouldClose(GLFWwindow *window))
    }

    void Renderer::draw_frame() {
        //auto fence_res = 
    }
}
