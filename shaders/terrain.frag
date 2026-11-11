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
uniform float uWaterLevel; // Mực nước chuẩn hóa (ví dụ: 0.52)
uniform int uColorMode;   // 0: Biomes tự nhiên, 1: Heatmap, 2: Grayscale

// Bảng màu cho Chế độ Biome tự nhiên chuẩn xác theo mô hình tham khảo
vec3 getBiomeColor(float t, float w, float slope) {
    vec3 c;
    if (t < w - 0.04) {
        // Đáy biển / Nước sâu nhìn xuyên qua làn nước trong
        c = vec3(0.18, 0.38, 0.45);
    } else if (t < w + 0.035) {
        // Bãi cát vàng ven biển (Sand beach)
        c = vec3(0.88, 0.82, 0.65);
    } else if (t < 0.62) {
        // Thảm cỏ và bình nguyên xanh tươi (Meadow grass)
        float k = clamp((t - (w + 0.035)) / 0.16, 0.0, 1.0);
        c = mix(vec3(0.44, 0.66, 0.38), vec3(0.32, 0.54, 0.26), k);
    } else if (t < 0.80) {
        // Sườn núi đá ấm / Rừng trên cao (Warm mountain slopes)
        float k = (t - 0.62) / 0.18;
        c = mix(vec3(0.32, 0.54, 0.26), vec3(0.72, 0.63, 0.56), k);
    } else {
        // Đỉnh núi tuyết trắng (Alpine Snow)
        float k = clamp((t - 0.80) / 0.10, 0.0, 1.0);
        c = mix(vec3(0.72, 0.63, 0.56), vec3(0.95, 0.96, 0.98), k);
    }

    // Hiệu ứng độ dốc (Slope): Vách dốc chuyển sang màu vách đá xám
    float s = clamp((slope - 0.35) / 0.40, 0.0, 1.0);
    if (t > w + 0.035) {
        c = mix(c, vec3(0.46, 0.40, 0.35), s * 0.85);
    }
    return c;
}

// Bảng màu Heatmap (Đo độ cao nhiệt đồ)
vec3 getHeatmapColor(float t) {
    if (t < 0.25) return mix(vec3(0.0, 0.1, 0.8), vec3(0.0, 0.8, 0.8), t / 0.25);
    if (t < 0.50) return mix(vec3(0.0, 0.8, 0.8), vec3(0.2, 0.85, 0.2), (t - 0.25) / 0.25);
    if (t < 0.75) return mix(vec3(0.2, 0.85, 0.2), vec3(0.95, 0.85, 0.1), (t - 0.50) / 0.25);
    return mix(vec3(0.95, 0.85, 0.1), vec3(0.95, 0.1, 0.1), (t - 0.75) / 0.25);
}

// Bảng màu Grayscale (Hiển thị bản đồ xám của Perlin Noise)
vec3 getGrayscaleColor(float t) {
    return vec3(t * 0.85 + 0.1);
}

