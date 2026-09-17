#include "texture.hpp"
#include "renderer/buffer_utils.hpp"
#include <cstdint>
#include <ktx.h>

namespace glimpse::renderer {
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
        auto [staging_buffer, stagging_buffer_memory] = std::move(staging_buf_res).value();
        ktxTexture_Destroy(texture);


        return Texture();
    }
}
