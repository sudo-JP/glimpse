#include "mesh.hpp"
#include "renderer/command_recorder.hpp"
#include <algorithm>
#include <cstdint>
#include <expected>
#include <string>
#include <tuple>
#include <type_traits>
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
            auto data_staging = static_cast<glimpse::renderer::VulkanVertex *>(staging_buffer_memory.mapMemory(offset, size));
            std::copy(data.begin(), data.end(), data_staging);
            staging_buffer_memory.unmapMemory();

            return std::pair{
                std::move(staging_buffer),
                std::move(staging_buffer_memory)
            };
        }

        std::expected<
            std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>, 
            std::string> create_vertex_buffer(
            const std::vector<glimpse::renderer::VulkanVertex>& vertices,
            const glimpse::renderer::VulkanContext& context,
            const glimpse::renderer::CommandRecorder& recorder
        ) {
            vk::DeviceSize buffer_size = sizeof(std::remove_cvref_t<decltype(vertices)>::value_type) * vertices.size();

            // Staging buffer 
            auto staging_buf_res = create_staging_buffer(
                context, 
                vertices, 
                buffer_size
            );

            if (!staging_buf_res) return std::unexpected(std::move(staging_buf_res).error());
            auto [staging_buffer, stagging_buffer_memory] = std::move(staging_buf_res).value();

            auto buffer_res = create_buffer(
                buffer_size, 
                vk::BufferUsageFlagBits::eVertexBuffer 
                | vk::BufferUsageFlagBits::eTransferDst, 
                vk::MemoryPropertyFlagBits::eDeviceLocal, 
                context
            );

            if (!buffer_res) return std::unexpected(std::move(buffer_res).error());
            auto [vertex_buffer, vertex_buffer_memory] = std::move(buffer_res).value();

            recorder.copy_and_submit_immediate(staging_buffer, vertex_buffer, buffer_size);

            return std::pair{
                std::move(vertex_buffer),
                std::move(vertex_buffer_memory)
            };
        }

        template <typename T>
        requires std::same_as<T, uint16_t> 
        || std::same_as<T, uint32_t>
        || std::same_as<T, uint64_t>
        std::expected<
            std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>, 
            std::string> create_index_buffer(
            const std::vector<T>& indices,
            const glimpse::renderer::VulkanContext& context,
            const glimpse::renderer::CommandRecorder& recorder
        ) {
            vk::DeviceSize buffer_size =  sizeof(std::remove_cvref_t<decltype(indices)>::value_type) * indices.size();

            auto staging_buf_res = create_staging_buffer(
                context, 
                indices, 
                buffer_size
            );

            if (!staging_buf_res) return std::unexpected(std::move(staging_buf_res).error());
            auto [staging_buffer, stagging_buffer_memory] = std::move(staging_buf_res).value();

            auto buffer_res = create_buffer(
                buffer_size, 
                vk::BufferUsageFlagBits::eIndexBuffer
                | vk::BufferUsageFlagBits::eTransferDst, 
                vk::MemoryPropertyFlagBits::eDeviceLocal, 
                context
            );

            if (!buffer_res) return std::unexpected(std::move(buffer_res).error());
            auto [index_buffer, index_buffer_memory] = std::move(buffer_res).value();

            recorder.copy_and_submit_immediate(staging_buffer, index_buffer, buffer_size);

            return std::pair{
                std::move(index_buffer),
                std::move(index_buffer_memory)
            };
        }

    } // End helper namespace
     
    template <typename T>
    requires std::same_as<T, uint16_t> 
    || std::same_as<T, uint32_t>
    || std::same_as<T, uint64_t>
    std::expected<Mesh, std::string> Mesh::new_mesh(
        const std::vector<glimpse::renderer::VulkanVertex>& vertices,
        const std::vector<T> indices,
        const glimpse::renderer::VulkanContext& context,
        const glimpse::renderer::CommandRecorder& recorder
    ) {
        // Vertex buffer creation
        auto vertex_buf_res = create_vertex_buffer(vertices, context, recorder);
        if (!vertex_buf_res) return std::unexpected(std::move(vertex_buf_res).error());
        auto [vertex_buffer, vertex_buffer_memory] = std::move(vertex_buf_res).value();

        auto index_buf_res = create_index_buffer(indices, context, recorder);
        if (!index_buf_res) return std::unexpected(std::move(index_buf_res).error());
        auto [index_buffer, index_buffer_memory] = std::move(index_buf_res).value();

        return Mesh(
            static_cast<uint32_t>(vertices.size()),
            std::move(vertex_buffer),
            std::move(vertex_buffer_memory),
            std::move(index_buffer),
            std::move(index_buffer_memory)
        );
    }

    Mesh::Mesh(
        uint32_t size,
        vk::raii::Buffer vertex_buffer,
        vk::raii::DeviceMemory vertex_buffer_memory,
        vk::raii::Buffer index_buffer,
        vk::raii::DeviceMemory index_buffer_memory
    ) : m_size(size),
    m_vertex_buffer(std::move(vertex_buffer)),
    m_vertex_buffer_memory(std::move(vertex_buffer_memory)),
    m_index_buffer(std::move(index_buffer)),
    m_index_buffer_memory(std::move(index_buffer_memory))
    {}

    const vk::raii::Buffer& Mesh::get_vertex_buffer() const {
        return m_vertex_buffer;
    }

    uint32_t Mesh::get_size() const { return m_size; }
}
