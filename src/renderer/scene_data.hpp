#pragma once

#include "renderer/entity.hpp"
#include "renderer/types.hpp"
#include <vector>
namespace glimpse::renderer {
struct SceneData {
    std::shared_ptr<const GraphicsPipeline> pipeline;
    std::unordered_map<std::string, std::shared_ptr<Texture>> texture_map;
    std::unordered_map<std::string, std::shared_ptr<const Material>> material_map;
    std::vector<Entity<MVP>> entities;
};
} // namespace glimpse::renderer
