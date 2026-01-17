#define _CRT_SECURE_NO_WARNINGS
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <iostream>
#include "Shader.h"
#include "SandMesh.h"
#include "Mesh.h"
#include "ParticleSystem.h"
#include "Sun.h"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

unsigned int crosshairVAO, crosshairVBO;

glm::vec3 cameraPos = glm::vec3(128.0f, 5.0f, 128.0f);  // Kamera na poziomie piasku
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);  // Kamera patrzy w stronê zbli¿on¹ do Z-ujemnej
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float deltaTime = 0.0f;    // Czas miêdzy obecn¹ a ostatni¹ klatk¹
float lastFrame = 0.0f; // Czas ostatniej klatki

double lastX = SCR_WIDTH / 2.0f;
double lastY = SCR_HEIGHT / 2.0f;
float yaw = -90.0f;
float pitch = 0.0f;
const float sensitivity = 0.1f;

glm::vec3 COLOR_DAY_SKY = glm::vec3(0.5f, 0.7f, 0.9f);
glm::vec3 COLOR_SUNSET_SKY = glm::vec3(0.8f, 0.4f, 0.2f);
glm::vec3 COLOR_NIGHT_SKY = glm::vec3(0.05f, 0.05f, 0.1f);

glm::vec3 COLOR_DAY_LIGHT = glm::vec3(1.0f, 1.0f, 0.9f);
glm::vec3 COLOR_SUNSET_LIGHT = glm::vec3(1.0f, 0.6f, 0.3f);
glm::vec3 COLOR_NIGHT_LIGHT = glm::vec3(0.1f, 0.1f, 0.2f);

// Raycasting - funkcja pomocnicza
glm::vec3 getRayFromMouse(double mouseX, double mouseY, int screenW, int screenH, glm::mat4 view, glm::mat4 projection) {
    float x = (2.0f * mouseX) / screenW - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenH;
    float z = 1.0f;
    glm::vec3 ray_nds = glm::vec3(x, y, z);
    glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);
    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
    glm::vec3 ray_wor = (glm::inverse(view) * ray_eye);
    ray_wor = glm::normalize(ray_wor);
    return ray_wor;
}

bool getRayPlaneIntersection(glm::vec3 rayOrigin, glm::vec3 rayDir, float planeY, glm::vec3& intersection) {
    if (abs(rayDir.y) < 1e-6) return false;
    float t = (planeY - rayOrigin.y) / rayDir.y;
    if (t < 0) return false;
    intersection = rayOrigin + rayDir * t;

    // Upewnij siê, ¿e kamera nie wychodzi powy¿ej powierzchni piasku
    if (intersection.y < 0.0f) {
        intersection.y = 0.0f;  // Ustaw na wysokoœæ powierzchni
    }

    return true;
}

unsigned int loadBMP(const char* imagepath) {
    unsigned char header[54];
    unsigned int dataPos;
    unsigned int width, height;
    unsigned int imageSize;

    FILE* file = fopen(imagepath, "rb");
    if (!file) {
        std::cout << "Image could not be opened: " << imagepath << std::endl;
        return 0;
    }

    if (fread(header, 1, 54, file) != 54) {
        std::cout << "Not a correct BMP file" << std::endl;
        fclose(file);
        return 0;
    }

    if (header[0] != 'B' || header[1] != 'M') {
        std::cout << "Not a correct BMP file" << std::endl;
        fclose(file);
        return 0;
    }

    dataPos = *(int*)&(header[0x0A]);
    imageSize = *(int*)&(header[0x22]);
    width = *(int*)&(header[0x12]);
    height = *(int*)&(header[0x16]);

    if (imageSize == 0)    imageSize = width * height * 3;
    if (dataPos == 0)      dataPos = 54;

    unsigned char* data = new unsigned char[imageSize];
    fseek(file, dataPos, SEEK_SET);
    fread(data, 1, imageSize, file);
    fclose(file);

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    delete[] data;
    return textureID;
}

