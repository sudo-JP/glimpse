#pragma once

#include "renderer/graphics_pipeline.hpp"
#include "renderer/texture.hpp"
#include <memory>
namespace glimpse {
namespace renderer {
class Material {
  public:
    Material(
        std::shared_ptr<const Texture> texture,
        std::shared_ptr<const GraphicsPipeline> pipeline
    );

    // getters
    const vk::raii::DescriptorSetLayout& get_descriptor_set_layout() const;
    const Texture& get_texture() const;

  private:
    std::shared_ptr<const Texture> m_texture;
    std::shared_ptr<const GraphicsPipeline> m_pipeline;
};
} // namespace renderer
} // namespace glimpse
