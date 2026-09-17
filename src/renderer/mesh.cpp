#include "mesh.hpp"
#include "renderer/command_recorder.hpp"
#include <algorithm>
#include <cstdint>
#include <expected>
#include <string>
#include "buffer_utils.hpp"
#include <type_traits>
#include <utility>
#include <vulkan/vulkan_raii.hpp>

namespace glimpse::renderer {
    namespace {
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
        std::expected<
            std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>, 
            std::string> create_index_buffer(
            const std::vector<T>& indices,
            const glimpse::renderer::VulkanContext& context,
            const glimpse::renderer::CommandRecorder& recorder
        ) {
            vk::DeviceSize buffer_size = sizeof(typename std::remove_cvref_t<decltype(indices)>::value_type) * indices.size();

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
    std::expected<Mesh, std::string> Mesh::new_mesh(
        const std::vector<glimpse::renderer::VulkanVertex>& vertices,
        const std::vector<T>& indices,
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

        AllocatedBuffer allocate_vert {
            std::move(vertex_buffer),
            std::move(vertex_buffer_memory),
            static_cast<uint32_t>(vertices.size())
        };

        AllocatedBuffer allocate_index {
            std::move(index_buffer),
            std::move(index_buffer_memory),
            static_cast<uint32_t>(indices.size())
        };

        return Mesh(
            std::move(allocate_vert),
            std::move(allocate_index)
        );
    }

    Mesh::Mesh(
        AllocatedBuffer vertex_buffer, 
        AllocatedBuffer index_buffer
    ) : m_vertex_buffer(std::move(vertex_buffer.buffer)),
    m_vertex_buffer_memory(std::move(vertex_buffer.buffer_memory)),
    m_vertices_size(std::move(vertex_buffer.size)),
    m_index_buffer(std::move(index_buffer.buffer)),
    m_index_buffer_memory(std::move(index_buffer.buffer_memory)),
    m_indices_size(std::move(index_buffer.size))
    {}

    const vk::raii::Buffer& Mesh::get_vertex_buffer() const {
        return m_vertex_buffer;
    }

    const vk::raii::Buffer& Mesh::get_index_buffer() const {
        return m_index_buffer;
    }

    uint32_t Mesh::get_vertices_size() const { return m_vertices_size; }
    uint32_t Mesh::get_indices_size() const { return m_indices_size; }

    template std::expected<Mesh, std::string>
    Mesh::new_mesh<uint16_t>(
        const std::vector<VulkanVertex>&,
        const std::vector<uint16_t>&,
        const VulkanContext&,
        const CommandRecorder&
    );

    template std::expected<Mesh, std::string>
    Mesh::new_mesh<uint32_t>(
        const std::vector<VulkanVertex>&,
        const std::vector<uint32_t>&,
        const VulkanContext&,
        const CommandRecorder&
    );

}