void setupCrosshair() {
    // Wspó³rzêdne krzy¿yka (X, Y)
    // Poniewa¿ ekrany s¹ szerokie (16:9), X musi byæ mniejszy ni¿ Y, ¿eby krzy¿yk by³ równy.
    float vertices[] = {
        // Pozioma linia (-)
        -0.02f,  0.0f,
         0.02f,  0.0f,

         // Pionowa linia (|)
          0.0f, -0.035f,
          0.0f,  0.035f
    };

    glGenVertexArrays(1, &crosshairVAO);
    glGenBuffers(1, &crosshairVBO);

    glBindVertexArray(crosshairVAO);
    glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void calculateEnvironment(float time, glm::vec3& sunPos, glm::vec3& skyColor, glm::vec3& lightColor) {
    // 1. Ruch s³oñca (Wolniejszy: 0.05f)
    float angle = time * 0.05f;
    float radius = 200.0f;

    sunPos.x = 128.0f + cos(angle) * radius;
    sunPos.y = sin(angle) * radius;
    sunPos.z = 128.0f;

    float sunHeight = sin(angle);

    float dayFactor = glm::smoothstep(0.0f, 0.4f, sunHeight);

    float sunsetFactor = 1.0f - abs(glm::smoothstep(-0.4f, 0.4f, sunHeight) * 2.0f - 1.0f);

    sunsetFactor = 1.0f - glm::smoothstep(0.0f, 0.3f, abs(sunHeight));

    float nightFactor = glm::smoothstep(0.0f, -0.4f, sunHeight);

    if (sunHeight > 0.0f) {
        // --- PRZEJŒCIE DZIEÑ <-> ZACHÓD ---
        float t = glm::smoothstep(0.0f, 0.5f, sunHeight);

        skyColor = mix(COLOR_SUNSET_SKY, COLOR_DAY_SKY, t);
        lightColor = mix(COLOR_SUNSET_LIGHT, COLOR_DAY_LIGHT, t);
    }
    else {
        float t = glm::smoothstep(0.0f, 0.4f, abs(sunHeight));

        skyColor = mix(COLOR_SUNSET_SKY, COLOR_NIGHT_SKY, t);
        lightColor = mix(COLOR_SUNSET_LIGHT, COLOR_NIGHT_LIGHT, t);
    }
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SandStick", NULL, NULL);
    if (window == NULL) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { return -1; }
    glEnable(GL_DEPTH_TEST);

    Sun sunObject;

    Shader sandShader("sand.vert", "sand.frag");
    SandMesh sandBox(512, 512, 0.5f);
    sandBox.generateIslandShape();

    Shader waterShader("water.vert", "water.frag");
    SandMesh waterMesh(512, 512, 5.0f);

    glm::vec3 waterPos(-1000.0f, -2.0f, -1000.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader particleShader("particle.vert", "particle.frag");
    ParticleSystem sandParticles;

    Shader uiShader("ui.vert", "ui.frag");
    setupCrosshair();

    Shader sunShader("sun.vert", "sun.frag");

    glEnable(GL_PROGRAM_POINT_SIZE);

    Mesh stickModel("Resources/HobbyHorse.obj");
    unsigned int stickTexture = loadBMP("Resources/HorseTexture.bmp");
    unsigned int texSandDry = loadBMP("Resources/test1.bmp");
    unsigned int texSandWet = loadBMP("Resources/test2.bmp");

    sandShader.use();
    sandShader.setInt("meshTexture", 0);
    sandShader.setInt("texSlot1", 0);
    sandShader.setInt("texSlot2", 1);

    glm::vec3 currentStickPos(0.0f);

    bool wireframeMode = false;
    bool vKeyPressed = false;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        float cameraSpeed = 10.0f * deltaTime;

        glm::vec3 flatFront = glm::vec3(cameraFront.x, 0.0f, cameraFront.z);
        if (glm::length(flatFront) > 0.001f) {
            flatFront = glm::normalize(flatFront);
        }
        else {
            flatFront = glm::vec3(0.0f, 0.0f, -1.0f);
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * flatFront;

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * flatFront;

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraUp;

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraUp;

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            sandBox.reset();
        }

        if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
            if (!vKeyPressed) {
                wireframeMode = !wireframeMode;
                if (wireframeMode) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                }
                else {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }
                vKeyPressed = true;
            }
        }
        else {
            vKeyPressed = false;
        }

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        float xOffset = xpos - lastX;
        float yOffset = lastY - ypos;  // Odwrócenie osi Y
        lastX = xpos;
        lastY = ypos;

        xOffset *= sensitivity;
        yOffset *= sensitivity;

        yaw += xOffset;
        pitch += yOffset;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        // Aktualizacja kierunku kamery
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 400.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        glm::vec3 rayDir = getRayFromMouse(SCR_WIDTH / 2.0, SCR_HEIGHT / 2.0, SCR_WIDTH, SCR_HEIGHT, view, projection);
        glm::vec3 hitPoint;

        bool hit = getRayPlaneIntersection(cameraPos, rayDir, 0.0f, hitPoint);

        bool isButtonPressed = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ||
            glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);

        sandParticles.Update(deltaTime, sandBox);
        sandBox.updateGeometry();

        if (hit) {
            currentStickPos = glm::mix(currentStickPos, hitPoint, 0.5f);

            if (isButtonPressed) {
                bool raise = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
                if (raise) {
                    glm::vec3 spawnPos = currentStickPos + glm::vec3(0.0f, 5.0f, 0.0f);
                    sandParticles.Spawn(spawnPos, 1);
                }
                else {
                    sandBox.deform(currentStickPos, 1.0f, -0.01f);
                }
            }
        }

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 sunPos, skyColor, lightColor;
        calculateEnvironment(glfwGetTime(), sunPos, skyColor, lightColor);

        glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        sandShader.use();
        sandShader.setVec3("fogColor", skyColor);   // Mg³a = kolor nieba
        sandShader.setVec3("lightColor", lightColor);
        sandShader.setMat4("projection", projection);
        sandShader.setMat4("view", view);

        sandShader.setVec3("viewPos", cameraPos);
        sandShader.setVec3("dirLightDir", -0.2f, -1.0f, -0.3f);

        glm::vec3 lightDir = glm::normalize(glm::vec3(128.0f, 0.0f, 128.0f) - sunPos);
        sandShader.setVec3("dirLightDir", lightDir);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texSandDry);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texSandWet);

        sandShader.setInt("objectType", 0);

        glm::mat4 model = glm::mat4(1.0f);
        sandShader.setMat4("model", model);
        sandBox.draw();

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, stickTexture);

            sandShader.setInt("objectType", 1);

            model = glm::mat4(1.0f);
            model = glm::translate(model, currentStickPos);
            model = glm::rotate(model, glm::radians(-35.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            //model = glm::rotate(model, glm::radians(15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            //model = glm::translate(model, glm::vec3(0.0f, 2.0f, 0.0f));
            model = glm::scale(model, glm::vec3(2.5f));

            sandShader.setMat4("model", model);
            stickModel.draw();
        }

        particleShader.use();
        particleShader.setMat4("projection", projection);
        particleShader.setMat4("view", view);
        particleShader.setVec3("lightColor", lightColor);
        sandParticles.Draw(particleShader);

        waterShader.use();
        waterShader.setVec3("fogColor", skyColor);
        waterShader.setMat4("projection", projection);
        waterShader.setMat4("view", view);
        waterShader.setFloat("time", glfwGetTime());

        waterShader.setVec3("viewPos", cameraPos);     // Dla mg³y
        waterShader.setVec3("fogColor", skyColor);     // Kolor nieba (z funkcji calculateEnvironment)
        waterShader.setVec3("lightColor", lightColor);

        glm::mat4 modelWater = glm::mat4(1.0f);
        modelWater = glm::translate(modelWater, waterPos);
        waterShader.setMat4("model", modelWater);

        waterMesh.draw();

        sunObject.Draw(sunShader, sunPos, view, projection);

        glDisable(GL_DEPTH_TEST);

        uiShader.use();

        // 2. Logika koloru (Informacja zwrotna dla gracza)
        glm::vec3 crosshairColor;

        if (hit) {
            // Jeœli celujemy w piasek i jesteœmy blisko -> ZIELONY (Mo¿na kopaæ)
            crosshairColor = glm::vec3(0.0f, 1.0f, 0.0f);
        }
        else {
            // Jeœli celujemy w niebo lub za daleko -> BIA£Y (pó³przezroczysty wizualnie przez cienkie linie)
            crosshairColor = glm::vec3(1.0f, 1.0f, 1.0f);
        }
        uiShader.setVec3("color", crosshairColor);

        // 3. Rysowanie linii
        glBindVertexArray(crosshairVAO);
        glDrawArrays(GL_LINES, 0, 4); // 4 punkty tworz¹ 2 linie
        glBindVertexArray(0);

        // 4. Przywracamy test g³êbokoœci dla nastêpnej klatki (WA¯NE!)
        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}