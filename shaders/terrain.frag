#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in float NormalizedHeight;

out vec4 FragColor;

uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uAmbientColor;
uniform vec3 uViewPos;

uniform float uWaterLevel;
uniform int uColorMode;

// Phan chia mau sinh thai (cat, co, rung, da, tuyet) theo do cao va do doc
vec3 getBiomeColor(float t, float w, float slope) {
    vec3 deepWater   = vec3(0.08, 0.26, 0.38);
    vec3 shallowSand = vec3(0.86, 0.80, 0.62);
    vec3 grassLush   = vec3(0.32, 0.60, 0.24);
    vec3 forestDark  = vec3(0.20, 0.44, 0.18);
    vec3 rockGray    = vec3(0.48, 0.45, 0.42);
    vec3 snowWhite   = vec3(0.96, 0.97, 0.99);

    vec3 c;
    if (t < w) {
        float depth = clamp((w - t) / 0.12, 0.0, 1.0);
        c = mix(shallowSand * 0.72, deepWater, depth);
    } else {
        float alt = t - w;
        if (alt < 0.04) {
            float k = alt / 0.04;
            c = mix(shallowSand, grassLush, smoothstep(0.0, 1.0, k));
        } else if (alt < 0.32) {
            float k = (alt - 0.04) / 0.28;
            c = mix(grassLush, forestDark, smoothstep(0.0, 1.0, k));
        } else if (alt < 0.52) {
            float k = (alt - 0.32) / 0.20;
            c = mix(forestDark, rockGray, smoothstep(0.0, 1.0, k));
        } else {
            float k = clamp((alt - 0.52) / 0.18, 0.0, 1.0);
            c = mix(rockGray, snowWhite, smoothstep(0.0, 1.0, k));
        }

        float cliffFactor = smoothstep(0.25, 0.65, slope);
        c = mix(c, rockGray, cliffFactor * 0.88);
    }
    return c;
}

// Bang mau nhiet do cao (Heatmap)
vec3 getHeatmapColor(float t) {
    if (t < 0.25) return mix(vec3(0.0, 0.15, 0.85), vec3(0.0, 0.85, 0.85), t / 0.25);
    if (t < 0.50) return mix(vec3(0.0, 0.85, 0.85), vec3(0.2, 0.85, 0.2), (t - 0.25) / 0.25);
    if (t < 0.75) return mix(vec3(0.2, 0.85, 0.2), vec3(0.95, 0.85, 0.1), (t - 0.50) / 0.25);
    return mix(vec3(0.95, 0.85, 0.1), vec3(0.95, 0.15, 0.15), (t - 0.75) / 0.25);
}

// Bang mau thang xam (Grayscale)
vec3 getGrayscaleColor(float t) {
    return vec3(t * 0.88 + 0.08);
}

void main() {
    vec3 norm = normalize(Normal);
    float slope = 1.0 - norm.y;

    vec3 albedo;
    float specStrength = 0.04;
    float shininess = 32.0;

    if (NormalizedHeight < -0.1) {
        float band = 0.5 + 0.5 * sin(FragPos.y * 1.5 + 0.8 * sin(FragPos.x * 0.12 + FragPos.z * 0.12));
        albedo = vec3(0.26 + 0.08 * band, 0.18 + 0.05 * band, 0.12);
        specStrength = 0.02;
    } else {
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

    // Chieu sang Blinn-Phong va anh sang moi truong ban cau
    float skyHemisphere = norm.y * 0.5 + 0.5;
    vec3 ambient = mix(uAmbientColor * 0.85, uAmbientColor * 1.15, skyHemisphere) * albedo;

    vec3 lightDir = normalize(-uLightDir);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor * albedo;

    vec3 viewDir = normalize(uViewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
    vec3 specular = specStrength * spec * uLightColor;

    vec3 finalColor = ambient + diffuse + specular;

    vec3 skyHorizon = vec3(0.60, 0.73, 0.88);
    float dist = length(uViewPos - FragPos);
    float fog = clamp((dist - 140.0) / 380.0, 0.0, 0.70);
    finalColor = mix(finalColor, skyHorizon, fog);

    FragColor = vec4(finalColor, 1.0);
}
