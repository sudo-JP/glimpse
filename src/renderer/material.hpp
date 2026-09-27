#pragma once

#include "renderer/graphics_pipeline.hpp"
#include "renderer/texture.hpp"
#include <memory>
namespace glimpse {
    namespace renderer {
        class Material {
        public:
        private:
            std::shared_ptr<const glimpse::renderer::Texture> m_texture;
            std::shared_ptr<const glimpse::renderer::GraphicsPipeline> m_pipeline;
        };
    }
}
