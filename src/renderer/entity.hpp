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
        const DescriptorAllocator& allocator
    );

    // getters
    Uniform<T>& get_uniform() const;
    const Mesh& get_mesh() const;

  private:
    std::vector<vk::raii::DescriptorSet> m_desciptor_sets;
    std::shared_ptr<const Mesh> m_mesh;
    std::shared_ptr<const Material> m_material;
    Uniform<T> m_uniform;
};

} // namespace glimpse::renderer
