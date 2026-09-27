#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan_raii.hpp>
#include "renderer/descriptor_allocator.hpp"
#include "renderer/material.hpp"
#include "renderer/mesh.hpp"
#include "renderer/texture.hpp"
#include "renderer/uniform_buffer.hpp"

namespace glimpse {
    namespace renderer {

        template <typename T>
        class Entity {
        public:
        private:
            std::shared_ptr<glimpse::renderer::Mesh> m_mesh; 
            std::shared_ptr<glimpse::renderer::Material> m_material;
            glimpse::renderer::UniformBuffer<T> m_uniform_buffer;
            std::vector<vk::raii::DescriptorSet> m_desciptor_sets;
        };

        template <typename T>
        class EntityBuilder {
        public:
            EntityBuilder<T>();
            Entity<T> build(
                glimpse::renderer::DescriptorAllocator& descriptor_allocator
            );

            EntityBuilder<T>& with_mesh(glimpse::renderer::Mesh);
        private:
            // TODO: make them optional
            std::optional<glimpse::renderer::Mesh> m_mesh = std::nullopt;
            glimpse::renderer::Texture m_texture;
            glimpse::renderer::UniformBuffer<T> m_uniform_buffer;
        };
    }
}
