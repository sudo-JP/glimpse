#include "swapchain.hpp"
#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstdint>
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

        uint32_t choose_swap_min_image_count(const vk::SurfaceCapabilitiesKHR& capabilities) {
            auto min_image_count = std::max(3u, capabilities.minImageCount);
            if ((0 < capabilities.maxImageCount) && (capabilities.maxImageCount < min_image_count)) 
            {
                min_image_count = capabilities.maxImageCount;
            }
            return min_image_count;
        }
    }

    std::expected<VulkanSwapchain, std::string> VulkanSwapchain::new_vk_swapchain(
        const renderer::VulkanContext& context,
        const glimpse::Window& window
    ) {
        auto physical_device = context.get_physical_device();
        const auto& surface = context.get_surface();

        // Get from physical device
        auto available_formats = physical_device.getSurfaceFormatsKHR(*surface);
        auto surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(*surface);
        auto available_presentation_modes = physical_device.getSurfacePresentModesKHR(*surface);

        // Choosing
        auto swapchain_extent = choose_swap_extent(surface_capabilities, window);
        auto min_image_count = choose_swap_min_image_count(surface_capabilities);

        // Err handlings 
        auto swapchain_surface_format_res = choose_swap_surface_format(available_formats);
        if (!swapchain_surface_format_res) return std::unexpected(swapchain_surface_format_res.error());
        auto swapchain_surface_format = std::move(swapchain_surface_format_res).value();

        auto swapchain_present_mode_res = choose_swap_presentation_mode(available_presentation_modes);
        if (!swapchain_present_mode_res) return std::unexpected(swapchain_present_mode_res.error());
        auto swapchain_present_mode = std::move(swapchain_present_mode_res).value();

        vk::SwapchainCreateInfoKHR swapchain_create_info = vk::SwapchainCreateInfoKHR()
            .setSurface(*surface)
            .setMinImageCount(min_image_count)
            .setImageFormat(swapchain_surface_format.format)
            .setImageColorSpace(swapchain_surface_format.colorSpace)
            .setImageExtent(swapchain_extent)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setImageSharingMode(vk::SharingMode::eExclusive)
            .setPreTransform(surface_capabilities.currentTransform)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setPresentMode(swapchain_present_mode)
            .setClipped(true);

        return std::unexpected("uh");
    }


    VulkanSwapchain::VulkanSwapchain(
        vk::raii::SwapchainKHR swapchain,
        std::vector<vk::Image> swapchain_images,
        vk::SurfaceFormatKHR swapchain_surface_format,
        vk::Extent2D swapchain_extent
    ) : m_swapchain(std::move(swapchain)), 
    m_swapchain_images(std::move(swapchain_images)),
    m_swapchain_surface_format(std::move(swapchain_surface_format)),
    m_swapchain_extent(std::move(swapchain_extent)) 
    {}
}
