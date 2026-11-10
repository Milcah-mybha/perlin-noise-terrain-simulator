#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in float aHeight;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out float Height;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    
    // Ma trận pháp tuyến đối với phép biến đổi đều
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;
    Height = aHeight;

    gl_Position = projection * view * worldPos;
}
