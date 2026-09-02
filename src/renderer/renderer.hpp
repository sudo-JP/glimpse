#pragma once

#include "renderer/context.hpp"
#include "renderer/graphics_pipeline.hpp"
#include "renderer/swapchain.hpp"
#include "renderer/command_recorder.hpp"
#include "window/window.hpp"

#include <cstdint>
#include <expected>
#include <string>
#include <vector>
#include <vulkan/vulkan_raii.hpp>
namespace glimpse {
    namespace renderer {
        class Renderer {
        public:
            static std::expected<Renderer, std::string> new_renderer();
            void run();
            std::expected<void, std::string> draw_frame();
        private:
            Renderer();

            void submit();
            void present(uint32_t image_idx);

            glimpse::renderer::VulkanContext m_vulkan_context;
            glimpse::renderer::VulkanSwapchain m_swapchain;
            glimpse::renderer::CommandRecorder m_command_recorder;
            glimpse::renderer::GraphicsPipeline m_pipeline;
            Window m_window;
            std::vector<vk::raii::Semaphore> m_present_complete_semaphores;
            std::vector<vk::raii::Semaphore> m_render_finished_semaphores;
            std::vector<vk::raii::Fence> m_draw_fences;
            uint32_t m_frame_index = 0; 
        };
    }
}
