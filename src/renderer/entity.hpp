#pragma once

#include "renderer/descriptor_allocator.hpp"
#include "renderer/material.hpp"
#include "renderer/mesh.hpp"
#include "renderer/uniform.hpp"
#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace glimpse::renderer {

template <typename T>
class Entity {
  public:
    Entity(
        std::shared_ptr<const Mesh> mesh,
        std::shared_ptr<const Material> material,
        Uniform<T> uniform,
        DescriptorAllocator& allocator
    );

    // getters
    Uniform<T>& get_uniform();
    const Mesh& get_mesh() const;
    const Material& get_material() const;

    const std::optional<std::reference_wrapper<const vk::raii::DescriptorSet>>
    get_descriptor_set(size_t index) const;

  private:
    std::vector<vk::raii::DescriptorSet> m_descriptor_sets;
    std::shared_ptr<const Mesh> m_mesh;
    std::shared_ptr<const Material> m_material;
    Uniform<T> m_uniform;
};

} // namespace glimpse::renderer
