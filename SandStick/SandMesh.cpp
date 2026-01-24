#include "SandMesh.h"

void SandMesh::deform(glm::vec3 hitPoint, float radius, float strength) {
    bool changed = false;

    for (int z = 0; z < depth; ++z) {
        for (int x = 0; x < width; ++x) {
            int idx = z * width + x;
            float dist = glm::distance(glm::vec2(vertices[idx].Position.x, vertices[idx].Position.z),
                glm::vec2(hitPoint.x, hitPoint.z));

            if (dist < radius) {
                float factor = glm::exp(-dist * dist / (radius * 0.5f));
                vertices[idx].Position.y += factor * strength;

                changed = true;
            }
        }
    }
}

float SandMesh::getHeight(float x, float z) {
    int ix = (int)(x / spacing);
    int iz = (int)(z / spacing);

    if (ix < 0 || ix >= width || iz < 0 || iz >= depth) {
        return -100.0f;
    }

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
            float h = (cos(normalizedDist * 3.14159f) + 1.0f) * 0.5f;

            v.Position.y = h * 9.0f - 5.0f;
        }
        else {
            v.Position.y = -5.0f;
        }
    }
    updateGeometry();
}

void SandMesh::recalculateNormals()
{
    for (int z = 0; z < depth; ++z) {
        for (int x = 0; x < width; ++x) {

            float hL = (x > 0) ? vertices[z * width + (x - 1)].Position.y : vertices[z * width + x].Position.y;
            float hR = (x < width - 1) ? vertices[z * width + (x + 1)].Position.y : vertices[z * width + x].Position.y;
            float hD = (z > 0) ? vertices[(z - 1) * width + x].Position.y : vertices[z * width + x].Position.y;
            float hU = (z < depth - 1) ? vertices[(z + 1) * width + x].Position.y : vertices[z * width + x].Position.y;

            glm::vec3 normal;
            normal.x = hL - hR;
            normal.y = 2.0f * spacing;
            normal.z = hD - hU;

            vertices[z * width + x].Normal = glm::normalize(normal);
        }
    }
}

void SandMesh::updateGeometry() {
    recalculateNormals();
    updateBuffers();
}
