#include "swapchain.hpp"
#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <expected>
#include <limits>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace glimpse::swapchain {
    namespace {
        std::expected<vk::SurfaceFormatKHR, std::string> choose_swap_surface_format(
            const std::vector<vk::SurfaceFormatKHR>& available_formats 
        ) {
            const auto format_it = std::ranges::find_if(
                available_formats,
                [](const auto& format) {
                    return format.format == vk::Format::eB8G8R8A8Srgb
                    && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
                }
            );
            if (format_it != available_formats.end()) return *format_it;
            else return available_formats[0];
        }

        std::expected<vk::PresentModeKHR, std::string> choose_swap_presentation_mode(
            const std::vector<vk::PresentModeKHR>& available_presentation_modes
        ) {
            const bool check_has_fifo = std::ranges::any_of(
                available_presentation_modes,
                [](const auto& present_mode) {
                    return present_mode == vk::PresentModeKHR::eFifo;
                }
            );

            if (!check_has_fifo) return std::unexpected("this is really bad if missing, must be default");
            const bool has_mailbox = std::ranges::any_of(
                available_presentation_modes, 
                [](const auto& present_mode) {
                    return present_mode == vk::PresentModeKHR::eMailbox;
                }
            );

            if (has_mailbox) return vk::PresentModeKHR::eMailbox;
            else return vk::PresentModeKHR::eFifo;
        }

        vk::Extent2D choose_swap_extent(
            const vk::SurfaceCapabilitiesKHR& capabilities,
            const glimpse::Window& window
        ) {
            if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                return capabilities.currentExtent;
            }

            int width, height;
            const auto& win = window.get_window();
            glfwGetFramebufferSize(win, &width, &height);

            return {
                std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
                std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
            };
        }
    }

    std::expected<VulkanSwapchain, std::string> VulkanSwapchain::new_vk_swapchain(
        const renderer::VulkanContext& context,
        const glimpse::Window& window
    ) {
        auto physical_device = context.get_physical_device();
        const auto& surface = context.get_surface();

        std::vector<vk::SurfaceFormatKHR> available_formats = physical_device.getSurfaceFormatsKHR(*surface);
        auto surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(*surface);
        std::vector<vk::PresentModeKHR> available_presentation_modes = physical_device.getSurfacePresentModesKHR(*surface);
        return std::unexpected("uh");
    }


    VulkanSwapchain::VulkanSwapchain() {
        
    }
}
