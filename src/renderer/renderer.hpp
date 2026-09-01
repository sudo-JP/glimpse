#pragma once

#include <expected>
#include <string>
#include <vulkan/vulkan_raii.hpp>
namespace glimpse {
    namespace renderer {
        class Renderer {
        public:
            static std::expected<Renderer, std::string> new_renderer();
            void run();
            void draw_frame();
        private:
            Renderer();
            vk::raii::Semaphore m_present_complete_semaphore = nullptr;
            vk::raii::Semaphore m_render_finished_semaphore = nullptr;
            vk::raii::Fence m_draw_fence = nullptr;
        };
    }
}
