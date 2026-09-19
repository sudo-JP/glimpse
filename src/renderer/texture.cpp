#include "texture.hpp"
#include "renderer/buffer_utils.hpp"
#include "renderer/context.hpp"
#include <cstdint>
#include <ktx.h>
#include <utility>

namespace glimpse::renderer {
    namespace {
        std::expected<std::pair<vk::raii::Image, vk::raii::DeviceMemory>, std::string> create_image(
            uint32_t width, uint32_t height, vk::Format format,
            vk::ImageTiling tiling, vk::ImageUsageFlags usage,
            vk::MemoryPropertyFlags properties,
            const glimpse::renderer::VulkanContext& context
        ) {
            auto image_info = vk::ImageCreateInfo()
                .setImageType(vk::ImageType::e2D)
                .setFormat(format)
                .setExtent({width, height, 1})
                .setMipLevels(1)
                .setArrayLayers(1)
                .setSamples(vk::SampleCountFlagBits::e1)
                .setTiling(tiling)
                .setUsage(usage)
                .setSharingMode(vk::SharingMode::eExclusive);

            const auto& device = context.get_device();
            auto image = vk::raii::Image(device, image_info);

            // Memory
            const auto memory_requirements = image.getMemoryRequirements();
            auto memory_type_index_res = find_memory_type(
                memory_requirements.memoryTypeBits, 
                properties, 
                context
            );
            if (!memory_type_index_res) return std::unexpected(std::move(memory_type_index_res).error());
            auto memory_type_index = std::move(memory_type_index_res).value();

            auto alloc_info = vk::MemoryAllocateInfo()
                .setAllocationSize(memory_requirements.size)
                .setMemoryTypeIndex(memory_type_index);

            auto image_memory = vk::raii::DeviceMemory(device, alloc_info);
            image.bindMemory(image_memory, 0);
            return std::pair{
                std::move(image),
                std::move(image_memory)
            };
        }
    } // end helper namespace


    std::expected<Texture, std::string> Texture::new_texture(
        const std::string& filename,
        const glimpse::renderer::VulkanContext& context
    ) {
        ktxTexture *texture = nullptr;
        const auto path = filename.c_str();
        auto result = ktxTexture_CreateFromNamedFile(
            path,
            KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
            &texture
        );
        if (result != KTX_SUCCESS) return std::unexpected("failed to load ktx texture");

        auto raw_pixels = ktxTexture_GetData(texture);
        auto image_size = static_cast<vk::DeviceSize>(ktxTexture_GetDataSize(texture));
        std::vector<uint8_t> pixels(raw_pixels, raw_pixels + image_size);


        auto staging_buf_res = create_staging_buffer(
            context, 
            pixels, 
            image_size
        );

        if (!staging_buf_res) return std::unexpected(std::move(staging_buf_res).error());
        auto [staging_buffer, staging_buffer_memory] = std::move(staging_buf_res).value();
        
        // Get the image and image memory data
        auto texture_image_res = create_image(
            texture->baseWidth, 
            texture->baseHeight, 
            vk::Format::eR8G8B8A8Srgb, 
            vk::ImageTiling::eOptimal, 
            vk::ImageUsageFlagBits::eTransferDst
            | vk::ImageUsageFlagBits::eSampled, 
            vk::MemoryPropertyFlagBits::eDeviceLocal, 
            context
        );
        if (!texture_image_res) return std::unexpected(std::move(texture_image_res).error());
        auto [texture_image, texture_image_memory] = std::move(texture_image_res).value();

        ktxTexture_Destroy(texture);
        return Texture(
            std::move(texture_image),
            std::move(texture_image_memory)
        );
    }

    Texture::Texture(
        vk::raii::Image texture_image,
        vk::raii::DeviceMemory texture_image_memory
    ) : m_texture_image(std::move(texture_image)),
    m_texture_image_memory(std::move(texture_image_memory))
    {}
}
