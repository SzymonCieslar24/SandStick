#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform vec3 fogColor;
uniform vec3 lightColor;
uniform vec3 dirLightDir;

void main()
{
    // 1. KOLOR BAZOWY
    // Ustawiamy mocny niebieski
    vec3 baseColor = vec3(0.0, 0.2, 0.8); 

    // 2. PRZYGOTOWANIE WEKTORÓW
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(-dirLightDir); // Odwracamy, by celowa³ w s³oñce

    // 3. OŒWIETLENIE (Diffuse + Ambient)
    // Ambient - ¿eby woda nigdy nie by³a czarna (minimum 0.4 jasnoœci)
    vec3 ambient = lightColor * 0.1;
    
    // Diffuse - cieniowanie fal
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * 0.6;

    // 4. ODBLASK (Specular - dla Blooma)
    vec3 reflectDir = reflect(-lightDir, norm);
    // Shininess 32.0 (œrednie skupienie)
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    // Mno¿nik x100.0 (wymuszenie HDR Bloom)
    vec3 specular = spec * lightColor * 10.0; 

    // 5. SK£ADANIE CA£OŒCI
    // Najpierw kolor wody
    vec3 result = (ambient + diffuse) * baseColor;
    
    // Potem mg³a (jeœli u¿ywasz) - opcjonalne, ale warto zostawiæ
    float dist = length(viewPos - FragPos);
    float fogFactor = clamp((dist - 50.0) / 250.0, 0.0, 1.0);
    result = mix(result, fogColor, fogFactor);

    // NA KOÑCU: Dodajemy odblask (dodawanie addytywne)
    // To sprawia, ¿e odblask jest "nadrukowany" na wodzie
    result += specular;

    // 6. WYJŒCIE
    // Alpha = 1.0 (Brak przezroczystoœci - woda musi byæ widoczna)
    FragColor = vec4(result, 1.0);
}