void main() {
    vec3 norm = normalize(Normal);
    float slope = 1.0 - norm.y; // Độ dốc: 0 là mặt phẳng, 1 là vách thẳng đứng

    vec3 albedo;
    float specStrength = 0.04;
    float shininess = 32.0;

    // Phân loại vật liệu dựa trên NormalizedHeight
    if (NormalizedHeight < -5.5) {
        // 1. Đèn treo trần triển lãm (Pendant Lamp)
        albedo = vec3(0.12, 0.12, 0.13);
        specStrength = 0.15;
        shininess = 16.0;
    } else if (NormalizedHeight < -4.5) {
        // 2. Tường phòng triển lãm (Gallery Room Walls)
        // Màu xám trung tính của phòng tranh cao cấp, có che tối nhẹ dưới góc chân tường
        float wallAmbient = smoothstep(-28.0, 60.0, FragPos.y) * 0.20 + 0.80;
        albedo = vec3(0.24, 0.25, 0.27) * wallAmbient;
        specStrength = 0.02;
    } else if (NormalizedHeight < -3.5) {
        // 3. Sàn gỗ sồi phòng triển lãm (Wooden Parquet Floor)
        // Tạo vân các thanh ván sàn so le
        float plankW = 4.8;
        float plankL = 28.0;
        float row = floor(FragPos.z / plankW);
        float col = floor((FragPos.x + mod(row, 2.0) * (plankL * 0.5)) / plankL);
        float plankHash = fract(sin(row * 12.9898 + col * 78.233) * 43758.5453);

        vec3 woodBase = mix(vec3(0.48, 0.33, 0.18), vec3(0.38, 0.25, 0.13), plankHash);
        float grain = 0.5 + 0.5 * sin(FragPos.x * 2.5 + sin(FragPos.z * 1.5) * 1.2);
        woodBase += (grain - 0.5) * 0.03;

        // Khe rãnh giữa các thanh ván sàn
        float edgeZ = abs(fract(FragPos.z / plankW) - 0.5);
        float edgeX = abs(fract((FragPos.x + mod(row, 2.0) * (plankL * 0.5)) / plankL) - 0.5);
        if (edgeZ > 0.465 || edgeX > 0.488) {
            woodBase *= 0.45;
        }

        // Vùng bóng tối dưới chân bệ sa bàn
        float distToPlinth = max(abs(FragPos.x) - 76.5, abs(FragPos.z) - 76.5);
        if (distToPlinth > 0.0 && distToPlinth < 25.0) {
            woodBase *= mix(0.65, 1.0, distToPlinth / 25.0);
        }

        albedo = woodBase;
        specStrength = 0.16;
        shininess = 20.0;
    } else if (NormalizedHeight < -2.8) {
        // 4. Biển đồng kỷ niệm khắc chữ "PERLIN NOISE" (Commemorative Brass Plaque)
        vec3 brass = vec3(0.85, 0.70, 0.28);
        float pBorderX = abs(FragPos.x) / 28.0;
        float pBorderY = abs(FragPos.y + 9.5) / 4.5;
        // Viền vát cạnh biển đồng
        if (pBorderX > 0.90 || pBorderY > 0.85) {
            brass *= 0.65;
        }
        // Giả lập rãnh chữ khắc đen bóng
        if (pBorderX < 0.75 && pBorderY < 0.50) {
            float textStripe = sin(FragPos.x * 1.6) * sin(FragPos.y * 3.2);
            if (textStripe > 0.25) brass *= 0.42;
        }
        albedo = brass;
        specStrength = 0.75;
        shininess = 64.0;
    } else if (NormalizedHeight < -1.8) {
        // 5. Bệ đá triển lãm 2 tầng (Stone Plinth)
        float stoneNoise = sin(FragPos.x * 0.25 + FragPos.y * 0.35) * sin(FragPos.z * 0.25);
        albedo = vec3(0.78, 0.77, 0.75) + stoneNoise * 0.02;
        specStrength = 0.06;
    } else if (NormalizedHeight < -1.1) {
        // 6. Trụ góc viền khay bằng đồng mạ vàng (Tray Brass Corner Brackets)
        albedo = vec3(0.82, 0.68, 0.26);
        specStrength = 0.70;
        shininess = 48.0;
    } else if (NormalizedHeight < -0.8) {
        // 7. Khung khay sa bàn bằng đồng tối (Tray Outer Rim)
        albedo = vec3(0.20, 0.16, 0.12);
        specStrength = 0.15;
    } else if (NormalizedHeight < -0.1) {
        // 8. Thành vách đất 4 cạnh bên (Soil strata skirt)
        float band = 0.5 + 0.5 * sin(FragPos.y * 1.8 + 1.2 * sin(FragPos.x * 0.15 + FragPos.z * 0.15));
        albedo = vec3(0.28 + 0.08 * band, 0.19 + 0.05 * band, 0.12);
    } else {
        // 9. Bề mặt địa hình tự nhiên
        if (uColorMode == 0) {
            albedo = getBiomeColor(NormalizedHeight, uWaterLevel, slope);
        } else if (uColorMode == 1) {
            albedo = getHeatmapColor(NormalizedHeight);
        } else {
            albedo = getGrayscaleColor(NormalizedHeight);
        }

        if (NormalizedHeight > 0.80) {
            specStrength = 0.35;
            shininess = 40.0;
        } else if (NormalizedHeight > uWaterLevel && NormalizedHeight < uWaterLevel + 0.04) {
            specStrength = 0.20;
            shininess = 20.0;
        }
    }

    // 2. Mô hình chiếu sáng Blinn-Phong & Ánh sáng môi trường hai tầng
    float skyHemisphere = norm.y * 0.5 + 0.5;
    vec3 ambient = mix(uAmbientColor * 0.85, uAmbientColor * 1.15, skyHemisphere) * albedo;

    vec3 lightDir = normalize(-uLightDir);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor * albedo;

    // Ánh sáng phản xạ (Specular)
    vec3 viewDir = normalize(uViewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
    vec3 specular = specStrength * spec * uLightColor;

    vec3 finalColor = ambient + diffuse + specular;

    FragColor = vec4(finalColor, 1.0);
}
