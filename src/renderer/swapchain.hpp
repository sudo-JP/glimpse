#pragma once
#include "renderer/context.hpp"
#include <expected>
#include <vector>
#include <vulkan/vulkan_raii.hpp>
#include "window/window.hpp"

namespace glimpse {
    namespace renderer {
        class VulkanSwapchain {
        public:
            static std::expected<VulkanSwapchain, std::string> new_vk_swapchain(
                const renderer::VulkanContext& context,
                const glimpse::Window& window
            );

            // getters
            const vk::Extent2D get_extent() const;
            
        private: 
            VulkanSwapchain(
                vk::raii::SwapchainKHR swapchain,
                std::vector<vk::Image> swapchain_images,
                vk::SurfaceFormatKHR swapchain_surface_format,
                vk::Extent2D swapchain_extent,
                std::vector<vk::raii::ImageView> swapchain_image_views
            );
            vk::raii::SwapchainKHR m_swapchain = nullptr;
            std::vector<vk::Image> m_swapchain_images;
            vk::SurfaceFormatKHR m_swapchain_surface_format;
            vk::Extent2D m_swapchain_extent;
            std::vector<vk::raii::ImageView> m_swapchain_image_views; 
        };
    }
}
