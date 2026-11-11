#include "Terrain.h"
#include <cmath>
#include <algorithm>
#include <iostream>

Terrain::Terrain(int resolution, float size)
    : gridResolution(resolution),
      worldSize(size),
      currentPreset(PRESET_ISLANDS),
      freq(2.2f),
      heightScale(24.0f),
      octaves(6),
      persistence(0.50f),
      lacunarity(2.10f),
      waterLevel(0.52f),
      VAO(0), VBO(0), EBO(0),
      indexCount(0),
      skirtVAO(0), skirtVBO(0), skirtEBO(0),
      skirtIndexCount(0),
      trayVAO(0), trayVBO(0), trayEBO(0),
      trayIndexCount(0),
      minHeight(0.0f),
      maxHeight(1.0f) {
}

Terrain::~Terrain() {
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    if (EBO != 0) glDeleteBuffers(1, &EBO);

    if (skirtVAO != 0) glDeleteVertexArrays(1, &skirtVAO);
    if (skirtVBO != 0) glDeleteBuffers(1, &skirtVBO);
    if (skirtEBO != 0) glDeleteBuffers(1, &skirtEBO);

    if (trayVAO != 0) glDeleteVertexArrays(1, &trayVAO);
    if (trayVBO != 0) glDeleteBuffers(1, &trayVBO);
    if (trayEBO != 0) glDeleteBuffers(1, &trayEBO);
}

void Terrain::applyPreset(PresetType preset, PerlinNoise& noise) {
    currentPreset = preset;
    switch (preset) {
        case PRESET_ROLLING_HILLS:
            freq = 2.5f;
            octaves = 5;
            lacunarity = 2.0f;
            persistence = 0.50f;
            heightScale = 26.0f;
            waterLevel = 0.30f;
            break;
        case PRESET_ALPINE_RIDGES:
            freq = 3.2f;
            octaves = 7;
            lacunarity = 2.0f;
            persistence = 0.55f;
            heightScale = 36.0f;
            waterLevel = 0.25f;
            break;
        case PRESET_ISLANDS:
            // Cấu hình chuẩn xác tạo quần đảo tự nhiên như mô hình tham khảo
            freq = 2.2f;
            octaves = 6;
            lacunarity = 2.1f;
            persistence = 0.50f;
            heightScale = 24.0f;
            waterLevel = 0.52f;
            break;
        case PRESET_SINGLE_OCTAVE:
            freq = 3.0f;
            octaves = 1;
            lacunarity = 2.0f;
            persistence = 0.50f;
            heightScale = 24.0f;
            waterLevel = 0.30f;
            break;
        case PRESET_ROUGH_CRAGS:
            freq = 4.0f;
            octaves = 8;
            lacunarity = 2.3f;
            persistence = 0.68f;
            heightScale = 30.0f;
            waterLevel = 0.20f;
            break;
        case PRESET_TERRACED:
            freq = 2.4f;
            octaves = 6;
            lacunarity = 2.0f;
            persistence = 0.50f;
            heightScale = 28.0f;
            waterLevel = 0.35f;
            break;
    }
    generate(noise);
}

