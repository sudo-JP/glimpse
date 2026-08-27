#include "graphics_pipeline.hpp"
#include <fstream>

namespace glimpse::renderer {
    namespace {
        std::expected<std::vector<char>, std::string> read_file(const std::string& filename) {
            std::ifstream file(filename, std::ios::ate | std::ios::binary);
            if (!file.is_open()) return std::unexpected("failed to open file");

            std::vector<char> buffer(file.tellg());
            file.seekg(0, std::ios::beg);
            file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
            file.close();
            return buffer;
        }
    }
    // Methods
    std::expected<vk::raii::ShaderModule, std::string> GraphicsPipeline::create_shader_module(
        const std::string& filename, const glimpse::renderer::VulkanContext& context
    ) {
        auto shader_file_res = read_file(filename);
        if (!shader_file_res) return std::unexpected(std::move(shader_file_res).error());
        auto shader_file = std::move(shader_file_res).value();

        vk::ShaderModuleCreateInfo create_info = vk::ShaderModuleCreateInfo()
            .setCodeSize(shader_file.size() * sizeof(char))
            .setPCode(reinterpret_cast<const uint32_t*>(shader_file.data()));

        const auto& device = context.get_device();

        vk::raii::ShaderModule shader_module(device, create_info);
        return shader_module;
    }
}
