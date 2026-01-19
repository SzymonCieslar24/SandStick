#include "ParticleSystem.h"
#include "SandMesh.h"

ParticleSystem::ParticleSystem() {
    // Konfiguracja buforów
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Rezerwujemy pamiêæ na max 10,000 cz¹steczek
    // GL_DYNAMIC_DRAW lub GL_STREAM_DRAW, bo dane zmieniamy co klatkê
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 10000, NULL, GL_STREAM_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glBindVertexArray(0);
}

// Dodaj nowe cz¹steczki
void ParticleSystem::spawn(glm::vec3 origin, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.Position = origin;

        // Losowy rozrzut (Velocity)
        float rX = ((rand() % 100) - 50) / 100.0f; // -0.5 do 0.5
        float rZ = ((rand() % 100) - 50) / 100.0f;
        float rY = ((rand() % 100) / 100.0f);      // 0.0 do 1.0 (lekki wyrzut w górê?)

        p.Velocity = glm::vec3(rX, rY, rZ) * 2.0f; // Prêdkoœæ pocz¹tkowa
        p.Life = 1.5f; // Czas ¿ycia (1 sekunda)

        particles.push_back(p);
    }
}

// Aktualizuj fizykê
void ParticleSystem::update(float dt, SandMesh& sandBox) {
    for (int i = 0; i < particles.size(); ++i) {
        Particle& p = particles[i];

        p.Life -= dt;
        p.Velocity.y -= 9.8f * dt; // Grawitacja
        p.Position += p.Velocity * dt;

        // --- KOLIZJA Z PIASKIEM ---
        float groundHeight = sandBox.getHeight(p.Position.x, p.Position.z);

        // Jeœli cz¹steczka dotknê³a ziemi (lub wpad³a pod ni¹)
        if (p.Position.y <= groundHeight) {

            // 1. Deformujemy teren w miejscu upadku
            // UWAGA: Si³a (strength) musi byæ bardzo ma³a (np. 0.05f), 
            // bo cz¹steczek jest du¿o i szybko usypi¹ wielk¹ górê!
            sandBox.deform(p.Position, 1.0f, 0.01f);

            // 2. Zabijamy cz¹steczkê (ju¿ "sta³a siê" czêœci¹ terenu)
            p.Life = -1.5f;
        }
    }

    // Usuwanie martwych cz¹steczek
    particles.erase(std::remove_if(particles.begin(), particles.end(),
        [](const Particle& p) { return p.Life <= 0.0f; }),
        particles.end());
}

void ParticleSystem::draw(Shader& shader) {
    if (particles.empty()) return;

    shader.use();

    // Zbieramy same pozycje do wys³ania na GPU
    std::vector<glm::vec3> positions;
    for (const auto& p : particles) {
        positions.push_back(p.Position);
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Aktualizujemy bufor (tylko tyle danych, ile mamy aktywnych cz¹steczek)
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), positions.data());

    glDrawArrays(GL_POINTS, 0, positions.size());
    glBindVertexArray(0);
}