#pragma once

#include "renderer/descriptor_allocator.hpp"
#include "renderer/material.hpp"
#include "renderer/mesh.hpp"
#include "renderer/uniform_buffer.hpp"
#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace glimpse {
namespace renderer {

template <typename T> class Entity {
  public:
  private:
    std::shared_ptr<Mesh> m_mesh;
    std::shared_ptr<Material> m_material;
    UniformBuffer<T> m_uniform_buffer;
    std::vector<vk::raii::DescriptorSet> m_desciptor_sets;
};

template <typename T> class EntityBuilder {
  public:
    EntityBuilder<T>();
    Entity<T> build(DescriptorAllocator &descriptor_allocator);

    EntityBuilder<T> &with_mesh(Mesh);

  private:
    // TODO: make them optional
    std::shared_ptr<Mesh> m_mesh;
    std::shared_ptr<Material> m_material;
    UniformBuffer<T> m_uniform_buffer;
};
} // namespace renderer
} // namespace glimpse
