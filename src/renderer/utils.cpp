#include "utils.hpp"
#include "types.hpp"
#include <cstdint>

// Bunch of shared helpers
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

    template <typename T>
    std::expected<std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>,
        std::string> create_staging_buffer(
        const glimpse::renderer::VulkanContext& context,
        const std::vector<T> data,
        const vk::DeviceSize size
    ) {
        auto staging_buffer_res = create_buffer(
            size, 
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible 
            | vk::MemoryPropertyFlagBits::eHostCoherent,
            context
        );
        if (!staging_buffer_res) return std::unexpected(std::move(staging_buffer_res).error());
        auto [staging_buffer, staging_buffer_memory] = std::move(staging_buffer_res).value();


        auto offset = vk::DeviceSize{0};
        auto data_staging = static_cast<T *>(staging_buffer_memory.mapMemory(offset, size));
        std::copy(data.begin(), data.end(), data_staging);
        staging_buffer_memory.unmapMemory();

        return std::pair{
            std::move(staging_buffer),
            std::move(staging_buffer_memory)
        };
    }

    vk::raii::ImageView create_image_view(
        const vk::Image& image, 
        vk::Format format,
        const vk::raii::Device& device
    ) {
        auto sub_resource_range = vk::ImageSubresourceRange()
            .setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel(0)
            .setLevelCount(0)
            .setBaseArrayLayer(0)
            .setLayerCount(1);
        auto view_info = vk::ImageViewCreateInfo()
            .setImage(image)
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(format)
            .setSubresourceRange(sub_resource_range);
        return vk::raii::ImageView(device, view_info);
    }

    template std::expected<std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>, std::string> 
    glimpse::renderer::create_staging_buffer<glimpse::renderer::VulkanVertex>(glimpse::renderer::VulkanContext const&, std::vector<glimpse::renderer::VulkanVertex>, unsigned long);

    template std::expected<std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>, std::string> 
    glimpse::renderer::create_staging_buffer<unsigned short>(glimpse::renderer::VulkanContext const&, std::vector<unsigned short>, unsigned long);

    template std::expected<std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>, std::string> 
    glimpse::renderer::create_staging_buffer<unsigned int>(glimpse::renderer::VulkanContext const&, std::vector<unsigned int>, unsigned long);

    template std::expected<std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>, std::string> 
    glimpse::renderer::create_staging_buffer<unsigned char>(glimpse::renderer::VulkanContext const&, std::vector<unsigned char>, unsigned long);
}
