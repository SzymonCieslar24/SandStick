#include "Sun.h"

Sun::Sun() {
    std::vector<float> vertices;

    float radius = 0.5f; // Promieñ bazowy (bêdziemy go skalowaæ w Draw)
    int sectorCount = 36; // Iloœæ "pionowych pasków" (g³adkoœæ w poziomie)
    int stackCount = 18;  // Iloœæ "poziomych pasków" (g³adkoœæ w pionie)

    float x, y, z, xy;                              // Wspó³rzêdne wierzcho³ka
    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;
    float sectorAngle, stackAngle;

    // Generowanie wierzcho³ków (TRIANGLES)
    // Dla ka¿dego "prostok¹ta" na siatce sfery tworzymy 2 trójk¹ty
    for (int i = 0; i < stackCount; ++i)
    {
        stackAngle = PI / 2 - i * stackStep;        // od PI/2 do -PI/2
        float nextStackAngle = PI / 2 - (i + 1) * stackStep;

        // Obliczamy Y i promieñ okrêgu na danej wysokoœci (xy) dla obecnego i nastêpnego paska
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        float nextXY = radius * cosf(nextStackAngle);
        float nextZ = radius * sinf(nextStackAngle);

        for (int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // od 0 do 2PI

            // --- TRÓJK¥T 1 ---
            // Wierzcho³ek 1 (Góra-Lewo)
            vertices.push_back(xy * cosf(sectorAngle)); // X
            vertices.push_back(z);                      // Y (u nas Z jest pionowo w kodzie sfery, ale w OpenGL to Y)
            vertices.push_back(xy * sinf(sectorAngle)); // Z

            // Wierzcho³ek 2 (Dó³-Lewo)
            vertices.push_back(nextXY * cosf(sectorAngle));
            vertices.push_back(nextZ);
            vertices.push_back(nextXY * sinf(sectorAngle));

            // Wierzcho³ek 3 (Góra-Prawo) - musimy wzi¹æ k¹t nastêpnego sektora
            float nextSectorAngle = (j + 1) * sectorStep;
            vertices.push_back(xy * cosf(nextSectorAngle));
            vertices.push_back(z);
            vertices.push_back(xy * sinf(nextSectorAngle));

            // --- TRÓJK¥T 2 ---
            // Wierzcho³ek 1 (Dó³-Lewo)
            vertices.push_back(nextXY * cosf(sectorAngle));
            vertices.push_back(nextZ);
            vertices.push_back(nextXY * sinf(sectorAngle));

            // Wierzcho³ek 2 (Dó³-Prawo)
            vertices.push_back(nextXY * cosf(nextSectorAngle));
            vertices.push_back(nextZ);
            vertices.push_back(nextXY * sinf(nextSectorAngle));

            // Wierzcho³ek 3 (Góra-Prawo)
            vertices.push_back(xy * cosf(nextSectorAngle));
            vertices.push_back(z);
            vertices.push_back(xy * sinf(nextSectorAngle));
        }
    }

    // Zapisujemy iloœæ wierzcho³ków (iloœæ floatów / 3 wspó³rzêdne)
    vertexCount = vertices.size() / 3;

    // Przes³anie do GPU
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // U¿ywamy .data() ¿eby pobraæ wskaŸnik do tablicy z wektora
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

    // Poniewa¿ algorytm generuje sferê "pionowo" wzd³u¿ osi Y, a w œwiecie gry Y to góra,
    // sfera jest u³o¿ona dobrze. Skalujemy j¹.
    model = glm::scale(model, glm::vec3(15.0f)); // Powiêkszamy s³oñce (np. 15x)

    shader.setMat4("model", model);
    shader.setVec3("color", glm::vec3(1.0f, 0.9f, 0.0f)); // ¯ó³te

    glBindVertexArray(VAO);
    // Rysujemy dynamiczn¹ iloœæ wierzcho³ków
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}