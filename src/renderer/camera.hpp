#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>

namespace glimpse::renderer {
class Camera {
public:
    Camera();
    glm::mat4 get_view();
    
private:
    glm::vec3 m_position;
    glm::vec3 m_target;
    glm::mat4 m_view;
    glm::mat4 m_proj;

    // Class constant 
    const glm::vec3 m_up{0.0f, 1.0f, 0.0f};
    const glm::vec3 m_front{0.0f, 0.0f, 1.0f};
    const float fov = 45.0f; 
};
} // namespace glimpse::renderer
