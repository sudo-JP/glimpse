#include "swapchain.hpp"
#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>
#include <expected>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace glimpse::swapchain {
    namespace {
        std::expected<vk::SurfaceFormatKHR, std::string> choose_swap_surface_format(
            std::vector<vk::SurfaceFormatKHR> const& available_formats 
        ) {
            if (available_formats.empty()) return std::unexpected("no available formats");
            return available_formats[0];
        }

    }

    std::expected<VulkanSwapchain, std::string> VulkanSwapchain::new_vk_swapchain(
        const renderer::VulkanContext& context) 
    {
        auto physical_device = context.get_physical_device();
        const auto& surface = context.get_surface();
        auto surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(*surface);
        std::vector<vk::SurfaceFormatKHR> available_formats = physical_device.getSurfaceFormatsKHR(*surface);
        return std::unexpected("uh");
    }


    VulkanSwapchain::VulkanSwapchain() {
        
    }
}
