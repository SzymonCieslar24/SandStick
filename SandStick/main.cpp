#define _CRT_SECURE_NO_WARNINGS
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <iostream>
#include "Shader.h"
#include "GridMesh.h"
#include "SandMesh.h"
#include "HobbyHorseMesh.h"
#include "ParticleSystem.h"
#include "Sun.h"
#include "FrameBuffer.h"

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

unsigned int crosshairVAO, crosshairVBO;

glm::vec3 cameraPos = glm::vec3(128.0f, 5.0f, 128.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;

double lastX = SCR_WIDTH / 2.0f;
double lastY = SCR_HEIGHT / 2.0f;
float yaw = -90.0f;
float pitch = 0.0f;
const float sensitivity = 0.1f;

glm::vec3 DAY_TOP = glm::vec3(0.0f, 0.3f, 0.8f);
glm::vec3 DAY_BOTTOM = glm::vec3(0.4f, 0.7f, 1.0f); 

glm::vec3 SUNSET_TOP = glm::vec3(0.2f, 0.0f, 0.4f);
glm::vec3 SUNSET_BOTTOM = glm::vec3(1.0f, 0.4f, 0.1f);

glm::vec3 NIGHT_TOP = glm::vec3(0.02f, 0.08f, 0.30f);
glm::vec3 NIGHT_BOTTOM = glm::vec3(0.10f, 0.20f, 0.45f);

glm::vec3 COLOR_DAY_LIGHT = glm::vec3(1.3f, 1.3f, 1.1f);
glm::vec3 COLOR_SUNSET_LIGHT = glm::vec3(1.5f, 0.8f, 0.4f);
glm::vec3 COLOR_NIGHT_LIGHT = glm::vec3(0.15f, 0.20f, 0.45f);

unsigned int quadVAO = 0;
unsigned int quadVBO;

void setupScreenQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            -1.0f,  1.0f,       0.0f, 1.0f,
            -1.0f, -1.0f,       0.0f, 0.0f,
             1.0f, -1.0f,       1.0f, 0.0f,

            -1.0f,  1.0f,       0.0f, 1.0f,
             1.0f, -1.0f,       1.0f, 0.0f,
             1.0f,  1.0f,       1.0f, 1.0f
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);

        glBindVertexArray(quadVAO);

        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindVertexArray(0);
    }
}

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

