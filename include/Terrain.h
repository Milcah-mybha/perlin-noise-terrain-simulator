// Định nghĩa lớp Terrain quản lý dữ liệu đỉnh và hiển thị địa hình
#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "PerlinNoise.h"
#include "Shader.h"

struct TerrainVertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    float NormalizedHeight;
};

class Terrain {
public:
    int gridResolution;
    float worldSize;

    PresetType currentPreset;
    float freq;
    float heightScale;
    int octaves;
    float persistence;
    float lacunarity;
    float waterLevel;

    Terrain(int resolution = 180, float size = 120.0f);
    ~Terrain();

    // Sinh bản đồ độ cao và dựng lưới 3D
    void generate(PerlinNoise& noise);

    // Áp dụng bộ thông số địa hình mẫu
    void applyPreset(PresetType preset, PerlinNoise& noise);

    // Vẽ địa hình lên GPU
    void draw(const Shader& shader) const;

    float getMinHeight() const { return minHeight; }
    float getMaxHeight() const { return maxHeight; }
    float getLandPercentage() const;
    int getTotalTriangles() const { return (indexCount + skirtIndexCount) / 3; }

private:
    unsigned int VAO, VBO, EBO;
    int indexCount;

    unsigned int skirtVAO, skirtVBO, skirtEBO;
    int skirtIndexCount;

    float minHeight;
    float maxHeight;

    std::vector<TerrainVertex> vertices;
    std::vector<unsigned int> indices;

    std::vector<TerrainVertex> skirtVertices;
    std::vector<unsigned int> skirtIndices;

    void setupMesh();
    void buildSkirt(const std::vector<float>& heightMap);
};

#endif // TERRAIN_H
