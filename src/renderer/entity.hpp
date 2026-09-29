#pragma once

#include "renderer/descriptor_allocator.hpp"
#include "renderer/material.hpp"
#include "renderer/mesh.hpp"
#include "renderer/uniform_buffer.hpp"
#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace glimpse::renderer {

template <typename T> class Entity {
  public:
    Entity<T>(
        std::shared_ptr<Mesh> m_mesh,
        std::shared_ptr<Material> m_material,
        UniformBuffer<T> m_uniform_buffer
    );
  private:
    std::shared_ptr<Mesh> m_mesh;
    std::shared_ptr<Material> m_material;
    UniformBuffer<T> m_uniform_buffer;
    std::vector<vk::raii::DescriptorSet> m_desciptor_sets;
};

} // namespace glimpse::renderer
