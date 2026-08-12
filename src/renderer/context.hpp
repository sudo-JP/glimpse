#pragma once
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
            VulkanContext(const ContextAppInfo& app_context, const ContextAppInfo &engine_context);
        private:
            vk::raii::Context m_context;
            vk::raii::Instance m_instance = nullptr;
        };
    }
}
