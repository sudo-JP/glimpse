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

namespace glimpse::renderer {
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

        std::vector<vk::raii::ImageView> create_image_views(
            const vk::SurfaceFormatKHR& swapchain_surface_format,
            const vk::raii::Device& device, 
            const std::vector<vk::Image>& swapchain_images
        ) {
            std::vector<vk::raii::ImageView> swapchain_image_views;
            auto image_view_create_info = vk::ImageViewCreateInfo()
                .setViewType(vk::ImageViewType::e2D)
                .setFormat(swapchain_surface_format.format)
                .setSubresourceRange({
                    vk::ImageAspectFlagBits::eColor,
                    0,
                    1,
                    0,
                    1
                });

            image_view_create_info.components = {
                vk::ComponentSwizzle::eIdentity, 
                vk::ComponentSwizzle::eIdentity, 
                vk::ComponentSwizzle::eIdentity, 
                vk::ComponentSwizzle::eIdentity
            };

            for (const auto& image: swapchain_images) {
                image_view_create_info.image = image;
                swapchain_image_views.emplace_back(device, image_view_create_info);
            }

            return swapchain_image_views;
        }

        struct SwapchainResources {
            vk::Extent2D swapchain_extent;
            vk::SurfaceFormatKHR swapchain_surface_format;
            vk::raii::SwapchainKHR swapchain = nullptr;
            std::vector<vk::Image> swapchain_images;
        };

        std::expected<SwapchainResources, std::string> create_swapchain(
            const renderer::VulkanContext& context,
            const glimpse::Window& window
        ) {
            const auto& physical_device = context.get_physical_device();
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
            if (!swapchain_surface_format_res) return std::unexpected(std::move(swapchain_surface_format_res).error());
            auto swapchain_surface_format = std::move(swapchain_surface_format_res).value();

            auto swapchain_present_mode_res = choose_swap_presentation_mode(available_presentation_modes);
            if (!swapchain_present_mode_res) return std::unexpected(std::move(swapchain_present_mode_res).error());
            auto swapchain_present_mode = std::move(swapchain_present_mode_res).value();

            auto swapchain_create_info = vk::SwapchainCreateInfoKHR()
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

            const auto& device = context.get_device();
            auto swapchain = vk::raii::SwapchainKHR(device, swapchain_create_info);
            const auto swapchain_images = swapchain.getImages();

            SwapchainResources resources {
                std::move(swapchain_extent),
                std::move(swapchain_surface_format),
                std::move(swapchain),
                std::move(swapchain_images)
            };
            return resources;
        }

    } // End helper namespace

    std::expected<VulkanSwapchain, std::string> VulkanSwapchain::new_vk_swapchain(
        const renderer::VulkanContext& context,
        const glimpse::Window& window
    ) {
        auto creation_res = create_swapchain(context, window);
        if (!creation_res) return std::unexpected(std::move(creation_res).error());
        auto resources = std::move(creation_res).value();

        // all hail destructuring, this is pretty cool
        auto&& [
            swapchain_extent, 
            swapchain_surface_format, 
            swapchain, 
            swapchain_images
        ] = std::move(resources);

        const auto& device = context.get_device();
        auto image_views = create_image_views(swapchain_surface_format, device, swapchain_images);

        return VulkanSwapchain(
            std::move(swapchain),
            std::move(swapchain_images),
            std::move(swapchain_surface_format),
            std::move(swapchain_extent),
            std::move(image_views)
        );
    }


    VulkanSwapchain::VulkanSwapchain(
        vk::raii::SwapchainKHR swapchain,
        std::vector<vk::Image> swapchain_images,
        vk::SurfaceFormatKHR swapchain_surface_format,
        vk::Extent2D swapchain_extent,
        std::vector<vk::raii::ImageView> swapchain_image_views
    ) : m_swapchain(std::move(swapchain)), 
    m_swapchain_images(std::move(swapchain_images)),
    m_swapchain_surface_format(std::move(swapchain_surface_format)),
    m_swapchain_extent(std::move(swapchain_extent)),
    m_swapchain_image_views(std::move(swapchain_image_views))
    {}


    std::expected<void, std::string> VulkanSwapchain::recreate_swapchain(
        const glimpse::renderer::VulkanContext& context, 
        const glimpse::Window& window
    ) {
        int width = 0, height = 0;
        const auto& win = window.get_window();
        glfwGetFramebufferSize(win, &width, &height);
        while ((width == 0 || height == 0) && !glfwWindowShouldClose(win)) {
            glfwGetFramebufferSize(win, &width, &height);
            glfwWaitEvents();
        }
        if (glfwWindowShouldClose(win)) return {};

        const auto& device = context.get_device();
        device.waitIdle();

        cleanup_swapchain();

        auto creation_res = create_swapchain(context, window);
        if (!creation_res) return std::unexpected(std::move(creation_res).error());
        auto resources = std::move(creation_res).value();

        auto&& [
            swapchain_extent, 
            swapchain_surface_format, 
            swapchain, 
            swapchain_images
        ] = std::move(resources);

        auto image_views = create_image_views(swapchain_surface_format, device, swapchain_images);
        m_swapchain_extent = std::move(swapchain_extent);
        m_swapchain_surface_format = std::move(swapchain_surface_format);
        m_swapchain = std::move(swapchain);
        m_swapchain_images = std::move(swapchain_images);
        m_swapchain_image_views = std::move(image_views);

        return {};
    }

    void VulkanSwapchain::cleanup_swapchain() {
        m_swapchain_image_views.clear();
        m_swapchain = nullptr;
    }

    // Getters
    vk::ResultValue<uint32_t> VulkanSwapchain::acquire_next_image(const vk::raii::Semaphore& semaphore) {
        return m_swapchain.acquireNextImage(UINT64_MAX, *semaphore, nullptr);
    }

    const vk::Extent2D VulkanSwapchain::get_extent() const {
        return m_swapchain_extent;
    }

    const vk::SurfaceFormatKHR VulkanSwapchain::get_format() const {
        return m_swapchain_surface_format;
    }

    const std::expected<vk::Image, std::string> VulkanSwapchain::get_image(uint32_t index) const {
        if (index < 0 || index >= m_swapchain_images.size()) return std::unexpected("invalid image index");

        return m_swapchain_images[index];
    }

    const std::expected<vk::ImageView, std::string> VulkanSwapchain::get_image_view(uint32_t index) const {
        if (index < 0 || index >= m_swapchain_images.size()) return std::unexpected("invalid image view index");

        return m_swapchain_image_views[index];
    }

    const vk::raii::SwapchainKHR& VulkanSwapchain::get_swapchain() const {
        return m_swapchain;
    }

    const std::vector<vk::Image>& VulkanSwapchain::get_swapchain_images() const {
        return m_swapchain_images;
    }

    VulkanSwapchain::~VulkanSwapchain() { cleanup_swapchain(); }
}
