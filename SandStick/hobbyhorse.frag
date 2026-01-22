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
    vec3 lightDir = normalize(-dirLightDir);

    // KROK 1: Mierzymy jak jasne jest œwiat³o padaj¹ce z nieba
    // Bierzemy najjaœniejszy kana³ (R, G lub B)
    float sunIntensity = max(lightColor.r, max(lightColor.g, lightColor.b));

    // KROK 2: Ustalamy parametry w zale¿noœci od pory dnia
    // smoothstep(0.1, 0.8, sunIntensity) zwróci:
    // 0.0 -> dla Nocy (œwiat³o < 0.1)
    // 1.0 -> dla Dnia (œwiat³o > 0.8)
    float dayFactor = smoothstep(0.1, 0.8, sunIntensity);

    // Dynamiczna Ekspozycja:
    // W nocy (0.0) -> mno¿nik 2.5 (Wzmacniamy œwiat³o ksiê¿yca!)
    // W dzieñ (1.0) -> mno¿nik 0.5 (Przyciemniamy s³oñce)
    float exposure = mix(2.5, 0.5, dayFactor);

    // Dynamiczny Ambient (Cienie):
    // W nocy (0.0) -> 0.4 (Chcemy widzieæ detale nawet w cieniu)
    // W dzieñ (1.0) -> 0.05 (Chcemy g³êbokie, kontrastowe cienie)
    float ambientStrength = mix(0.4, 0.05, dayFactor);


    // KROK 3: Aplikujemy obliczone wartoœci
    
    // Ambient
    vec3 ambient = lightColor * ambientStrength * baseColor;
    
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * baseColor * exposure;

    // Specular
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * specularStrength * lightColor * 0.5; 

    // Suma
    vec3 result = ambient + diffuse + specular;

    // --- 3. MG£A ---
    float dist = length(viewPos - FragPos);
    float fogStart = 20.0;
    float fogEnd = 150.0;
    float fogFactor = clamp((dist - fogStart) / (fogEnd - fogStart), 0.0, 1.0);

    vec3 finalColor = mix(result, fogColor, fogFactor);
    FragColor = vec4(finalColor, 1.0);
}