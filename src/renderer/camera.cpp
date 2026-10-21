#include "camera.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace glimpse::renderer {
Camera::Camera(glm::vec3 position, glm::vec3 target) :
m_position(position),
m_target(target)
{}

glm::mat4 Camera::get_projection() {
    auto projection = glm::perspective(
        m_fov,
        m_aspect,
        m_z_near,
        m_z_far
    );
    projection[1][1] *= -1.0f;
    return projection;
}

glm::mat4 Camera::get_view() {
    return glm::lookAt(
        m_position,
        m_target,
        m_up
    );
}
void Camera::set_position(glm::vec3 position) {
    m_position = position;
}
} // namespace glimpse::renderer
