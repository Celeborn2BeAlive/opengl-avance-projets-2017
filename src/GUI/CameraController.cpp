#include "CameraController.hpp"
#include "Scene/Camera.hpp"
#include <glm/gtx/rotate_vector.hpp>

CameraController::CameraController()
:   m_camera(nullptr), m_speed(1.f)
{}

void CameraController::attach(Camera& camera) {
    m_camera = &camera;
}

void CameraController::inputYaw(float theta) {
    if (!m_camera) return;
    m_camera->forward(glm::rotate(m_camera->forward(), theta, m_camera->up()));
}

void CameraController::inputPitch(float theta) {
    if (!m_camera) return;
    m_camera->forward(glm::rotate(m_camera->forward(), theta, m_camera->right()));
}

void CameraController::inputRoll(float theta) {
    if (!m_camera) return;
    m_camera->up(glm::rotate(m_camera->up(), theta, m_camera->forward()));
}

void CameraController::inputForward(float move) {
    if (!m_camera) return;
    m_camera->eye(m_camera->eye() + m_camera->forward() * move * m_speed);
}

void CameraController::inputVertical(float move) {
    if (!m_camera) return;
    m_camera->eye(m_camera->eye() + m_camera->up() * move * m_speed);
}

void CameraController::inputLateral(float move) {
    if (!m_camera) return;
    m_camera->eye(m_camera->eye() + m_camera->right() * move * m_speed);
}

void CameraController::resize(int width, int height) {
    if (!m_camera) return;
    m_camera->resize(width, height);
}
