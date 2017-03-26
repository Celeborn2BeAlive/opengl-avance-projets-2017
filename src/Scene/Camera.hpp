#pragma once

#include <glm/glm.hpp>

class Camera {
public:
    // Constructor.
    Camera(glm::vec3 eye = glm::vec3(0.f, 0.f, 5.f), glm::vec3 forward = glm::vec3(1.f, 0.f, 0.f), glm::vec3 up = glm::vec3(0.f, 0.f, 1.f), float fov = 70.f, float near = 1.f, float setFar = 100.f);

    // Forward vector.
    glm::vec3 const& forward() const { return m_forward; }
    void forward(glm::vec3 const& forward) { m_forward = abs(glm::dot(forward, m_up)) < 0.99f ? glm::normalize(forward) : m_forward; }

    // Up vector.
    glm::vec3 up() const { return glm::normalize(glm::cross(right(), m_forward)); }
    void up(glm::vec3 up) { m_up = glm::normalize(up); }

    // Right vector.
    glm::vec3 right() const { return glm::normalize(glm::cross(m_forward, m_up)); }

    // Camera eye.
    glm::vec3 const& eye() const { return m_eye; }
    void eye(glm::vec3 eye) { m_eye = eye; }

    // Near and far.
    void setNear(float n) { m_near = n; }
    void setFar(float f) { m_far = f; }

    // Notify a window resize.
    void resize(int width, int height);

    // Return the view matrix.
    glm::mat4 view() const;

    // Return the projection matrix.
    glm::mat4 projection() const;

private:
    glm::vec3 m_eye;
    glm::vec3 m_forward;
    glm::vec3 m_up;

    float m_near, m_far;
    float m_fov;
    float m_aspect;
};