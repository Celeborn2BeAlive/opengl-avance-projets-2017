#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <utility>

Camera::Camera(glm::vec3 eye, glm::vec3 forward, glm::vec3 up, float fov, float near, float far)
:   m_eye(std::move(eye)), m_forward(std::move(forward)), m_up(std::move(up)), 
    m_near(near), m_far(far), m_fov(fov), m_aspect(1.f)
{
    m_forward = glm::normalize(m_forward);
    m_up = glm::normalize(m_up);
}

void Camera::resize(int width, int height) {
    m_aspect = width / (float)height;
}

glm::mat4 Camera::view() const {
    return glm::lookAt(m_eye, m_eye + m_forward, m_up);
}

glm::mat4 Camera::projection() const {
    return glm::perspective(m_fov, m_aspect, m_near, m_far);
}