#include "SandMesh.h"

SandMesh::SandMesh(int w, int d, float s) : width(w), depth(d), spacing(s)
{
    generateMesh();
    setupBuffers();
}

void SandMesh::draw()
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void SandMesh::deform(glm::vec3 hitPoint, float radius, float strength)
{
    bool changed = false;

    // Przeliczamy pozycjê œwiata na indeksy siatki
    // Zak³adamy, ¿e siatka zaczyna siê w (0,0) lub jest wycentrowana.
    // Tutaj zak³adam start od (0,0,0) w górê X i Z.

    for (int z = 0; z < depth; ++z) {
        for (int x = 0; x < width; ++x) {
            int idx = z * width + x;
            float dist = glm::distance(glm::vec2(vertices[idx].Position.x, vertices[idx].Position.z),
                glm::vec2(hitPoint.x, hitPoint.z));

            if (dist < radius) {
                // Wzór na krzyw¹ dzwonow¹ (smooth falloff)
                float factor = glm::exp(-dist * dist / (radius * 0.5f));
                vertices[idx].Position.y += factor * strength;

                // Ograniczenie, ¿eby piasek nie wyszed³ za bardzo
                if (vertices[idx].Position.y < -2.0f) vertices[idx].Position.y = -2.0f;
                if (vertices[idx].Position.y > 5.0f) vertices[idx].Position.y = 5.0f;

                changed = true;
            }
        }
    }

    if (changed) {
        recalculateNormals();
        updateBuffers();
    }
}

void SandMesh::generateMesh()
{
    // 1. Wierzcho³ki
    for (int z = 0; z < depth; ++z) {
        for (int x = 0; x < width; ++x) {
            Vertex v;
            v.Position = glm::vec3(x * spacing, 0.0f, z * spacing);
            v.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
            vertices.push_back(v);
        }
    }
    // 2. Indeksy (trójk¹ty)
    for (int z = 0; z < depth - 1; ++z) {
        for (int x = 0; x < width - 1; ++x) {
            int topLeft = (z * width) + x;
            int topRight = topLeft + 1;
            int bottomLeft = ((z + 1) * width) + x;
            int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
}

void SandMesh::recalculateNormals()
{
    // Reset normalnych
    for (auto& v : vertices) v.Normal = glm::vec3(0.0f);

    // Sumowanie normalnych œcian dla ka¿dego wierzcho³ka (smooth shading)
    for (size_t i = 0; i < indices.size(); i += 3) {
        unsigned int i1 = indices[i];
        unsigned int i2 = indices[i + 1];
        unsigned int i3 = indices[i + 2];

        glm::vec3 v1 = vertices[i1].Position;
        glm::vec3 v2 = vertices[i2].Position;
        glm::vec3 v3 = vertices[i3].Position;

        // Iloczyn wektorowy krawêdzi
        glm::vec3 edge1 = v2 - v1;
        glm::vec3 edge2 = v3 - v1;
        glm::vec3 normal = glm::cross(edge1, edge2);

        vertices[i1].Normal += normal;
        vertices[i2].Normal += normal;
        vertices[i3].Normal += normal;
    }

    // Normalizacja
    for (auto& v : vertices) v.Normal = glm::normalize(v.Normal);
}

void SandMesh::setupBuffers()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    glBindVertexArray(0);
}

void SandMesh::updateBuffers()
{
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Nadpisujemy dane wierzcho³ków (nowe Y i normale)
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
