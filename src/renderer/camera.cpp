#include "camera.hpp"

namespace glimpse::renderer {
glm::mat4 Camera::get_view() {
    return m_view;
}
} // namespace glimpse::renderer
