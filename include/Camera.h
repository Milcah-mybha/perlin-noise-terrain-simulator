// Định nghĩa lớp Camera điều khiển góc nhìn 3D
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

    glm::vec3 targetVelocity;
    glm::vec3 currentVelocity;
    float targetYaw;
    float targetPitch;
    float targetZoom;
    float damping;
    float mouseDamping;
    bool isSprint;

    // Khởi tạo camera
    Camera(glm::vec3 position = glm::vec3(0.0f, 45.0f, 125.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f,
           float pitch = -25.0f);

    // Lấy ma trận quan sát View Matrix
    glm::mat4 GetViewMatrix() const;

    // Xử lý di chuyển bàn phím
    void ProcessKeyboard(Camera_Movement direction);

    // Xử lý xoay góc nhìn chuột
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

    // Xử lý cuộn chuột zoom FOV
    void ProcessMouseScroll(float yoffset);

    void SetSprint(bool enable) { isSprint = enable; }

    // Cập nhật tọa độ mượt mà mỗi frame
    void Update(float deltaTime);

    // Đặt lại góc nhìn mặc định
    void Reset(glm::vec3 position, float yaw = -90.0f, float pitch = -25.0f);

private:
    void updateCameraVectors();
};

#endif // CAMERA_H
