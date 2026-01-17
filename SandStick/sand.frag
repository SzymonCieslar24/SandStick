#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;

// Tekstury
uniform sampler2D texSlot1; // Suchy (Dry)
uniform sampler2D texSlot2; // Mokry (Wet)

// 0 = Piasek, 1 = Patyk
uniform int objectType; 

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};
uniform PointLight lights[2];
uniform vec3 dirLightDir;

void main()
{
    vec3 baseColor;
    float shininess;      // Skupienie odblasku (im wy¿sze, tym mniejsza kropka œwiat³a)
    float specularStrength; // Si³a odblasku (mokry = mocny, suchy = s³aby)

    if (objectType == 0) {
        // --- ULEPSZONA LOGIKA PIASKU ---
        
        // 1. Tiling (Kafelkowanie) - zagêszczamy teksturê
        vec2 tiledCoords = TexCoords * 8.0; 
        
        vec3 dryColor = texture(texSlot1, tiledCoords).rgb;
        vec3 wetColor = texture(texSlot2, tiledCoords).rgb;

        // 2. Obliczanie czynnika wysokoœci (Smoothstep dla g³adszego przejœcia)
        // Zakres: od -1.0 (do³ek) do 0.5 (ma³a górka). Powy¿ej 0.5 jest ju¿ ca³kiem sucho.
        float h = smoothstep(-2.5, 1.5, FragPos.y);
        
        // Mieszanie koloru
        baseColor = mix(wetColor, dryColor, h);

        // 3. FIZYKA PIASKU (Klucz do wygl¹du!)
        // Mokry piasek mocno odbija œwiat³o (b³oto), suchy piasek jest matowy.
        // Jeœli h jest niskie (mokro) -> specular du¿y. Jeœli wysokie (sucho) -> specular ma³y.
        // 1. Si³a b³ysku: Zmieñ 0.05 na 0.0 (ca³kowity brak b³ysku dla suchego)
        specularStrength = mix(0.0, 0.0, h); 
        
        // 2. Skupienie b³ysku: Zmniejsz drug¹ wartoœæ (np. na 2.0), 
        // ¿eby ewentualne resztki œwiat³a by³y "rozlane" a nie punktowe
        shininess = mix(32.0, 2.0, h);

    } else {
        // --- PATYK ---
        baseColor = texture(texSlot1, TexCoords).rgb;
        specularStrength = 0.3;
        shininess = 32.0;
    }

    // --- OŒWIETLENIE (PHONG) ---
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = vec3(0.0);

    // 1. Œwiat³o kierunkowe (S³oñce/Lampa)
    vec3 lightDir = normalize(-dirLightDir);
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    // Specular (Odblask)
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    
    vec3 ambient = vec3(0.3) * baseColor; // Trochê jaœniejsze otoczenie
    vec3 diffuse = diff * vec3(0.9) * baseColor;
    vec3 specular = spec * specularStrength * vec3(1.0); // Bia³e œwiat³o odblasku
    
    result += (ambient + diffuse + specular);

    // 2. Œwiat³a punktowe
    for(int i = 0; i < 2; i++) {
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

    FragColor = vec4(result, 1.0);
}