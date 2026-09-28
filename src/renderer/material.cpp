#include "material.hpp"
#include <memory>

namespace glimpse::renderer {
Material::Material(
    std::shared_ptr<const Texture> texture,
    std::shared_ptr<const GraphicsPipeline> pipeline
)
    : m_texture(std::move(texture)), m_pipeline(std::move(pipeline)) {}

const vk::raii::DescriptorSetLayout &
Material::get_descriptor_set_layout() const {
    return m_pipeline.get()->get_descriptor_set_layout();
}

const Texture &Material::get_texture() const { return *m_texture.get(); }
} // namespace glimpse::renderer
