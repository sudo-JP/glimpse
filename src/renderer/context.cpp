#include "context.hpp"
#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>
#include <expected>
#include <string_view>
#include <vulkan/vulkan_core.h>

namespace glimpse::renderer {

    std::expected<VulkanContext, std::string> VulkanContext::new_vk_context(
        const ContextAppInfo& app_context, const ContextAppInfo &engine_context
    ) {
        const char *app_name_c = app_context.name.c_str();
        const char *engine_name_c = engine_context.name.c_str();

        vk::ApplicationInfo app_info{
            app_name_c,
            VK_MAKE_VERSION(app_context.version.major, app_context.version.minor, app_context.version.patch),
            engine_name_c,
            VK_MAKE_VERSION(engine_context.version.major, engine_context.version.minor, engine_context.version.patch),
            vk::ApiVersion14
        };

        vk::raii::Context context;

        uint32_t glfw_extension_count = 0; 
        auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        
        auto extension_properties = context.enumerateInstanceExtensionProperties();

        for (auto i = 0; i < glfw_extension_count; i++) {
            auto check_glfw = [glfw_extension = glfw_extensions[i]](auto const& extension_property) {
                std::string_view extension_name = extension_property.extensionName;
                return extension_name.compare(glfw_extension) == 0;
            };
            if (std::ranges::none_of(extension_properties, check_glfw)) {
                return std::unexpected(
                    "required GLFW extension not supported: " 
                    + std::string(glfw_extensions[i])
                ); 
            }
        }

        vk::InstanceCreateInfo create_info{
            {},
            &app_info,
            glfw_extension_count,
            glfw_extensions,
        };
        vk::raii::Instance instance(context, create_info);
        vk::raii::PhysicalDevice phys_device = instance.enumeratePhysicalDevices().front();
        vk::raii::Device device(phys_device, vk::DeviceCreateInfo{});
        return VulkanContext(app_context, engine_context);    
    }
    VulkanContext::VulkanContext(const ContextAppInfo& app_context, const ContextAppInfo &engine_context) {

        //m_instance = vk::raii::Instance(m_context, create_info);
    }
}
