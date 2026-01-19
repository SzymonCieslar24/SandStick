#pragma once
#include "GridMesh.h"
#include <cmath>

class SandMesh : public GridMesh {
public:
    SandMesh(int w, int d, float s) : GridMesh(w, d, s) {}

    void deform(glm::vec3 hitPoint, float radius, float strength);
    float getHeight(float x, float z);
    void reset();
    void generateIslandShape();
    void updateGeometry();

private:
    void recalculateNormals();
};