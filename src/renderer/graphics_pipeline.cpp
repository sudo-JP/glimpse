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
        [[nodiscard]] std::expected<vk::raii::ShaderModule, std::string> create_shader_module(
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

    std::expected<GraphicsPipeline, std::string> GraphicsPipeline::new_graphics_pipeline(
        const ShaderStageConfig& shader_config, const glimpse::renderer::VulkanContext& context
    ) {
        auto shader_module_res = create_shader_module(shader_config.filename, context);
        if (!shader_module_res) return std::unexpected(std::move(shader_module_res).error());
        auto shader_module = std::move(shader_module_res).value();

        const char *vertex_entry = shader_config.vert_entry.c_str();
        const char *fragment_entry = shader_config.frag_entry.c_str();

        // Vertex
        vk::PipelineShaderStageCreateInfo vertex_shader_create_info = vk::PipelineShaderStageCreateInfo()
            .setStage(vk::ShaderStageFlagBits::eVertex)
            .setModule(shader_module)
            .setPName(vertex_entry);

        // Fragment
        vk::PipelineShaderStageCreateInfo fragment_shader_create_info = vk::PipelineShaderStageCreateInfo()
            .setStage(vk::ShaderStageFlagBits::eFragment)
            .setModule(shader_module)
            .setPName(fragment_entry);

        vk::PipelineShaderStageCreateInfo shader_stages[] = {
            vertex_shader_create_info,
            fragment_shader_create_info
        };

        std::vector<vk::DynamicState> dynamic_states = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };

        vk::PipelineVertexInputStateCreateInfo vertex_input_info;
        
        return GraphicsPipeline();
    }
}
