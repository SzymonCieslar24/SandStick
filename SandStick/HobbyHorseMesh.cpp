#include "HobbyHorseMesh.h"

HobbyHorseMesh::HobbyHorseMesh(const char* path) {
    loadOBJ(path);
    setupMesh();
}

void HobbyHorseMesh::draw() {
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    glBindVertexArray(0);
}

void HobbyHorseMesh::loadOBJ(const char* path) {
    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec3> temp_normals;
    std::vector<glm::vec2> temp_uvs; // Bufor na UV

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "ERROR: Nie mozna otworzyc pliku: " << path << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") {
            glm::vec3 pos;
            ss >> pos.x >> pos.y >> pos.z;
            temp_positions.push_back(pos);
        }
        else if (prefix == "vt") { // Parsowanie UV
            glm::vec2 uv;
            ss >> uv.x >> uv.y;
            temp_uvs.push_back(uv);
        }
        else if (prefix == "vn") {
            glm::vec3 norm;
            ss >> norm.x >> norm.y >> norm.z;
            temp_normals.push_back(norm);
        }
        else if (prefix == "f") {
            std::string vertexStr;
            while (ss >> vertexStr) {
                MeshVertex v;
                unsigned int vIdx = 0, vtIdx = 0, vnIdx = 0;

                // Format: v/vt/vn
                size_t firstSlash = vertexStr.find('/');
                size_t secondSlash = vertexStr.find('/', firstSlash + 1);

                vIdx = std::stoi(vertexStr.substr(0, firstSlash));

                // Czytamy UV (pomiêdzy slashami)
                if (firstSlash != std::string::npos && secondSlash != std::string::npos) {
                    std::string vtStr = vertexStr.substr(firstSlash + 1, secondSlash - firstSlash - 1);
                    if (!vtStr.empty()) vtIdx = std::stoi(vtStr);
                }

                if (secondSlash != std::string::npos) {
                    vnIdx = std::stoi(vertexStr.substr(secondSlash + 1));
                }

                // Przypisanie danych (OBJ indeksuje od 1)
                v.Position = temp_positions[vIdx - 1];

                if (vtIdx > 0) v.TexCoords = temp_uvs[vtIdx - 1];
                else v.TexCoords = glm::vec2(0.0f, 0.0f);

                if (vnIdx > 0) v.Normal = temp_normals[vnIdx - 1];
                else v.Normal = glm::vec3(0, 1, 0);

                vertices.push_back(v);
            }
        }
    }
}

void HobbyHorseMesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(MeshVertex), vertices.data(), GL_STATIC_DRAW);

    // 1. Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)0);
    // 2. Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, Normal));
    // 3. TexCoords (Nowy atrybut na pozycji 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, TexCoords));

    glBindVertexArray(0);
}