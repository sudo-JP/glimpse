#pragma once

#include <memory>
#include <optional>
#include "graphics_pipeline.hpp"
#include "renderer/descriptor_allocator.hpp"
#include "renderer/mesh.hpp"
#include "renderer/texture.hpp"
#include "renderer/uniform_buffer.hpp"

namespace glimpse {
    namespace renderer {

        class Entity {
        public:
        private:
            std::shared_ptr<GraphicsPipeline> m_pipeline;
        };

        template <typename T>
        class EntityBuilder {
        public:
            Entity build(
                glimpse::renderer::DescriptorAllocator descriptor_allocator
            );
        private:
            std::optional<glimpse::renderer::Mesh> m_mesh;
            std::optional<glimpse::renderer::Texture> m_texture;
            glimpse::renderer::UniformBuffer<T> m_uniform_buffer;
        };
    }
}
