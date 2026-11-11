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
    float NormalizedHeight; // t trong khoảng 0.0 -> 1.0
};

class Terrain {
public:
    int gridResolution; // Số lượng đỉnh trên mỗi cạnh (180x180)
    float worldSize;    // Kích thước sa bàn (120m)

    PresetType currentPreset;
    float freq;         // Tần số ô lưới trên toàn bộ sa bàn (Lattice cells across)
    float heightScale;  // Chiều cao tối đa (m)
    int octaves;        // Số tầng chi tiết
    float persistence;  // Hệ số suy giảm biên độ
    float lacunarity;   // Hệ số tăng tần số
    float waterLevel;   // Mực nước chuẩn hóa (0.0 -> 1.0)

    Terrain(int resolution = 180, float size = 120.0f);
    ~Terrain();

    // Sinh sa bàn địa hình (gồm lưới bề mặt, thành vách skirt và khung khay sa bàn)
    void generate(PerlinNoise& noise);

    // Áp dụng Preset mẫu chuẩn xác theo mô hình tham khảo
    void applyPreset(PresetType preset, PerlinNoise& noise);

    // Vẽ toàn bộ sa bàn địa hình
    void draw(const Shader& shader) const;

    float getMinHeight() const { return minHeight; }
    float getMaxHeight() const { return maxHeight; }
    float getLandPercentage() const;
    int getTotalTriangles() const { return (indexCount + skirtIndexCount + trayIndexCount) / 3; }
    int getTotalVertices() const { return static_cast<int>(vertices.size() + skirtVertices.size() + trayVertices.size()); }

private:
    unsigned int VAO, VBO, EBO;
    int indexCount;

    // Thành vách đất 4 cạnh bên (Skirt)
    unsigned int skirtVAO, skirtVBO, skirtEBO;
    int skirtIndexCount;

    // Khung viền khay trưng bày sa bàn (Plinth / Tray Frame)
    unsigned int trayVAO, trayVBO, trayEBO;
    int trayIndexCount;

    float minHeight;
    float maxHeight;

    std::vector<TerrainVertex> vertices;
    std::vector<unsigned int> indices;

    std::vector<TerrainVertex> skirtVertices;
    std::vector<unsigned int> skirtIndices;

    std::vector<TerrainVertex> trayVertices;
    std::vector<unsigned int> trayIndices;

    void setupMesh();
    void buildSkirt(const std::vector<float>& heightMap);
    void buildTray();
};

#endif // TERRAIN_H
