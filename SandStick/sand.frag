#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform vec3 sandColor;

// Struktura œwiat³a punktowego
struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};

uniform PointLight lights[2]; // Dwa œwiat³a boczne
uniform vec3 dirLightDir;     // Kierunek œwiat³a sufitowego

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = vec3(0.0);

    // 1. Œwiat³o kierunkowe (Sufitowe)
    vec3 lightDir = normalize(-dirLightDir);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(0.8); // Bia³e œwiat³o
    result += diffuse * sandColor;

    // 2. Œwiat³a punktowe (Boczne)
    for(int i = 0; i < 2; i++) {
        vec3 pLightDir = normalize(lights[i].position - FragPos);
        float pDiff = max(dot(norm, pLightDir), 0.0);
        
        // Proste zanikanie (attenuation)
        float distance = length(lights[i].position - FragPos);
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
        
        vec3 pDiffuse = pDiff * lights[i].color * lights[i].intensity;
        result += pDiffuse * sandColor * attenuation;
    }

    FragColor = vec4(result, 1.0);
}