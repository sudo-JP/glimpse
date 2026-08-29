#include "graphics_pipeline.hpp"
#include "vulkan/vulkan.hpp"
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

            auto create_info = vk::ShaderModuleCreateInfo()
                .setCodeSize(shader_file.size() * sizeof(char))
                .setPCode(reinterpret_cast<const uint32_t*>(shader_file.data()));

            const auto& device = context.get_device();

            vk::raii::ShaderModule shader_module(device, create_info);
            return shader_module;
        }
    } // end helper namespace

    std::expected<GraphicsPipeline, std::string> GraphicsPipeline::new_graphics_pipeline(
        const ShaderStageConfig& shader_config, 
        const glimpse::renderer::VulkanContext& context,
        const glimpse::renderer::VulkanSwapchain& swapchain
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
        auto fragment_shader_create_info = vk::PipelineShaderStageCreateInfo()
            .setStage(vk::ShaderStageFlagBits::eFragment)
            .setModule(shader_module)
            .setPName(fragment_entry);

        vk::PipelineShaderStageCreateInfo shader_stages[] = {
            vertex_shader_create_info,
            fragment_shader_create_info
        };

        vk::PipelineVertexInputStateCreateInfo vertex_input_info;

        // Fixed functions set up
        auto input_assembly = vk::PipelineInputAssemblyStateCreateInfo()
            .setTopology(vk::PrimitiveTopology::eTriangleList);

        const auto& swapchain_extent = swapchain.get_extent();

        vk::Viewport viewport = vk::Viewport()
            .setX(0.0f)
            .setY(0.0f)
            .setHeight(static_cast<float>(swapchain_extent.width))
            .setWidth(static_cast<float>(swapchain_extent.height));

        vk::Offset2D scissor_offset = vk::Offset2D()
            .setX(0)
            .setY(0);

        vk::Rect2D scissor = vk::Rect2D()
            .setOffset(scissor_offset)
            .setExtent(swapchain_extent);

        std::vector<vk::DynamicState> dynamic_states = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };
        auto dynamic_state = vk::PipelineDynamicStateCreateInfo()
            .setDynamicStateCount(static_cast<uint32_t>(dynamic_states.size()))
            .setPDynamicStates(dynamic_states.data());

        auto viewport_state = vk::PipelineViewportStateCreateInfo()
            .setViewportCount(1)
            .setScissorCount(1)
            .setPViewports(&viewport)
            .setPScissors(&scissor);

        auto rasterizer = vk::PipelineRasterizationStateCreateInfo()
            .setDepthClampEnable(vk::False)
            .setRasterizerDiscardEnable(vk::False)
            .setPolygonMode(vk::PolygonMode::eFill)
            .setCullMode(vk::CullModeFlagBits::eBack)
            .setFrontFace(vk::FrontFace::eClockwise)
            .setDepthBiasEnable(vk::False)
            .setLineWidth(1.0f);

        auto multisampling = vk::PipelineMultisampleStateCreateInfo()
            .setRasterizationSamples(vk::SampleCountFlagBits::e1)
            .setSampleShadingEnable(vk::False);

        auto color_blend_attachment = vk::PipelineColorBlendAttachmentState()
            .setBlendEnable(vk::False)
            .setColorWriteMask(
            vk::ColorComponentFlagBits::eR
                | vk::ColorComponentFlagBits::eG
                | vk::ColorComponentFlagBits::eB
            );

        auto color_blending = vk::PipelineColorBlendStateCreateInfo()
            .setLogicOpEnable(vk::False)
            .setLogicOp(vk::LogicOp::eCopy)
            .setAttachmentCount(1)
            .setPAttachments(&color_blend_attachment);

        auto pipeline_layout_info = vk::PipelineLayoutCreateInfo()
            .setSetLayoutCount(0)
            .setPushConstantRangeCount(0);

        const auto& device = context.get_device();
        auto pipeline_layout = vk::raii::PipelineLayout(device, pipeline_layout_info);

        auto const& swapchain_format = swapchain.get_format();

        auto pipeline_create_info_chain = vk::StructureChain<
            vk::GraphicsPipelineCreateInfo, 
            vk::PipelineRenderingCreateInfo
        >();

        pipeline_create_info_chain.get<vk::GraphicsPipelineCreateInfo>()
            .setStageCount(2)
            .setPStages(shader_stages)
            .setPVertexInputState(&vertex_input_info)
            .setPInputAssemblyState(&input_assembly)
            .setPViewportState(&viewport_state)
            .setPRasterizationState(&rasterizer)
            .setPMultisampleState(&multisampling)
            .setPColorBlendState(&color_blending)
            .setLayout(pipeline_layout)
            .setPDynamicState(&dynamic_state)
            .setRenderPass(nullptr);

        pipeline_create_info_chain.get<vk::PipelineRenderingCreateInfo>()
            .setColorAttachmentCount(1)
            .setPColorAttachmentFormats(&swapchain_format.format);

        auto graphics_pipeline = vk::raii::Pipeline(
            device, nullptr, pipeline_create_info_chain.get<vk::GraphicsPipelineCreateInfo>()
        );

        return GraphicsPipeline(
            std::move(pipeline_layout),
            std::move(graphics_pipeline)
        );
    }


    GraphicsPipeline::GraphicsPipeline(
        vk::raii::PipelineLayout pipeline_layout,
        vk::raii::Pipeline graphics_pipeline
    ) :
    m_pipeline_layout(std::move(pipeline_layout)),
    m_graphics_pipeline(std::move(graphics_pipeline))
    {}
}
