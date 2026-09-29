#pragma once

#include "renderer/command_recorder.hpp"
#include "renderer/context.hpp"
#include "renderer/graphics_pipeline.hpp"
#include "renderer/swapchain.hpp"
#include "renderer/texture.hpp"
#include "renderer/types.hpp"
#include "renderer/uniform_buffer.hpp"
#include "window/window.hpp"

#include <cstdint>
#include <expected>
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan_raii.hpp>
namespace glimpse::renderer {
class Renderer {
  public:
    static std::expected<Renderer, std::string> new_renderer();
    void run();
    std::expected<void, std::string> draw_frame();

  private:
    struct VulkanCore {
        std::unique_ptr<VulkanContext> vulkan_context;
        VulkanSwapchain swapchain;
        CommandRecorder command_recorder;
        GraphicsPipeline pipeline;
    };
    struct VulkanSyncPrimitives {
        std::vector<vk::raii::Semaphore> present_complete_semaphores;
        std::vector<vk::raii::Semaphore> render_finished_semaphores;
        std::vector<vk::raii::Fence> in_flight_fences;
    };
    Renderer(
        VulkanCore core,
        VulkanSyncPrimitives sync_primitives,
        UniformBuffer<MVP> uniform_buffer,
        Texture texture,
        Window window
    );

    void submit();
    std::expected<void, std::string> present(uint32_t image_idx);

    std::unique_ptr<VulkanContext> m_vulkan_context;
    VulkanSwapchain m_swapchain;
    CommandRecorder m_command_recorder;
    GraphicsPipeline m_pipeline;

    // Window
    Window m_window;

    // Sync
    std::vector<vk::raii::Semaphore> m_present_complete_semaphores;
    std::vector<vk::raii::Semaphore> m_render_finished_semaphores;
    std::vector<vk::raii::Fence> m_in_flight_fences;

    UniformBuffer<MVP> m_uniform_buffer;
    Texture m_texture;

    // Frame tracking
    size_t m_frame_index = 0;
    static constexpr size_t m_max_frames_in_flight = 2;
};
} // namespace glimpse::renderer
