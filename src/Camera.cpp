#include "Camera.h"
#include <cmath>
#include <algorithm>

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
      MovementSpeed(SPEED),
      MouseSensitivity(SENSITIVITY),
      Zoom(ZOOM),
      targetVelocity(0.0f),
      currentVelocity(0.0f),
      targetYaw(yaw),
      targetPitch(pitch),
      targetZoom(ZOOM),
      damping(10.0f),
      mouseDamping(22.0f),
      isSprint(false) {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(Camera_Movement direction) {
    float speedMultiplier = isSprint ? 2.3f : 1.0f;
    float currentSpeed = MovementSpeed * speedMultiplier;

    if (direction == FORWARD)
        targetVelocity += Front * currentSpeed;
    if (direction == BACKWARD)
        targetVelocity -= Front * currentSpeed;
    if (direction == LEFT)
        targetVelocity -= Right * currentSpeed;
    if (direction == RIGHT)
        targetVelocity += Right * currentSpeed;
    if (direction == UP)
        targetVelocity += WorldUp * currentSpeed;
    if (direction == DOWN)
        targetVelocity -= WorldUp * currentSpeed;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    targetYaw   += xoffset;
    targetPitch += yoffset;

    if (constrainPitch) {
        if (targetPitch > 89.0f)
            targetPitch = 89.0f;
        if (targetPitch < -89.0f)
            targetPitch = -89.0f;
    }
}

void Camera::ProcessMouseScroll(float yoffset) {
    targetZoom -= yoffset * 2.5f;
    if (targetZoom < 15.0f)
        targetZoom = 15.0f;
    if (targetZoom > 65.0f)
        targetZoom = 65.0f;
}

void Camera::Update(float deltaTime) {
    // 1. Làm mịn vị trí di chuyển với gia tốc và quán tính lướt êm ái
    float moveBlend = 1.0f - std::exp(-damping * deltaTime);
    currentVelocity = glm::mix(currentVelocity, targetVelocity, moveBlend);
    Position += currentVelocity * deltaTime;

    // Reset lực đẩy mục tiêu cho frame tiếp theo
    targetVelocity = glm::vec3(0.0f);

    // 2. Làm mịn góc xoay chuột loại bỏ hoàn toàn vi giật (Micro-stutter)
    float rotBlend = 1.0f - std::exp(-mouseDamping * deltaTime);
    Yaw   = glm::mix(Yaw, targetYaw, rotBlend);
    Pitch = glm::mix(Pitch, targetPitch, rotBlend);

    // 3. Làm mịn thu phóng tầm nhìn (FoV Zoom)
    float zoomBlend = 1.0f - std::exp(-15.0f * deltaTime);
    Zoom  = glm::mix(Zoom, targetZoom, zoomBlend);

    updateCameraVectors();
}

void Camera::Reset(glm::vec3 position, float yaw, float pitch) {
    Position = position;
    targetVelocity = glm::vec3(0.0f);
    currentVelocity = glm::vec3(0.0f);
    Yaw = yaw;
    Pitch = pitch;
    targetYaw = yaw;
    targetPitch = pitch;
    Zoom = ZOOM;
    targetZoom = ZOOM;
    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);

    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up    = glm::normalize(glm::cross(Right, Front));
}
