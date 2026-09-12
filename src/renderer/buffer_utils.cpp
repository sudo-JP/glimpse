#include "buffer_utils.hpp"
#include <cstdint>

namespace glimpse::renderer {
        std::expected<uint32_t, std::string> find_memory_type(
            uint32_t type_filter,
            vk::MemoryPropertyFlags properties,
            const glimpse::renderer::VulkanContext& context
        ) {
            const auto& phys_device = context.get_physical_device();
            auto memory_properties = phys_device.getMemoryProperties();

            for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
                if ((type_filter & (1 << i))
                && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) return i;
            }
            return std::unexpected("failed to find suitable memory type");
        }

        std::expected<
            std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>, 
            std::string> create_buffer(
            vk::DeviceSize size, 
            vk::BufferUsageFlags usage,
            vk::MemoryPropertyFlags properties,
            const glimpse::renderer::VulkanContext& context
        ) {
            auto buffer_info = vk::BufferCreateInfo()
                .setSize(size)
                .setUsage(usage)
                .setSharingMode(vk::SharingMode::eExclusive);

            const auto& device = context.get_device();

            auto vertex_buffer = vk::raii::Buffer(device, buffer_info);

            const auto memory_requirements = vertex_buffer.getMemoryRequirements();
            auto appropriate_memory = find_memory_type(
                memory_requirements.memoryTypeBits, 
                properties,
                context
            );
            if (!appropriate_memory) return std::unexpected(std::move(appropriate_memory).error());
            auto memory_type_index = std::move(appropriate_memory).value();

            auto memory_allocate_info = vk::MemoryAllocateInfo()
                .setAllocationSize(memory_requirements.size)
                .setMemoryTypeIndex(memory_type_index);

            auto vertex_buffer_memory = vk::raii::DeviceMemory(device, memory_allocate_info);

            // Filling the vertex buffer 
            auto offset = vk::DeviceSize{0};
            vertex_buffer.bindMemory(*vertex_buffer_memory, offset);
            
            return std::pair{
                std::move(vertex_buffer),
                std::move(vertex_buffer_memory)
            };
        }
}
