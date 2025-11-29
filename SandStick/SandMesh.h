#pragma once

#include <vector>
#include <cmath>
#include <glm.hpp>
#include <glad/glad.h> // Upewnij siê, ¿e masz to lub GLEW

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
};

class SandMesh {
public:
    int width, depth;
    float spacing;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO, VBO, EBO;

    SandMesh(int w, int d, float s);

    void draw();

    void deform(glm::vec3 hitPoint, float radius, float strength);

private:
    void generateMesh();

    void recalculateNormals();

    void setupBuffers();

    void updateBuffers();
};