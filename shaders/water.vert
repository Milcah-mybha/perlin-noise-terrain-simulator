/**
 * @file water.vert
 * @brief Vertex Shader cho Mặt nước: tính toán độ cao sóng nhấp nhô thời gian thực (Gerstner Waves) và vector pháp tuyến.
 */

#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float uTime;
uniform float uWaterHeight;

void main() {
    vec3 pos = aPos;

    // Sóng nhấp nhô thời gian thực
    float wave1 = sin(pos.x * 0.15 + uTime * 2.0) * 0.25;
    float wave2 = cos(pos.z * 0.20 + uTime * 1.5) * 0.20;
    float wave3 = sin((pos.x + pos.z) * 0.10 + uTime * 2.5) * 0.15;
    pos.y = uWaterHeight + wave1 + wave2 + wave3;

    // Đạo hàm tính vector pháp tuyến của sóng
    float dx = 0.15 * cos(pos.x * 0.15 + uTime * 2.0) * 0.25 + 0.10 * cos((pos.x + pos.z) * 0.10 + uTime * 2.5) * 0.15;
    float dz = -0.20 * sin(pos.z * 0.20 + uTime * 1.5) * 0.20 + 0.10 * cos((pos.x + pos.z) * 0.10 + uTime * 2.5) * 0.15;
    vec3 waveNormal = normalize(vec3(-dx, 1.0, -dz));

    vec4 worldPos = model * vec4(pos, 1.0);
    FragPos = worldPos.xyz;
    Normal = mat3(transpose(inverse(model))) * waveNormal;
    TexCoords = aTexCoords;

    gl_Position = projection * view * worldPos;
}
