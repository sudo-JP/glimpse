#include "context.hpp"
#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

namespace glimpse::renderer {
    VulkanContext::VulkanContext(const ContextAppInfo& app_context, const ContextAppInfo &engine_context) {
        const char *app_name_c = app_context.name.c_str();
        const char *engine_name_c = engine_context.name.c_str();

        vk::ApplicationInfo app_info{
            app_name_c,
            VK_MAKE_VERSION(app_context.version.major, app_context.version.minor, app_context.version.patch),
            engine_name_c,
            VK_MAKE_VERSION(engine_context.version.major, engine_context.version.minor, engine_context.version.patch),
            vk::ApiVersion14
        };

        vk::InstanceCreateInfo create_info{
            {},
            &app_info
        };

        uint32_t glfw_extension_count = 0; 
        auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        
        auto exntesion_properties = m_context.enumerateInstanceExtensionProperties();

        /*for (auto i = 0; i < glfw_extension_count; i++) {
            if (std::ranges::none_of(exntesion_properties,))
        }*/
        m_instance = nullptr; 
    }
}
