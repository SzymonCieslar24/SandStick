#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <iostream>
#include "Shader.h"
#include "SandMesh.h"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

glm::vec3 cameraPos = glm::vec3(25.0f, 30.0f, 40.0f);
glm::vec3 cameraFront = glm::normalize(glm::vec3(25.0f, 0.0f, 25.0f) - cameraPos);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Raycasting - funkcja pomocnicza
glm::vec3 getRayFromMouse(double mouseX, double mouseY, int screenW, int screenH, glm::mat4 view, glm::mat4 projection) {
    // Normalised Device Coordinates (NDC)
    float x = (2.0f * mouseX) / screenW - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenH;
    float z = 1.0f;
    glm::vec3 ray_nds = glm::vec3(x, y, z);

    // Clip Coordinates -> Eye Coordinates
    glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);
    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

    // Eye Coordinates -> World Coordinates
    glm::vec3 ray_wor = (glm::inverse(view) * ray_eye);
    ray_wor = glm::normalize(ray_wor);
    return ray_wor;
}

// Przeciêcie promienia z p³aszczyzn¹ Y=0 (uproszczenie dla piasku)
bool getRayPlaneIntersection(glm::vec3 rayOrigin, glm::vec3 rayDir, float planeY, glm::vec3& intersection) {
    if (abs(rayDir.y) < 1e-6) return false; // Równoleg³y do p³aszczyzny
    float t = (planeY - rayOrigin.y) / rayDir.y;
    if (t < 0) return false; // P³aszczyzna za kamer¹
    intersection = rayOrigin + rayDir * t;
    return true;
}

int main()
{
    // 1. Inicjalizacja GLFW i okna (standard)
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SandStick - Step 1", NULL, NULL);
    if (window == NULL) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { return -1; }

    glEnable(GL_DEPTH_TEST);

    // 2. Tworzenie shaderów i obiektów
    // UWAGA: U¿yj swojej klasy Shader lub skompiluj shadery rêcznie tutaj
    Shader sandShader("sand.vert", "sand.frag");

    // Tworzymy piaskownicê 50x50 jednostek, gêstoœæ co 0.5
    SandMesh sandBox(100, 100, 0.5f);

    // 3. Pêtla renderowania
    while (!glfwWindowShouldClose(window))
    {
        // Input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Obs³uga myszy (RzeŸbienie)
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ||
            glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            // Macierze potrzebne do Raycastingu
            glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

            glm::vec3 rayDir = getRayFromMouse(xpos, ypos, SCR_WIDTH, SCR_HEIGHT, view, projection);
            glm::vec3 hitPoint;

            // Sprawdzamy gdzie mysz trafia w "pod³ogê" piaskownicy (Y ~ 0)
            if (getRayPlaneIntersection(cameraPos, rayDir, 0.0f, hitPoint)) {
                bool raise = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
                // Si³a rzeŸbienia: +0.2 dla góry, -0.2 dla do³ka
                float strength = raise ? 0.2f : -0.2f;
                // Promieñ pêdzla: 3.0 jednostki
                sandBox.deform(hitPoint, 3.0f, strength);
            }
        }

        // Renderowanie
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        sandShader.use();

        // Ustawienie macierzy transformacji
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 model = glm::mat4(1.0f);

        sandShader.setMat4("projection", projection);
        sandShader.setMat4("view", view);
        sandShader.setMat4("model", model);

        // Ustawienie oœwietlenia (Uniformy)
        sandShader.setVec3("viewPos", cameraPos);
        sandShader.setVec3("sandColor", 0.76f, 0.7f, 0.5f); // Kolor piasku (be¿owy)
        sandShader.setVec3("dirLightDir", -0.2f, -1.0f, -0.3f);

        // Œwiat³o 1 (Ciep³e - lewa)
        sandShader.setVec3("lights[0].position", glm::vec3(0.0f, 10.0f, 0.0f));
        sandShader.setVec3("lights[0].color", glm::vec3(1.0f, 0.6f, 0.0f)); // Pomarañcz
        sandShader.setFloat("lights[0].intensity", 1.5f);

        // Œwiat³o 2 (Ch³odne - prawa)
        sandShader.setVec3("lights[1].position", glm::vec3(50.0f, 10.0f, 50.0f));
        sandShader.setVec3("lights[1].color", glm::vec3(0.2f, 0.2f, 1.0f)); // Niebieski
        sandShader.setFloat("lights[1].intensity", 1.5f);

        // Rysowanie piasku
        sandBox.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}