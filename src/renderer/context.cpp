// Misc
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "context.hpp"

// std
#include <utility>
#include <vector>
#include <algorithm>
#include <expected>
#include <print>
#include <string_view>

// Vukan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_to_string.hpp>

namespace glimpse::renderer {
    namespace {
        // Validation layers
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

        // Extension layers
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
            return extensions;
        }

        // Debug call
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

        // Debug messenger
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

        // Helper for picking out physical device
        // wtf...
        bool is_device_suitable(const vk::raii::PhysicalDevice& physical_device) {
            auto device_properties = physical_device.getProperties();
            auto device_features = physical_device.getFeatures();

            // Support 1.3 api
            bool supports_vulkan1_3 = device_properties.apiVersion >= vk::ApiVersion13;

            // Queue family support graphics operations
            auto queue_families = physical_device.getQueueFamilyProperties();
            bool support_graphics = std::ranges::any_of(
                queue_families, [](auto const &qfq) { return !!(qfq.queueFlags & vk::QueueFlagBits::eGraphics); }
            );
            std::vector<const char*> required_device_extension = {vk::KHRSwapchainExtensionName};

            auto available_device_extensions = physical_device.enumerateDeviceExtensionProperties();
            bool supports_all_required_extensions = 
                std::ranges::all_of(required_device_extension,
                    [&available_device_extensions](auto const& required_device_extension) {
                    return std::ranges::any_of(available_device_extensions,
                        [required_device_extension](auto const& available_device_extension) {
                            std::string_view extension_name = available_device_extension.extensionName;
                            return extension_name.compare(required_device_extension) == 0; 
                        }
                    );
                }
            );

            auto features = physical_device.template getFeatures2<
                vk::PhysicalDeviceFeatures2,
                vk::PhysicalDeviceVulkan11Features,
                vk::PhysicalDeviceVulkan13Features,
                vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

            bool supports_required_features = 
                features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters
                && features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering
                && features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

            if (device_properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu
                && device_features.geometryShader) return true; 

            return supports_vulkan1_3 
            && support_graphics
            && supports_all_required_extensions
            && supports_required_features;
        }

        // Pick out physical device
        std::expected<vk::raii::PhysicalDevice, std::string> pick_physical_device(
            const vk::raii::Instance& instance
        ) {
            auto physical_devices = instance.enumeratePhysicalDevices();
            if (physical_devices.empty()) {
                return std::unexpected("failed to find GPUs with vulkan support");
            }
            for (const auto& pd: physical_devices) {
                if (is_device_suitable(pd)) return std::move(pd);
            }
            return std::unexpected("failed to find a suitable discrete GPU");
        }

