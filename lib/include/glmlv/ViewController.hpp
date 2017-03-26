#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct GLFWwindow;

namespace glmlv 
{

class ViewController 
{
public:
    ViewController(GLFWwindow* window, float speed = 1.f) :
        m_pWindow(window), m_fSpeed(speed) {
    }

    ViewController(GLFWwindow* window, float speed = 1.f, float rotationSpeed = 0.1f) :
            m_pWindow(window), m_fSpeed(speed), m_fRotationSpeed(rotationSpeed) {
    }

    void setSpeed(float speed) {
        m_fSpeed = speed;
    }

    float getSpeed() const {
        return m_fSpeed;
    }

    void increaseSpeed(float delta) {
        m_fSpeed += delta;
        m_fSpeed = glm::max(m_fSpeed, 0.f);
    }

    float getCameraSpeed() const {
        return m_fSpeed;
    }

    void setRotationSpeed(float rotationSpeed) {
        m_fRotationSpeed = rotationSpeed;
    }

    float getRotationSpeed() const {
        return m_fRotationSpeed;
    }

    void increaseRotationSpeed(float delta) {
        m_fRotationSpeed += delta;
        m_fRotationSpeed = glm::max(m_fRotationSpeed, 0.f);
    }

    bool update(float elapsedTime);

    void setViewMatrix(const glm::mat4& viewMatrix) {
        m_ViewMatrix = viewMatrix;
        m_RcpViewMatrix = glm::inverse(viewMatrix);
    }

    const glm::mat4& getViewMatrix() const {
        return m_ViewMatrix;
    }

    const glm::mat4& getRcpViewMatrix() const {
        return m_RcpViewMatrix;
    }

private:
    GLFWwindow* m_pWindow = nullptr;
    float m_fSpeed = 0.f;
    float m_fRotationSpeed = 0.01f;
    bool m_LeftButtonPressed = false;
    glm::dvec2 m_LastCursorPosition;

    glm::mat4 m_ViewMatrix = glm::mat4(1);
    glm::mat4 m_RcpViewMatrix = glm::mat4(1);
};

}