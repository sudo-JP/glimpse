#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/trigonometric.hpp>

namespace glimpse::renderer {
class Camera {
public:
    Camera(glm::vec3 position, glm::vec3 target);

    glm::mat4 get_view();
    glm::mat4 get_projection();

    void set_position(glm::vec3 position);

private:
    glm::vec3 m_position;
    glm::vec3 m_target;

    // Class constant
    const glm::vec3 m_up{0.0f, 0.0f, 1.0f};

    // Float values
    const float m_fov = glm::radians(45.0f);
    const float m_aspect = 1920.0f / 1080.0f;
    const float m_z_near = 0.1f;
    const float m_z_far = 100.0f;
};
} // namespace glimpse::renderer
