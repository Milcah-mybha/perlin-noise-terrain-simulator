#include "Terrain.h"
#include <cmath>
#include <algorithm>
#include <iostream>

Terrain::Terrain(int width, int height, float cell)
    : gridWidth(width),
      gridHeight(height),
      cellSize(cell),
      currentPreset(PRESET_ROLLING_HILLS),
      noiseScale(0.012f),
      heightScale(32.0f),
      octaves(5),
      persistence(0.48f),
      lacunarity(2.0f),
      redistribution(1.35f),
      VAO(0), VBO(0), EBO(0),
      indexCount(0),
      minHeight(0.0f),
      maxHeight(1.0f) {
}

Terrain::~Terrain() {
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    if (EBO != 0) glDeleteBuffers(1, &EBO);
}

void Terrain::applyPreset(PresetType preset, PerlinNoise& noise) {
    currentPreset = preset;
    switch (preset) {
        case PRESET_ROLLING_HILLS:
            octaves = 5;
            noiseScale = 0.012f;
            heightScale = 30.0f;
            persistence = 0.48f;
            lacunarity = 2.0f;
            redistribution = 1.35f;
            break;
        case PRESET_ALPINE_RIDGES:
            octaves = 6;
            noiseScale = 0.014f;
            heightScale = 45.0f;
            persistence = 0.52f;
            lacunarity = 2.1f;
            redistribution = 1.15f;
            break;
        case PRESET_ISLANDS:
            octaves = 6;
            noiseScale = 0.013f;
            heightScale = 35.0f;
            persistence = 0.48f;
            lacunarity = 2.0f;
            redistribution = 1.30f;
            break;
        case PRESET_SINGLE_OCTAVE:
            octaves = 1;
            noiseScale = 0.009f;
            heightScale = 25.0f;
            persistence = 0.5f;
            lacunarity = 2.0f;
            redistribution = 1.0f;
            break;
        case PRESET_ROUGH_CRAGS:
            octaves = 7;
            noiseScale = 0.022f;
            heightScale = 38.0f;
            persistence = 0.62f;
            lacunarity = 2.2f;
            redistribution = 1.2f;
            break;
        case PRESET_TERRACED:
            octaves = 6;
            noiseScale = 0.012f;
            heightScale = 32.0f;
            persistence = 0.5f;
            lacunarity = 2.0f;
            redistribution = 1.0f;
            break;
    }
    generate(noise);
}

void Terrain::generate(PerlinNoise& noise) {
    vertices.clear();
    indices.clear();

    vertices.resize(gridWidth * gridHeight);
    std::vector<float> heightMap(gridWidth * gridHeight, 0.0f);

    minHeight = 1e9f;
    maxHeight = -1e9f;

    float halfW = (gridWidth - 1) * 0.5f;
    float halfH = (gridHeight - 1) * 0.5f;

    // 1. Tính toán bản đồ độ cao (Height Map)
    for (int z = 0; z < gridHeight; ++z) {
        for (int x = 0; x < gridWidth; ++x) {
            float sampleX = static_cast<float>(x) * noiseScale;
            float sampleZ = static_cast<float>(z) * noiseScale;

            float rawHeight = 0.0f;

            if (currentPreset == PRESET_ALPINE_RIDGES) {
                rawHeight = noise.ridged2D(sampleX, sampleZ, octaves, persistence, lacunarity);
                rawHeight = std::pow(rawHeight, redistribution);
            }
            else if (currentPreset == PRESET_ISLANDS) {
                // Tính khoảng cách chuẩn hóa từ tâm đảo ra mép
                float dx = (x - halfW) / halfW;
                float dz = (z - halfH) / halfH;
                float normDist = std::sqrt(dx * dx + dz * dz);
                rawHeight = noise.island2D(sampleX, sampleZ, normDist, octaves);
                rawHeight = std::pow(rawHeight, redistribution);
            }
            else if (currentPreset == PRESET_TERRACED) {
                rawHeight = noise.terraced2D(sampleX, sampleZ, octaves, 8.0f);
            }
            else if (currentPreset == PRESET_SINGLE_OCTAVE) {
                rawHeight = (noise.noise2D(sampleX, sampleZ) + 1.0f) * 0.5f;
            }
            else {
                rawHeight = noise.fbm2D(sampleX, sampleZ, octaves, persistence, lacunarity);
                rawHeight = std::pow(rawHeight, redistribution);
            }

            float elevation = rawHeight * heightScale;
            heightMap[z * gridWidth + x] = elevation;

            if (elevation < minHeight) minHeight = elevation;
            if (elevation > maxHeight) maxHeight = elevation;
        }
    }

    float offsetX = -(gridWidth - 1) * cellSize * 0.5f;
    float offsetZ = -(gridHeight - 1) * cellSize * 0.5f;

    // 2. Tạo đỉnh và tính Vector pháp tuyến (Normals) bằng sai phân trung tâm
    for (int z = 0; z < gridHeight; ++z) {
        for (int x = 0; x < gridWidth; ++x) {
            int idx = z * gridWidth + x;
            float h = heightMap[idx];

            float worldX = offsetX + x * cellSize;
            float worldZ = offsetZ + z * cellSize;

            int leftX   = std::max(0, x - 1);
            int rightX  = std::min(gridWidth - 1, x + 1);
            int downZ   = std::max(0, z - 1);
            int upZ     = std::min(gridHeight - 1, z + 1);

            float hL = heightMap[z * gridWidth + leftX];
            float hR = heightMap[z * gridWidth + rightX];
            float hD = heightMap[downZ * gridWidth + x];
            float hU = heightMap[upZ * gridWidth + x];

            glm::vec3 normal(hL - hR, 2.0f * cellSize, hD - hU);
            normal = glm::normalize(normal);

            vertices[idx].Position = glm::vec3(worldX, h, worldZ);
            vertices[idx].Normal = normal;
            vertices[idx].TexCoords = glm::vec2(static_cast<float>(x) / (gridWidth - 1),
                                                static_cast<float>(z) / (gridHeight - 1));
            vertices[idx].Height = h;
        }
    }

    // 3. Tạo chỉ số tam giác (Indices / EBO)
    for (int z = 0; z < gridHeight - 1; ++z) {
        for (int x = 0; x < gridWidth - 1; ++x) {
            unsigned int topLeft     = z * gridWidth + x;
            unsigned int topRight    = topLeft + 1;
            unsigned int bottomLeft  = (z + 1) * gridWidth + x;
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
    setupMesh();
}

float Terrain::getLandPercentage(float waterHeight) const {
    if (vertices.empty()) return 100.0f;
    int landCount = 0;
    for (const auto& v : vertices) {
        if (v.Position.y >= waterHeight) {
            landCount++;
        }
    }
    return (static_cast<float>(landCount) / vertices.size()) * 100.0f;
}

void Terrain::setupMesh() {
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

    // 0: Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)0);

    // 1: Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)offsetof(TerrainVertex, Normal));

    // 2: TexCoords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)offsetof(TerrainVertex, TexCoords));

    // 3: Height
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)offsetof(TerrainVertex, Height));

    glBindVertexArray(0);
}

void Terrain::draw(const Shader& shader) const {
    if (VAO == 0 || indexCount == 0) return;
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
