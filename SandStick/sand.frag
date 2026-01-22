#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texSlot1; // Suchy
uniform sampler2D texSlot2; // Mokry

uniform vec3 lightColor;
uniform vec3 dirLightDir;
uniform vec3 viewPos;
uniform vec3 fogColor;

// Dynamiczny poziom wody (z main.cpp)
uniform float waterLevel; 

void main()
{
    // --- 1. MATERIA£ PIASEK ---
    // Przywracamy Tiling (zagêszczanie), ¿eby piasek by³ wyraŸny jak w starej wersji
    vec2 tiledCoords = TexCoords * 8.0; 

    vec3 dryColor = texture(texSlot1, tiledCoords).rgb;
    vec3 wetColor = texture(texSlot2, tiledCoords).rgb;

    // Mieszanie: U¿ywamy waterLevel zamiast sztywnych liczb!
    // Piasek nasi¹ka do 1.5m powy¿ej aktualnego poziomu wody
    float h = smoothstep(waterLevel - 0.5, waterLevel + 1.5, FragPos.y);
    
    // Mieszamy kolory (h=0 mokry, h=1 suchy)
    vec3 baseColor = mix(wetColor, dryColor, h);

    // W³aœciwoœci materia³u (ze starego shadera - bo to wygl¹da³o dobrze)
    // Mokry b³yszczy (0.5), suchy matowy (0.0)
    float specularStrength = mix(0.5, 0.0, h); 
    // Mokry ma ostry b³ysk (32.0), suchy rozlany (2.0)
    float shininess = mix(32.0, 2.0, h);


// --- 2. OŒWIETLENIE (Z KOREKCJ¥ JASNOŒCI) ---
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(-dirLightDir); // Pamiêtamy o minusie!

    // PARAMETR PRZYCIEMNIANIA (Im mniej, tym ciemniejszy piasek)
    // 0.6 = 60% jasnoœci. Jeœli nadal za jasno, daj 0.4.
    float sandExposure = 0.6; 

    // Ambient (Cienie)
    // Mno¿ymy przez baseColor, ¿eby cienie mia³y kolor piasku, a nie szaroœci
    // Zmniejszamy do 0.05, ¿eby cienie by³y g³êbsze (lepszy kontrast)
    vec3 ambient = lightColor * 0.05 * baseColor; 
    
    // Diffuse (S³oñce)
    float diff = max(dot(norm, lightDir), 0.0);
    // Tutaj aplikujemy przyciemnienie (sandExposure)
    vec3 diffuse = diff * lightColor * baseColor * sandExposure;

    // Specular (B³ysk na mokrym)
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // Odblask te¿ lekko przygaszamy, ¿eby nie oœlepia³
    vec3 specular = spec * specularStrength * lightColor * 0.5; 

    // Sumowanie
    vec3 result = ambient + diffuse + specular;

    // --- 3. MG£A ---
    float dist = length(viewPos - FragPos);
    float fogFactor = clamp((dist - 20.0) / 130.0, 0.0, 1.0);
    
    vec3 finalColor = mix(result, fogColor, fogFactor);

    // Tone Mapping
    finalColor = min(finalColor, vec3(1.0));

    FragColor = vec4(finalColor, 1.0);
}