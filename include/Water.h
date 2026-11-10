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

    Water(float size = 300.0f, float height = 6.0f);
    ~Water();

    void draw(const Shader& shader, float time) const;

private:
    unsigned int VAO, VBO, EBO;
    int indexCount;

    void setupMesh();
};

#endif // WATER_H