void Terrain::generate(PerlinNoise& noise) {
    vertices.clear();
    indices.clear();

    int n = gridResolution;
    vertices.resize(n * n);
    std::vector<float> heightMap(n * n, 0.0f);
    std::vector<float> normHeightMap(n * n, 0.0f);

    minHeight = 1e9f;
    maxHeight = -1e9f;

    // 1. Tính toán bản đồ độ cao theo thuật toán fBm chuẩn mực
    for (int j = 0; j < n; ++j) {
        for (int i = 0; i < n; ++i) {
            float u = static_cast<float>(i) / (n - 1);
            float v = static_cast<float>(j) / (n - 1);

            float sampleX = u * freq;
            float sampleZ = v * freq;

            float raw = 0.0f;
            if (currentPreset == PRESET_ALPINE_RIDGES) {
                raw = noise.ridged2D(sampleX, sampleZ, octaves, persistence, lacunarity);
            } else if (currentPreset == PRESET_TERRACED) {
                raw = noise.terraced2D(sampleX, sampleZ, octaves, 8.0f);
            } else if (currentPreset == PRESET_SINGLE_OCTAVE) {
                raw = (noise.noise2D(sampleX, sampleZ) + 1.0f) * 0.5f;
            } else {
                raw = noise.fbm2D(sampleX, sampleZ, octaves, persistence, lacunarity);
            }

            // Kỹ thuật tương phản độ cao (Disp function): Đẩy độ cao tương phản tự nhiên
            float t = std::clamp(0.5f + (raw - 0.5f) * 1.5f, 0.0f, 1.0f);
            float elevation = t * heightScale;

            int idx = j * n + i;
            heightMap[idx] = elevation;
            normHeightMap[idx] = t;

            if (elevation < minHeight) minHeight = elevation;
            if (elevation > maxHeight) maxHeight = elevation;
        }
    }

    float halfSize = worldSize * 0.5f;
    float step = worldSize / (n - 1);

    // 2. Tạo đỉnh và tính Vector pháp tuyến (Normals) bằng sai phân trung tâm
    for (int j = 0; j < n; ++j) {
        for (int i = 0; i < n; ++i) {
            int idx = j * n + i;
            float h = heightMap[idx];
            float t = normHeightMap[idx];

            float worldX = -halfSize + i * step;
            float worldZ = -halfSize + j * step;

            int leftI  = std::max(0, i - 1);
            int rightI = std::min(n - 1, i + 1);
            int downJ  = std::max(0, j - 1);
            int upJ    = std::min(n - 1, j + 1);

            float hL = heightMap[j * n + leftI];
            float hR = heightMap[j * n + rightI];
            float hD = heightMap[downJ * n + i];
            float hU = heightMap[upJ * n + i];

            glm::vec3 normal(hL - hR, 2.0f * step, hD - hU);
            normal = glm::normalize(normal);

            vertices[idx].Position = glm::vec3(worldX, h, worldZ);
            vertices[idx].Normal = normal;
            vertices[idx].TexCoords = glm::vec2(static_cast<float>(i) / (n - 1), static_cast<float>(j) / (n - 1));
            vertices[idx].NormalizedHeight = t;
        }
    }

    // 3. Tạo chỉ số tam giác EBO cho bề mặt địa hình
    for (int j = 0; j < n - 1; ++j) {
        for (int i = 0; i < n - 1; ++i) {
            unsigned int topLeft     = j * n + i;
            unsigned int topRight    = topLeft + 1;
            unsigned int bottomLeft  = (j + 1) * n + i;
            unsigned int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    indexCount = static_cast<int>(indices.size());

    // 4. Tạo thành vách 4 cạnh bên (Skirt) và Khung khay trưng bày sa bàn
    buildSkirt(heightMap);
    buildTray();

    setupMesh();
}

void Terrain::buildSkirt(const std::vector<float>& heightMap) {
    skirtVertices.clear();
    skirtIndices.clear();

    int n = gridResolution;
    float baseY = -1.0f; // Đáy thành vách ăn xuống dưới khay

    // Tạo 4 cạnh thành vách
    auto getIdx = [n](int edge, int i) -> int {
        if (edge == 0) return 0 * n + i;           // Cạnh Bắc (Z âm)
        if (edge == 1) return (n - 1) * n + i;     // Cạnh Nam (Z dương)
        if (edge == 2) return i * n + 0;           // Cạnh Tây (X âm)
        return i * n + (n - 1);                    // Cạnh Đông (X dương)
    };

    glm::vec3 normals[4] = {
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, 0.0f,  1.0f),
        glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3( 1.0f, 0.0f, 0.0f)
    };

    int vertOffset = 0;
    for (int e = 0; e < 4; ++e) {
        for (int i = 0; i < n; ++i) {
            int k = getIdx(e, i);
            glm::vec3 topPos = vertices[k].Position;
            glm::vec3 botPos = glm::vec3(topPos.x, baseY, topPos.z);

            // Đỉnh trên
            TerrainVertex vTop;
            vTop.Position = topPos;
            vTop.Normal = normals[e];
            vTop.TexCoords = glm::vec2(static_cast<float>(i) / (n - 1), 1.0f);
            vTop.NormalizedHeight = -0.5f; // Đánh dấu thành vách đất trầm tích
            skirtVertices.push_back(vTop);

            // Đỉnh dưới
            TerrainVertex vBot;
            vBot.Position = botPos;
            vBot.Normal = normals[e];
            vBot.TexCoords = glm::vec2(static_cast<float>(i) / (n - 1), 0.0f);
            vBot.NormalizedHeight = -0.8f;
            skirtVertices.push_back(vBot);
        }

        for (int i = 0; i < n - 1; ++i) {
            unsigned int a = vertOffset + i * 2;
            unsigned int b = a + 1;
            unsigned int c = a + 2;
            unsigned int d = a + 3;

            skirtIndices.push_back(a);
            skirtIndices.push_back(b);
            skirtIndices.push_back(c);

            skirtIndices.push_back(c);
            skirtIndices.push_back(b);
            skirtIndices.push_back(d);
        }
        vertOffset += n * 2;
    }

    skirtIndexCount = static_cast<int>(skirtIndices.size());
}

void Terrain::buildTray() {
    trayVertices.clear();
    trayIndices.clear();

    float half = worldSize * 0.5f; // 60.0f

    // Hàm tiện ích thêm hình hộp chữ nhật (Box)
    auto addBox = [this](float minX, float maxX, float minY, float maxY, float minZ, float maxZ, float tag) {
        auto addQuad = [this, tag](glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, glm::vec3 norm) {
            int baseIdx = static_cast<int>(trayVertices.size());
            TerrainVertex v1, v2, v3, v4;
            v1.Position = p1; v1.Normal = norm; v1.TexCoords = glm::vec2(0, 0); v1.NormalizedHeight = tag;
            v2.Position = p2; v2.Normal = norm; v2.TexCoords = glm::vec2(1, 0); v2.NormalizedHeight = tag;
            v3.Position = p3; v3.Normal = norm; v3.TexCoords = glm::vec2(1, 1); v3.NormalizedHeight = tag;
            v4.Position = p4; v4.Normal = norm; v4.TexCoords = glm::vec2(0, 1); v4.NormalizedHeight = tag;

            trayVertices.push_back(v1);
            trayVertices.push_back(v2);
            trayVertices.push_back(v3);
            trayVertices.push_back(v4);

            trayIndices.push_back(baseIdx + 0);
            trayIndices.push_back(baseIdx + 1);
            trayIndices.push_back(baseIdx + 2);
            trayIndices.push_back(baseIdx + 0);
            trayIndices.push_back(baseIdx + 2);
            trayIndices.push_back(baseIdx + 3);
        };

        // 6 mặt của hộp
        addQuad(glm::vec3(minX, maxY, maxZ), glm::vec3(maxX, maxY, maxZ), glm::vec3(maxX, maxY, minZ), glm::vec3(minX, maxY, minZ), glm::vec3(0, 1, 0));  // Top
        addQuad(glm::vec3(minX, minY, minZ), glm::vec3(maxX, minY, minZ), glm::vec3(maxX, minY, maxZ), glm::vec3(minX, minY, maxZ), glm::vec3(0, -1, 0)); // Bottom
        addQuad(glm::vec3(minX, minY, maxZ), glm::vec3(maxX, minY, maxZ), glm::vec3(maxX, maxY, maxZ), glm::vec3(minX, maxY, maxZ), glm::vec3(0, 0, 1));  // Front (+Z)
        addQuad(glm::vec3(maxX, minY, minZ), glm::vec3(minX, minY, minZ), glm::vec3(minX, maxY, minZ), glm::vec3(maxX, maxY, minZ), glm::vec3(0, 0, -1)); // Back (-Z)
        addQuad(glm::vec3(minX, minY, minZ), glm::vec3(minX, minY, maxZ), glm::vec3(minX, maxY, maxZ), glm::vec3(minX, maxY, minZ), glm::vec3(-1, 0, 0)); // Left (-X)
        addQuad(glm::vec3(maxX, minY, maxZ), glm::vec3(maxX, minY, minZ), glm::vec3(maxX, maxY, minZ), glm::vec3(maxX, maxY, maxZ), glm::vec3(1, 0, 0));  // Right (+X)
    };

    // Hàm thêm mặt phẳng đơn lẻ (Quad)
    auto addQuadFace = [this](glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, glm::vec3 norm, float tag) {
        int baseIdx = static_cast<int>(trayVertices.size());
        TerrainVertex v1, v2, v3, v4;
        v1.Position = p1; v1.Normal = norm; v1.TexCoords = glm::vec2(0, 0); v1.NormalizedHeight = tag;
        v2.Position = p2; v2.Normal = norm; v2.TexCoords = glm::vec2(1, 0); v2.NormalizedHeight = tag;
        v3.Position = p3; v3.Normal = norm; v3.TexCoords = glm::vec2(1, 1); v3.NormalizedHeight = tag;
        v4.Position = p4; v4.Normal = norm; v4.TexCoords = glm::vec2(0, 1); v4.NormalizedHeight = tag;

        trayVertices.push_back(v1);
        trayVertices.push_back(v2);
        trayVertices.push_back(v3);
        trayVertices.push_back(v4);

        trayIndices.push_back(baseIdx + 0);
        trayIndices.push_back(baseIdx + 1);
        trayIndices.push_back(baseIdx + 2);
        trayIndices.push_back(baseIdx + 0);
        trayIndices.push_back(baseIdx + 2);
        trayIndices.push_back(baseIdx + 3);
    };

    // ================= 1. KHAY ĐỒNG CHỨA SA BÀN (TRAY FRAME) =================
    // Gờ viền khay đồng cao 14.5 để chứa trọn mặt nước biển bên trong
    float trayBorder = 3.5f;
    float trayOut = half + trayBorder; // 63.5f
    float rimY = 14.5f;
    float trayBotY = 0.0f;

    // 4 thanh viền khay bằng đồng tối (tag = -1.0f)
    addBox(-trayOut,  trayOut, trayBotY, rimY,  half,  trayOut, -1.0f); // Thanh trước (+Z)
    addBox(-trayOut,  trayOut, trayBotY, rimY, -trayOut, -half, -1.0f); // Thanh sau (-Z)
    addBox(-trayOut, -half,    trayBotY, rimY, -half,     half, -1.0f); // Thanh trái (-X)
    addBox( half,     trayOut, trayBotY, rimY, -half,     half, -1.0f); // Thanh phải (+X)

    // 4 trụ góc bằng đồng thau mạ vàng có hoa văn nổi (tag = -1.3f)
    float cSize = 4.2f;
    float cHeight = rimY + 1.2f; // Cao hơn gờ khay 1.2 đơn vị
    addBox(-trayOut - 0.4f, -trayOut + cSize, trayBotY, cHeight,  trayOut - cSize,  trayOut + 0.4f, -1.3f); // Góc trước trái
    addBox( trayOut - cSize,  trayOut + 0.4f, trayBotY, cHeight,  trayOut - cSize,  trayOut + 0.4f, -1.3f); // Góc trước phải
    addBox(-trayOut - 0.4f, -trayOut + cSize, trayBotY, cHeight, -trayOut - 0.4f, -trayOut + cSize, -1.3f); // Góc sau trái
    addBox( trayOut - cSize,  trayOut + 0.4f, trayBotY, cHeight, -trayOut - 0.4f, -trayOut + cSize, -1.3f); // Góc sau phải

    // ================= 2. BỆ ĐÁ TRƯNG BÀY TRIỂN LÃM (PLINTH) =================
    // Bệ đá trắng 2 tầng kiến trúc chuẩn Blender-modelled plinth
    // Tầng trên (Upper Tier): tag = -2.0f
    float plinthTopHalf = half + 7.5f; // 67.5f
    addBox(-plinthTopHalf, plinthTopHalf, -18.0f, 0.0f, -plinthTopHalf, plinthTopHalf, -2.0f);

    // Biển tên đồng khắc chữ "PERLIN NOISE" trên mặt trước bệ đá (tag = -3.0f)
    float plaqueW = 28.0f;
    float plaqueMinY = -14.0f;
    float plaqueMaxY = -5.0f;
    float plaqueZ = plinthTopHalf + 0.35f;
    addQuadFace(glm::vec3(-plaqueW, plaqueMinY, plaqueZ), glm::vec3(plaqueW, plaqueMinY, plaqueZ),
                glm::vec3(plaqueW, plaqueMaxY, plaqueZ),  glm::vec3(-plaqueW, plaqueMaxY, plaqueZ),
                glm::vec3(0, 0, 1), -3.0f);

    // Tầng bệ dưới mở rộng (Lower Tier Base): tag = -2.5f
    float plinthBotHalf = half + 16.5f; // 76.5f
    addBox(-plinthBotHalf, plinthBotHalf, -28.0f, -18.0f, -plinthBotHalf, plinthBotHalf, -2.5f);

    // ================= 3. SÀN GỖ PHÒNG TRIỂN LÃM (WOODEN PARQUET FLOOR) =================
    // Sàn gỗ tại Y = -28.0f (tag = -4.0f)
    float floorHalf = 240.0f;
    addQuadFace(glm::vec3(-floorHalf, -28.0f,  floorHalf), glm::vec3( floorHalf, -28.0f,  floorHalf),
                glm::vec3( floorHalf, -28.0f, -floorHalf), glm::vec3(-floorHalf, -28.0f, -floorHalf),
                glm::vec3(0, 1, 0), -4.0f);

    // ================= 4. TƯỜNG PHÒNG TRIỂN LÃM (GALLERY ROOM WALLS) =================
    // Hai bức tường gặp nhau tại góc phòng phía sau (tag = -5.0f)
    float wallDist = 210.0f;
    float wallTop = 160.0f;
    float wallBot = -28.0f;

    // Tường sau (Back Wall, Z = -wallDist, hướng +Z)
    addQuadFace(glm::vec3(-wallDist, wallBot, -wallDist), glm::vec3(wallDist, wallBot, -wallDist),
                glm::vec3(wallDist, wallTop, -wallDist), glm::vec3(-wallDist, wallTop, -wallDist),
                glm::vec3(0, 0, 1), -5.0f);

    // Tường trái (Left Wall, X = -wallDist, hướng +X)
    addQuadFace(glm::vec3(-wallDist, wallBot,  wallDist), glm::vec3(-wallDist, wallBot, -wallDist),
                glm::vec3(-wallDist, wallTop, -wallDist), glm::vec3(-wallDist, wallTop,  wallDist),
                glm::vec3(1, 0, 0), -5.0f);

    // ================= 5. ĐÈN TREO TRẦN TRIỂN LÃM (PENDANT LAMP) =================
    // Dây treo đèn và chao đèn đen tối giản ngay phía trên sa bàn (tag = -6.0f)
    // Dây treo từ trần nhà
    addBox(-0.35f, 0.35f, 75.0f, wallTop, -0.35f, 0.35f, -6.0f);
    // Chao đèn
    addBox(-4.5f, 4.5f, 65.0f, 75.0f, -4.5f, 4.5f, -6.0f);

    trayIndexCount = static_cast<int>(trayIndices.size());
}

float Terrain::getLandPercentage() const {
    if (vertices.empty()) return 50.0f;
    int landCount = 0;
    for (const auto& v : vertices) {
        if (v.NormalizedHeight > waterLevel) {
            landCount++;
        }
    }
    return (static_cast<float>(landCount) / vertices.size()) * 100.0f;
}

void Terrain::setupMesh() {
    // 1. VAO Bề mặt địa hình
    if (VAO == 0) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
    }
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(TerrainVertex), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)offsetof(TerrainVertex, Normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)offsetof(TerrainVertex, TexCoords));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)offsetof(TerrainVertex, NormalizedHeight));
    glBindVertexArray(0);

    // 2. VAO Thành vách 4 cạnh bên (Skirt)
    if (skirtVAO == 0) {
        glGenVertexArrays(1, &skirtVAO);
        glGenBuffers(1, &skirtVBO);
        glGenBuffers(1, &skirtEBO);
    }
    glBindVertexArray(skirtVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skirtVBO);
    glBufferData(GL_ARRAY_BUFFER, skirtVertices.size() * sizeof(TerrainVertex), skirtVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skirtEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, skirtIndices.size() * sizeof(unsigned int), skirtIndices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)offsetof(TerrainVertex, Normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)offsetof(TerrainVertex, TexCoords));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)offsetof(TerrainVertex, NormalizedHeight));
    glBindVertexArray(0);

    // 3. VAO Khung viền khay sa bàn (Tray Frame)
    if (trayVAO == 0) {
        glGenVertexArrays(1, &trayVAO);
        glGenBuffers(1, &trayVBO);
        glGenBuffers(1, &trayEBO);
    }
    glBindVertexArray(trayVAO);
    glBindBuffer(GL_ARRAY_BUFFER, trayVBO);
    glBufferData(GL_ARRAY_BUFFER, trayVertices.size() * sizeof(TerrainVertex), trayVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, trayEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, trayIndices.size() * sizeof(unsigned int), trayIndices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)offsetof(TerrainVertex, Normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)offsetof(TerrainVertex, TexCoords));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)offsetof(TerrainVertex, NormalizedHeight));
    glBindVertexArray(0);
}

void Terrain::draw(const Shader& shader) const {
    if (VAO == 0 || indexCount == 0) return;

    // Vẽ bề mặt địa hình
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

    // Vẽ thành vách 4 cạnh bên (đất trầm tích)
    if (skirtVAO != 0 && skirtIndexCount > 0) {
        glBindVertexArray(skirtVAO);
        glDrawElements(GL_TRIANGLES, skirtIndexCount, GL_UNSIGNED_INT, 0);
    }

    // Vẽ khung viền khay sa bàn gỗ
    if (trayVAO != 0 && trayIndexCount > 0) {
        glBindVertexArray(trayVAO);
        glDrawElements(GL_TRIANGLES, trayIndexCount, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
}
