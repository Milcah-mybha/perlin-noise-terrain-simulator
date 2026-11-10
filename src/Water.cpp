#include "Water.h"

Water::Water(float s, float h) : waterHeight(h), size(s), VAO(0), VBO(0), EBO(0), indexCount(0) {
    setupMesh();
}

Water::~Water() {
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    if (EBO != 0) glDeleteBuffers(1, &EBO);
}

void Water::setupMesh() {
    // Lưới phẳng nước với số phân đoạn vừa phải để shader làm sóng nhấp nhô
    const int segments = 60;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float halfSize = size * 0.5f;
    float step = size / segments;

    for (int z = 0; z <= segments; ++z) {
        for (int x = 0; x <= segments; ++x) {
            float vx = -halfSize + x * step;
            float vz = -halfSize + z * step;

            // Vị trí (x, y, z)
            vertices.push_back(vx);
            vertices.push_back(waterHeight);
            vertices.push_back(vz);

            // Vector pháp tuyến ban đầu (0, 1, 0)
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
            vertices.push_back(0.0f);

            // Tọa độ UV
            vertices.push_back(static_cast<float>(x) / segments);
            vertices.push_back(static_cast<float>(z) / segments);
        }
    }

    for (int z = 0; z < segments; ++z) {
        for (int x = 0; x < segments; ++x) {
            unsigned int topLeft     = z * (segments + 1) + x;
            unsigned int topRight    = topLeft + 1;
            unsigned int bottomLeft  = (z + 1) * (segments + 1) + x;
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

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position (3 floats)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    // Normal (3 floats)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    // TexCoords (2 floats)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);
}

void Water::draw(const Shader& shader, float time) const {
    if (VAO == 0) return;
    shader.use();
    shader.setFloat("uTime", time);
    shader.setFloat("uWaterHeight", waterHeight);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
