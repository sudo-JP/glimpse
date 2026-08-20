#include "swapchain.hpp"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

namespace glimpse::swapchain {
    std::expected<VulkanSwapchain, std::string> VulkanSwapchain::new_vk_swapchain() {
        return std::unexpected("uh");
    }


    VulkanSwapchain::VulkanSwapchain() {
        
    }
}
