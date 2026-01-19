#pragma once

#include <glad/glad.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <vector>
#include <cmath> 

#include "Shader.h"

const float PI = 3.14159265359f;

class Sun {
public:
    unsigned int VAO, VBO;
    int vertexCount;

    Sun();

    void draw(Shader& shader, glm::vec3 position, glm::mat4 view, glm::mat4 projection);
};