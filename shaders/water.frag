#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uViewPos;

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(uViewPos - FragPos);
    vec3 lightDir = normalize(-uLightDir);

    // Màu nước biển xanh trong veo chuẩn như mô hình mẫu
    vec3 shallowColor = vec3(0.22, 0.55, 0.76);
    vec3 deepColor    = vec3(0.12, 0.35, 0.55);

    // Hiệu ứng phản xạ Fresnel
    float fresnel = pow(1.0 - max(dot(norm, viewDir), 0.0), 3.0);
    vec3 waterColor = mix(deepColor, shallowColor, 0.5);

    // Ánh sáng phản xạ mặt trời trên mặt nước
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 96.0);
    vec3 specular = spec * uLightColor * 1.5;

    // Phản chiếu ánh sáng bầu trời
    vec3 skyReflection = vec3(0.80, 0.90, 1.0) * fresnel * 0.75;

    vec3 finalColor = waterColor + specular + skyReflection;

    FragColor = vec4(finalColor, 0.82);
}
