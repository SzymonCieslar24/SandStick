#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;

// Tekstura Patyka/Konia
uniform sampler2D texSlot1; // G³ówna tekstura (drewno/patyk)

// Œwiat³a
struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};
uniform PointLight lights[2];
uniform vec3 dirLightDir;

// Zmienne Atmosferyczne
uniform vec3 fogColor;
uniform vec3 lightColor;

void main()
{
    vec3 baseColor;
    float shininess;      
    float specularStrength; 

    // --- 1. MATERIA£ PATYK ---
    // U¿ywamy oryginalnych UV (bez mno¿enia * 8.0)
    baseColor = texture(texSlot1, TexCoords).rgb;
    
    // Sta³e w³aœciwoœci drewna
    specularStrength = 0.1; // Lekki po³ysk
    shininess = 16.0;

    // --- 2. OŒWIETLENIE (PHONG) ---
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = vec3(0.0);

    // A. Œwiat³o Kierunkowe
    vec3 lightDir = normalize(-dirLightDir);
    
    vec3 ambient = (0.3 * lightColor + vec3(0.05)) * baseColor; 
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * baseColor;

    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * specularStrength * lightColor; 
    
    result += (ambient + diffuse + specular);

    // B. Œwiat³a Punktowe
    for(int i = 0; i < 2; i++) {
        if(lights[i].intensity <= 0.01) continue;

        vec3 pLightDir = normalize(lights[i].position - FragPos);
        float pDiff = max(dot(norm, pLightDir), 0.0);
        
        vec3 pReflectDir = reflect(-pLightDir, norm);
        float pSpec = pow(max(dot(viewDir, pReflectDir), 0.0), shininess);

        float distance = length(lights[i].position - FragPos);
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
        
        vec3 pDiffuse = pDiff * lights[i].color * lights[i].intensity * baseColor;
        vec3 pSpecular = pSpec * specularStrength * lights[i].color * lights[i].intensity;
        
        result += (pDiffuse + pSpecular) * attenuation;
    }

    // --- 3. MG£A ---
    float dist = length(viewPos - FragPos);
    float fogStart = 20.0;
    float fogEnd = 150.0;
    float fogFactor = clamp((dist - fogStart) / (fogEnd - fogStart), 0.0, 1.0);

    vec3 finalColor = mix(result, fogColor, fogFactor);
    FragColor = vec4(finalColor, 1.0);
}