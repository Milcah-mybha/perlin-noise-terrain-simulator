// Định nghĩa lớp Water quản lý mặt phẳng nước và sóng biển
#ifndef WATER_H
#define WATER_H

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Shader.h"

class Water {
public:
    float waterHeight;
    float size;

    Water(float size = 420.0f, float height = 6.0f);
    ~Water();

    // Vẽ mặt nước đại dương
    void draw(const Shader& shader, float time) const;

private:
    unsigned int VAO, VBO, EBO;
    int indexCount;

    void setupMesh();
};

#endif // WATER_H
