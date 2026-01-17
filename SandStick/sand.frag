#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;

// Tekstury
uniform sampler2D texSlot1; // Suchy piasek (lub tekstura patyka)
uniform sampler2D texSlot2; // Mokry piasek

// 0 = Piasek, 1 = Patyk
uniform int objectType; 

// Œwiat³a
struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};
uniform PointLight lights[2];
uniform vec3 dirLightDir;

// --- ZMIENNE ATMOSFERYCZNE (Z main.cpp) ---
uniform vec3 fogColor;   // Kolor nieba/mg³y
uniform vec3 lightColor; // Kolor œwiat³a s³onecznego

void main()
{
    vec3 baseColor;
    float shininess;      
    float specularStrength; 

    // --- 1. USTALANIE MATERIA£U ---
    if (objectType == 0) {
        // >> PIASEK <<
        
        // Tiling (zagêszczanie tekstury piasku)
        vec2 tiledCoords = TexCoords * 8.0; 
        
        vec3 dryColor = texture(texSlot1, tiledCoords).rgb;
        vec3 wetColor = texture(texSlot2, tiledCoords).rgb;

        // Mieszanie na podstawie wysokoœci (Smoothstep)
        // -2.5 (mokro) do 1.5 (sucho)
        float h = smoothstep(-2.5, 1.5, FragPos.y);
        
        baseColor = mix(wetColor, dryColor, h);

        // Fizyka odblasków:
        // Mokry (h=0) -> specular 0.5 (b³yszczy)
        // Suchy (h=1) -> specular 0.0 (matowy)
        specularStrength = mix(0.5, 0.0, h); 
        
        // Mokry -> ostry b³ysk (32.0), Suchy -> rozlany b³ysk (2.0)
        shininess = mix(32.0, 2.0, h);

    } else {
        // >> PATYK <<
        baseColor = texture(texSlot1, TexCoords).rgb;
        specularStrength = 0.1; // Lekki po³ysk drewna
        shininess = 16.0;
    }

    // --- 2. OŒWIETLENIE (PHONG) ---
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = vec3(0.0);

    // A. Œwiat³o Kierunkowe (S³oñce)
    vec3 lightDir = normalize(-dirLightDir);
    
    // Ambient (Otoczenie) - zale¿y od lightColor (ciemne w nocy)
    // max(..., 0.05) zapewnia minimaln¹ widocznoœæ nawet w g³êbokiej nocy
    vec3 ambient = (0.3 * lightColor + vec3(0.05)) * baseColor; 

    // Diffuse (Rozproszone) - zale¿y od lightColor (pomarañczowe o zachodzie)
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * baseColor;

    // Specular (Odblask) - te¿ zale¿y od lightColor
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * specularStrength * lightColor; 
    
    result += (ambient + diffuse + specular);

    // B. Œwiat³a Punktowe (np. pochodnie - opcjonalne)
    for(int i = 0; i < 2; i++) {
        // Jeœli intensywnoœæ 0, pomijamy (optymalizacja)
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

    // --- 3. MG£A (FOG) ---
    float dist = length(viewPos - FragPos);
    
    // Mg³a zaczyna siê 20 jednostek od gracza, koñczy na 150 (horyzont)
    float fogStart = 20.0;
    float fogEnd = 150.0;
    float fogFactor = clamp((dist - fogStart) / (fogEnd - fogStart), 0.0, 1.0);

    // Mieszamy wynikowy kolor oœwietlonego obiektu z kolorem nieba/mg³y
    vec3 finalColor = mix(result, fogColor, fogFactor);

    FragColor = vec4(finalColor, 1.0);
}