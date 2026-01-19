#pragma once

#include <vector>
#include <glm.hpp>
#include <glad/glad.h>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

class GridMesh {
protected:
    int width, depth;
    float spacing;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO, VBO, EBO;

    void generateFlatGrid();
    void setupBuffers();

public:
    GridMesh(int w, int d, float s);
    virtual ~GridMesh();

    void draw();

    void updateBuffers();
};