#include "Light.hpp"
#include "Scene/Scene.hpp"

#include <glm/gtc/matrix_transform.hpp>

Light::Light(glm::vec3 direction, glm::vec3 intensity)
:   m_direction(glm::normalize(direction)), m_intensity(std::move(intensity)), m_viewproj(1.f)
{}

glm::mat4 Light::viewproj(Scene const& scene) const {
    float radius = scene.size() * 0.5f;
    glm::vec3 center = scene.center();
    glm::vec3 up = (std::abs(m_direction.y) > 0.99f) ? glm::vec3(1.f, 0.f, 0.f) : glm::vec3(0.f, 1.f, 0.f);

    glm::mat4 projection = glm::ortho(-radius, radius, -radius, radius, 0.01f * radius, 2.f * radius);
    glm::mat4 view = glm::lookAt(center + m_direction * radius, center, up);

    return projection * view;
}