bool getRayTerrainIntersection(glm::vec3 rayOrigin, glm::vec3 rayDir, SandMesh& sandBox, glm::vec3& outputHitPoint) {

    float stepSize = 0.1f;
    float maxDist = 25.0f;

    glm::vec3 currentPoint = rayOrigin;
    glm::vec3 step = rayDir * stepSize;

    for (float d = 0.0f; d < maxDist; d += stepSize) {

        currentPoint += step;
        float terrainHeight = sandBox.getHeight(currentPoint.x, currentPoint.z);

        if (currentPoint.y <= terrainHeight) {
            outputHitPoint = currentPoint;
            return true;
        }

        if (currentPoint.y < -10.0f) return false;
    }

    return false;
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
    float vertices[] = {
        -0.02f,  0.0f,
         0.02f,  0.0f,
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

void calculateSunPosition(float time, glm::vec3& sunPos) {
    float angle = time * 0.05f;
    float radius = 200.0f;

    sunPos.x = 128.0f + cos(angle) * radius;
    sunPos.y = sin(angle) * radius;
    sunPos.z = 128.0f;
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
    Shader hobbyHorseShader("hobbyhorse.vert", "hobbyhorse.frag");

    SandMesh sandBox(512, 512, 0.5f);
    sandBox.generateIslandShape();

    Shader waterShader("water.vert", "water.frag");
    GridMesh waterMesh(512, 512, 5.0f);
    glm::vec3 waterPos(-1000.0f, -4.5f, -1000.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader particleShader("particle.vert", "particle.frag");
    ParticleSystem sandParticles;

    Shader uiShader("ui.vert", "ui.frag");
    setupCrosshair();

    Shader sunShader("sun.vert", "sun.frag");

    glEnable(GL_PROGRAM_POINT_SIZE);

    HobbyHorseMesh stickModel("Resources/HobbyHorse.obj");
    unsigned int stickTexture = loadBMP("Resources/HorseTexture.bmp");
    unsigned int texSandDry = loadBMP("Resources/DrySandTexture.bmp");
    unsigned int texSandWet = loadBMP("Resources/WetSandTexture.bmp");

    sandShader.use();
    sandShader.setInt("texSlot1", 0);
    sandShader.setInt("texSlot2", 1);

    hobbyHorseShader.use();
    hobbyHorseShader.setInt("texSlot1", 0);

    glm::vec3 currentStickPos(0.0f);

    bool wireframeMode = false;
    bool vKeyPressed = false;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Shader extractShader("screen.vert", "extract.frag");
    Shader blurShader("screen.vert", "blur.frag");
    Shader finalShader("screen.vert", "final.frag");

    Shader skyShader("sky.vert", "sky.frag");
    Sun skySphere;

    FrameBuffer sceneFBO(SCR_WIDTH, SCR_HEIGHT);
    setupScreenQuad(); 

    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
    }

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        float cameraSpeed = 10.0f * deltaTime;
        glm::vec3 flatFront = glm::vec3(cameraFront.x, 0.0f, cameraFront.z);
        if (glm::length(flatFront) > 0.001f) flatFront = glm::normalize(flatFront);
        else flatFront = glm::vec3(0.0f, 0.0f, -1.0f);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += cameraSpeed * flatFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= cameraSpeed * flatFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) cameraPos += cameraSpeed * cameraUp;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) cameraPos -= cameraSpeed * cameraUp;

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) sandBox.reset();

        if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
            if (!vKeyPressed) {
                wireframeMode = !wireframeMode;
                vKeyPressed = true;
            }
        }
        else {
            vKeyPressed = false;
        }

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        float xOffset = xpos - lastX;
        float yOffset = lastY - ypos;
        lastX = xpos;
        lastY = ypos;
        xOffset *= sensitivity;
        yOffset *= sensitivity;
        yaw += xOffset;
        pitch += yOffset;
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 400.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        glm::vec3 rayDir = getRayFromMouse(SCR_WIDTH / 2.0, SCR_HEIGHT / 2.0, SCR_WIDTH, SCR_HEIGHT, view, projection);
        glm::vec3 hitPoint;
        bool hit = getRayTerrainIntersection(cameraPos, rayDir, sandBox, hitPoint);
        bool isButtonPressed = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ||
            glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);

        sandParticles.update(deltaTime, sandBox);
        sandBox.updateGeometry();

        if (hit) {
            currentStickPos = glm::mix(currentStickPos, hitPoint, 0.5f);

            if (isButtonPressed) {
                bool raise = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

                if (raise) {
                    glm::vec3 spawnPos = currentStickPos + glm::vec3(0.0f, 4.0f, 0.0f);
                    sandParticles.spawn(spawnPos, 2);
                }
                else {
                    sandBox.deform(currentStickPos, 1.0f, -0.05f);
                }
            }
        }

        glm::vec3 sunPos;
        calculateSunPosition(glfwGetTime(), sunPos);

        glm::vec3 lightDir = glm::normalize(glm::vec3(128.0f, 0.0f, 128.0f) - sunPos);

        float timeVal = glfwGetTime();
        float sunHeight = sin(timeVal * 0.05f);

        glm::vec3 currentTop, currentBottom, currentLightColor;

        if (sunHeight > 0.0f) {
            float t = glm::smoothstep(0.0f, 0.5f, sunHeight);
            currentTop = glm::mix(SUNSET_TOP, DAY_TOP, t);
            currentBottom = glm::mix(SUNSET_BOTTOM, DAY_BOTTOM, t);
            currentLightColor = glm::mix(COLOR_SUNSET_LIGHT, COLOR_DAY_LIGHT, t);
        }
        else {
            float t = glm::smoothstep(0.0f, 0.4f, abs(sunHeight));
            currentTop = glm::mix(SUNSET_TOP, NIGHT_TOP, t);
            currentBottom = glm::mix(SUNSET_BOTTOM, NIGHT_BOTTOM, t);
            currentLightColor = glm::mix(COLOR_SUNSET_LIGHT, COLOR_NIGHT_LIGHT, t);
        }

        sceneFBO.Bind();
        glEnable(GL_DEPTH_TEST);

        glClearColor(currentBottom.r, currentBottom.g, currentBottom.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        glDisable(GL_CULL_FACE);

        skyShader.use();
        skyShader.setMat4("projection", projection);
        skyShader.setMat4("view", view);
        skyShader.setVec3("topColor", currentTop);
        skyShader.setVec3("bottomColor", currentBottom);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);

        skySphere.draw(skyShader, glm::vec3(0.0f), view, projection);

        glEnable(GL_CULL_FACE);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);

        if (wireframeMode)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        sandShader.use();
        sandShader.setVec3("fogColor", currentBottom);
        sandShader.setVec3("lightColor", currentLightColor);
        sandShader.setMat4("projection", projection);
        sandShader.setMat4("view", view);
        sandShader.setVec3("viewPos", cameraPos);
        sandShader.setFloat("waterLevel", waterPos.y);
        sandShader.setVec3("dirLightDir", lightDir);

        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, texSandDry);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, texSandWet);
        sandShader.setMat4("model", glm::mat4(1.0f));
        sandBox.draw();

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            hobbyHorseShader.use();
            hobbyHorseShader.setVec3("fogColor", currentBottom);
            hobbyHorseShader.setVec3("lightColor", currentLightColor);
            hobbyHorseShader.setMat4("projection", projection);
            hobbyHorseShader.setMat4("view", view);
            hobbyHorseShader.setVec3("dirLightDir", lightDir);
            hobbyHorseShader.setVec3("viewPos", cameraPos);
            hobbyHorseShader.setInt("texSlot1", 0);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, stickTexture);

            glm::mat4 model = glm::translate(glm::mat4(1.0f), currentStickPos);
            model = glm::rotate(model, glm::radians(-35.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(2.5f));
            hobbyHorseShader.setMat4("model", model);

            stickModel.draw();
        }

        particleShader.use();
        particleShader.setMat4("projection", projection);
        particleShader.setMat4("view", view);
        particleShader.setVec3("lightColor", currentLightColor);
        sandParticles.draw(particleShader);

        waterShader.use();
        waterShader.setMat4("projection", projection);
        waterShader.setMat4("view", view);
        waterShader.setFloat("time", glfwGetTime());
        waterShader.setVec3("viewPos", cameraPos);

        waterShader.setVec3("fogColor", currentBottom);
        waterShader.setVec3("lightColor", currentLightColor);
        waterShader.setVec3("dirLightDir", lightDir);

        glm::mat4 modelWater = glm::mat4(1.0f);
        modelWater = glm::translate(modelWater, waterPos);
        waterShader.setMat4("model", modelWater);
        waterMesh.draw();

        sunObject.draw(sunShader, sunPos, view, projection);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        sceneFBO.Unbind();

        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[0]);
        extractShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sceneFBO.texture);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        bool horizontal = true;
        unsigned int amount = 5;
        blurShader.use();
        for (unsigned int i = 0; i < amount; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            blurShader.setInt("horizontal", horizontal);
            glActiveTexture(GL_TEXTURE0);

            if (i == 0) glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[0]);
            else glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);

            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            horizontal = !horizontal;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        finalShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sceneFBO.texture);
        finalShader.setInt("scene", 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        finalShader.setInt("bloomBlur", 1);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisable(GL_DEPTH_TEST);

        uiShader.use();
        glm::vec3 crosshairColor = hit ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 1.0f, 1.0f);
        uiShader.setVec3("color", crosshairColor);

        glBindVertexArray(crosshairVAO);
        glDrawArrays(GL_LINES, 0, 4);
        glBindVertexArray(0);

        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}