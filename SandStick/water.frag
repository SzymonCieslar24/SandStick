#version 330 core
out vec4 FragColor;

in vec3 FragPos; // Pozycja fragmentu w œwiecie (z Vertex Shadera)

uniform vec3 viewPos;    // Pozycja kamery (do obliczania dystansu)
uniform vec3 fogColor;   // Kolor nieba/mg³y (zmienny: dzieñ/noc/zachód)
uniform vec3 lightColor; // Kolor œwiat³a (wp³ywa na odcieñ wody)

void main()
{
    // 1. Bazowy kolor wody (niebieski, pó³przezroczysty)
    vec4 baseWater = vec4(0.0, 0.4, 0.8, 0.6);

    // 2. Zastosowanie oœwietlenia
    // Mno¿ymy kolor wody przez kolor œwiat³a. 
    // Dziêki temu w nocy woda bêdzie ciemna (granatowa), a o zachodzie lekko brudna/pomarañczowa.
    // Dodajemy "max", ¿eby woda nie by³a totalnie czarna (0.2 to minimalna jasnoœæ - œwiat³o ksiê¿yca/gwiazd)
    vec3 ambientLight = max(lightColor, vec3(0.2)); 
    vec3 litWaterColor = baseWater.rgb * ambientLight;

    // 3. Obliczanie Mg³y (Fog)
    float dist = length(viewPos - FragPos);
    float fogStart = 50.0;  // Mg³a zaczyna siê 50 jednostek od nas
    float fogEnd = 300.0;   // Pe³na mg³a na horyzoncie (woda jest du¿a)
    
    float fogFactor = clamp((dist - fogStart) / (fogEnd - fogStart), 0.0, 1.0);

    // 4. Mieszanie kolorów (Final Color)
    // Mieszamy oœwietlon¹ wodê z kolorem nieba (fogColor)
    vec3 finalRGB = mix(litWaterColor, fogColor, fogFactor);

    // 5. Mieszanie przezroczystoœci (Final Alpha)
    // Blisko nas woda jest przezroczysta (0.6).
    // Daleko (we mgle) staje siê nieprzezroczysta (1.0), ¿eby ukryæ krawêdŸ œwiata.
    float finalAlpha = mix(baseWater.a, 1.0, fogFactor);

    FragColor = vec4(finalRGB, finalAlpha);
}