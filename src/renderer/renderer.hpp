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
            struct VulkanCore {
                glimpse::renderer::VulkanContext vulkan_context;
                glimpse::renderer::VulkanSwapchain swapchain;
                glimpse::renderer::CommandRecorder command_recorder;
                glimpse::renderer::GraphicsPipeline pipeline;
            };
            struct VulkanSyncPrimitives {
                std::vector<vk::raii::Semaphore> present_complete_semaphores;
                std::vector<vk::raii::Semaphore> render_finished_semaphores;
                std::vector<vk::raii::Fence> in_flight_fences;
            };
            Renderer(
                VulkanCore core,  
                VulkanSyncPrimitives sync_primitives,
                Window window
            );

            void submit();
            std::expected<void, std::string> present(uint32_t image_idx);

            glimpse::renderer::VulkanContext m_vulkan_context;
            glimpse::renderer::VulkanSwapchain m_swapchain;
            glimpse::renderer::CommandRecorder m_command_recorder;
            glimpse::renderer::GraphicsPipeline m_pipeline;

            // Window
            Window m_window;

            // Sync
            std::vector<vk::raii::Semaphore> m_present_complete_semaphores;
            std::vector<vk::raii::Semaphore> m_render_finished_semaphores;
            std::vector<vk::raii::Fence> m_in_flight_fences;

            // Frame tracking
            size_t m_frame_index = 0; 
            static constexpr size_t m_max_frames_in_flight = 2;
        };
    }
}
