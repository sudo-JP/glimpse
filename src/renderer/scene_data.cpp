#include "scene_data.hpp"

namespace glimpse::renderer {
void SceneData::add_entity(Entity<MVP> entity) {
    m_entities.push_back(std::move(entity));
}
} // namespace glimpse::renderer
