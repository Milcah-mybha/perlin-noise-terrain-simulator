#include "Camera.h"
#include <cmath>
#include <algorithm>

// Khoi tao camera voi vi tri va goc nhin ban dau
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
      MovementSpeed(42.0f),
      MouseSensitivity(0.09f),
      Zoom(45.0f),
      targetVelocity(0.0f),
      currentVelocity(0.0f),
      targetYaw(yaw),
      targetPitch(pitch),
      targetZoom(45.0f),
      damping(10.0f),
      mouseDamping(22.0f),
      isSprint(false) {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

// Tinh ma tran View Matrix cho OpenGL
glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(Position, Position + Front, Up);
}

// Xu ly di chuyen camera bang ban phim (WASD, Space, Ctrl)
void Camera::ProcessKeyboard(Camera_Movement direction) {
    float speedMultiplier = isSprint ? 2.3f : 1.0f;
    float currentSpeed = MovementSpeed * speedMultiplier;

    if (direction == FORWARD)  targetVelocity += Front * currentSpeed;
    if (direction == BACKWARD) targetVelocity -= Front * currentSpeed;
    if (direction == LEFT)     targetVelocity -= Right * currentSpeed;
    if (direction == RIGHT)    targetVelocity += Right * currentSpeed;
    if (direction == UP)       targetVelocity += WorldUp * currentSpeed;
    if (direction == DOWN)     targetVelocity -= WorldUp * currentSpeed;
}

// Xu ly xoay goc nhin camera bang chuot
void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    targetYaw   += xoffset;
    targetPitch += yoffset;

    if (constrainPitch) {
        if (targetPitch > 89.0f)  targetPitch = 89.0f;
        if (targetPitch < -89.0f) targetPitch = -89.0f;
    }
}

// Quay camera quanh mot diem muc tieu, giu nguyen khoang cach hien tai
void Camera::OrbitAround(const glm::vec3& target, float xoffset, float yoffset) {
    glm::vec3 offset = Position - target;
    float radius = glm::length(offset);
    if (radius < 1.0f) {
        radius = 1.0f;
        offset = glm::vec3(0.0f, 0.0f, radius);
    }

    float horizontalDistance = std::sqrt(offset.x * offset.x + offset.z * offset.z);
    float azimuth = std::atan2(offset.z, offset.x);
    float elevation = std::atan2(offset.y, horizontalDistance);

    azimuth -= glm::radians(xoffset * MouseSensitivity);
    elevation += glm::radians(yoffset * MouseSensitivity);
    elevation = std::clamp(elevation, glm::radians(5.0f), glm::radians(85.0f));

    float horizontalRadius = radius * std::cos(elevation);
    Position = target + glm::vec3(
        horizontalRadius * std::cos(azimuth),
        radius * std::sin(elevation),
        horizontalRadius * std::sin(azimuth)
    );

    glm::vec3 lookDirection = glm::normalize(target - Position);
    float lookYaw = glm::degrees(std::atan2(lookDirection.z, lookDirection.x));
    float lookPitch = glm::degrees(std::asin(std::clamp(lookDirection.y, -1.0f, 1.0f)));

    Yaw = targetYaw = lookYaw;
    Pitch = targetPitch = lookPitch;
    targetVelocity = glm::vec3(0.0f);
    currentVelocity = glm::vec3(0.0f);
    updateCameraVectors();
}

// Xu ly phong to thu nho (FoV Zoom) bang cuon chuot
void Camera::ProcessMouseScroll(float yoffset) {
    targetZoom -= yoffset * 2.5f;
    if (targetZoom < 15.0f) targetZoom = 15.0f;
    if (targetZoom > 65.0f) targetZoom = 65.0f;
}

// Cap nhat vi tri va goc quay camera voi noi suy quan tinh muot ma
void Camera::Update(float deltaTime) {
    float moveBlend = 1.0f - std::exp(-damping * deltaTime);
    currentVelocity = glm::mix(currentVelocity, targetVelocity, moveBlend);
    Position += currentVelocity * deltaTime;
    targetVelocity = glm::vec3(0.0f);

    float rotBlend = 1.0f - std::exp(-mouseDamping * deltaTime);
    Yaw   = glm::mix(Yaw, targetYaw, rotBlend);
    Pitch = glm::mix(Pitch, targetPitch, rotBlend);

    float zoomBlend = 1.0f - std::exp(-15.0f * deltaTime);
    Zoom  = glm::mix(Zoom, targetZoom, zoomBlend);

    updateCameraVectors();
}

// Dat lai goc nhin camera ve vi tri mac dinh
void Camera::Reset(glm::vec3 position, float yaw, float pitch) {
    Position = position;
    targetVelocity = glm::vec3(0.0f);
    currentVelocity = glm::vec3(0.0f);
    Yaw = yaw;
    Pitch = pitch;
    targetYaw = yaw;
    targetPitch = pitch;
    Zoom = 45.0f;
    targetZoom = 45.0f;
    updateCameraVectors();
}

// Cap nhat cac vector huong nhin Front, Right, Up
void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up    = glm::normalize(glm::cross(Right, Front));
}
