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

const float YAW         = -90.0f;
const float PITCH       = -25.0f;
const float SPEED       = 42.0f;
const float SENSITIVITY = 0.09f;
const float ZOOM        = 45.0f;

class Camera {
public:
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

    // Các biến phục vụ làm mịn chuyển động và quán tính (Smooth Camera Damping)
    glm::vec3 targetVelocity;
    glm::vec3 currentVelocity;
    float targetYaw;
    float targetPitch;
    float targetZoom;
    float damping;       // Hệ số làm mịn di chuyển (quán tính lướt)
    float mouseDamping;  // Hệ số làm mịn xoay góc nhìn chuột
    bool isSprint;       // Tăng tốc bay nhanh

    Camera(glm::vec3 position = glm::vec3(0.0f, 45.0f, 125.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = YAW,
           float pitch = PITCH);

    glm::mat4 GetViewMatrix() const;

    void ProcessKeyboard(Camera_Movement direction);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yoffset);
    void SetSprint(bool enable) { isSprint = enable; }

    // Cập nhật tọa độ và góc nhìn mượt mà ở mỗi khung hình
    void Update(float deltaTime);

    // Đặt lại vị trí ban đầu
    void Reset(glm::vec3 position, float yaw = YAW, float pitch = PITCH);

private:
    void updateCameraVectors();
};

#endif // CAMERA_H
