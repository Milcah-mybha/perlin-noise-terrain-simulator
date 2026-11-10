#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in float Height;

out vec4 FragColor;

// Tham số ánh sáng (Blinn-Phong)
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uAmbientColor;
uniform vec3 uViewPos;

// Tham số địa hình
uniform float uMinHeight;
uniform float uMaxHeight;
uniform int uColorMode; // 0: Realistic Biomes, 1: Elevation Heatmap, 2: Grayscale Heightmap

// Tham số sương mù (Atmospheric Fog)
uniform bool uFogEnabled;
uniform vec3 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;

// Bảng màu cho Chế độ Biome tự nhiên
vec3 getBiomeColor(float h, float slope) {
    float t = clamp((h - uMinHeight) / max(0.001, uMaxHeight - uMinHeight), 0.0, 1.0);

    vec3 sand  = vec3(0.86, 0.78, 0.55); // Cát bờ biển
    vec3 grass = vec3(0.28, 0.58, 0.22); // Cỏ xanh mướt
    vec3 forest= vec3(0.18, 0.42, 0.16); // Rừng cây xanh thẫm
    vec3 rock  = vec3(0.50, 0.47, 0.44); // Đá vách núi
    vec3 snow  = vec3(0.95, 0.96, 0.98); // Băng tuyết đỉnh núi

    vec3 baseColor;

    if (t < 0.15) {
        baseColor = sand;
    } else if (t < 0.40) {
        float factor = (t - 0.15) / 0.25;
        baseColor = mix(sand, grass, factor);
    } else if (t < 0.65) {
        float factor = (t - 0.40) / 0.25;
        baseColor = mix(grass, forest, factor);
    } else if (t < 0.82) {
        float factor = (t - 0.65) / 0.17;
        baseColor = mix(forest, rock, factor);
    } else {
        float factor = (t - 0.82) / 0.18;
        baseColor = mix(rock, snow, factor);
    }

    // Hiệu ứng độ dốc (Slope effect): Vách dốc thẳng đứng sẽ luôn lộ đá sỏi xám
    if (slope < 0.70 && t > 0.12) {
        float rockFactor = clamp((0.70 - slope) / 0.25, 0.0, 1.0);
        baseColor = mix(baseColor, rock, rockFactor * 0.85);
    }

    return baseColor;
}

// Bảng màu Heatmap (Đo độ cao nhiệt đồ)
vec3 getHeatmapColor(float h) {
    float t = clamp((h - uMinHeight) / max(0.001, uMaxHeight - uMinHeight), 0.0, 1.0);
    if (t < 0.25) return mix(vec3(0.0, 0.0, 0.8), vec3(0.0, 0.8, 0.8), t / 0.25);
    if (t < 0.50) return mix(vec3(0.0, 0.8, 0.8), vec3(0.2, 0.8, 0.2), (t - 0.25) / 0.25);
    if (t < 0.75) return mix(vec3(0.2, 0.8, 0.2), vec3(0.9, 0.8, 0.1), (t - 0.50) / 0.25);
    return mix(vec3(0.9, 0.8, 0.1), vec3(0.9, 0.1, 0.1), (t - 0.75) / 0.25);
}

// Bảng màu Grayscale (Hiển thị bản đồ xám của Perlin Noise)
vec3 getGrayscaleColor(float h) {
    float t = clamp((h - uMinHeight) / max(0.001, uMaxHeight - uMinHeight), 0.0, 1.0);
    return vec3(t);
}

void main() {
    vec3 norm = normalize(Normal);
    float slope = norm.y;

    // 1. Xác định màu cơ sở (Base Albedo)
    vec3 albedo;
    if (uColorMode == 0) {
        albedo = getBiomeColor(Height, slope);
    } else if (uColorMode == 1) {
        albedo = getHeatmapColor(Height);
    } else {
        albedo = getGrayscaleColor(Height);
    }

    // 2. Mô hình chiếu sáng Blinn-Phong
    vec3 ambient = uAmbientColor * albedo;

    vec3 lightDir = normalize(-uLightDir);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor * albedo;

    vec3 viewDir = normalize(uViewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 16.0);
    vec3 specular = 0.15 * spec * uLightColor;

    vec3 litColor = ambient + diffuse + specular;

    // 3. Sương mù khí quyển (Atmospheric Distance Fog)
    if (uFogEnabled) {
        float dist = length(uViewPos - FragPos);
        float fogFactor = clamp((uFogEnd - dist) / (uFogEnd - uFogStart), 0.0, 1.0);
        litColor = mix(uFogColor, litColor, fogFactor);
    }

    FragColor = vec4(litColor, 1.0);
}
