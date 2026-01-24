#include "ParticleSystem.h"
#include "SandMesh.h"

ParticleSystem::ParticleSystem() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 10000, NULL, GL_STREAM_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glBindVertexArray(0);
}

void ParticleSystem::spawn(glm::vec3 origin, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.Position = origin;

        float rX = ((rand() % 100) - 50) / 100.0f;
        float rZ = ((rand() % 100) - 50) / 100.0f;
        float rY = ((rand() % 100) / 100.0f);

        p.Velocity = glm::vec3(rX, rY, rZ) * 2.0f;
        p.Life = 1.5f;

        particles.push_back(p);
    }
}

void ParticleSystem::update(float dt, SandMesh& sandBox) {
    for (int i = 0; i < particles.size(); ++i) {
        Particle& p = particles[i];

        p.Life -= dt;
        p.Velocity.y -= 9.8f * dt;
        p.Position += p.Velocity * dt;

        float groundHeight = sandBox.getHeight(p.Position.x, p.Position.z);

        if (p.Position.y <= groundHeight) {
            sandBox.deform(p.Position, 1.0f, 0.01f);

            p.Life = -1.5f;
        }
    }

    particles.erase(std::remove_if(particles.begin(), particles.end(),
        [](const Particle& p) { return p.Life <= 0.0f; }),
        particles.end());
}

void ParticleSystem::draw(Shader& shader) {
    if (particles.empty()) return;

    shader.use();

    std::vector<glm::vec3> positions;
    for (const auto& p : particles) {
        positions.push_back(p.Position);
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), positions.data());

    glDrawArrays(GL_POINTS, 0, positions.size());
    glBindVertexArray(0);
}