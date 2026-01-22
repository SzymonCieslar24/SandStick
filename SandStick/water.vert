#version 330 core
// Location 0 musi byæ vec3, bo tak masz w GridMesh.cpp (glVertexAttribPointer(0, 3...))
layout (location = 0) in vec3 aPos; 
// Location 1 to normale z GridMesh (ignorujemy je, bo liczymy w³asne na falach)
layout (location = 1) in vec3 aNormal; 

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

out vec3 FragPos;
out vec3 Normal; // To wyliczamy tutaj dynamicznie

void main() {
    // GridMesh wysy³a Y=0.0, wiêc bierzemy X i Z jako p³aszczyznê
    vec3 pos = aPos; 
    
    // --- PARAMETRY FALI ---
    float freq = 0.2;
    float amp = 0.5;   // Wysokoœæ fali
    float speed = 1.5; // Prêdkoœæ
    
    // Obliczamy k¹t (animacja w czasie)
    float angle = pos.x * freq + time * speed + pos.z * 0.1;
    
    // Modyfikujemy wysokoœæ (Y)
    pos.y = sin(angle) * amp;

    // --- OBLICZANIE NORMALNYCH (MATEMATYKA) ---
    // Liczymy pochodne, ¿eby wiedzieæ jak nachylona jest fala w tym punkcie
    float dYdX = freq * amp * cos(angle); // Pochodna po X
    float dYdZ = 0.1 * amp * cos(angle);  // Pochodna po Z (s³absza)

    // Wektory styczne (opisuj¹ nachylenie powierzchni)
    vec3 tangentX = vec3(1.0, dYdX, 0.0);
    vec3 tangentZ = vec3(0.0, dYdZ, 1.0);

    // Iloczyn wektorowy daje wektor prostopad³y (Normalny)
    // To on decyduje, w któr¹ stronê odbija siê s³oñce!
    vec3 localNormal = normalize(cross(tangentZ, tangentX));

    // Transformacja do œwiata
    Normal = mat3(transpose(inverse(model))) * localNormal;
    
    // Pozycje finalne
    FragPos = vec3(model * vec4(pos, 1.0));
    gl_Position = projection * view * model * vec4(pos, 1.0);
}