#pragma once

#include "renderer/entity.hpp"
#include "renderer/types.hpp"
#include <vector>
namespace glimpse::renderer {
class SceneData {
public:
    void add_entity(Entity<MVP> entity);
private: 
    std::vector<Entity<MVP>> m_entities;
};
} // namespace glimpse::renderer
