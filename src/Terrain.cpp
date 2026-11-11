#include "Terrain.h"
#include <cmath>
#include <algorithm>

Terrain::Terrain(int resolution, float size)
    : gridResolution(resolution),
      worldSize(size),
      currentPreset(PRESET_ISLANDS),
      freq(2.4f),
      heightScale(26.0f),
      octaves(6),
      persistence(0.50f),
      lacunarity(2.1f),
      waterLevel(0.44f),
      VAO(0), VBO(0), EBO(0), indexCount(0),
      skirtVAO(0), skirtVBO(0), skirtEBO(0), skirtIndexCount(0),
      minHeight(0.0f), maxHeight(30.0f) {
}

Terrain::~Terrain() {
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    if (EBO != 0) glDeleteBuffers(1, &EBO);

    if (skirtVAO != 0) glDeleteVertexArrays(1, &skirtVAO);
    if (skirtVBO != 0) glDeleteBuffers(1, &skirtVBO);
    if (skirtEBO != 0) glDeleteBuffers(1, &skirtEBO);
}

void Terrain::applyPreset(PresetType preset, PerlinNoise& noise) {
    currentPreset = preset;
    switch (preset) {
        case PRESET_ISLANDS:
            freq = 2.2f;
            octaves = 6;
            lacunarity = 2.1f;
            persistence = 0.50f;
            heightScale = 30.0f;
            waterLevel = 0.35f;
            break;
        case PRESET_ROLLING_HILLS:
            freq = 2.0f;
            octaves = 5;
            lacunarity = 2.0f;
            persistence = 0.45f;
            heightScale = 22.0f;
            waterLevel = 0.22f;
            break;
        case PRESET_ALPINE_RIDGES:
            freq = 3.0f;
            octaves = 7;
            lacunarity = 2.1f;
            persistence = 0.55f;
            heightScale = 38.0f;
            waterLevel = 0.20f;
            break;
        case PRESET_TERRACED:
            freq = 2.2f;
            octaves = 6;
            lacunarity = 2.0f;
            persistence = 0.50f;
            heightScale = 30.0f;
            waterLevel = 0.28f;
            break;
        case PRESET_VOLCANO:
            freq = 2.8f;
            octaves = 6;
            lacunarity = 2.0f;
            persistence = 0.50f;
            heightScale = 32.0f;
            waterLevel = 0.35f;
            break;
        case PRESET_SINGLE_OCTAVE:
            freq = 2.5f;
            octaves = 1;
            lacunarity = 2.0f;
            persistence = 0.50f;
            heightScale = 20.0f;
            waterLevel = 0.30f;
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

    for (int j = 0; j < n; ++j) {
        for (int i = 0; i < n; ++i) {
            float u = static_cast<float>(i) / (n - 1);
            float v = static_cast<float>(j) / (n - 1);

            float sampleX = u * freq;
            float sampleZ = v * freq;

            float dx = (u - 0.5f) * 2.0f;
            float dz = (v - 0.5f) * 2.0f;
            float distFromCenter = std::sqrt(dx * dx + dz * dz);

            float t = 0.0f;

            if (currentPreset == PRESET_ISLANDS) {
                t = noise.island2D(sampleX, sampleZ, distFromCenter, octaves, persistence, lacunarity);
            } else if (currentPreset == PRESET_VOLCANO) {
                float cone = std::clamp(1.0f - distFromCenter * 1.25f, 0.0f, 1.0f);
                float crater = std::clamp((0.24f - distFromCenter) * 4.2f, 0.0f, 1.0f);
                float volcano = std::max(0.0f, cone - crater * 0.75f);
                float fbm = noise.fbm2D(sampleX, sampleZ, octaves, persistence, lacunarity);
                t = std::clamp(volcano * 0.82f + (fbm - 0.5f) * 0.32f, 0.0f, 1.0f);
            } else if (currentPreset == PRESET_ALPINE_RIDGES) {
                float ridged = noise.ridged2D(sampleX, sampleZ, octaves, persistence, lacunarity);
                t = std::clamp(ridged * 1.05f, 0.0f, 1.0f);
            } else if (currentPreset == PRESET_TERRACED) {
                t = noise.terraced2D(sampleX, sampleZ, octaves, 8.0f);
            } else if (currentPreset == PRESET_SINGLE_OCTAVE) {
                t = (noise.noise2D(sampleX, sampleZ) + 1.0f) * 0.5f;
            } else {
                t = noise.fbm2D(sampleX, sampleZ, octaves, persistence, lacunarity);
            }

            t = std::clamp(t, 0.0f, 1.0f);
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

            float dx = (rightI - leftI) * step;
            if (dx < 0.001f) dx = step * 2.0f;

            glm::vec3 normal = glm::normalize(glm::vec3(hL - hR, dx, hD - hU));

            vertices[idx].Position = glm::vec3(worldX, h, worldZ);
            vertices[idx].Normal = normal;
            vertices[idx].TexCoords = glm::vec2(static_cast<float>(i) / (n - 1), static_cast<float>(j) / (n - 1));
            vertices[idx].NormalizedHeight = t;
        }
    }

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

    buildSkirt(heightMap);
    setupMesh();
}

void Terrain::buildSkirt(const std::vector<float>& heightMap) {
    skirtVertices.clear();
    skirtIndices.clear();

    int n = gridResolution;
    float baseY = -6.0f;

    auto getIdx = [n](int edge, int i) -> int {
        if (edge == 0) return 0 * n + i;
        if (edge == 1) return (n - 1) * n + i;
        if (edge == 2) return i * n + 0;
        return i * n + (n - 1);
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

            TerrainVertex vTop;
            vTop.Position = topPos;
            vTop.Normal = normals[e];
            vTop.TexCoords = glm::vec2(static_cast<float>(i) / (n - 1), 1.0f);
            vTop.NormalizedHeight = -0.5f;
            skirtVertices.push_back(vTop);

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
}

void Terrain::draw(const Shader& shader) const {
    if (VAO == 0 || indexCount == 0) return;

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

    if (skirtVAO != 0 && skirtIndexCount > 0) {
        glBindVertexArray(skirtVAO);
        glDrawElements(GL_TRIANGLES, skirtIndexCount, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
}
