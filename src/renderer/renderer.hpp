#pragma once

#include "renderer/command_recorder.hpp"
#include "renderer/context.hpp"
#include "renderer/entity.hpp"
#include "renderer/graphics_pipeline.hpp"
#include "renderer/swapchain.hpp"
#include "renderer/texture.hpp"
#include "renderer/types.hpp"
#include "window/window.hpp"

#include <cstdint>
#include <expected>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_raii.hpp>
namespace glimpse::renderer {
class Renderer {
  public:
    static std::expected<Renderer, std::string> new_renderer();
    void run();
    std::expected<void, std::string> draw_frame();

  private:
    struct RuntimeCore {
        std::unique_ptr<VulkanContext> vulkan_context;
        DescriptorAllocator descriptor_allocator;
        VulkanSwapchain swapchain;
        CommandRecorder command_recorder;
    };
    struct VulkanSyncPrimitives {
        std::vector<vk::raii::Semaphore> present_complete_semaphores;
        std::vector<vk::raii::Semaphore> render_finished_semaphores;
        std::vector<vk::raii::Fence> in_flight_fences;
    };

    struct SceneData {
        std::shared_ptr<const GraphicsPipeline> pipeline;
        std::unordered_map<std::string, std::shared_ptr<const Texture>> texture_map;
        std::unordered_map<std::string, std::shared_ptr<const Material>> material_map;
        std::vector<Entity<MVP>> entities;
    };

    Renderer(
        RuntimeCore core,
        VulkanSyncPrimitives sync_primitives,
        SceneData scene_data,
        Window window
    );

    void submit();
    std::expected<void, std::string> present(uint32_t image_idx);

    std::unique_ptr<VulkanContext> m_vulkan_context;
    DescriptorAllocator m_descriptor_allocator;
    VulkanSwapchain m_swapchain;
    CommandRecorder m_command_recorder;
    std::shared_ptr<const GraphicsPipeline> m_pipeline;
    std::unordered_map<std::string, std::shared_ptr<const Texture>> m_texture_map;
    std::unordered_map<std::string, std::shared_ptr<const Material>> m_material_map;

    // Window
    Window m_window;

    // Sync
    std::vector<vk::raii::Semaphore> m_present_complete_semaphores;
    std::vector<vk::raii::Semaphore> m_render_finished_semaphores;
    std::vector<vk::raii::Fence> m_in_flight_fences;

    std::vector<Entity<MVP>> m_entities;

    // Frame tracking
    size_t m_frame_index = 0;
    static constexpr size_t m_max_frames_in_flight = 2;
};
} // namespace glimpse::renderer
