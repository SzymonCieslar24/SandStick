#pragma once

#include <glad/glad.h>
#include <glm.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

struct MeshVertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

class HobbyHorseMesh {
public:
    std::vector<MeshVertex> vertices;
    unsigned int VAO, VBO;

    HobbyHorseMesh(const char* path);

    void draw();

private:
    void loadOBJ(const char* path);
    void setupMesh();
};
