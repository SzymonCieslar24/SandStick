#include "SandMesh.h"

void SandMesh::deform(glm::vec3 hitPoint, float radius, float strength) {
    bool changed = false;

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
                if (vertices[idx].Position.y < -5.0f) vertices[idx].Position.y = -5.0f;
                if (vertices[idx].Position.y > 5.0f) vertices[idx].Position.y = 5.0f;

                changed = true;
            }
        }
    }
}

float SandMesh::getHeight(float x, float z) {
    // Przeliczamy wspó³rzêdne œwiata na indeksy siatki
    int ix = (int)(x / spacing);
    int iz = (int)(z / spacing);

    // Sprawdzenie granic (czy cz¹steczka jest nad piaskownic¹?)
    if (ix < 0 || ix >= width || iz < 0 || iz >= depth) {
        return -100.0f; // Dziura bez dna poza piaskownic¹
    }

    // Zwracamy aktualn¹ wysokoœæ w tym punkcie
    return vertices[iz * width + ix].Position.y;
}

void SandMesh::reset() {
    generateIslandShape();
}

void SandMesh::generateIslandShape() {
    float centerX = width * spacing / 2.0f;
    float centerZ = depth * spacing / 2.0f;
    float maxRadius = (width * spacing) / 2.0f;

    for (auto& v : vertices) {
        float dist = glm::distance(glm::vec2(v.Position.x, v.Position.z), glm::vec2(centerX, centerZ));

        float normalizedDist = dist / maxRadius;

        if (normalizedDist < 1.0f) {
            // Obliczamy kszta³t kopu³y (od 1.0 na œrodku do 0.0 na brzegu)
            float h = (cos(normalizedDist * 3.14159f) + 1.0f) * 0.5f;

            // --- ZMIANA TUTAJ ---
            // Wzór: h * (MaxWysokoœæ - Dno) + Dno
            // h * (4.0 - (-5.0)) + (-5.0) => h * 9.0 - 5.0

            v.Position.y = h * 9.0f - 5.0f;
        }
        else {
            v.Position.y = -5.0f; // Dno morza
        }
    }
    updateGeometry();
}

void SandMesh::recalculateNormals()
{
    // Metoda analityczna (a'la Sobel/Central Difference) dla Heightmapy
    // Zamiast liczyæ trójk¹ty, sprawdzamy nachylenie miêdzy s¹siadami.

    for (int z = 0; z < depth; ++z) {
        for (int x = 0; x < width; ++x) {

            // Pobieramy wysokoœci s¹siadów (z obs³ug¹ granic mapy)
            // Jeœli jesteœmy na krawêdzi, bierzemy wysokoœæ aktualnego punktu (brak nachylenia na krawêdzi)
            float hL = (x > 0) ? vertices[z * width + (x - 1)].Position.y : vertices[z * width + x].Position.y;
            float hR = (x < width - 1) ? vertices[z * width + (x + 1)].Position.y : vertices[z * width + x].Position.y;
            float hD = (z > 0) ? vertices[(z - 1) * width + x].Position.y : vertices[z * width + x].Position.y; // Down (Z-1)
            float hU = (z < depth - 1) ? vertices[(z + 1) * width + x].Position.y : vertices[z * width + x].Position.y; // Up (Z+1)

            // Obliczamy wektor normalny
            // Normal.x = wysokoœæ lewego - wysokoœæ prawego
            // Normal.z = wysokoœæ dolnego - wysokoœæ górnego
            // Normal.y = 2.0f * odstep_miedzy_wierzcholkami (skalowanie pionowe)

            glm::vec3 normal;
            normal.x = hL - hR;
            normal.y = 2.0f * spacing; // Sta³a wp³ywaj¹ca na "ostroœæ" normalnych
            normal.z = hD - hU;

            vertices[z * width + x].Normal = glm::normalize(normal);
        }
    }
}

void SandMesh::updateGeometry() {
    recalculateNormals();
    updateBuffers(); // Wywo³uje metodê z klasy bazowej GridMesh
}
