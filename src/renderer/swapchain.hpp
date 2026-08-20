#pragma once
#include <expected>
#include <vulkan/vulkan_raii.hpp>

namespace glimpse {
    namespace swapchain {
        class VulkanSwapchain {
        public:
            static std::expected<VulkanSwapchain, std::string> new_vk_swapchain(
                 
            );
            
        private: 
            VulkanSwapchain();
        };
    }
}
