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

    vec3 shallowColor = vec3(0.20, 0.62, 0.80);
    vec3 deepColor    = vec3(0.08, 0.32, 0.54);

    // Phan xa Fresnel, mau nuoc bien va anh sang mat troi tren song
    float fresnel = pow(1.0 - max(dot(norm, viewDir), 0.0), 3.0);
    vec3 waterColor = mix(deepColor, shallowColor, 0.55);

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 96.0);
    vec3 specular = spec * uLightColor * 1.6;

    vec3 skyReflection = vec3(0.85, 0.92, 1.0) * fresnel * 0.75;
    vec3 finalColor = waterColor + specular + skyReflection;

    vec3 skyHorizon = vec3(0.60, 0.73, 0.88);
    float dist = length(uViewPos - FragPos);
    float fog = clamp((dist - 140.0) / 380.0, 0.0, 0.70);
    finalColor = mix(finalColor, skyHorizon, fog);

    FragColor = vec4(finalColor, 0.82);
}
