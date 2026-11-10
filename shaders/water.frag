#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uViewPos;

uniform bool uFogEnabled;
uniform vec3 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(uViewPos - FragPos);
    vec3 lightDir = normalize(-uLightDir);

    // Màu nước biển chuyển từ xanh ngọc bích (Shallow Cyan) sang xanh dương thẳm (Deep Blue)
    vec3 shallowColor = vec3(0.08, 0.45, 0.55);
    vec3 deepColor    = vec3(0.02, 0.18, 0.38);

    // Hiệu ứng Fresnel: nhìn thẳng thấy nước sâu trong, nhìn nghiêng thấy phản chiếu mạnh
    float fresnel = pow(1.0 - max(dot(norm, viewDir), 0.0), 3.0);
    vec3 waterColor = mix(deepColor, shallowColor, 0.5);

    // Ánh sáng phản xạ lấp lánh mạnh của mặt trời trên mặt nước (Sun Glint / Specular)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 128.0);
    vec3 specular = spec * uLightColor * 1.5;

    // Phản xạ bầu trời khi nhìn nghiêng
    vec3 skyReflection = vec3(0.7, 0.85, 1.0) * fresnel * 0.8;

    vec3 finalColor = waterColor + specular + skyReflection;

    // Sương mù
    if (uFogEnabled) {
        float dist = length(uViewPos - FragPos);
        float fogFactor = clamp((uFogEnd - dist) / (uFogEnd - uFogStart), 0.0, 1.0);
        finalColor = mix(uFogColor, finalColor, fogFactor);
    }

    // Độ trong suốt nhẹ của nước
    FragColor = vec4(finalColor, 0.88);
}
