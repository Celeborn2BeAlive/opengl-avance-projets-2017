#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

class Scene;

class Light {
public:
    Light(glm::vec3 direction, glm::vec3 intensity);
    ~Light() = default;

    glm::mat4 viewproj(Scene const& scene) const;

    glm::vec3 const& direction() const { return m_direction; }
    glm::vec3 const& intensity() const { return m_intensity; }

private:
    glm::vec3 m_direction;
    glm::vec3 m_intensity;

    glm::mat4 m_viewproj;
};