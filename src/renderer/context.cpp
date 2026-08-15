#include "context.hpp"
#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <expected>
#include <iostream>
#include <print>
#include <string_view>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_to_string.hpp>

namespace glimpse::renderer {
    namespace {
        std::expected<std::vector<const char*>, std::string> required_layers(const vk::raii::Context& context) {

            // validation layers
            const std::vector<char const*> validation_layers = {
                "VK_LAYER_KHRONOS_validation"
            };
            std::vector<char const*> required_layers; 
            required_layers.assign(validation_layers.begin(), validation_layers.end());

            auto layer_properties = context.enumerateInstanceLayerProperties();
            auto unsupported_layer_it = std::ranges::find_if(
                required_layers,
                [&layer_properties](auto const &required_layer) {
                    return std::ranges::none_of(layer_properties,
                        [required_layer](auto const& layer_property)  {
                        std::string_view layer_name = layer_property.layerName;
                        return layer_name.compare(required_layer) == 0;
                });
            });

            if (unsupported_layer_it != required_layers.end()) {
                return std::unexpected("required layer not supported: " + std::string(*unsupported_layer_it));
            }

            return required_layers;
        }

        std::expected<std::vector<const char*>, std::string> required_extensions(
                const vk::raii::Context& context, const bool debug) {
            uint32_t glfw_extension_count = 0; 
            auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
            
            std::vector extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
            if (debug) {
                extensions.push_back(vk::EXTDebugUtilsExtensionName);
            }
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
            return std::vector<const char*>(glfw_extensions, glfw_extensions + glfw_extension_count);
        }

        static VKAPI_ATTR vk::Bool32 VKAPI_CALL debug_callback(
            vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
            vk::DebugUtilsMessageTypeFlagsEXT type,
            const vk::DebugUtilsMessengerCallbackDataEXT *p_callback_data, 
            void *
        ) {
            if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
                || severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
                std::println(stderr, "validation layer: type {} msg: {}", vk::to_string(type), p_callback_data->pMessage);
            }
            return vk::False;
        }

        vk::raii::DebugUtilsMessengerEXT setup_debug_messenger(
            const vk::raii::Instance& instance
        ) {
            vk::DebugUtilsMessageSeverityFlagsEXT severity_flags(
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
            );
            vk::DebugUtilsMessageTypeFlagsEXT message_type_flags(
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
                | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
            );

            vk::DebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info_ext{
                vk::DebugUtilsMessengerCreateFlagsEXT{}, 
                severity_flags,
                message_type_flags,
                &debug_callback
            };

            return instance.createDebugUtilsMessengerEXT(debug_utils_messenger_create_info_ext);
        }
    }

    std::expected<VulkanContext, std::string> VulkanContext::new_vk_context(
        const ContextAppInfo& app_context, 
        const ContextAppInfo &engine_context,
        const bool debug_mode
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

        try {
            vk::raii::Context context;

            std::vector<const char*> layers; 
            std::vector<const char*> glfw_extensions; 

            auto extension_result = required_extensions(context, debug_mode);
            if (extension_result) {
                glfw_extensions = std::move(*extension_result);
            } else {
                return std::unexpected(extension_result.error());
            }
            if (debug_mode) {
                auto validation_result = required_layers(context);
                if (validation_result) {
                    layers = std::move(*validation_result);
                } else {
                    return std::unexpected(validation_result.error());
                }

            }

            vk::InstanceCreateInfo create_info{
                {},
                &app_info,
                static_cast<uint32_t>(layers.size()),
                layers.data(),
                static_cast<uint32_t>(glfw_extensions.size()),
                glfw_extensions.data(),
            };

            vk::raii::Instance instance(context, create_info);
            vk::raii::PhysicalDevice phys_device = instance.enumeratePhysicalDevices().front();
            vk::raii::Device device(phys_device, vk::DeviceCreateInfo{});
            std::optional<vk::raii::DebugUtilsMessengerEXT> debug_messenger = std::nullopt; 

            if (debug_mode) {
                debug_messenger = setup_debug_messenger(instance);
            }

            return VulkanContext(
                std::move(context), 
                std::move(instance), 
                std::move(phys_device), 
                std::move(device),
                std::move(debug_messenger)
            );    
        } catch (const vk::SystemError& err) {
            return std::unexpected(err.what());
        } 
        return std::unexpected("failed to initialize vulkan context");
    }

    VulkanContext::VulkanContext(
        vk::raii::Context context, 
        vk::raii::Instance instance, 
        vk::raii::PhysicalDevice physical_device, 
        vk::raii::Device device,
        std::optional<vk::raii::DebugUtilsMessengerEXT> debug_messenger
    ) : m_context(std::move(context)),
    m_instance(std::move(instance)),
    m_physical_device(std::move(physical_device)),
    m_device(std::move(device)),
    m_debug_messenger(std::move(debug_messenger))
    {}
}
