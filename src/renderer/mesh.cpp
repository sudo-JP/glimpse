#include "mesh.hpp"
#include <cstdint>
#include <expected>
#include <string>
#include <utility>
#include <vulkan/vulkan_raii.hpp>

namespace glimpse::renderer {
    namespace {
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
    }
     
    std::expected<Mesh, std::string> Mesh::new_mesh(
        const std::vector<glimpse::renderer::VulkanVertex>& vertices,
        const glimpse::renderer::VulkanContext& context
    ) {
        auto buffer_info = vk::BufferCreateInfo()
            .setSize(sizeof(vertices[0]) * vertices.size())
            .setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
            .setSharingMode(vk::SharingMode::eExclusive);

        const auto& device = context.get_device();

        auto vertex_buffer = vk::raii::Buffer(device, buffer_info);

        const auto memory_requirements = vertex_buffer.getMemoryRequirements();
        auto appropriate_memory = find_memory_type(
            memory_requirements.memoryTypeBits, 
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, 
            context
        );
        if (!appropriate_memory) return std::unexpected(std::move(appropriate_memory).error());
        auto memory_type_index = std::move(appropriate_memory).value();

        auto memory_allocate_info = vk::MemoryAllocateInfo()
            .setAllocationSize(memory_requirements.size)
            .setMemoryTypeIndex(memory_type_index);

        return Mesh();
    }
}
