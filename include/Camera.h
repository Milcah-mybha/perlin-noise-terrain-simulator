/**
 * @file Camera.h
 * @brief Định nghĩa lớp Camera 3D điều khiển góc nhìn tự do (FPS/Orbit), hỗ trợ xoay chuột mượt mà và tính ma trận View.
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera {
public:
    // === CÁC THÔNG SỐ VỊ TRÍ VÀ HƯỚNG NHÌN ===
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // === CÁC BIẾN NỘI SUY LÀM MỊN (SMOOTH DAMPING) ===
    glm::vec3 targetVelocity;
    glm::vec3 currentVelocity;
    float targetYaw;
    float targetPitch;
    float targetZoom;
    float damping;
    float mouseDamping;
    bool isSprint;

    Camera(glm::vec3 position = glm::vec3(0.0f, 45.0f, 125.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f,
           float pitch = -25.0f);

    glm::mat4 GetViewMatrix() const;

    // === XỬ LÝ ĐIỀU KHIỂN BÀN PHÍM VÀ CHUỘT ===
    void ProcessKeyboard(Camera_Movement direction);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yoffset);
    void SetSprint(bool enable) { isSprint = enable; }

    void Update(float deltaTime);
    void Reset(glm::vec3 position, float yaw = -90.0f, float pitch = -25.0f);

private:
    void updateCameraVectors();
};

#endif // CAMERA_H
