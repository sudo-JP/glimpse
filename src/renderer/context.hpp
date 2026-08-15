#pragma once
#include <expected>
#include <string>
#include <vulkan/vulkan_raii.hpp>

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
                const bool debug_mode
            );
        private:
            VulkanContext(
                vk::raii::Context context, 
                vk::raii::Instance instance, 
                vk::raii::PhysicalDevice physical_device, 
                vk::raii::Device device 
            );
            vk::raii::Context m_context;
            vk::raii::Instance m_instance = nullptr;
            vk::raii::PhysicalDevice m_physical_device; 
            vk::raii::Device m_device; 
        };
    }
}
