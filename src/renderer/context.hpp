#pragma once
#include "window/window.hpp"
#include <expected>
#include <string>
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>

namespace glimpse {
    struct Version {
        int major; 
        int minor;
        int patch;
    };

    struct ContextAppInfo {
        const std::string name; 
        const Version version;
    };

    namespace renderer {

        /*
         * Hold the context and instance of vulkan
         * */
        class VulkanContext {
        public:
            static std::expected<VulkanContext, std::string> new_vk_context(
                const ContextAppInfo& app_context, 
                const ContextAppInfo &engine_context,
                const Window& window, 
                const bool debug_mode
            );
            
            const vk::raii::PhysicalDevice& get_physical_device() const;
            const vk::raii::SurfaceKHR& get_surface() const;

        private:
            struct DeviceResources {
                vk::raii::Device device;
                vk::raii::Queue graphics_queue;
                vk::raii::PhysicalDevice physical_device;
            };

            struct ContextInstance {
                vk::raii::Context context; 
                vk::raii::Instance instance;
                vk::raii::SurfaceKHR surface; 
            };

            struct VKDebug {
                vk::raii::DebugUtilsMessengerEXT debug_messenger;
            };

            VulkanContext(
                ContextInstance context_instance, 
                DeviceResources device_resources, 
                std::optional<vk::raii::DebugUtilsMessengerEXT>debug_messenger
            );
            vk::raii::Context m_context;
            vk::raii::Instance m_instance = nullptr;
            vk::raii::PhysicalDevice m_physical_device; 
            vk::raii::Device m_device; 
            vk::raii::Queue m_graphics_queue = nullptr; 
            std::optional<vk::raii::DebugUtilsMessengerEXT> m_debug_messenger;
            vk::raii::SurfaceKHR m_surface = nullptr;
        };
    }
}
