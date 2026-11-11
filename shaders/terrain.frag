#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in float NormalizedHeight;

out vec4 FragColor;

// Tham số ánh sáng (Blinn-Phong)
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uAmbientColor;
uniform vec3 uViewPos;

// Tham số địa hình
uniform float uWaterLevel; // Mực nước chuẩn hóa (ví dụ: 0.44)
uniform int uColorMode;   // 0: Biomes tự nhiên, 1: Heatmap, 2: Grayscale

// Bảng màu cho Chế độ Biome tự nhiên cao cấp
vec3 getBiomeColor(float t, float w, float slope) {
    vec3 c;
    if (t < w - 0.04) {
        // Đáy biển sâu nhìn xuyên qua làn nước đại dương
        c = vec3(0.14, 0.30, 0.38);
    } else if (t < w + 0.035) {
        // Bãi cát vàng mịn ven biển (Golden sand beach)
        c = vec3(0.88, 0.82, 0.62);
    } else if (t < 0.60) {
        // Đồng cỏ và bình nguyên xanh tươi (Meadow grass)
        float k = clamp((t - (w + 0.035)) / 0.16, 0.0, 1.0);
        c = mix(vec3(0.38, 0.65, 0.30), vec3(0.24, 0.48, 0.22), k);
    } else if (t < 0.78) {
        // Rừng thông & sườn đồi cao (Highland forest / rock)
        float k = (t - 0.60) / 0.18;
        c = mix(vec3(0.24, 0.48, 0.22), vec3(0.55, 0.50, 0.45), k);
    } else {
        // Đỉnh núi băng tuyết trắng xoá (Alpine snow caps)
        float k = clamp((t - 0.78) / 0.12, 0.0, 1.0);
        c = mix(vec3(0.55, 0.50, 0.45), vec3(0.96, 0.97, 0.99), k);
    }

    // Hiệu ứng độ dốc (Slope): Vách dốc đứng lộ ra đá núi trầm tích
    float s = clamp((slope - 0.32) / 0.42, 0.0, 1.0);
    if (t > w + 0.035) {
        c = mix(c, vec3(0.46, 0.42, 0.38), s * 0.85);
    }
    return c;
}

// Bảng màu Heatmap (Đo độ cao nhiệt đồ)
vec3 getHeatmapColor(float t) {
    if (t < 0.25) return mix(vec3(0.0, 0.15, 0.85), vec3(0.0, 0.85, 0.85), t / 0.25);
    if (t < 0.50) return mix(vec3(0.0, 0.85, 0.85), vec3(0.2, 0.85, 0.2), (t - 0.25) / 0.25);
    if (t < 0.75) return mix(vec3(0.2, 0.85, 0.2), vec3(0.95, 0.85, 0.1), (t - 0.50) / 0.25);
    return mix(vec3(0.95, 0.85, 0.1), vec3(0.95, 0.15, 0.15), (t - 0.75) / 0.25);
}

// Bảng màu Grayscale (Hiển thị bản đồ xám của Perlin Noise)
vec3 getGrayscaleColor(float t) {
    return vec3(t * 0.88 + 0.08);
}

void main() {
    vec3 norm = normalize(Normal);
    float slope = 1.0 - norm.y; // Độ dốc: 0 là mặt phẳng, 1 là vách thẳng đứng

    vec3 albedo;
    float specStrength = 0.04;
    float shininess = 32.0;

    if (NormalizedHeight < -0.1) {
        // Tầng địa chất trầm tích bên dưới (Geological strata)
        float band = 0.5 + 0.5 * sin(FragPos.y * 1.5 + 0.8 * sin(FragPos.x * 0.12 + FragPos.z * 0.12));
        albedo = vec3(0.26 + 0.08 * band, 0.18 + 0.05 * band, 0.12);
        specStrength = 0.02;
    } else {
        // Bề mặt địa hình tự nhiên
        if (uColorMode == 0) {
            albedo = getBiomeColor(NormalizedHeight, uWaterLevel, slope);
        } else if (uColorMode == 1) {
            albedo = getHeatmapColor(NormalizedHeight);
        } else {
            albedo = getGrayscaleColor(NormalizedHeight);
        }

        if (NormalizedHeight > 0.80) {
            specStrength = 0.35;
            shininess = 48.0;
        } else if (NormalizedHeight > uWaterLevel && NormalizedHeight < uWaterLevel + 0.04) {
            specStrength = 0.22;
            shininess = 24.0;
        }
    }

    // 2. Mô hình chiếu sáng Blinn-Phong & Ánh sáng bầu trời
    float skyHemisphere = norm.y * 0.5 + 0.5;
    vec3 ambient = mix(uAmbientColor * 0.85, uAmbientColor * 1.15, skyHemisphere) * albedo;

    vec3 lightDir = normalize(-uLightDir);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor * albedo;

    // Phản xạ mặt trời (Specular)
    vec3 viewDir = normalize(uViewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
    vec3 specular = specStrength * spec * uLightColor;

    vec3 finalColor = ambient + diffuse + specular;

    // Sương mù khí quyển nhẹ nhàng nơi chân trời
    vec3 skyHorizon = vec3(0.60, 0.73, 0.88);
    float dist = length(uViewPos - FragPos);
    float fog = clamp((dist - 140.0) / 380.0, 0.0, 0.70);
    finalColor = mix(finalColor, skyHorizon, fog);

    FragColor = vec4(finalColor, 1.0);
}
