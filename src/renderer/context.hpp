#pragma once
#include <vulkan/vulkan_raii.hpp>

namespace glimpse {
    class VulkanContext {
    public:
    private:
        vk::raii::Context context;
        vk::raii::Instance instance = nullptr;
    };
}
