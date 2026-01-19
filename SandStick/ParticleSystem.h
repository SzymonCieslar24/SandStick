#pragma once

#include "Shader.h"
#include "SandMesh.h"
#include <glad/glad.h>
#include <glm.hpp>
#include <vector>
#include <cstdlib>

struct Particle {
    glm::vec3 Position;
    glm::vec3 Velocity;
    float Life;
};

class ParticleSystem {
public:
    std::vector<Particle> particles;
    unsigned int VAO, VBO;

    ParticleSystem();

    void spawn(glm::vec3 origin, int count);
    void update(float dt, SandMesh& sandBox);
    void draw(Shader& shader);

};
