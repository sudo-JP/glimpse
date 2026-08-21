#pragma once
#include "renderer/context.hpp"
#include <expected>
#include <vulkan/vulkan_raii.hpp>
#include "window/window.hpp"

namespace glimpse {
    namespace swapchain {
        class VulkanSwapchain {
        public:
            static std::expected<VulkanSwapchain, std::string> new_vk_swapchain(
                const renderer::VulkanContext& context,
                const glimpse::Window& window
            );
            
        private: 
            VulkanSwapchain();
        };
    }
}
