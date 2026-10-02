#pragma once

#include "renderer/descriptor_allocator.hpp"
#include "renderer/material.hpp"
#include "renderer/mesh.hpp"
#include "renderer/uniform_buffer.hpp"
#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace glimpse::renderer {

template <typename T>
class Entity {
  public:
    Entity<T>(
        std::shared_ptr<const Mesh> mesh,
        std::shared_ptr<const Material> material,
        UniformBuffer<T> uniform_buffer,
        const DescriptorAllocator& allocator
    );
  private:
    std::vector<vk::raii::DescriptorSet> m_desciptor_sets;
    std::shared_ptr<const Mesh> m_mesh;
    std::shared_ptr<const Material> m_material;
    UniformBuffer<T> m_uniform_buffer;
};

} // namespace glimpse::renderer
