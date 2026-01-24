#include "Sun.h"

Sun::Sun() {
    std::vector<float> vertices;
    float radius = 0.5f;

    int sectorCount = 16;
    int stackCount = 8;

    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i < stackCount; ++i) {
        stackAngle = PI / 2 - i * stackStep;
        float nextStackAngle = PI / 2 - (i + 1) * stackStep;

        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        float nextXY = radius * cosf(nextStackAngle);
        float nextZ = radius * sinf(nextStackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;
            float nextSectorAngle = (j + 1) * sectorStep;

            vertices.push_back(xy * cosf(sectorAngle)); vertices.push_back(z); vertices.push_back(xy * sinf(sectorAngle));
            vertices.push_back(nextXY * cosf(sectorAngle)); vertices.push_back(nextZ); vertices.push_back(nextXY * sinf(sectorAngle));
            vertices.push_back(xy * cosf(nextSectorAngle)); vertices.push_back(z); vertices.push_back(xy * sinf(nextSectorAngle));

            vertices.push_back(nextXY * cosf(sectorAngle)); vertices.push_back(nextZ); vertices.push_back(nextXY * sinf(sectorAngle));
            vertices.push_back(nextXY * cosf(nextSectorAngle)); vertices.push_back(nextZ); vertices.push_back(nextXY * sinf(nextSectorAngle));
            vertices.push_back(xy * cosf(nextSectorAngle)); vertices.push_back(z); vertices.push_back(xy * sinf(nextSectorAngle));
        }
    }

    vertexCount = vertices.size() / 3;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void Sun::draw(Shader& shader, glm::vec3 position, glm::mat4 view, glm::mat4 projection) {
    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(15.0f));
    shader.setMat4("model", model);
    shader.setVec3("color", glm::vec3(50.0f, 40.0f, 10.0f));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}