        std::expected<std::pair<vk::raii::Device, vk::raii::Queue>, std::string> 
            create_logical_device(
            const vk::raii::PhysicalDevice& physical_device,
            const vk::SurfaceKHR& surface
        ) {
            // Queue stuff
            std::vector<vk::QueueFamilyProperties> queue_family_properties = physical_device.getQueueFamilyProperties();

            std::optional<uint32_t> queue_idx;

            for (uint32_t qfq_idx = 0; qfq_idx < queue_family_properties.size(); qfq_idx++) {
                auto queue_flag = queue_family_properties[qfq_idx].queueFlags;
                if ((queue_flag & vk::QueueFlagBits::eGraphics)
                && physical_device.getSurfaceSupportKHR(qfq_idx, surface)) {
                    queue_idx= qfq_idx;
                    break;
                }

            }

            if (!queue_idx.has_value()) return std::unexpected("could not find queue for graphics and present");
            auto graphics_queue_idx = queue_idx.value();

            float queue_priority = 1.0f;
            int queue_count = 1; 
            vk::DeviceQueueCreateInfo device_queue_create_info(
                vk::DeviceQueueCreateFlags(),
                graphics_queue_idx,
                queue_count,
                &queue_priority
            );

            vk::PhysicalDeviceFeatures device_features; 

            vk::StructureChain<
                vk::PhysicalDeviceFeatures2,
                vk::PhysicalDeviceVulkan11Features,
                vk::PhysicalDeviceVulkan13Features,
                vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
            > feature_chain = {
                // vk::PhysicalDeviceFeatures2
                {}, 
                // Enable shader draw params from 1.1
                {true},
                // Enable dynamic rendering from vk 1.3
                {true},
                // Enable extended dyanmic state from extension
                {true},
            };
            std::vector<const char*> required_device_extension = {
                vk::KHRSwapchainExtensionName
            };

            int queue_create_info_count = 1; 
            vk::DeviceCreateInfo device_create_info = vk::DeviceCreateInfo()
                .setPNext(&feature_chain.get<vk::PhysicalDeviceFeatures2>())
                .setQueueCreateInfoCount(queue_create_info_count)
                .setPQueueCreateInfos(&device_queue_create_info)
                .setEnabledExtensionCount(static_cast<uint32_t>(required_device_extension.size()))
                .setPpEnabledExtensionNames(required_device_extension.data());

            
            vk::raii::Device device(physical_device, device_create_info);
            vk::raii::Queue graphics_queue = vk::raii::Queue(device, graphics_queue_idx, 0);
            return std::pair{std::move(device), std::move(graphics_queue)};
        }
    }

    std::expected<VulkanContext, std::string> VulkanContext::new_vk_context(
        const ContextAppInfo& app_context, 
        const ContextAppInfo &engine_context,
        const Window& window, 
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
            if (!extension_result) return std::unexpected(extension_result.error());

            glfw_extensions = std::move(extension_result).value();

            if (debug_mode) {
                auto validation_result = required_layers(context);
                if (!validation_result) return std::unexpected(validation_result.error());
                
                layers = std::move(validation_result).value();
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

            // Surface setup
            vk::raii::SurfaceKHR surface = nullptr; 
            VkSurfaceKHR _surface; 
            if (glfwCreateWindowSurface(*instance, window.get_window(), nullptr, &_surface) != 0) {
                return std::unexpected("failed to create window surface");
            }
            surface = vk::raii::SurfaceKHR(instance, _surface);

            ContextInstance context_instance {
                std::move(context), 
                std::move(instance)
            };

            auto physical_device_result = pick_physical_device(instance);
            vk::raii::PhysicalDevice phys_device = nullptr;
            if (!physical_device_result) return std::unexpected(physical_device_result.error());

            phys_device = std::move(physical_device_result).value();
            std::println("Selected GPU: {}", phys_device.getProperties().deviceName.data());

            auto device_result = create_logical_device(phys_device, surface);
            if (!device_result) return std::unexpected(device_result.error());

            auto [device, graphics_queue] = std::move(device_result).value();
            DeviceResources device_resources {
                std::move(device),
                std::move(graphics_queue),
                std::move(phys_device), 
            };

            std::optional<vk::raii::DebugUtilsMessengerEXT> debug_messenger = std::nullopt; 

            if (debug_mode) {
                debug_messenger = setup_debug_messenger(instance);
            }

            return VulkanContext(
                std::move(context_instance),
                std::move(device_resources),
                std::move(debug_messenger)
            );    
        } catch (const vk::SystemError& err) {
            return std::unexpected(err.what());
        } 
        return std::unexpected("failed to initialize vulkan context");
    }

    VulkanContext::VulkanContext(
        ContextInstance context_instance, 
        DeviceResources device_resources, 
        std::optional<vk::raii::DebugUtilsMessengerEXT>debug_messenger) 
    // The actual init
    : m_context(std::move(context_instance.context)),
    m_instance(std::move(context_instance.instance)),
    m_physical_device(std::move(device_resources.physical_device)),
    m_device(std::move(device_resources.device)),
    m_graphics_queue(std::move(device_resources.graphics_queue)),
    m_debug_messenger(std::move(debug_messenger))
    {}
}
