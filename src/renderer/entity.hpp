#pragma once

#include <memory>
#include "graphics_pipeline.hpp"

namespace glimpse {
    namespace renderer {
        class Entity {
        public:
        private:
            std::shared_ptr<GraphicsPipeline> m_pipeline;
        };
    }
}
