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
    float Height;
};

class Terrain {
public:
    int gridWidth;
    int gridHeight;
    float cellSize;

    PresetType currentPreset;
    float noiseScale;
    float heightScale;
    int octaves;
    float persistence;
    float lacunarity;
    float redistribution;

    Terrain(int width = 220, int height = 220, float cell = 1.0f);
    ~Terrain();

    // Sinh lại lưới địa hình theo thuật toán
    void generate(PerlinNoise& noise);

    // Áp dụng các cấu hình mẫu (Presets)
    void applyPreset(PresetType preset, PerlinNoise& noise);

    // Vẽ địa hình
    void draw(const Shader& shader) const;

    float getMinHeight() const { return minHeight; }
    float getMaxHeight() const { return maxHeight; }
    float getLandPercentage(float waterHeight) const;
    int getTotalTriangles() const { return indexCount / 3; }
    int getTotalVertices() const { return static_cast<int>(vertices.size()); }

private:
    unsigned int VAO, VBO, EBO;
    int indexCount;
    float minHeight;
    float maxHeight;

    std::vector<TerrainVertex> vertices;
    std::vector<unsigned int> indices;

    void setupMesh();
};

#endif // TERRAIN_H
