#pragma once

class Camera;

class CameraController {
public:
    // Constructor.
    CameraController();

    // Attach a camera to the controller.
    void attach(Camera& camera);

    // Input rotations.
    void inputYaw(float theta);
    void inputPitch(float theta);
    void inputRoll(float theta);

    // Input translations.
    void inputForward(float move);
    void inputVertical(float move);
    void inputLateral(float move);

    // Notify window size.
    void resize(int width, int height);

    // Change the speed of the controller.
    void speed(float speed) { m_speed = speed; }

private:
    // The controlled camera. 
    Camera* m_camera;

    // Speed of the controller.
    float m_speed;
};